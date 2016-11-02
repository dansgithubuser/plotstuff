import argparse, glob, os, re, subprocess, time

parser=argparse.ArgumentParser()
parser.add_argument('--type', '-t', choices=['line', 'heat'], default='line')
parser.add_argument('--build', '-b', action='store_true')
parser.add_argument('globs', nargs='+', help='what to plot; list of globs')

start_path=os.getcwd()
script_path=os.path.split(os.path.realpath(__file__))[0]
plotstuff_path=os.path.join(script_path, 'build', 'plotstuff')

def build():
	os.chdir(os.path.join(script_path, 'build'))
	subprocess.check_call('cmake .', shell=True)
	subprocess.check_call('cmake --build .', shell=True)
	os.chdir(start_path)

class Runner():
	def __init__(self): self.files=[]; self.types=[]
	def set_globs(self, globs): self.globs=globs; return self
	def set_type(self, type): self.type=type; return self
	def set_files(self, files): self.files=files; return self
	def set_types(self, types): self.types=types; return self
	def run(self):
		if not os.path.isfile(plotstuff_path) and not os.path.isfile(plotstuff_path+'.exe'): build()
		if hasattr(self, 'globs'):
			for i in self.globs: self.files+=sorted(glob.glob(i))
		if hasattr(self, 'type'):
			self.types=self.types+[self.type]*(len(self.files)-len(self.types))
		invocation=[plotstuff_path]
		for i in range(len(self.files)): invocation+=[self.types[i], os.path.realpath(self.files[i])]
		subprocess.check_call(invocation)

class Plot():
	def __init__(self):
		self.type=None
		self.values=[]

class Parser():
	#interface
	def __init__(self, plots, filters):
		import collections
		self.plots={i: Plot() for i in plots}
		self.filters=filters

	def run(self, file_path):
		with open(file_path) as file:
			lines=file.readlines()
			for i in range(len(lines)):
				for pattern, action in self.filters:
					if re.search(pattern, lines[i]): action(self, lines, i)
		def name_to_file(name): return name+'.parsed.txt'
		for name, contents in self.plots.items():
			with open(name_to_file(name), 'w') as file:
				if contents.type==None:
					if len(contents.values):
						if hasattr(contents.values[0], '__iter__'): contents.type='scatter'
						else: contents.type='line'
					else: contents.type='line'#default to line if no values
				if contents.type=='line':
					for value in contents.values: file.write('{}\n'.format(value))
				elif contents.type in ['heat', 'scatter']:
					for value in contents.values: file.write('{} {}\n'.format(value[0], value[1]))
				else:
					raise Exception("unknown plot type")
		files=[]
		types=[]
		for name, contents in self.plots.items():
			files.append(name_to_file(name))
			types.append(contents.type)
		Runner().set_files(files).set_types(types).run()

	#helpers
	def time_to_x(self, year=1970, month=1, day=1, hour=0, minute=0, second=0, millisecond=0):
		return time.mktime((year, month, day, hour, minute, second, 0, 0, 0))+millisecond/1000.0

if __name__=='__main__':
	args=parser.parse_args()
	if args.build: build()
	Runner().set_globs(args.globs).set_type(args.type).run()
