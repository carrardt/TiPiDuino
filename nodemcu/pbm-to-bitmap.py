#!/usr/bin/python

import sys

fin=open(sys.argv[1],"r")
fin.readline()
fin.readline()
(width,height) = [ int(x) for x in fin.readline().split(' ') ]
data=fin.read().replace('\n','')
fin.close()
fin=None
print( "Width=%d, Height=%d, size=%d" % (width,height,len(data)) )

NCOLS=width
NLINES=height/8

fout=open(sys.argv[2],"wb")

for l in range(NLINES):
  for col in range(NCOLS):
    D=0
    Y=l*8
    X=col
    for b in range(8):
      D += int(data[(Y+b)*84+X]) << b
    fout.write('%c'%D)
fout.close()

