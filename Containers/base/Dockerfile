# 20/Jul/2022 Daniel Weber (Apache License 2.0)
# Added this file and the corresponding .tar.gz archive
FROM ubuntu:latest AS basebuildcorbaplayground

COPY omniORB-4.3.0.tar.bz2 /opt/omniORB-4.3.0.tar.bz2

RUN apt-get update 
RUN apt-get -y install bzip2 tar omniorb apt-file libomniorb4-2 libomnithread4 gcc python3 g++ make python3-dev
RUN apt-file update && apt-file list omniorb
RUN cd /opt && bunzip2 omniORB-4.3.0.tar.bz2
RUN cd /opt && tar xvf omniORB-4.3.0.tar
RUN cd /opt/omniORB-4.3.0 && ls
RUN cd /opt/omniORB-4.3.0 && mkdir build && cd build && ../configure && make && make install
RUN ldconfig
RUN echo '#!/bin/bash' > start.sh
RUN echo 'export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/usr/local/lib'
RUN echo '/usr/local/bin/omniNames -always -logdir /var/log/omniORB' >> start.sh
RUN chmod +rwx start.sh
RUN mkdir -p /var/log/omniORB
RUN chmod -R uoa+rwx /var/log/omniORB

CMD [ "start.sh" ]
