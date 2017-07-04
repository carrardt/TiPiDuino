from Geometry import *
from GCode import *

class Evaluator:
	def __init__(self):
		self.stack = []
	
	def push(self,gc):
		self.stack.insert(0,gc)
	
	def pop(self):
		if len(self.stack)==0:
			raise BaseException("Stack underflow")
		return self.stack.pop(0)
	
	def read(self,fileName):
		gc = GCode()
		gc.readFromFile(fileName)
		self.push(gc)
	
	def cutOff(self,plane,nStackElements=1):
		if nStackElements<=0: return
		gcList = self.stack[:nStackElements]
		for gc in gcList:
			gc.cutOff(plane)
	
	def swap(self):
		A = self.pop()
		B = self.pop()
		self.push(A)
		self.push(B)
	
	def split(self):
		l = self.pop().split()
		for gc in l:
			self.push(gc)

	def merge(self):
		A = self.pop()
		B = self.pop()
		A.merge(B)
		self.push(A)
	
	def subdivide(self,nPasses=1,axis=None,position=None,threshold=0.2):
		if nPasses<=0: return
		gc = self.pop()
		(l,r) = gc.subdivide(axis,position,threshold)
		self.push(l)
		self.subdivide(nPasses-1,axis,position,threshold)
		self.push(r)
		self.subdivide(nPasses-1,axis,position,threshold)

	def optimize(self,p,nStackElements=1):
		if nStackElements<=0: return
		gcList = self.stack[:nStackElements]
		for gc in gcList:
			p = gc.optimizeSegments(p)
		return p

	def connect(self,travelPlane,travelCode='G00',nStackElements=1):
		if nStackElements<=0: return
		gcList = self.stack[:nStackElements]
		for gc in gcList:
			gc.addTravelOffset( travelPlane, travelCode )
			gc.linkSegments()

	def origin(self,p=Point(0.0,0.0,0.0),nStackElements=1):
		if nStackElements<=0: return
		gcList = self.stack[:nStackElements]
		for gc in gcList:
			t = Point()
			l = gc.bbox.lower()
			if math.isnan(p.x): t.x = 0.0
			else: t.x = p.x - l.x
			if math.isnan(p.y): t.y = 0.0
			else: t.y = p.y - l.y
			if math.isnan(p.z): t.z = 0.0
			else: t.z = p.z - l.z
			gc.translate(t)

	def scale(self,s,nStackElements=1):
		if nStackElements<=0: return
		gcList = self.stack[:nStackElements]
		for gc in gcList:
			gc.scale(s)

	def translate(self,t,nStackElements=1):
		if nStackElements<=0: return
		gcList = self.stack[:nStackElements]
		for gc in gcList:
			gc.translate(t)

	def stats(self,nStackElements=1):
		if nStackElements<=0: return
		gcList = self.stack[:nStackElements]
		for gc in gcList:
			gc.printStats()

	def write(self,fileName):
		self.stack[-1].write( open(fileName,'w') )
