# Install libuio and example applications
# requires: test_build
cd libuio
make install
ldconfig
# leave build directory
cd
rm -rf libuio
# check correct installation of one example application
lsuio --version
