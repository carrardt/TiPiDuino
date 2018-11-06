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

fout.write("unsigned char digits_bitmap[] = {")

lf = 0
for l in range(NLINES):
  for col in range(NCOLS):
    D=0
    Y=l*8
    X=col
    for b in range(8):
      D += (1-int(data[(Y+b)*NCOLS+X])) << b
    if lf%16 == 0:
		fout.write("\n")
    lf = lf + 1
    fout.write("0x%02X," %D )

fout.write("\n0 };\n")
fout.close()

