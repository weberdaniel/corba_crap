FROM ubuntu:latest AS basebuildcorbaplayground

COPY omniORB-4.3.0.tar.bz2 /opt/omniORB-4.3.0.tar.bz2

RUN apt-get update 
RUN apt-get -y install bzip2 tar omniorb apt-file libomniorb4-2 libomnithread4 gcc python3 g++ make python3-dev
RUN apt-file update && apt-file list omniorb
RUN cd /opt && bunzip2 omniORB-4.3.0.tar.bz2
RUN cd /opt && tar xvf omniORB-4.3.0.tar
RUN cd /opt/omniORB-4.3.0 && ls
RUN cd /opt/omniORB-4.3.0 && mkdir build && cd build && ../configure && make && make install

CMD [ "/usr/local/bin/omniNames", "-start", "-logdir", "/var/log/omniORB" ]
