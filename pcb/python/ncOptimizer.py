#!/c/Users/thier/AppData/Local/Programs/Python/Python35-32/python.exe

def main():
	import argparse
	import GCode
	import Geometry
	parser = argparse.ArgumentParser(description='GCode processor')
	parser.add_argument("input", type=str, nargs='+', help="input CNC files to process")
	parser.add_argument("-o", "--output", type=str, required=True, help="output CNC file")
	parser.add_argument("-z", "--z-cut", dest='zCutOff', type=float, nargs='?', help="cut part of segments crossing this Z level", const=0.0)
	parser.add_argument("-c", "--z-connect", dest='zConnect', const=True, default=False, action='store_const', help="reconnect segments, travelling at specified z level")
	parser.add_argument("-s", "--subdivide", type=int, help="number of subdivision passes (split into 2^N parts)")
	parser.add_argument("-O", "--optimize", dest='optimize', const=True, default=False, action='store_const', help="Optimize segments")
	parser.add_argument("-v", "--verbose", help="material surface Z level", const=True, default=False, action='store_const')
	parser.add_argument("-p", "--print", help="print result to console", const=True, default=False, action='store_const')
	parser.add_argument("-m", "--multiple-output", dest='multipleOutput', help="generate multiple output files, one per segment", const=True, default=False, action='store_const')
	args = parser.parse_args()
	gcode = GCode.GCode()
	
	for s in args.input:
		print("read %s"%s)
		gcode.readFromFile(s)
		
	if args.zCutOff!=None:
		print("cut surface at Z=%g"%args.zCutOff)
		gcode.zCutOff( args.zCutOff )
	
	startPoint = Geometry.Point(0.0,0.0,gcode.travelZ)
	
	if args.subdivide!=None:
		gcodeList = [gcode]
		N = int(args.subdivide)
		while N>0:
			L = 0.0
			G = 0.0
			for gc in gcodeList:
				L += gc.totalLength()
				G += gc.gapDistance()
			print("subdivision, pass #%d, L=%g, G=%g" % (args.subdivide-N+1,L,G))
			sl=[]
			for gc in gcodeList:
				(l,r) = gc.subdivide()
				sl += [l,r]
			gcodeList = sl
			N -= 1
		L = 0.0
		G = 0.0
		for gc in gcodeList:
			L += gc.totalLength()		
			G += gc.gapDistance()
		print("Subdivision end, L=%g, G=%g"%(L,G))
		if args.optimize:
			print("optimize segments (before merge)")
			p = startPoint
			L = 0.0
			G = 0.0
			for gc in gcodeList:
				p = gc.optimizeSegments(p)
				L += gc.totalLength()
				G += gc.gapDistance()
			print("optimize end, L=%g, G=%g"%(L,G))
		gcode = gcodeList[0]
		for gc in gcodeList[1:]:
			gcode.merge( gc )
		gcodeList = None
	else:
		if args.optimize:
			(L1,G1) = (gcode.totalLength(),gcode.gapDistance())
			print("Optimize start : L=%g, G=%g"%(L1,G1))
			gcode.optimizeSegments(startPoint)
			(L2,G2) = (gcode.totalLength(),gcode.gapDistance())
			print("Optimize end : L=%g, G=%g"%(L2,G2))
	
	if args.zConnect:
		print("reconnect segments")
		TravelPlane = Geometry.Plane( 0.0,0.0,1.0,-gcode.travelZ )
		gcode.addTravelOffset( TravelPlane, 'G00' )
		gcode.mergeSegments()

	if args.verbose:
		gcode.printStats()

	if args.print:
		gcode.print()

	if args.multipleOutput:
		N = gcode.numberOfSegments()
		for i in range(N):
			print("output segment #%d to %s.%d"%(i,args.output,i))
			gcode.writeSingleSegment( open(args.output+'.'+str(i),'w') )
	else:
		print("output result to %s"%args.output)
		gcode.write( open(args.output,'w') )

if __name__=="__main__":
	main()