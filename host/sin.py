#!/usr/bin/python

import sys
from math import *
scale = float(sys.argv[1]);

#for i in range(16):
#	print sin( (i/8.0) * scale * pi ) * 0.5 + 0.5

for i in range(16):
	print int( (sin( (i/8.0) * scale * pi ) * 0.5 + 0.5)*1024.0 )
