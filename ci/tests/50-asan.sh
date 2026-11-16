# Build and run libuio with asan config
# requires: test_lspci
rm -rf libuio
mkdir libuio
tar xvf "$SRC_TARBALL" -C libuio --strip-components=1
cd libuio
autoconf --version
./configure CFLAGS="-fsanitize=address -fno-omit-frame-pointer -g -O1" LDFLAGS="-fsanitize=address"
make
make install
ldconfig
# leave build directory
cd
rm -rf libuio
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
