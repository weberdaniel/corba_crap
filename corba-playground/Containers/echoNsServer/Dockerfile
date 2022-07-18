FROM ubuntu:latest

ADD echoNsServer /

RUN \
    apt-get update && \
    apt-get -y install omniorb && \
    echo "InitRef = NameService=corbaname::nameservice:2809" > /etc/omniORB.cfg && \
    echo "traceLevel = 10" >> /etc/omniORB.cfg && \
    echo "supportBootstrapAgen = 1" >> /etc/omniORB.cfg

CMD [ "/echoNsServer" ]
