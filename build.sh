#!/bin/bash

RELEASE=release-1.22-latest
PWD=$(pwd)

# run the following command in the docker:
# git config --global --add safe.directory /work
# make build_envoy
# cp /work/bazel-bin/envoy /work/envoy

# echo the command
echo "docker run -it -w /work -v ${PWD}:/work gcr.io/istio-testing/build-tools-proxy:${RELEASE} /bin/bash -c \"git config --global --add safe.directory /work && make build_envoy && cp /work/bazel-bin/envoy /work/envoy\""

docker run -it -w /work -v ${PWD}:/work gcr.io/istio-testing/build-tools-proxy:${RELEASE} /bin/bash -c "git config --global --add safe.directory /work && make build_envoy && cp /work/bazel-bin/envoy /work/envoy"
