#!/bin/sh
# Build the Debian ppc64 (64-bit big-endian) cloud image for VM_ARCH=ppc64 of
# ci/vm-test.sh. Debian has no official ppc64 cloud images, so the image is
# bootstrapped from Debian ports (unstable) in an amd64 builder VM, which is a
# Debian 13 cloud image booted with KVM like the amd64 VM test.
#
# Debian ports unstable changes all the time and is not always installable,
# so build the image once, keep it and set VM_BASE_IMAGE_PPC64 to it.
#
# Usage: ci/ppc64-image.sh <output.qcow2>
#
# Environment:
#   VM_BASE_IMAGE_AMD64  Debian 13 generic amd64 cloud image (qcow2) for the
#                        builder VM, used read-only as backing file.
#                        Downloaded if empty.
#   PPC64_IMAGE_SIZE     Virtual disk size of the image, default 4G

set -ex

OUT=${1:?usage: $0 <output.qcow2>}
: "${VM_BASE_IMAGE_AMD64:=}"
: "${PPC64_IMAGE_SIZE:=4G}"

VM_BASE_IMAGE_URL=https://cloud.debian.org/images/cloud/trixie/latest/debian-13-generic-amd64.qcow2
# VM name and prefix of its files in /dev/shm
VM=ci-libuio-ppc64-builder
LOG=/dev/shm/$VM-test.log
DISK=/dev/shm/$VM-disk.raw
VIRSH="virsh -c qemu:///session"

cleanup() {
	set +e
	$VIRSH destroy "$VM"
	$VIRSH undefine --nvram "$VM"
	rm -fv "/dev/shm/$VM-base.qcow2" "/dev/shm/$VM-overlay.qcow2" \
		"/dev/shm/$VM-seed.iso" "/dev/shm/$VM-libvirt.xml" "$LOG" "$DISK"
}
trap cleanup EXIT
cleanup

# Get the base image of the builder VM
if [ -z "$VM_BASE_IMAGE_AMD64" ]; then
	VM_BASE_IMAGE_AMD64=/dev/shm/$VM-base.qcow2
	curl -fsSL -o "$VM_BASE_IMAGE_AMD64" "$VM_BASE_IMAGE_URL"
fi

# Create the builder VM: the amd64 VM test domain with its own name and files,
# more memory and the empty disk for the image as /dev/vdb
qemu-img create -f qcow2 -F qcow2 -b "$VM_BASE_IMAGE_AMD64" \
	"/dev/shm/$VM-overlay.qcow2"
qemu-img create -f raw "$DISK" "$PPC64_IMAGE_SIZE"
xorriso -volid cidata -joliet on \
	-outdev "stdio:/dev/shm/$VM-seed.iso" \
	-map ci/ppc64-image/user-data /user-data \
	-map ci/meta-data /meta-data \
	-map ci/ppc64-image/build.sh /build.sh
sed -e "s#ci-libuio-amd64#$VM#g" \
	-e "s#<memory unit='KiB'>1048576#<memory unit='KiB'>2097152#" \
	-e "s#<currentMemory unit='KiB'>1048576#<currentMemory unit='KiB'>2097152#" \
	-e "s#<devices>#<devices>\n    <disk type='file' device='disk'>\n      <driver name='qemu' type='raw'/>\n      <source file='$DISK'/>\n      <target dev='vdb' bus='virtio'/>\n    </disk>#" \
	ci/libvirt-amd64.xml > "/dev/shm/$VM-libvirt.xml"
$VIRSH define "/dev/shm/$VM-libvirt.xml"

# Run the builder VM, it powers off when done
$VIRSH start "$VM"
set +x
echo "Waiting for the builder VM to build the image..."
while [ "$($VIRSH domstate "$VM" 2>/dev/null)" = "running" ]; do
	sleep 5
done
set -x

cp "$LOG" ppc64-image.log
cat ppc64-image.log
grep -q "IMAGE BUILD SUCCEEDED" ppc64-image.log

qemu-img convert -c -f raw -O qcow2 "$DISK" "$OUT"
qemu-img info "$OUT"
