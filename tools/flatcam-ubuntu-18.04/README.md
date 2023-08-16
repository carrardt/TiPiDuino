Ubuntu 18.04 container for FlatCAM
==================================
1. Prerequisites
recent linux distro (tested on Ubuntu 22.04) with network access
2Gb of available disk space

2. Run
# first run will build the docker image be patient
# may take up to 1 hour depending on you internet connection
# FlatCam runs isolated in a docker, it cannot access all of your system files
# $HOME/flatcam directory is created and used as docker's home to run FlatCAM
# place your files in $HOME/flatcam to see them while running the software
./flatcam.sh

