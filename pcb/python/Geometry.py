#!/c/Users/thier/AppData/Local/Programs/Python/Python35-32/python.exe

import math

class AABB:
	def __init__(self):
		self.xMin=1.e12
		self.xMax=-1.e12
		self.yMin=1.e12
		self.yMax=-1.e12
		self.zMin=1.e12
		self.zMax=-1.e12
		
	def union(self, o):
		if self.empty():
			return o.copy()
		if o.empty():
			return self.copy()
		bb = AABB()
		bb.xMin = min( self.xMin , o.xMin )
		bb.yMin = min( self.yMin , o.yMin )
		bb.zMin = min( self.zMin , o.zMin )
		bb.xMax = max( self.xMax , o.xMax )
		bb.yMax = max( self.yMax , o.yMax )
		bb.zMax = max( self.zMax , o.zMax )
		return bb
	
	def insertPoint(self, p):
		self.xMin = min( self.xMin , p.x )
		self.yMin = min( self.yMin , p.y )
		self.zMin = min( self.zMin , p.z )
		self.xMax = max( self.xMax , p.x )
		self.yMax = max( self.yMax , p.y )
		self.zMax = max( self.zMax , p.z )

	def contains(self,p):
		if self.empty():
			return False
		else:
			return p.x>=self.xMin and p.x<=self.xMax and p.y>=self.yMin and p.y<=self.yMax and p.z>=self.zMin and p.z<=self.zMax

	def containsBounds(self,bb):
		if bb.empty():
			return True
		elif self.empty():
			return False
		else:
			return bb.xMin>=self.xMin and bb.xMax<=self.xMax and bb.yMin>=self.yMin and bb.yMax<=self.yMax and bb.zMin>=self.zMin and bb.zMax<=self.zMax

	def copy(self):
		bb = AABB()
		bb.xMin = self.xMin
		bb.xMax = self.xMax
		bb.yMin = self.yMin
		bb.yMax = self.yMax
		bb.zMin = self.zMin
		bb.zMax = self.zMax
		return bb
		
	def size(self):
		return (self.xMax-self.xMin,self.yMax-self.yMin,self.zMax-self.zMin)
		
	def center(self):
		return ( (self.xMax+self.xMin)*0.5 , (self.yMax+self.yMin)*0.5 , (self.zMax+self.zMin)*0.5 )
		
	def intersection(self,bb):
		r = AABB()
		if not bb.empty() and not self.empty():
			r.xMin = max(self.xMin,bb.xMin)
			r.xMax = min(self.xMax,bb.xMax)
			r.yMin = max(self.yMin,bb.yMin)
			r.yMax = min(self.yMax,bb.yMax)
			r.zMin = max(self.zMin,bb.zMin)
			r.zMax = min(self.zMax,bb.zMax)
		return r
		
	def xyArea(self):
		if self.empty(): return 0.0
		s = self.size()
		return s[0]*s[1]
		
	def volume(self):
		if self.empty(): return 0.0
		s = self.size()
		return s[0]*s[1]*s[2]

	def empty(self):
		return self.xMin>self.xMax or self.yMin>self.yMax or self.zMin>self.zMax
		
	def __str__(self):
		return "X[%g;%g] Y[%g;%g] Z[%g;%g]"%(self.xMin,self.xMax,self.yMin,self.yMax,self.zMin,self.zMax)

class Point:
	def __init__(self,x,y,z,c="G01"):
		self.x=x
		self.y=y
		self.z=z
		self.c=c
		
	def dist2( self , b ):
		x=self.x-b.x;
		y=self.y-b.y;
		z=self.z-b.z;
		return x*x+y*y+z*z
		
	def dist(self,b):
		return math.sqrt(self.dist2(b))
		
	def scale(self,s):
		return Point(self.x*s,self.y*s,self.z*s)

	def imult(self,s):
		self.x *= s.x
		self.y *= s.y
		self.z *= s.z

	def iadd(self,t):
		self.x += t.x
		self.y += t.y
		self.z += t.z

	def add(self,p):
		return Point(self.x+p.x,self.y+p.y,self.z+p.z)

	def sub(self,p):
		return Point(self.x-p.x,self.y-p.y,self.z-p.z)

	def equals(self,p):
		return self.x==p.x and self.y==p.y and self.z==p.z

	def coords(self):
		return (self.x,self.y,self.z)
		
	def __str__(self):
		return "X%gY%gZ%g"%(self.x,self.y,self.z)

class Plane:
	def __init__(self,A=1.0,B=0.0,C=0.0,D=0.0):
		self.A = A
		self.B = B
		self.C = C
		self.D = D
		
	def distance(self,P):
		return self.A * P.x + self.B * P.y + self.C * P.z + self.D
	
	def project(self,P):
		d = self.distance(P)
		return Point(P.x-self.A*d,P.y-self.B*d,P.z-self.C*d)

	def lineIntersection(self,p1,p2):
		d1=self.distance(p1)
		d2=self.distance(p2)
		if (d2-d1)==0.0:
			return p1
		t = d1 / (d1-d2)
		return p1.add( p2.sub(p1).scale(t) )
		
class Segment:
	def __init__(self,S=14000.0,F=100.0):
		self.nodes=[]
		self.bbox=AABB()
		self.spindle = S
		self.speed = F

	def updateBounds(self):
		self.bbox=AABB()
		for node in self.nodes:
			self.bbox.insertPoint(node)
			
	def bounds(self):
		return self.bbox
		
	def setSpindle(self,s):
		self.spindle = s
		
	def getSpindle(self):
		return self.spindle
		
	def setSpeed(self,f):
		self.speed = f
		
	def getSpeed(self):
		return self.speed
		
	def addNode(self,n):
		self.nodes.append(n)
		self.bbox.insertPoint(n)
	
	def firstNode(self):
		if self.numberOfNodes()==0:
			return None
		else:
			return self.nodes[0]

	def lastNode(self):
		if self.numberOfNodes()==0:
			return None
		else:
			return self.nodes[-1]
	
	def findNearestNode(self,p):
		nn = self.numberOfNodes()
		if nn==0:
			return 0.0
		if nn==1:
			return (0,self.firstNode().dist(p))
		if self.isLoop():
			minDist = self.firstNode().dist(p)
			minI = 0
			for (i,n) in enumerate(self.nodes):
				d = n.dist(p)
				if d<minDist:
					minDist = d
					minI = i
			return (minI,minDist)
		else:
			ds = self.firstNode().dist(p)
			de = self.lastNode().dist(p)
			if ds<de:
				return (0,ds)
			else:
				return (nn-1,de)
	
	def rotateNodes(self,i):
		nn = self.numberOfNodes()
		if i<0 or i>=nn:
			raise BaseException("start point is not a segment node")
		isloop = self.isLoop()
		if not isloop and i!=0 and i!=(nn-1):
			raise BaseException("segment is not a loop, start point must be an end")
		if isloop:
			if i>0: i -= 1
			polyline = self.nodes[1:] # remove duplicated point (that's makes it a loop)
			rotnodes = polyline[i:] + polyline[:i] + [polyline[i]] # re-duplicate new starting point at the end
			self.nodes = rotnodes
		elif i==(self.numberOfNodes()-1):
			self.nodes.reverse()
		elif i!=0:
			raise BaseException("a non-loop segment can only begin by one of its two ends")
	
	def length(self):
		if len(self.nodes)==0:
			return 0.0
		l = 0.0
		no = self.nodes[0]
		for n in self.nodes[1:]:
			l += no.dist(n)
			no = n
		return l
		
	def gapDistance(self,s):
		if len(self.nodes)==0 or len(s.nodes)==0:
			return 0.0
		return self.nodes[-1].dist(s.nodes[0])
		
	def write(self,stream):
		if len(self.nodes)==0:
			return
		stream.write("F%g\nM03 S%g\nG4\nP1\n" % (self.getSpeed(),self.getSpindle()))
		(x,y,z) = (None,None,None)
		for n in self.nodes:
			(nx,ny,nz) = n.coords()
			coordStr = n.c+' '
			if nx!=x: coordStr += "X%.4f" % nx
			if ny!=y: coordStr += "Y%.4f" % ny
			if nz!=z: coordStr += "Z%.4f" % nz
			if (x,y,z) != (nx,ny,nz):
				stream.write(coordStr+"\n")
				(x,y,z) = (nx,ny,nz)
	
	def numberOfNodes(self):
		return len(self.nodes)
		
	def headerLength(self):
		return len(self.header)
		
	def empty(self):
		return self.numberOfNodes()==0
		
	def isLoop(self):
		if len(self.nodes)<2:
			return False
		return self.nodes[0].equals( self.nodes[-1] )
		
	def planeCut(self,plane):
		l=[]
		r=[]
		ls = Segment(self.getSpindle(),self.getSpeed())
		rs = Segment(self.getSpindle(),self.getSpeed())
		for n in self.nodes:
			if plane.distance(n)<=0: # point is left
				if not rs.empty():
					I = plane.lineIntersection(rs.nodes[-1],n)
					rs.nodes.append(I)
					rs.updateBounds()
					r.append(rs)
					rs = Segment(self.getSpindle(),self.getSpeed())
					ls.nodes.append(I)
				ls.nodes.append(n)
			else: # point is right
				if not ls.empty():
					I = plane.lineIntersection(ls.nodes[-1],n)
					ls.nodes.append(I)
					ls.updateBounds()
					l.append(ls)
					ls = Segment(self.getSpindle(),self.getSpeed())
					rs.nodes.append(I)
				rs.nodes.append(n)
		if not ls.empty():
			l.append(ls)
		if not rs.empty():
			r.append(rs)
		return (l,r)
		
	def cutOff(self,plane):
		(l,r) = self.planeCut( plane )
		return l
	
	def translate(self,t):
		for n in self.nodes:
			n.iadd(t)
		self.updateBounds()

	def scale(self,s):
		for n in self.nodes:
			n.imult(s)
		self.updateBounds()

	def printStats(self):
		print("Number of nodes = %d"%self.numberOfNodes())
		print("Length = %g"%self.length())
		print("Nodes = %s"%" ".join([str(x) for x in self.nodes[:4]]))

	def print(self):
		print("Number of nodes = %d"%self.numberOfNodes())
		print("Length = %g"%self.length())
		print("Nodes = %s"%" ".join([str(x) for x in self.nodes]))
		
def main():
	from random import random
	for i in range(1000):
		p1 = Point( random()-0.5 , random()-0.5 , random()-0.5 )
		p2 = Point( random()-0.5 , random()-0.5 , random()-0.5 )
		plane = Plane( random()-0.5 , random()-0.5 , random()-0.5 , random()-0.5 )
		I = plane.lineIntersection(p1,p2)
		if plane.distance(I)>1.e-6:
			raise BaseException("line intersection test failed")
	print("Line intersection test OK")
	bb = AABB()
	for i in range(100):
		if bb.contains( Point(random()*1.e9-5.e8,random()*1.e9-5.e8,random()*1.e9-5.e8) ):
			raise BaseException("empty AABB must NOT contain any point")
	bb.insertPoint( Point(1.0,1.0,1.0) ) 
	if not bb.contains( Point(1.0,1.0,1.0) ):
		raise BaseException("AABB built with only one point P should contain point P")
	bb.insertPoint( Point(0.0,2.0,1.0) )
	if bb.contains( Point(0.0,0.0,0.0) ):
		raise BaseException("AABB basic contain test failed")
	bb = AABB()
	for i in range(100):
		p = Point(random()*1.e9-5.e8,random()*1.e9-5.e8,random()*1.e9-5.e8)
		oldbb = bb.copy()
		bb.insertPoint( p )
		if not bb.containsBounds(oldbb):
			raise BaseException("inserting a point to an AABB can only make it bigger")
	print("AABB unit test Ok")

if __name__=="__main__":
	main()
