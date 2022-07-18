#!/bin/bash

cp ../build/echo-ns/echoNsServer echoNsServer/
cp ../build/echo-ns/echoNsClient echoNsClient/

cd ns
docker build -t omninames .
# podman build --format docker -t omninames:latest -t fir.love.io:3005/omninames:latest .
# podman push fir.love.io:3005/omninames:latest
cd ..

cd echoNsServer
docker build -t echo-ns-server .
# If tagging images and pushing to local docker registry
# podman build --format docker -t echo-ns-server:latest -t fir.love.io:3005/echo-ns-server:latest .
# podman push fir.love.io:3005/echo-ns-server:latest
cd ..

cd echoNsClient
docker build -t echo-ns-client .
# If tagging images and pushing to local docker registry
# podman build --format docker -t echo-ns-client:latest -t fir.love.io:3005/echo-ns-client:latest .
# podman push fir.love.io:3005/echo-ns-client:latest
cd ..
