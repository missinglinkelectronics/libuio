#!/bin/sh
# Runs as root in the amd64 builder VM started by ci/ppc64-image.sh: install
# Debian unstable for ppc64 (big-endian) from Debian ports on the empty disk
# given as argument and make it a cloud image bootable by SLOF on the QEMU
# pseries machine.
#
# Layout of the disk (MBR, as supported by SLOF):
#   1: PReP boot partition with the GRUB core image, loaded by SLOF
#   2: ext4 root file system, last so cloud-init can grow it

set -ex

disk=${1:?usage: $0 <disk>}
mirror=http://deb.debian.org/debian-ports
keyring=/usr/share/keyrings/debian-ports-archive-keyring.gpg
target=/mnt

# Check that ppc64 binaries can be run (qemu-user-binfmt) before starting
test -e /proc/sys/fs/binfmt_misc/qemu-ppc64
cat /proc/sys/fs/binfmt_misc/qemu-ppc64

sfdisk "$disk" <<EOF
label: dos
,8MiB,41,*
,,83
EOF
udevadm settle
mkfs.ext4 -L debian-root "${disk}2"
uuid=$(blkid -s UUID -o value "${disk}2")
mount "${disk}2" "$target"

# The kernel symlinks /boot/vmlinux and /boot/initrd.img are used by grub.cfg.
# mmdebstrap does not install Recommends, so everything needed is listed.
mmdebstrap --arch=ppc64 \
	--essential-hook='echo "link_in_boot = yes" > "$1/etc/kernel-img.conf"' \
	--include=systemd-sysv,udev,dbus,kmod,systemd-resolved,initramfs-tools \
	--include=linux-image-powerpc64,grub-ieee1275-bin,e2fsprogs,fdisk \
	--include=cloud-init,cloud-guest-utils,sudo,openssh-server,pciutils \
	--include=ca-certificates,netbase,iproute2,debian-ports-archive-keyring \
	unstable "$target" \
	"deb [signed-by=$keyring] $mirror unstable main" \
	"deb [signed-by=$keyring] $mirror unreleased main"

echo "UUID=$uuid / ext4 errors=remount-ro 0 1" > "$target/etc/fstab"

# DHCP on every Ethernet interface, in case cloud-init writes no network
# configuration
cat > "$target/etc/systemd/network/99-dhcp.network" <<EOF
[Match]
Type=ether

[Network]
DHCP=yes
EOF
chroot "$target" systemctl enable systemd-networkd systemd-resolved

# Only look for the NoCloud seed ISO, probing other clouds takes long
cat > "$target/etc/cloud/cloud.cfg.d/90-datasource.cfg" <<EOF
datasource_list: [ NoCloud, None ]
EOF

# Make each VM unique on first boot, like the official cloud images
: > "$target/etc/machine-id"
rm -f "$target/var/lib/dbus/machine-id" "$target"/etc/ssh/ssh_host_*

# GRUB: the core image in the PReP partition finds the root file system by
# its UUID and loads /boot/grub/grub.cfg from it
mkdir -p "$target/boot/grub"
cat > "$target/boot/grub/early.cfg" <<EOF
search --no-floppy --fs-uuid --set=root $uuid
set prefix=(\$root)/boot/grub
EOF
cat > "$target/boot/grub/grub.cfg" <<EOF
set timeout=1
menuentry 'Debian GNU/Linux' {
	linux /boot/vmlinux root=UUID=$uuid ro console=hvc0
	initrd /boot/initrd.img
}
EOF
chroot "$target" grub-mkimage -O powerpc-ieee1275 \
	-o /boot/grub/core.elf -c /boot/grub/early.cfg -p /boot/grub \
	part_msdos ext2 search search_fs_uuid configfile normal linux
dd if="$target/boot/grub/core.elf" of="${disk}1" conv=fsync

ls -l "$target/boot" "$target/boot/grub"
test -e "$target/boot/vmlinux"
test -e "$target/boot/initrd.img"
chroot "$target" dpkg -l
du -sh "$target"

umount "$target"
fsck.ext4 -fn "${disk}2"
sync
