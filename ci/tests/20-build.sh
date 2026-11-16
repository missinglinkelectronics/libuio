# Build libuio and example applications with default config
rm -rf libuio
mkdir libuio
tar xvf "$SRC_TARBALL" -C libuio --strip-components=1
cd libuio
autoconf --version
./configure
make
# check existence of one compiled example application
test -e lsuio
