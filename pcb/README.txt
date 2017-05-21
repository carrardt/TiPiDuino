to obtain a complete CNC job :
generate bottom.nc top.nc drill.nc and cutout.nc from flatcam
then use the python script this way :

# Horizontal side by side top and bottom layers
ncOptimizer.py -s -n T top.nc drill.nc cutout.nc
ncOptimizer.py -transform -1 1 0 0 -s -n B bottom.nc drill.nc cutout.nc
ncOptimizer.py -n -o final.nc *.0 *.1 *.2

# Vertical side by side top and bottom layers
ncOptimizer.py -s -n T top.nc drill.nc cutout.nc
ncOptimizer.py -transform 1 -1 0 0 -s -n B bottom.nc drill.nc cutout.nc
ncOptimizer.py -n -o final.nc *.0 *.1 *.2
