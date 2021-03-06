from Geometry import *

class GCode:
	def __init__(self):
		self.segments=[]
		self.bbox=AABB()
		self.name = "anonymous"
		
	def readFromFile(self,filename):
		self.name = filename
		self.readFromStream(open(filename,"r"))

	def readFromStream(self,fin):
		commands = fin.read().split('\n')
		x = 0.0
		y = 0.0
		z = 0.0
		specialCommands=[]
		segment=Segment()
		S = segment.getSpindle()
		F = segment.getSpeed()
		for l in commands:
			if l.startswith("G0"):
				c = l[:3]
				if l.find('X')>=0: x = float(l.replace('Y','X').replace('Z','X').split('X')[1])
				if l.find('Y')>=0: y = float(l.replace('Z','Y').split('Y')[1])
				if l.find('Z')>=0: z = float(l.split('Z')[1])
				segment.addNode( Point(x,y,z) )
			elif len(l)>0:
				if segment.numberOfNodes()>0:
					segment.setSpindle(S)
					segment.setSpeed(F)
					self.segments.append(segment)
					segment = Segment()
				cmds = l.split(' ')
				for c in cmds:
					if len(c)>0:
						if c.startswith("F"):
							F = float(c[1:])
						if c.startswith("S"):
							S = float(c[1:])
		if not segment.empty():
			self.segments.append(segment)
		self.updateBounds()

	def startPoint(self):
		sp = None
		for s in self.segments:
			if s.numberOfNodes()>=1 and sp==None:
				sp = s.firstNode().copy()
		if sp == None:
			sp = self.bbox.upper()
		return sp

	def header(self):
		sp = self.startPoint()
		return "G21\nG90\nG94\nG00 X%gY%gZ%g\n" % (sp.x,sp.y,sp.z)

	def write(self,stream):
		stream.write(self.header())
		for s in self.segments:
			s.write(stream)
		stream.write("M05\n")

	def addTravelOffset(self,TravelPlane,Code):
		for s in self.segments:
			if s.numberOfNodes()>0:
				Ps = TravelPlane.project( s.nodes[0] )
				Ps.c = Code
				Pe = TravelPlane.project( s.nodes[-1] )
				Pe.code = Code
				s.nodes = [Ps]+s.nodes+[Pe]
				s.updateBounds()
		self.updateBounds()

	def linkSegments(self):
		if self.numberOfSegments()<2:
			return
		prevSeg = self.segments[0]
		segList=[]
		for s in self.segments[1:]:
			if s.getSpindle()==prevSeg.getSpindle() and s.getSpeed()==prevSeg.getSpeed():
				prevSeg.nodes += s.nodes
			else:
				segList.append(prevSeg)
				prevSeg = s
		segList.append(prevSeg)
		self.segments = segList

	def updateBounds(self):
		self.bbox=AABB()
		for s in self.segments:
			self.bbox = self.bbox.union(s.bbox)

	def gapDistance(self):
		if len(self.segments)==0:
			return 0.0
		s=self.segments[0]
		l=0.0
		for ns in self.segments[1:]:
			l+=s.gapDistance(ns)
			s=ns
		return l

	def totalLength(self):
		l = 0.0
		for s in self.segments:
			l += s.length()
		return l
		
	def numberOfNodes(self):
		n=0
		for s in self.segments:
			n+=s.numberOfNodes()
		return n

	def numberOfSegments(self):
		return len(self.segments)
		
	def cutOff(self,plane):
		newSegments = []
		for s in self.segments:
			l = s.cutOff(plane)
			newSegments += l
		self.segments = newSegments
		self.updateBounds()
		
	def numberOfLoops(self):
		n = 0
		for s in self.segments:
			if s.isLoop():
				n+=1
		return n

	def subdivide(self,axis=None,position=None,threshold=0.2):
		bb1 = self.bbox.copy() # aka left part
		bb2 = self.bbox.copy() # aka right part		
		(W,H,Z) = self.bbox.size()
		cutPlane = None
		if axis==None:
			if W>=H and W>=Z: axis=0
			elif H>=Z and H>=W: axis=1
			else: axis=2
		if position==None:
			position = self.bbox.center()[axis]
		if axis==0:
			bb1.xMax = position
			bb2.xMin = position
			cutPlane = Plane(1.0,0.0,0.0,-position)
		elif axis==1:			
			bb1.yMax = position
			bb2.yMin = position
			cutPlane = Plane(0.0,1.0,0.0,-position)
		else:
			bb1.zMax = position
			bb2.zMin = position
			cutPlane = Plane(0.0,0.0,1.0,-position)
		axisName = ('x','y','z')[axis]
		bb1Length = bb1.size()[axis]
		bb2Length = bb2.size()[axis]
		totalLength = self.bbox.size()[axis]
		if totalLength==0.0:
			return (self,None)
		if math.fabs((bb1Length+bb2Length)-totalLength)>1.e-12:
			raise BaseException("Cut position error : L=%g, l1=%g, l2=%g, diff=%g"%(totalLength,bb1Length,bb2Length,bb1Length+bb2Length-totalLength))
		print("split: axis=%s, position=%g, length=%g"%(axisName,position,totalLength))
		s1 = []
		s2 = []
		nCuts = 0
		maxOverlapRatio = 1.0 + threshold
		for s in self.segments:
			sbb = s.bounds()
			assignToLeft = bb1.containsBounds(sbb)
			assignToRight = bb2.containsBounds(sbb)
			r1=0.0
			r2=0.0
			if not assignToLeft and not assignToRight:
				r1 = sbb.union(bb1).size()[axis] / bb1Length
				r2 = sbb.union(bb2).size()[axis] / bb2Length
				if r1<=r2 and r1<=maxOverlapRatio:
					assignToLeft = True
				elif r2<r1 and r2<=maxOverlapRatio:
					assignToRight = True
			if assignToLeft:
				s1.append(s)
			elif assignToRight:
				s2.append(s)
			else:
				#print("segment cut : r1=%g, r2=%g, sbb=%s"%(r1,r2,str(sbb)))
				( leftList , rightList ) = s.planeCut( cutPlane )
				s1 += leftList
				s2 += rightList
				nCuts += 1
		leftGCode = GCode()
		leftGCode.bbox = bb1
		leftGCode.segments = s1
		leftGCode.name = self.name+"0"
		leftGCode.updateBounds()
		rightGCode = GCode()
		rightGCode.bbox = bb2
		rightGCode.segments = s2
		rightGCode.name = self.name+"1"
		rightGCode.updateBounds()
		#print("split end : %d cuts, overlap=%g," % (nCuts,leftGCode.bbox.intersection(rightGCode.bbox).size()[axis]/totalLength) )
		return ( leftGCode , rightGCode )
	
	def merge(self,gc):
		self.segments += gc.segments
		self.bbox = self.bbox.union( gc.bbox )
		self.name = "(%s,%s)"%(self.name,gc.name)
	
	def split(self):
		gcList=[]
		for (i,s) in enumerate(self.segments):
			gc = GCode()
			gc.segments = [s]
			gc.name="%s[%d]"%(self.name,i)
			gc.updateBounds()
			gcList.append(gc)
		return gcList
			
	def findNearestSegment(self,p):
		ns = self.numberOfSegments()
		if ns==0:
			return None
		minSI = 0
		(minNodeI,minNodeD) = self.segments[0].findNearestNode(p)
		for si in range(1,ns):
			s = self.segments[si]
			(nodei,noded) = s.findNearestNode(p)
			if noded < minNodeD:
				(minSI,minNodeI,minNodeD) = (si,nodei,noded)
		return (minSI,minNodeI,minNodeD)

	def optimizeSegments(self,p):
		ns = self.numberOfSegments()
		if ns==0: return p
		if ns==1: return self.segments[0].lastNode()
		segList = []
		if p==None:
			p = Point(0.0,0.0,self.travelZ)
		while len(self.segments)>0:
			(si,nodei,noded) = self.findNearestSegment(p)
			s = self.segments.pop(si)
			s.rotateNodes(nodei)
			p = s.lastNode()
			segList.append( s )
		self.segments = segList
		return p
	
	def translate(self,t):
		for s in self.segments:
			s.translate(t)
		self.updateBounds()

	def scale(self,sf):
		for seg in self.segments:
			seg.scale(sf)
		self.updateBounds()

	def printStats(self):
		print("Number of segments = %d"%len(self.segments))
		print("Number of nodes = %d"%self.numberOfNodes())
		print("Loops = %d"%self.numberOfLoops())
		print("Total length = %g"%self.totalLength())
		print("Total gap distance = %g"%self.gapDistance())
		print("Start = %s" % str(self.startPoint()) )
		print("Bounds = %s"%str(self.bbox))

	def print(self):
		self.printStats()
		for (i,s) in enumerate(self.segments):
			print("\nSegment %d"%i)
			s.print()
		