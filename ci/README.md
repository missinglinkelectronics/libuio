# libuio CI

The CI builds libuio, then boots Debian VMs with two QEMU `edu` PCI devices
and runs integration tests in it against real UIO devices.
The VM test run for amd64 (with KVM), arm64 and big-endian ppc64 (both
emulated by QEMU without KVM, as the current CI hosts are x86-64). As the
emulated tests are slow, they only run on request by default.
The same scripts are used by GitLab CI (`.gitlab-ci.yml`) and GitHub Actions
(`.github/workflows/ci.yml`), and can also be run locally.

**These CI runs are meant to be regression tests and should not be used to
obtain Debian packages for installation.
Please install packages from official sources.**

The pipeline has three stages:

1. **build**: build libuio and create the release tarball
2. **qemu**: run the integration tests in the VM, one job per architecture
3. **package**: once the amd64 tests passed, build Debian packages of the current
   source with the [Debian packaging](https://salsa.debian.org/debian/libuio),
   to find changes that break the Debian package early

## Files

| File | Runs on | Purpose |
| --- | --- | --- |
| `build.sh` | host | Build libuio and create `libuio-<version>.tar.gz` (`make distcheck`) |
| `version.sh` | host | Print `<version>` from `AC_INIT` in `configure.ac`, used to find the tarball |
| `vm-test.sh` | host | Create and start the VM, wait for it, write `vm-test.log` and `report.xml` |
| `vm-cleanup.sh` | host | Remove the VM and its temporary files; run it also after failures |
| `log2junit.py` | host | Convert the VM log to a JUnit report (and a GitHub job summary) |
| `debian-package.sh` | host | Build and lintian-check Debian packages |
| `libvirt-amd64.xml`, `libvirt-arm64.xml`, `libvirt-ppc64.xml` | host | libvirt domain definitions of the VM per architecture |
| `ppc64-image.sh` | host | Build the ppc64 cloud image, see [ppc64 big-endian image](#ppc64-big-endian-image) |
| `ppc64-image/user-data`, `ppc64-image/build.sh` | builder VM | cloud-init configuration and script of the VM building the ppc64 image |
| `user-data`, `meta-data` | VM | cloud-init configuration: installs packages and starts `run-tests.sh` |
| `run-tests.sh` | VM | Run all `tests/*.sh` and print the results |
| `tests/*.sh` | VM | The individual tests |

The VM gets the source tarball (as `libuio.tar.gz`, independent of the
version), `run-tests.sh` and `tests/` on the cloud-init seed ISO. The tests
write their output to the virtio-serial port
`/dev/virtio-ports/org.libuio.ci.log`, which is written to
`/dev/shm/ci-libuio-<arch>-test.log` on the host. `vm-test.sh` turns it into
the report after the VM powered off.

The arm64 domain is similar to the amd64 one with the necessary changes:
machine `virt` instead of q35, an emulated Cortex-A57 CPU, GICv3, the seed
ISO on virtio-scsi instead of SATA and a PL011 serial console. The `edu`
devices are attached the same way to the PCIe root bus.

The ppc64 domain tests on a 64-bit big-endian architecture. It uses the
emulated `pseries` machine with a POWER9 CPU and the SLOF firmware, the seed
ISO on virtio-scsi and the hypervisor console `hvc0`. pseries has a
conventional PCI host bridge, so all devices including the `edu` devices are
on bus `pci.0`.

## Variables

`vm-test.sh` is configured with environment variables. In GitLab they are
defined in `.gitlab-ci.yml` with defaults that work without any site-specific
setup; override them in the project's *Settings > CI/CD > Variables* or when
running a pipeline manually.

| Variable | Default | Description |
| --- | --- | --- |
| `VM_ARCH` | `amd64` | Architecture of the VM, `amd64`, `arm64` or `ppc64`. Set by the CI jobs for each architecture. |
| `VM_BASE_IMAGE_AMD64`, `VM_BASE_IMAGE_ARM64` | empty | Path to a [Debian 13 generic cloud image](https://cloud.debian.org/images/cloud/trixie/latest/) (`debian-13-generic-amd64.qcow2`, `debian-13-generic-arm64.qcow2`) for `VM_ARCH` on the runner host. It is only used read-only as backing file. If empty, the image (about 400 MB) is downloaded for every run. |
| `VM_BASE_IMAGE_PPC64` | empty | Path to the ppc64 image built with `ppc64-image.sh` on the runner host, used read-only as backing file. Required for `VM_ARCH=ppc64`. |
| `VM_NETWORK` | `user` | `user` for QEMU user-mode networking, which needs no host setup. `bridge` to attach the VM to the host bridge `VM_BRIDGE`. |
| `VM_BRIDGE` | empty | Host bridge for `VM_NETWORK=bridge`, see [Bridge networking](#bridge-networking). |
| `VM_RUNNER_TAG` | empty | GitLab only: runner tag for the VM test jobs. If empty, the jobs `qemu-amd64`, `qemu-arm64` and `qemu-ppc64` run untagged; otherwise the jobs `qemu-amd64-tagged`, `qemu-arm64-tagged` and `qemu-ppc64-tagged` run with this tag. |
| `VM_ARM64` | `manual` | GitLab only: `manual` adds the arm64 VM test as manual job, which is started with its play button in the pipeline and does not block the pipeline otherwise. `always` runs it automatically, `never` leaves it out. |
| `VM_PPC64` | `manual` | GitLab only: the same for the ppc64 VM test. It needs `VM_BASE_IMAGE_PPC64`. |

The VM needs network access to install its packages, which works with either
network mode.

`debian-package.sh` uses these variables:

| Variable | Default | Description |
| --- | --- | --- |
| `DEBIAN_PACKAGING_URL` | `https://salsa.debian.org/debian/libuio.git` | Git repository with the Debian packaging (`debian/` directory only) |
| `DEBIAN_PACKAGING_REF` | `debian/unstable` | Branch or tag of `DEBIAN_PACKAGING_URL` to use |
| `PACKAGE_RUNNER_TAG` | empty | GitLab only: runner tag for the package job. If empty, the job `debian-package` runs untagged; otherwise the job `debian-package-tagged` runs with this tag. |

## Debian package job

**These CI runs are meant to be regression tests and should not be used to
obtain Debian packages for installation.
Please install packages from official sources.**

The `debian-package` job takes the release tarball from the `build` job as
upstream source (e.g. `libuio_0.4.0.orig.tar.gz`), adds the `debian/`
directory from `DEBIAN_PACKAGING_URL`, and adds a changelog entry with version
`<version>-0+ci1` (e.g. `0.4.0-0+ci1`) by the author of the tested commit. If
the Debian changelog already has a newer version for this upstream release
(e.g. `0.4.0-1`), `+ci1` is appended to it instead (`0.4.0-1+ci1`), because
`dch` refuses to go back in version. It then builds unsigned source and binary
packages with `dpkg-buildpackage -us -uc` and checks them with lintian.

The job fails if

- a build dependency is missing on the runner,
- a patch in `debian/patches` does not apply any more, e.g. because the
  change was merged upstream (refresh the packaging then),
- the package does not build, or files do not match the `*.install` files,
- lintian reports an error; warnings are only shown in the log.

The `lintian.log` is kept as job artifact in `debian-packages/`.

## GitLab runner setup

The `build` job runs on any runner with a C toolchain and the build
dependencies:

    autoconf automake autopoint gettext libtool pkg-config libglib2.0-dev texinfo
    texlive

To select such a runner, set `BUILD_RUNNER_TAG` to its tag (GitLab only). If
it is empty, the job `build` runs untagged; otherwise the job `build-tagged`
runs with this tag.

The `debian-package` job runs on any Debian or Ubuntu based runner with the
packaging tools, git and the `Build-Depends` of the Debian packaging
installed, and network access to `DEBIAN_PACKAGING_URL`. Nothing is
installed by the job. The runner's distribution must be recent enough for
the packaging, e.g. the current packaging needs `pkgconf`, not `pkg-config`,
and `debhelper-compat (= 14)`, i.e. debhelper 14.0 or newer. Debian 13
(trixie) has debhelper 14 in `trixie-backports`; Debian testing and
unstable and Ubuntu 26.10 have it in the release. Ubuntu 24.04 and 26.04 LTS
are too old.
Use `PACKAGE_RUNNER_TAG` to select a suitable runner. For the current packaging
on Debian 13:

    echo 'deb http://deb.debian.org/debian trixie-backports main' \
        > /etc/apt/sources.list.d/backports.list
    apt update
    apt install dpkg-dev devscripts lintian fakeroot git ca-certificates \
        pkgconf libglib2.0-dev texinfo
    apt install -t trixie-backports debhelper

The lintian of Debian 13 may warn about a `Standards-Version` newer than it
knows; this does not fail the job. Install lintian from `trixie-backports`
as well to avoid it.

The VM test jobs need a **shell executor** runner on a Linux x86-64 host with:

- KVM, with `/dev/kvm` accessible by the runner user (usually by adding it to
  the `kvm` group)
- libvirt with the QEMU driver; the job uses the unprivileged
  `qemu:///session` connection of the runner user, so no libvirt permissions
  are needed
- QEMU for x86-64, aarch64 and ppc64, UEFI firmware for x86-64 and aarch64
  (OVMF, AAVMF), qemu-img, xorriso, curl and python3
- for the ppc64 test, an image built with `ppc64-image.sh` (see
  [ppc64 big-endian image](#ppc64-big-endian-image)) and `VM_BASE_IMAGE_PPC64`
  set to it

On Debian or Ubuntu:

    apt install libvirt-daemon-system libvirt-clients qemu-system-x86 \
        qemu-system-arm qemu-system-ppc qemu-utils ovmf qemu-efi-aarch64 \
        xorriso curl python3
    adduser gitlab-runner kvm

Each VM uses about 1 GB of RAM. The disk overlay, seed ISO, log and (if
downloaded) the base image are kept in `/dev/shm`, so the host needs some
free space there per VM: a few hundred MB for the overlay, plus about 400 MB
if the base image is downloaded.
The arm64 and ppc64 VMs are emulated and thus much slower than the amd64 one.

Only **one VM test job per architecture may run at a time per host**,
as the VM name (`ci-libuio-<arch>`) and the files in `/dev/shm` are fixed
per architecture. Within the project, GitLab ensures this with the resource
groups `qemu-amd64`, `qemu-arm64` and `qemu-ppc64`: jobs of different
architectures run in parallel, while jobs of the same architecture, e.g. from
pushes to several branches, wait for each other. To run all architectures at
the same time, the runner needs at least `limit = 3` in its `config.toml`
section. If the
runner user also runs VM test jobs of other projects (e.g. forks), which the
resource groups do not cover, use `limit = 1` or a runner with
`concurrent = 1` instead.

To make sure the VM test only runs on such a runner, give the runner a tag
and set `VM_RUNNER_TAG` to it. Otherwise any runner accepting untagged jobs
may pick up the job.

### Bridge networking

User-mode networking is enough for the tests. To attach the VM to a host
bridge instead (e.g. to reach it from the network while debugging), set
`VM_NETWORK=bridge` and `VM_BRIDGE` to the bridge name. Since the VM runs
unprivileged, QEMU attaches it via `qemu-bridge-helper`, which must be
allowed to use the bridge in `/etc/qemu/bridge.conf`:

    allow br0

### Example

A site with a local copy of the image, a bridge `br0` and a runner tagged
`kvm` sets these project variables:

    VM_RUNNER_TAG       = kvm
    VM_BASE_IMAGE_AMD64 = /srv/images/debian-13-generic-amd64.qcow2
    VM_BASE_IMAGE_ARM64 = /srv/images/debian-13-generic-arm64.qcow2
    VM_BASE_IMAGE_PPC64 = /srv/images/debian-sid-ppc64.qcow2
    VM_NETWORK          = bridge
    VM_BRIDGE           = br0

## GitHub Actions

The workflow runs on GitHub-hosted `ubuntu-24.04` runners. It enables KVM for
the runner user, installs the packages listed above and runs the scripts with
the default variables, i.e. it downloads the image and uses user-mode
networking, and the Debian packaging is taken from salsa. It has the same
three stages as jobs: `debian-package` runs after `build` and the `qemu` jobs
passed. As the debhelper of Ubuntu 24.04 and 26.04 is too old for the Debian
packaging, `debian-package` runs in a `debian:unstable` container.
On push and pull request, the `qemu` job only runs for amd64. To also run it
for arm64 or ppc64, start the workflow with *Run workflow* in the *Actions*
tab and select the arm64 or ppc64 checkbox. `debian-package` then also waits
for these.
As there is no place to keep the ppc64 image, the ppc64 job builds it with
`ppc64-image.sh` before the test, which makes the job take longer and fail
whenever Debian ports unstable is not installable. Its `ppc64-image.log` is
kept with the test results.
No setup is needed.

## Running locally

On a host prepared like a GitLab runner, from the top of the source tree:

    ci/build.sh
    ci/vm-test.sh
    ci/vm-cleanup.sh
    ci/debian-package.sh

Set the variables as needed, e.g.
`VM_BASE_IMAGE_AMD64=/path/to/debian-13-generic-amd64.qcow2 ci/vm-test.sh`.
For arm64, set `VM_ARCH=arm64` for both `vm-test.sh` and `vm-cleanup.sh`.

## ppc64 big-endian image

Debian has no ppc64 (64-bit big-endian PowerPC) release and no cloud image
for it, only the unofficial [Debian ports](https://www.ports.debian.org/)
unstable. `ppc64-image.sh` builds a cloud image from it, using the same
methodology as the tests: it boots a Debian 13 amd64 cloud image as builder
VM with KVM, derived from `libvirt-amd64.xml`, with an additional empty raw
disk. cloud-init in the builder VM installs `mmdebstrap` and
`qemu-user-binfmt` and runs `ppc64-image/build.sh`, which

- partitions the disk with an MBR: a PReP boot partition and the ext4 root
  file system,
- installs Debian ports unstable with the kernel, cloud-init, systemd-networkd
  and some basics with `mmdebstrap --arch=ppc64` (ppc64 programs are run by
  qemu-user),
- writes a GRUB core image (`grub-mkimage`) to the PReP partition, from where
  SLOF boots it. GRUB loads `/boot/grub/grub.cfg`, which boots
  `/boot/vmlinux` and `/boot/initrd.img`.

The builder log is written to `ppc64-image.log`, and the disk is converted to
the compressed qcow2 given as argument. The host needs nothing beyond the VM
test requirements (in particular no root access), and the build takes about
10 minutes. As with `vm-test.sh`, `VM_BASE_IMAGE_AMD64` avoids downloading
the builder image:

    VM_BASE_IMAGE_AMD64=/srv/images/debian-13-generic-amd64.qcow2 \
        ci/ppc64-image.sh /srv/images/debian-sid-ppc64.qcow2

Debian ports unstable changes all the time and is not always installable, so
the image is not built for every test run: build it once, keep it on the
runner host and set `VM_BASE_IMAGE_PPC64` to it. Rebuild it when needed, e.g.
when installing the test dependencies with cloud-init fails because the
image got too old. Only one image build may run at a time per host.

Then run the test like for the other architectures, which takes about 10
minutes, as ppc64 is emulated without KVM:

    VM_ARCH=ppc64 VM_BASE_IMAGE_PPC64=/srv/images/debian-sid-ppc64.qcow2 \
        ci/vm-test.sh
    VM_ARCH=ppc64 ci/vm-cleanup.sh

## Adding tests

Add a script `tests/NN-name.sh`; tests run in the order of `NN` and are
reported as `test_name`. Each test

- starts in the home directory of root in the VM,
- is run with `sh -ex`, so every command is shown in the log and the first
  failing command fails the test,
- finds the source tarball at `$SRC_TARBALL`,
- can depend on other tests with a line `# requires: test_a test_b`; it is
  then skipped if any of those did not pass.
