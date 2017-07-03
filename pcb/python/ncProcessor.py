#!/c/Users/thier/AppData/Local/Programs/Python/Python35-32/python.exe



def main():
	import argparse
	import GCode
	import Geometry
	import Evaluator
	import sys
	
	processor = Evaluator.Evaluator()
	
	args = sys.argv[1:]
	print("args = ",args)
	while len(args)>0:
		a = args.pop(0)
		nRepeats = 1
		if a.lower()=='x':
			nRepeats = int(args.pop(0))
			a = args.pop(0)
		if a.lower()=='read':
			fileName = args.pop(0)
			print("read '%s'"%fileName)
			processor.read( fileName )
		elif a.lower()=='cutoff':
			plane = Geometry.Plane( float(args.pop(0)) , float(args.pop(0)) , float(args.pop(0)) , float(args.pop(0)) )
			processor.cutOff(plane,nRepeats)
		elif a.lower()=='concat':
			processor.concat()
		elif a.lower()=='subdivide':
			nPasses = int(args.pop(0))
			processor.subdivide(nPasses)
		elif a.lower()=='optimize':
			p = Geometry.Point( float(args.pop(0)) , float(args.pop(0)) , float(args.pop(0)) )
			processor.optimize(p,nRepeats)
		elif a.lower()=='connect':
			plane = Geometry.Plane( float(args.pop(0)) , float(args.pop(0)) , float(args.pop(0)) , float(args.pop(0)) )
			code = args.pop(0)
			processor.connect(plane,code,nRepeats)
		elif a.lower()=='origin':
			p = Geometry.Point( float(args.pop(0)) , float(args.pop(0)) , float(args.pop(0)) )
			processor.origin(p,nRepeats)
		elif a.lower()=='scale':
			p = Geometry.Point( float(args.pop(0)) , float(args.pop(0)) , float(args.pop(0)) )
			processor.scale(p,nRepeats)
		elif a.lower()=='translate':
			p = Geometry.Point( float(args.pop(0)) , float(args.pop(0)) , float(args.pop(0)) )
			processor.translate(p,nRepeats)
		elif a.lower()=='stats':
			processor.stats(nRepeats)
		elif a.lower()=='print':
			print(args.pop(0))
		elif a.lower()=='write':
			fileName = args.pop(0)
			processor.write(fileName)
		elif a.lower()=='pop':
			processor.pop()
		else:
			raise BaseException("Unknown operator '%s'"%a)
	
	if len(processor.stack)!=0:
		print("Warning, stack not empty : %d remaining items"%len(processor.stack))

if __name__=="__main__":
	main()