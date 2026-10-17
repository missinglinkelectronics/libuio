# Run example applications against QEMU educational PCI device
# requires: test_lspci test_install
lsuio
val=$(readuio uio_pci_generic 0x00)
printf "0x%08xu\n" ${val#* }
# check the UIO device can be listed (most basic functionality)
lsuio | grep uio0
