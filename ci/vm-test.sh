#!/bin/sh
# Run the VM integration tests: boot a Debian cloud image with libvirt, let
# cloud-init run ci/run-tests.sh in it and convert the log to a JUnit
# report. Run ci/vm-cleanup.sh afterwards, also if this script fails.
#
# Expects the release tarball (see ci/build.sh) in the current directory.
# Writes report.xml and vm-test.log to the current directory.
#
# Environment:
#   VM_ARCH        Architecture of the VM: "amd64" (default) or "arm64". arm64
#                  is emulated without KVM and therefore much slower.
#   VM_BASE_IMAGE_AMD64, VM_BASE_IMAGE_ARM64
#                  Debian 13 generic cloud image (qcow2) for VM_ARCH on this
#                  host, used read-only as backing file. Downloaded for every
#                  run if empty.
#   VM_NETWORK     "user" (default) for QEMU user-mode networking, which needs
#                  no host setup, or "bridge" to attach the VM to VM_BRIDGE
#   VM_BRIDGE      Host bridge for VM_NETWORK=bridge. The user running the
#                  tests must be allowed to use it in /etc/qemu/bridge.conf.

set -ex

: "${VM_ARCH:=amd64}"
: "${VM_BASE_IMAGE_AMD64:=}"
: "${VM_BASE_IMAGE_ARM64:=}"
: "${VM_NETWORK:=user}"
: "${VM_BRIDGE:=}"

case "$VM_ARCH" in
amd64)
	VM_BASE_IMAGE=$VM_BASE_IMAGE_AMD64
	;;
arm64)
	VM_BASE_IMAGE=$VM_BASE_IMAGE_ARM64
	;;
*)
	echo "unknown VM_ARCH: $VM_ARCH" >&2
	exit 1
	;;
esac

VM_BASE_IMAGE_URL=https://cloud.debian.org/images/cloud/trixie/latest/debian-13-generic-$VM_ARCH.qcow2
TARBALL=libuio-0.4.0.tar.gz
# VM name and prefix of its files in /dev/shm, as in ci/libvirt-$VM_ARCH.xml
VM=ci-libuio-$VM_ARCH
LOG=/dev/shm/$VM-test.log
VIRSH="virsh -c qemu:///session"

# Check the network configuration before doing anything else
case "$VM_NETWORK" in
user)
	;;
bridge)
	case "$VM_BRIDGE" in
	"" | *[!A-Za-z0-9_.-]*)
		echo "VM_NETWORK=bridge needs a valid VM_BRIDGE, got '$VM_BRIDGE'" >&2
		exit 1
		;;
	esac
	;;
*)
	echo "unknown VM_NETWORK: $VM_NETWORK" >&2
	exit 1
	;;
esac

# Clean up any old logs so we don't accidentially interpret those
rm -fv "$LOG"

# Get the base image
if [ -z "$VM_BASE_IMAGE" ]; then
	VM_BASE_IMAGE=/dev/shm/$VM-base.qcow2
	curl -fsSL -o "$VM_BASE_IMAGE" "$VM_BASE_IMAGE_URL"
fi

# Create the VM
qemu-img create -f qcow2 -F qcow2 -b "$VM_BASE_IMAGE" \
	"/dev/shm/$VM-overlay.qcow2"
xorriso -volid cidata -joliet on \
	-outdev "stdio:/dev/shm/$VM-seed.iso" \
	-map ci/user-data /user-data \
	-map ci/meta-data /meta-data \
	-map "$TARBALL" "/$TARBALL" \
	-map ci/run-tests.sh /run-tests.sh \
	-map ci/tests /tests
if [ "$VM_NETWORK" = bridge ]; then
	sed "s#<interface type='user'>#<interface type='bridge'>\n      <source bridge='$VM_BRIDGE'/>#" \
		"ci/libvirt-$VM_ARCH.xml" > "/dev/shm/$VM-libvirt.xml"
else
	cp "ci/libvirt-$VM_ARCH.xml" "/dev/shm/$VM-libvirt.xml"
fi
$VIRSH define "/dev/shm/$VM-libvirt.xml"

# Run the VM with autostarting tests
$VIRSH start "$VM"
set +x
echo "Waiting for VM to complete tests..."
while [ "$($VIRSH domstate "$VM" 2>/dev/null)" = "running" ]; do
	sleep 5
done
set -x

# Keep the raw log, print it and parse it to a JUnit XML
cp "$LOG" vm-test.log
cat vm-test.log
python3 ci/log2junit.py vm-test.log report.xml ci/tests
