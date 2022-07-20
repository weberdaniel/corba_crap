# Change 20/Jul/2022 by Daniel Weber
# Changed Fedora to basebuildcorbaplayground image
# removed installation of omniOrb and shifted it to above image
FROM basebuildcorbaplayground

ADD echoNsServer /

RUN \
    echo "InitRef = NameService=corbaname::nameservice:2809" > /etc/omniORB.cfg && \
    echo "traceLevel = 10" >> /etc/omniORB.cfg && \
    echo "supportBootstrapAgen = 1" >> /etc/omniORB.cfg

CMD [ "/echoNsServer" ]
