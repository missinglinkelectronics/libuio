# libuio CI

The CI builds libuio, then boots Debian 13 VMs with two QEMU `edu` PCI devices
and runs integration tests in it against real UIO devices.
The VM test run for amd64 (with KVM) and arm64 (emulated by QEMU without KVM,
as the current CI hosts are x86-64). As the emulated arm64 test is slow, it
only runs on request by default.
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
| `build.sh` | host | Build libuio and create `libuio-0.4.0.tar.gz` (`make distcheck`) |
| `vm-test.sh` | host | Create and start the VM, wait for it, write `vm-test.log` and `report.xml` |
| `vm-cleanup.sh` | host | Remove the VM and its temporary files; run it also after failures |
| `log2junit.py` | host | Convert the VM log to a JUnit report (and a GitHub job summary) |
| `debian-package.sh` | host | Build and lintian-check Debian packages |
| `libvirt-amd64.xml`, `libvirt-arm64.xml` | host | libvirt domain definitions of the VM per architecture |
| `user-data`, `meta-data` | VM | cloud-init configuration: installs packages and starts `run-tests.sh` |
| `run-tests.sh` | VM | Run all `tests/*.sh` and print the results |
| `tests/*.sh` | VM | The individual tests |

The VM gets the source tarball, `run-tests.sh` and `tests/` on the cloud-init
seed ISO. The tests write their output to the virtio-serial port
`/dev/virtio-ports/org.libuio.ci.log`, which is written to
`/dev/shm/ci-libuio-<arch>-test.log` on the host. `vm-test.sh` turns it into
the report after the VM powered off.

The arm64 domain is similar to the amd64 one with the necessary changes:
machine `virt` instead of q35, an emulated Cortex-A57 CPU, GICv3, the seed
ISO on virtio-scsi instead of SATA and a PL011 serial console. The `edu`
devices are attached the same way to the PCIe root bus.

## Variables

`vm-test.sh` is configured with environment variables. In GitLab they are
defined in `.gitlab-ci.yml` with defaults that work without any site-specific
setup; override them in the project's *Settings > CI/CD > Variables* or when
running a pipeline manually.

| Variable | Default | Description |
| --- | --- | --- |
| `VM_ARCH` | `amd64` | Architecture of the VM, `amd64` or `arm64`. Set by the CI jobs for each architecture. |
| `VM_BASE_IMAGE_AMD64`, `VM_BASE_IMAGE_ARM64` | empty | Path to a [Debian 13 generic cloud image](https://cloud.debian.org/images/cloud/trixie/latest/) (`debian-13-generic-amd64.qcow2`, `debian-13-generic-arm64.qcow2`) for `VM_ARCH` on the runner host. It is only used read-only as backing file. If empty, the image (about 400 MB) is downloaded for every run. |
| `VM_NETWORK` | `user` | `user` for QEMU user-mode networking, which needs no host setup. `bridge` to attach the VM to the host bridge `VM_BRIDGE`. |
| `VM_BRIDGE` | empty | Host bridge for `VM_NETWORK=bridge`, see [Bridge networking](#bridge-networking). |
| `VM_RUNNER_TAG` | empty | GitLab only: runner tag for the VM test jobs. If empty, the jobs `qemu-amd64` and `qemu-arm64` run untagged; otherwise the jobs `qemu-amd64-tagged` and `qemu-arm64-tagged` run with this tag. |
| `VM_ARM64` | `manual` | GitLab only: `manual` adds the arm64 VM test as manual job, which is started with its play button in the pipeline and does not block the pipeline otherwise. `always` runs it automatically, `never` leaves it out. |

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
upstream source (`libuio_0.4.0.orig.tar.gz`), adds the `debian/` directory
from `DEBIAN_PACKAGING_URL`, and adds a changelog entry with version
`0.4.0-0+ci1` by the author of the tested commit. It then builds unsigned source
and binary packages with `dpkg-buildpackage -us -uc` and checks them with
lintian.

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
the packaging, e.g. the current packaging needs `pkgconf`, not `pkg-config`.
Use `PACKAGE_RUNNER_TAG` to select a suitable runner. For the current packaging:

    apt install dpkg-dev devscripts lintian fakeroot git \
        debhelper pkgconf libglib2.0-dev texinfo

The VM test jobs need a **shell executor** runner on a Linux x86-64 host with:

- KVM, with `/dev/kvm` accessible by the runner user (usually by adding it to
  the `kvm` group)
- libvirt with the QEMU driver; the job uses the unprivileged
  `qemu:///session` connection of the runner user, so no libvirt permissions
  are needed
- QEMU for x86-64 and aarch64, UEFI firmware for both (OVMF, AAVMF),
  qemu-img, xorriso, curl and python3

On Debian or Ubuntu:

    apt install libvirt-daemon-system libvirt-clients qemu-system-x86 \
        qemu-system-arm qemu-utils ovmf qemu-efi-aarch64 xorriso curl python3
    adduser gitlab-runner kvm

Each VM uses about 1 GB of RAM. The disk overlay, seed ISO, log and (if
downloaded) the base image are kept in `/dev/shm`, so the host needs some
free space there per VM: a few hundred MB for the overlay, plus about 400 MB
if the base image is downloaded.
The arm64 VM is emulated and thus much slower than the amd64 one.

Only **one VM test job per architecture may run at a time per host**,
as the VM name (`ci-libuio-<arch>`) and the files in `/dev/shm` are fixed
per architecture. Within the project, GitLab ensures this with the resource
groups `qemu-amd64` and `qemu-arm64`: the amd64 and arm64 jobs run in
parallel, while jobs of the same architecture, e.g. from pushes to several
branches, wait for each other. To run both architectures at the same time,
the runner needs at least `limit = 2` in its `config.toml` section. If the
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
    VM_NETWORK          = bridge
    VM_BRIDGE           = br0

## GitHub Actions

The workflow runs on GitHub-hosted `ubuntu-24.04` runners. It enables KVM for
the runner user, installs the packages listed above and runs the scripts with
the default variables, i.e. it downloads the image and uses user-mode
networking, and the Debian packaging is taken from salsa. It has the same
three stages as jobs: `debian-package` runs after `build` and the `qemu` jobs
passed.
On push and pull request, the `qemu` job only runs for amd64. To also run it
for arm64, start the workflow with *Run workflow* in the *Actions* tab and
select the arm64 checkbox. `debian-package` then also waits for arm64.
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

## Adding tests

Add a script `tests/NN-name.sh`; tests run in the order of `NN` and are
reported as `test_name`. Each test

- starts in the home directory of root in the VM,
- is run with `sh -ex`, so every command is shown in the log and the first
  failing command fails the test,
- finds the source tarball at `$SRC_TARBALL`,
- can depend on other tests with a line `# requires: test_a test_b`; it is
  then skipped if any of those did not pass.
