# Use the official image as a parent image
FROM ubuntu:18.04

# Update the system
RUN apt-get update 
# edit to match you local timzone
RUN DEBIAN_FRONTEND="noninteractive" TZ="Europe/Paris" apt install -y tzdata
RUN apt-get upgrade -y

# install flatcamp prerequisites
RUN apt-get install -y libpng-dev libfreetype6 libfreetype6-dev python3-dev python3-simplejson python3-pyqt4 python3-numpy \
                    python3-scipy python3-matplotlib libgeos-dev python-shapely python-pip libspatialindex-dev \
                    python3-tk python3-shapely python3-rtree python3-svg.path git

# install X11 forwarding tools
RUN apt-get install -y x11-apps mesa-utils

# download flatcam
RUN mkdir -p /usr/local
RUN git clone https://bitbucket.org/jpcgt/flatcam.git /usr/local/flatcam
RUN chmod a+rX -R /usr/local/flatcam

# create FlatCAM user and group
RUN addgroup flatcam
RUN adduser --home /home/flatcam --ingroup flatcam --disabled-password --gecos "" flatcam
RUN echo "flatcam:flatcam" | chpasswd
RUN chown -R flatcam:flatcam /home/flatcam

ENV NOTVISIBLE "in users profile"
RUN echo "export VISIBLE=now" >> /etc/profile

# run as user
USER flatcam
ENV HOME /home/flatcam
ENV LIBGL_ALWAYS_SOFTWARE 1
ENV QT_GRAPHICSSYSTEM native
WORKDIR /home/flatcam

# Run FlatCAM software
CMD [ "/usr/local/flatcam/flatcam" ]

