import sys, glob, os, subprocess
files=[]
for arg in sys.argv[1:]: files+=glob.glob(arg)
script_path=os.path.split(os.path.realpath(__file__))[0]
subprocess.call([os.path.join(script_path, 'build', 'plotstuff')]+files)
