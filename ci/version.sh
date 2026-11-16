#!/bin/sh
# Print the libuio version from AC_INIT in configure.ac, so the CI scripts
# find the release tarball libuio-<version>.tar.gz without a fixed version.
set -e

version=$(sed -n 's/^AC_INIT([^,]*, *\[\{0,1\}\([^],]*\).*/\1/p' \
	"$(dirname "$0")/../configure.ac")
if [ -z "$version" ]; then
	echo "no version found in AC_INIT of configure.ac" >&2
	exit 1
fi
echo "$version"
