#!/usr/bin/bash

[ $HOSTNAME ] || HOSTNAME=`hostname`
WORKDIR=`dirname $0`
echo "*** Docker based FlatCAM runnin on $HOSTNAME (WD=$WORKDIR) ***"

[ $DOCKER ] || DOCKER=`which docker`
[ ! $DOCKER ] && echo "Installing docker.io ..." && sudo apt install docker.io && DOCKER=`which docker`
[ ! $DOCKER ] && echo "Failed to find docker, aborting" && exit 1

# Ensures we have a home/workdir for FlatCAM
mkdir -p $HOME/flatcam

# build docker image if not present
DOCKERBUILT=`sudo $DOCKER images | grep -c "flatcam.*latest"`
[ $DOCKERBUILT -eq 0 ] && echo "Building Ubuntu-18.04 based FlatCAM docker image ..." && sudo $DOCKER build -t flatcam $WORKDIR
DOCKERBUILT=`sudo $DOCKER images | grep -c "flatcam.*latest"`
[ $DOCKERBUILT -eq 0 ] && echo "Failed to build docker image, aborting" && exit 1

# Run FlatCAM in docker
sudo $DOCKER run --network=host --interactive \
                -v /tmp/.X11-unix:/tmp/.X11-unix -v /tmp/.X11-unix:/tmp/.X11-unix \
                -v $HOME/flatcam:/home/flatcam  -v $HOME/.Xauthority:/home/flatcam/.Xauthority \
                -e DISPLAY=$DISPLAY -h $HOSTNAME flatcam

