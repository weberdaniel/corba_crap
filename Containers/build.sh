#!/bin/bash
# Changed by Daniel Weber 20. Jul 2022:
# podman replaced with docker, also added base folder

cp ../build/echo-ns/echoNsServer echoNsServer/
cp ../build/echo-ns/echoNsClient echoNsClient/

cd base 
docker build -t basebuildcorbaplayground .
cd ..

cd ns
docker build -t omninames .
cd ..

cd echoNsServer
docker build -t echo-ns-server .
cd ..

cd echoNsClient
docker build -t echo-ns-client .
cd ..
