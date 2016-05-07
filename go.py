import argparse, glob, os, subprocess

parser=argparse.ArgumentParser()
parser.add_argument('globs', nargs='+', help='what to plot; list of globs')
args=parser.parse_args()

start_path=os.getcwd()
script_path=os.path.split(os.path.realpath(__file__))[0]
plotstuff_path=os.path.join(script_path, 'build', 'plotstuff')

if not os.path.isfile(plotstuff_path) and not os.path.isfile(plotstuff_path+'.exe'):
	os.chdir(os.path.join(script_path, 'build'))
	subprocess.check_call('cmake .', shell=True)
	subprocess.check_call('cmake --build .', shell=True)
	os.chdir(start_path)

files=[]
for i in args.globs: files+=glob.glob(i)
files=[os.path.realpath(i) for i in files]
subprocess.check_call([plotstuff_path]+files)
