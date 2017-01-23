to obtain a complete CNC job :
generate Bottom_iso.nc Top_iso.nc drill.nc cutout.nc from flatcam
then use the python script this way :

ncOptimizer.py -r -s bottom Bottom_iso.nc drill.nc cutout.nc
ncOptimizer.py -y -s top Top_iso.nc drill.nc cutout.nc
ncOptimizer.py -o SMCHat.nc bottom.0 top.0 top.1 bottom.1