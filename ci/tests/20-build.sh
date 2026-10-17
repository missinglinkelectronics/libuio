# Build libuio and example applications with default config
rm -rf libuio-0.4.0
tar xvf "$SRC_TARBALL"
cd libuio-0.4.0
autoconf --version
./configure
make
# check existence of one compiled example application
test -e lsuio
