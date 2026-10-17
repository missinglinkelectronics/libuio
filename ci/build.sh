#!/bin/sh
# Build libuio and create the release tarball used by the VM tests
set -ex

./autogen.sh
./configure
make
make distcheck
