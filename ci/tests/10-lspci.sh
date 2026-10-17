# Check for QEMU educational PCI device and bind it to uio_pci_generic
modprobe uio_pci_generic
echo "1234 11e8" > /sys/bus/pci/drivers/uio_pci_generic/new_id
lspci -vt
lspci -vvv -d 1234:11e8
# expect to find the QEMU educational device with driver loaded
lspci -vvv -d 1234:11e8 | grep "driver in use: uio_pci_generic"
