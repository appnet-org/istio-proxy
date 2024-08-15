#!/bin/bash

RELEASE=release-1.22-latest

ENVOY_ROOT=$(dirname $(realpath -s $0))
APPNET_ROOT=$(dirname $(dirname $(dirname $(dirname $(realpath -s $0)))))
echo "ENVOY_ROOT: ${ENVOY_ROOT}"
echo "APPNET_ROOT: ${APPNET_ROOT}"
# like /mnt/appnet/compiler/compiler/graph/generated/istio_envoy

# SUBMODULE_DOTGIT=

USER_ID=$(id -u)
GROUP_ID=$(id -g)

# echo the command
echo "docker run -it -w /work -v ${ENVOY_ROOT}:/work --user ${USER_ID}:${GROUP_ID} gcr.io/istio-testing/build-tools-proxy:${RELEASE} /bin/bash -c \"git config --global --add safe.directory /work && make build_envoy && cp /work/bazel-bin/envoy /work/envoy\""

docker run -it -w /work -v ${ENVOY_ROOT}:/work --user ${USER_ID}:${GROUP_ID} gcr.io/istio-testing/build-tools-proxy:${RELEASE} /bin/bash -c "git config --global --add safe.directory /work && make build_envoy && cp /work/bazel-bin/envoy /work/envoy"
