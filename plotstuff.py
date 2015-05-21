import Tkinter as tkinter
import glob

class Stuff:
	def show(self, files, plot_width=200, plot_height=100):
		file_names=glob.glob(files)
		try: self.root.destroy()
		except: pass
		self.root=tkinter.Tk()
		self.canvases=[]
		self.labels=[]
		for file_name in file_names:
			with open(file_name) as file: lines=file.readlines()
			values=[[float(x) for x in line.split()] for line in lines]
			if len(values[0])>2: raise Exception('dimension too high in file '+file_name)
			if len(values[0])==1: values=[[i]+values[i] for i in range(len(values))]
			xMin=min([x[0] for x in values])
			yMin=min([x[1] for x in values])
			xMax=max([x[0] for x in values])
			yMax=max([x[1] for x in values])
			self.canvases.append(tkinter.Canvas(width=plot_width, height=plot_height))
			for i in range(len(values)-1):
				def interpolate(i, iMin, iMax, oMin, oMax):
					return 1.0*(i-iMin)/(iMax-iMin)*(oMax-oMin)+oMin
				xi=interpolate(values[i+0][0], xMin, xMax, 0, plot_width)
				yi=interpolate(values[i+0][1], yMin, yMax, plot_height, 0)
				xf=interpolate(values[i+1][0], xMin, xMax, 0, plot_width)
				yf=interpolate(values[i+1][1], yMin, yMax, plot_height, 0)
				self.canvases[-1].create_line(xi, yi, xf, yf)
			self.canvases[-1].pack()
			self.labels.append(tkinter.Label(self.canvases[-1], text=file_name, anchor='n'))
			self.labels[-1].pack()
			self.canvases[-1].create_window(plot_width/2, 10, window=self.labels[-1])
