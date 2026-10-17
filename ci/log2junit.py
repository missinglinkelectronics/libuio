#!/usr/bin/env python3

"""Convert the VM test log written by ci/run-tests.sh to JUnit XML.

usage: log2junit.py <log> <report.xml> [<testdir>]

If <testdir> is given, every test script in it that did not report a result
(e.g. because the VM hung or crashed) is reported as failed.

When run in GitHub Actions, a result table is also added to the job summary.

Exits non-zero if any test failed.
"""

import os
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

TEST_RE = re.compile(r"TEST:\s+(\S+)\s+([A-Za-z_]+)(?:\s+(\d+)s)?")
BEGIN_RE = re.compile(r"^=== BEGIN (\S+) ===$")
END_RE = re.compile(r"^=== END (\S+) ===$")


def expected_tests(testdir):
    return [
        "test_" + re.sub(r"^[0-9]*-", "", p.stem)
        for p in sorted(Path(testdir).glob("*.sh"))
    ]


def main():
    if len(sys.argv) not in (3, 4):
        sys.exit(__doc__)
    log, report = sys.argv[1], sys.argv[2]

    results = {}  # name -> (status, seconds)
    output = {}  # name -> captured lines
    current = None

    with open(log, "r", errors="replace") as f:
        for line in f:
            stripped = line.rstrip("\r\n")
            m = BEGIN_RE.match(stripped)
            if m:
                current = m.group(1)
                output[current] = []
                continue
            if END_RE.match(stripped):
                current = None
                continue
            if current is not None:
                output[current].append(stripped)
                continue
            m = TEST_RE.search(stripped)
            if m:
                name, status, seconds = m.groups()
                results[name] = (status, seconds)

    names = list(results)
    if len(sys.argv) == 4:
        names += [n for n in expected_tests(sys.argv[3]) if n not in results]

    testsuite = ET.Element("testsuite", name="vm_integration_tests")
    has_fail = False
    summary = ["| Test | Result | Time |", "| --- | --- | --- |"]

    for name in names:
        tc = ET.SubElement(testsuite, "testcase", name=name)
        status, seconds = results.get(name, (None, None))
        summary.append(f"| {name} | {status or 'NO RESULT'} | {seconds + 's' if seconds else '-'} |")
        if seconds is not None:
            tc.set("time", seconds)
        if status is None:
            ET.SubElement(tc, "failure", message="Test did not report a result")
            has_fail = True
        elif status == "SKIPPED":
            ET.SubElement(tc, "skipped", message="Dependency did not pass")
        elif status != "PASSED":
            ET.SubElement(tc, "failure", message=f"Test failed with status {status}")
            has_fail = True
        if name in output:
            ET.SubElement(tc, "system-out").text = "\n".join(output[name])

    ET.ElementTree(testsuite).write(report, encoding="utf-8", xml_declaration=True)

    if "GITHUB_STEP_SUMMARY" in os.environ:
        with open(os.environ["GITHUB_STEP_SUMMARY"], "a") as f:
            f.write("## VM integration tests\n\n" + "\n".join(summary) + "\n")

    if has_fail:
        sys.exit(1)


if __name__ == "__main__":
    main()
