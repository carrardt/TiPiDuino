from Geometry import *
from GCode import *

class Evaluator:
	def __init__(self):
		self.stack = []
	
	def push(self,gc):
		self.stack.insert(0,gc)
	
	def pop(self):
		if len(self.stack)==0:
			return None
		else:
			self.stack.pop(0)
	
	def read(self,fileName):
		gc = GCode()
		gc.readFromFile(fileName)
		self.push(gc)
	
	def cutOff(self,plane,nStackElements=1):
		if nStackElements<=0: return
		gcList = self.stack[-nStackElements:]
		for gc in gcList:
			gc.cutOff(plane)
	
	def concat(self):
		gc1 = self.pop()
		gc2 = self.pop()
		gc1.concat(gc2)
		self.push(gc1)
	
	def subdivide(self,nPasses=1,axis='?',threshold=0.2):
		if nPasses<=0: return
		gc = self.pop()
		(l,r) = gc.subdivide(axis,threshold)
		self.push(l)
		self.subdivide(nPasses-1,axis,threshold)
		self.push(r)
		self.subdivide(nPasses-1,axis,threshold)

	def optimize(self,p,nStackElements=1):
		if nStackElements<=0: return
		gcList = self.stack[-nStackElements:]
		for gc in gcList:
			p = gc.optimizeSegments(p)
		return p

	def connect(self,travelPlane,travelCode='G00',nStackElements=1):
		if nStackElements<=0: return
		gcList = self.stack[-nStackElements:]
		for gc in gcList:
			gc.addTravelOffset( travelPlane, travelCode )
			gc.mergeSegments()

	def origin(self,p=Point(0.0,0.0,0.0),nStackElements=1):
		if nStackElements<=0: return
		gcList = self.stack[-nStackElements:]
		for gc in gcList:
			t = p.sub( Point(gc.bbox.xMin,gc.bbox.yMin,gc.bbox.zMin) )
			gc.translate(t)

	def scale(self,s,nStackElements=1):
		if nStackElements<=0: return
		gcList = self.stack[-nStackElements:]
		for gc in gcList:
			gc.scale(s)

	def translate(self,t,nStackElements=1):
		if nStackElements<=0: return
		gcList = self.stack[-nStackElements:]
		for gc in gcList:
			gc.translate(t)

	def stats(self,nStackElements=1):
		if nStackElements<=0: return
		gcList = self.stack[-nStackElements:]
		for gc in gcList:
			gc.printStats()

	def write(self,fileName):
		self.stack[-1].write( open(fileName,'w') )
	