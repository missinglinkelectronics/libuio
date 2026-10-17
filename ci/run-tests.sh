#!/bin/sh
# Run all test scripts in the given directory in lexical order.
#
# Each test script signals its result via its exit code and is executed with
# "sh -ex", so every command it runs shows up in the log. A test may declare
# a dependency on other tests with a "# requires: test_a test_b" line; it is
# skipped if any of those did not pass.
#
# For each test the runner prints:
#   === BEGIN <name> ===
#   ... output of the test ...
#   === END <name> ===
#   TEST: <name> PASSED|FAILED|SKIPPED <seconds>s

testdir=${1:?usage: $0 <testdir>}

# Location of the source tarball, used by the build tests
export SRC_TARBALL="${SRC_TARBALL:-/media/libuio-0.4.0.tar.gz}"

not_passed=""

for t in "$testdir"/*.sh; do
	name=test_$(basename "$t" .sh | sed 's/^[0-9]*-//')
	status=""
	start=$(date +%s)
	echo "=== BEGIN $name ==="

	for dep in $(sed -n 's/^# requires: *//p' "$t"); do
		case " $not_passed " in
		*" $dep "*)
			echo "$name: skipped because $dep did not pass"
			status=SKIPPED
			;;
		esac
	done

	if [ -z "$status" ]; then
		if (cd && sh -ex "$t"); then
			status=PASSED
		else
			status=FAILED
		fi
	fi
	end=$(date +%s)
	echo "=== END $name ==="

	[ "$status" = PASSED ] || not_passed="$not_passed $name"
	echo "TEST: $name $status $((end - start))s"
done
