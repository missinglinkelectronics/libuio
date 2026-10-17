# Build and run libuio with asan config
# requires: test_lspci
rm -rf libuio-0.4.0
tar xvf "$SRC_TARBALL"
cd libuio-0.4.0
autoconf --version
./configure CFLAGS="-fsanitize=address -fno-omit-frame-pointer -g -O1" LDFLAGS="-fsanitize=address"
make
make install
ldconfig
# leave build directory
cd
rm -rf libuio-0.4.0
# uio_find_by_uio_name() must free the second matching candidate, which is
# only exercised if both edu devices are bound to uio_pci_generic
test "$(cat /sys/class/uio/uio0/name)" = uio_pci_generic
test "$(cat /sys/class/uio/uio1/name)" = uio_pci_generic
# make leaks fail the test (via return code)
export ASAN_OPTIONS=detect_leaks=1
# check basic functionality and see if asan errors are reported (via return code)
lsuio
val=$(readuio uio_pci_generic 0x00)
printf "0x%08xu\n" ${val#* }
