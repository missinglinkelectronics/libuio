#!/bin/sh
# Build Debian packages of the current source with the packaging from Debian,
# to find changes that break the Debian package early.
#
# Expects the release tarball (see ci/build.sh) in the current directory.
# Writes the lintian.log to debian-packages/.
#
# The build dependencies of the package, dpkg-dev, devscripts, lintian and git
# must be installed; nothing is installed by this script.
#
# Environment:
#   DEBIAN_PACKAGING_URL  Git repository with the debian/ directory
#   DEBIAN_PACKAGING_REF  Branch or tag to use from it

set -ex

: "${DEBIAN_PACKAGING_URL:=https://salsa.debian.org/debian/libuio.git}"
: "${DEBIAN_PACKAGING_REF:=debian/unstable}"

NAME=libuio
VERSION=$(ci/version.sh)
TARBALL=$NAME-$VERSION.tar.gz
OUT=debian-packages
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

rm -rf "$OUT"
mkdir -p "$OUT"

# Use the release tarball as upstream source
cp "$TARBALL" "$WORK/${NAME}_$VERSION.orig.tar.gz"
tar -C "$WORK" -xf "$TARBALL"
SRC=$WORK/$NAME-$VERSION

# Add the Debian packaging
git clone --depth 1 --branch "$DEBIAN_PACKAGING_REF" \
	"$DEBIAN_PACKAGING_URL" "$WORK/packaging"
cp -r "$WORK/packaging/debian" "$SRC/"

# Mark the package as CI build of this commit, by its author. The version must
# be newer than the latest one in debian/changelog: if Debian already packages
# this upstream version (e.g. 0.4.0-1), append +ci1 to the Debian revision.
DESC=$(git describe --tags --always)
export DEBFULLNAME="$(git log -1 --format=%an)"
export DEBEMAIL="$(git log -1 --format=%ae)"
CURRENT=$(cd "$SRC" && dpkg-parsechangelog -S Version)
if dpkg --compare-versions "$CURRENT" lt "$VERSION-0+ci1"; then
	CI_VERSION=$VERSION-0+ci1
else
	CI_VERSION=$CURRENT+ci1
fi
(cd "$SRC" && dch --newversion "$CI_VERSION" --distribution UNRELEASED \
	"CI build of upstream $DESC")

# Build source and binary packages without signing. This fails if build
# dependencies are missing or the Debian patches do not apply any more.
(cd "$SRC" && dpkg-buildpackage -us -uc)

# Check the result; lintian fails on errors only
CHANGES=$(ls "$WORK"/${NAME}_*.changes)
set +e
lintian --info --display-info --fail-on error "$CHANGES" \
	> "$OUT/lintian.log" 2>&1
lintian_status=$?
set -e
cat "$OUT/lintian.log"

# Keep the packages
#dcmd cp "$CHANGES" "$OUT/"
#ls -l "$OUT"

exit $lintian_status
