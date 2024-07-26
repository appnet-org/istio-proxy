# How to use your Istio proxy with custom Envoy
```
# This builds Envoy binary in an container for building. The binary is copied out to the root dir.
./build.sh

# Build your own Istio proxy image.
# Make sure you replace all the same string in appnet proj, if you want to use a new image.

sudo docker build -t docker.io/jokerwyt/istio-proxy-1.22:latest -f Dockerfile.istioproxy .

# Push that image. Docker login if you need.

docker push docker.io/jokerwyt/istio-proxy-1.22:latest


# restart current appnet

kubectl delete all,sa,pvc,pv,envoyfilters,appnetconfigs --all

# under appnet root folder
kubectl apply -f config/samples/echo/sample_echo_sidecar.yaml
```

# Istio Proxy

The Istio Proxy is a microservice proxy that can be used on the client and server side, and forms a microservice mesh.
It is based on [Envoy](http://envoyproxy.io) with the addition of several policy and telemetry extensions.

According to the [conclusion from Istio workgroup meeting on 4-17-2024](https://docs.google.com/document/d/1wsa06GGiq1LEGwhkiPP0FKIZJqdAiue-VeBonWAzAyk/edit#heading=h.ma5hboh81yw):

- New extensions are not added unless they are part of core APIs