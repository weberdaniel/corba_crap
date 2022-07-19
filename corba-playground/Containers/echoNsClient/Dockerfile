FROM basebuildcorbaplayground

ADD echoNsClient /

RUN \
    echo "InitRef = NameService=corbaname::nameservice:2809" > /etc/omniORB.cfg && \
    echo "traceLevel = 10" >> /etc/omniORB.cfg

CMD [ "/echoNsClient" ]
