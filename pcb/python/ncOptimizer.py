#!/c/Users/thier/AppData/Local/Programs/Python/Python35-32/python.exe

import os
import sys
import string
import math
import random

OPTIMIZATION_ITERATION_COUNT=1000
OPTIMIZATION_RANDOM_SWAPS=13

class AABB:
	def __init__(self):
		self.xMin=1.e6
		self.xMax=-1.e6
		self.yMin=1.e6
		self.yMax=-1.e6
		self.zMin=1.e6
		self.zMax=-1.e6
	def union(self, o):
		self.xMin = min( self.xMin , o.xMin )
		self.yMin = min( self.yMin , o.yMin )
		self.zMin = min( self.zMin , o.zMin )
		self.xMax = max( self.xMax , o.xMax )
		self.yMax = max( self.yMax , o.yMax )
		self.zMax = max( self.zMax , o.zMax )

class SegmentNode:
	def __init__(self):
		self.code="G01"
		self.x=0.0
		self.y=0.0
		self.z=0.0

class Segment:
	def __init__(self):
		self.nodes=[]

class GCode:
	def __init__(self):
		self.xMin=1.e6
		self.xMax=-1.e6
		self.yMin=1.e6
		self.yMax=-1.e6
		self.zMin=1.e6
		self.zMax=-1.e6
		self.header=[]
		self.body=[]
		self.footer=[]
	def writeHeader(self,stream):
		for cl in self.header:
			for c in cl:
				stream.write( " ".join(c) + "\n" )
	def writeFooter(self,stream,terminate=False):
		for cl in self.footer:
			for c in cl:
				cmd = " ".join(c)
				if cmd!="M05" or terminate: stream.write( cmd + "\n" )
	def writeBody(self,stream,tf):
		for cl in self.body:
			(x,y,z) = tf( cl[0][1] )
			z = 0.0
			stream.write("G00 Z%.4f\nG00 X%.4fY%.4f\nG00 Z0\n"%(self.zMax,x,y))
			for c in cl:
				(nx,ny,nz) = tf( c[1] )
				coordStr = ""
				if nx!=x: coordStr += "X%.4f" % nx
				if ny!=y: coordStr += "Y%.4f" % ny
				if nz!=z: coordStr += "Z%.4f" % nz
				(x,y,z) = (nx,ny,nz)
				if coordStr!="": stream.write(c[0]+" "+coordStr+"\n")
		stream.write("G00 Z%.4f\n"%self.zMax)
	def write(self,stream,terminate=False, tf=lambda p:p):
		self.writeHeader(stream)
		self.writeBody(stream,tf)
		self.writeFooter(stream,terminate)
	def updateXYBounds(self):
		self.xMin=1.e6
		self.xMax=-1.e6
		self.yMin=1.e6
		self.yMax=-1.e6
		for cl in self.body:
			for c in cl:
				(x,y,z) = c[1]
				if x<self.xMin: self.xMin=x
				if x>self.xMax: self.xMax=x
				if y<self.yMin: self.yMin=y
				if y>self.yMax: self.yMax=y

def readGCodeFromFile(filename):
	gcode = GCode()
	commands = open(filename,"r").read().split('\n')
	x = 0.0
	y = 0.0
	z = 0.0
	specialSegment = []
	lineSegment = []
	block = []
	zMin = 1.e6
	zMax = -1.e6
	for l in commands:
		if l.startswith("G0"):
			c = l[:3]
			if len(specialSegment)>0:
				block.append( specialSegment )
				specialSegment = []
			if l.find('X')>=0: x = float(l.replace('Y','X').replace('Z','X').split('X')[1])
			if l.find('Y')>=0: y = float(l.replace('Z','Y').split('Y')[1])
			if l.find('Z')>=0: z = float(l.split('Z')[1])
			if z<=0: lineSegment.append( (c,(x,y,z)) )
			else:
				if z>zMax: zMax = z
				if z<zMin: zMin = z
				if len(lineSegment)>0:
					block.append( lineSegment )
					lineSegment = []
		elif len(l)>0:
			if len(lineSegment)>0:
				block.append( lineSegment )
				lineSegment = []
			specialSegment.append( l.split() )
	if len(lineSegment)>0: block.append( lineSegment )
	if len(specialSegment)>0: block.append( specialSegment )
	header = []
	body = []
	footer = []
	for segment in block:
		if segment[0][0].startswith("G0"):
			if len(footer)>0:
				print("Special commands found between segments, giving up")
				return (0,0,[],[],[])
			body.append( segment )
		elif len(body)==0:
			header.append(segment)
		else:
			footer.append(segment)
	gcode.zMin = zMin
	gcode.zMax = zMax
	gcode.header = header
	gcode.body = body
	gcode.footer = footer
	return gcode

def computeGapDistance( segmentList ):
	if len(segmentList)==0:
		return 0.0
	(x,y,z) = (0.0,0.0,0.0)
	started = False
	l = 0.0
	for segment in segmentList:
		(sx,sy,sz) = segment[0][1]
		if not started:
			(x,y,z) = (sx,sy,sz)
			started = True
		x-=sx ; y-=sy
		l += math.sqrt( x*x + y*y )
		(x,y,z) = segment[-1][1]
	return l

def dist2( a , b ):
	x=a[0]-b[0];
	y=a[1]-b[1];
	return x*x+y*y
	
def nearestNeighborsSort( segmentListIn , maxLength ):
	if len(segmentListIn)==0: return []
	segmentList = segmentListIn.copy()
	l = []
	op = segmentList[0][-1][1]
	dMax = maxLength*maxLength
	while len(segmentList) > 0:
		iMin = -1
		dMin = dMax
		for (i,s) in enumerate(segmentList):
			p = s[0][1]
			d = dist2( op , p )
			if d < dMin:
				iMin = i
				dMin = d
		op = segmentList[iMin][-1][1]
		l.append( segmentList.pop(iMin) )
	return l

def randomSwapList( inputList , N ):
	l = inputList.copy()
	for i in range(N):
		i=random.randint(0,len(l)-1)
		j=random.randint(0,len(l)-1)
		a = l[i]
		b = l[j]
		l[j] = a
		l[i] = b
	return l

def optimizeSegmentList( segmentListIn ):
	segmentList = segmentListIn.copy()
	cost = computeGapDistance( segmentList )
	for i in range(OPTIMIZATION_ITERATION_COUNT):
		optList = randomSwapList( segmentList, OPTIMIZATION_RANDOM_SWAPS )
		optList = nearestNeighborsSort(optList,1.e6)
		optCost = computeGapDistance(optList)
		if optCost<cost:
			cost=optCost
			segmentList = optList
	return segmentList

if "-h" in sys.argv:
	print("""
Usage: ncOptimizer [-transform sx sy ax ay] [-n] [-o] [-s] <output_file> <input_file1> <input_file2> ...
Options:
	-h : this help
	-transform sx sy ax ay : multiply X'=X*sx+ax Y'=Y*sy+ay
	-n : normalize origin, so that bounding box' min is 0,0
	-o : optimize paths
	-s : split blocks to separate files
	""")
	sys.exit(0)

(SX,SY,AX,AY) = (1.,1.,0.,0.)
if "-transform" in sys.argv:
	i = sys.argv.index("-transform")
	sys.argv.pop(i)
	(SX,SY,AX,AY) = ( float(sys.argv[i]) , float(sys.argv[i+1]) , float(sys.argv[i+2]) , float(sys.argv[i+3]) )
	sys.argv.pop(i)
	sys.argv.pop(i)
	sys.argv.pop(i)
	sys.argv.pop(i)
	print("Result will be horizontaly flipped")

OffsetOrigin = False
if "-n" in sys.argv:
	OffsetOrigin = True
	sys.argv.remove("-n")
	print("Origin wil be normilized to 0,0")

OptimizeWires = False
if "-o" in sys.argv:
	OptimizeWires = True
	sys.argv.remove("-o")
	print("Result will be optimized")

SeparateFiles = False
if "-s" in sys.argv:
	SeparateFiles = True
	sys.argv.remove("-s")
	print("Result will be split into several files")

print( "input : ", sys.argv[2:] )
print( "output : ", sys.argv[1] )

bbox = AABB()

gcodeList = []
for fileName in sys.argv[2:]:
	print("Open %s ..."%fileName)
	gcode = readGCodeFromFile( fileName )
	if gcode.zMin != gcode.zMax:
		print("variable travel Z, giving up")
		sys.exit(1)
	gcode.updateXYBounds()
	bbox.union(gcode)
	if len(gcodeList)>0 and gcode.header==gcodeList[-1].header and gcode.footer==gcodeList[-1].footer and gcode.zMax==gcodeList[-1].zMax:
		print("compatible blocks, merging")
		gcodeList[-1].body += gcode.body
	else:
		gcodeList.append( gcode )


if OffsetOrigin:
	AX -= SX * bbox.xMin
	AY -= SY * bbox.yMin

def transformCoord( p ):
	(x,y,z) = p
	return (x*SX+AX,y*SY+AY,z)

output=None
if not SeparateFiles:
	output=open(sys.argv[1],"w")

Fi=0
for gcode in gcodeList:
	if SeparateFiles:
		output=open(sys.argv[1]+".%d"%Fi,"w")
	gapLength = computeGapDistance(gcode.body)
	print("Header:",gcode.header)
	print("Footer:",gcode.footer)
	print("before: %d segments, travel Z=%g, total gaps=%g"%(len(gcode.body),gcode.zMax,gapLength))
	if OptimizeWires:
		gcode.body = optimizeSegmentList( gcode.body )
		gapLength = computeGapDistance(gcode.body)
		print("after: %d segments, travel Z=%g, total gaps=%g"%(len(gcode.body),gcode.zMax,gapLength))
	gcode.write(output,False,transformCoord)
	if SeparateFiles:
		output.write("M05\n")	
		output.close()
		output=None
		Fi=Fi+1

if not SeparateFiles:
	output.write("M05\n")	
	output.close()
	output=None

print("Done")
