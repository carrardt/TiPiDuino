How to install flatcam for Ubuntu 18.04 in a container
======================================================
1. Install docker.io on host
sudo apt update && sudo apt upgrade -y && sudo apt install docker.io

2. Build the docker image with flatcam installed
sudo docker build -t flatcam -f flatcam-ubuntu-18.04.docker .

3. Create a working directory for FlatCAM.
# It will be used as home folder of docker's internal user
# and it will remain if you scratch everything and start again
# usefull to keep track of your FlatCAM configuration and to place files
# that are to be accessible from within FlatCAM
mkdir -p $HOME/flatcam

3. Run the docker
sudo docker run --network=host --interactive \
                -v /tmp/.X11-unix:/tmp/.X11-unix -v /tmp/.X11-unix:/tmp/.X11-unix \
                -v ~/flatcam:/home/flatcam  -v ~/.Xauthority:/home/flatcam/.Xauthority \
                -e DISPLAY=$DISPLAY -h $HOSTNAME flatcam

4. Clean-up container wastes
sudo docker container prune

