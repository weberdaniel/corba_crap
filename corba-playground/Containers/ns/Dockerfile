FROM ubuntu:latest

RUN apt-get update 
RUN apt-get -y install omniorb apt-file libomniorb4-2 libomnithread4
RUN apt-file update && apt-file list omniorb

CMD [ "/usr/local/bin/omniNames", "-start", "-logdir", "/var/log/omniORB" ]
