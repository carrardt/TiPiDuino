#!/c/Users/thier/AppData/Local/Programs/Python/Python35-32/python.exe

	
def main():
	import argparse
	import GCode
	import Geometry
	import Evaluator
	import sys
	import math

	def FloatArg(args):
		if len(args)==0: return None
		a = args.pop(0)
		if a=='*': return math.nan
		return float(a)

	def PointArg(args):
		return Geometry.Point( FloatArg(args) , FloatArg(args) , FloatArg(args) )

	def PlaneArg(args):
		return Geometry.Plane( FloatArg(args) , FloatArg(args) , FloatArg(args) , FloatArg(args) )
	
	processor = Evaluator.Evaluator()
	
	args = sys.argv[1:]
	print("args = ",args)
	while len(args)>0:
		a = args.pop(0)
		nRepeats = 1
		if a.lower()=='x':
			nRepeats = int(args.pop(0))
			a = args.pop(0)

		if a.lower()=='prog':
			fileName = args.pop(0)
			print("Append commands from '%s'"%fileName)
			for a in open(fileName).read().replace('\n',' ').replace('\t',' ').split(' '):
				if a!='':
					args.append(a)
			print("args = ",args)
		elif a.lower()=='read':
			fileName = args.pop(0)
			print("* read '%s'"%fileName)
			processor.read( fileName )
		elif a.lower()=='cutoff':
			plane = PlaneArg(args)
			print("* cutoff : %s"%str(plane))
			processor.cutOff(plane,nRepeats)
		elif a.lower()=='merge':
			print("* merge")
			for i in range(nRepeats):
				processor.merge()
		elif a.lower()=='subdivide':
			nPasses = int(args.pop(0))
			print("* subdivide %d"%nPasses)
			processor.subdivide(nPasses)
		elif a.lower()=='split':
			print("* split")
			processor.split()
		elif a.lower()=='swap':
			print("* swap")
			processor.swap()
		elif a.lower()=='optimize':
			p = PointArg(args)
			print("* optimize sp=%s"%str(p))
			processor.optimize(p,nRepeats)
		elif a.lower()=='connect':
			plane = PlaneArg(args)
			code = args.pop(0)
			print("* connect : plane=%s, code=%s"%(str(plane),code))
			processor.connect(plane,code,nRepeats)
		elif a.lower()=='origin':
			p = PointArg(args)
			print("* origin %s"%str(p))
			processor.origin(p,nRepeats)
		elif a.lower()=='scale':
			p = PointArg(args)
			print("* scale %s"%str(p))
			processor.scale(p,nRepeats)
		elif a.lower()=='translate':
			p = PointArg(args)
			print("* translate %s"%str(p))
			processor.translate(p,nRepeats)
		elif a.lower()=='stats':
			processor.stats(nRepeats)
		elif a.lower()=='stack':
			print("Stack (%d) : %s"%(len(processor.stack),', '.join([x.name for x in processor.stack]))  )
		elif a.lower()=='print':
			s=''
			a=args.pop(0)
			l=[]
			while a!=';':
				l.append(a)
				a=args.pop(0)
			print(' '.join(l))
		elif a.lower()=='write':
			fileName = args.pop(0)
			print("* write %s"%fileName)
			processor.write(fileName)
		elif a.lower()=='pop':
			processor.pop()
		else:
			raise BaseException("Unknown operator '%s'"%a)
	
	if len(processor.stack)!=0:
		print("Warning, stack not empty : %d remaining items"%len(processor.stack))

if __name__=="__main__":
	main()