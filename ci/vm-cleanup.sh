#!/bin/sh
# Clean up the VM and files created by ci/vm-test.sh
set -x

VM=ci-libuio-${VM_ARCH:-amd64}

virsh -c qemu:///session destroy "$VM"
virsh -c qemu:///session undefine --nvram "$VM"
rm -fv "/dev/shm/$VM-base.qcow2"
rm -fv "/dev/shm/$VM-overlay.qcow2"
rm -fv "/dev/shm/$VM-seed.iso"
rm -fv "/dev/shm/$VM-libvirt.xml"
rm -fv "/dev/shm/$VM-test.log"

exit 0
