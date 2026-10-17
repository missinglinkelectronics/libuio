# Install libuio and example applications
# requires: test_build
cd libuio-0.4.0
make install
ldconfig
# leave build directory
cd
rm -rf libuio-0.4.0
# check correct installation of one example application
lsuio --version
