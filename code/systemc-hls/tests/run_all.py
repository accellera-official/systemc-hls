#!/usr/bin/env python3
## ****************************************************************************
##
##  Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
##  more contributor license agreements.  See the NOTICE file distributed
##  with this work for additional information regarding copyright ownership.
##  Accellera licenses this file to you under the Apache License, Version 2.0
##  (the "License"); you may not use this file except in compliance with the
##  License.  You may obtain a copy of the License at
##
##   http://www.apache.org/licenses/LICENSE-2.0
##
##  Unless required by applicable law or agreed to in writing, software
##  distributed under the License is distributed on an "AS IS" BASIS,
##  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
##  implied.  See the License for the specific language governing
##  permissions and limitations under the License.
##
## ****************************************************************************

"""
Parallel Test Runner for SystemC-HLS Unit Tests

This script runs all unit tests in parallel and reports results.
It uses all available CPU cores to maximize throughput.

Each test directory contains a Makefile with a 'test' target that:
  - Builds all test binaries
  - Runs each binary and checks exit codes
  - Prints "Results: X passed, Y failed"

Usage:
    ./run_all.py [options]

Options:
    --jobs N        Number of parallel jobs (default: all CPUs)
    --verbose       Show output for all tests, not just failures
    --help          Show this help message
"""

import os
import sys
import re
import subprocess
import concurrent.futures
from pathlib import Path
from datetime import datetime
import multiprocessing
import time
import argparse
import platform


class Colors:
    """ANSI color codes for terminal output"""
    GREEN  = '\033[92m'
    RED    = '\033[91m'
    YELLOW = '\033[93m'
    BLUE   = '\033[94m'
    BOLD   = '\033[1m'
    RESET  = '\033[0m'


def discover_tests(base_dir):
    """
    Find all test directories matching pattern [0-9][0-9]_*

    Args:
        base_dir: Base directory to search in

    Returns:
        List of (name, path) tuples sorted by name
    """
    tests = []
    base_path = Path(base_dir)

    for item in sorted(base_path.iterdir()):
        if item.is_dir() and len(item.name) >= 3:
            # Check if name starts with two digits and underscore
            if item.name[0].isdigit() and item.name[1].isdigit() and item.name[2] == '_':
                # Only include if a Makefile exists
                if (item / 'Makefile').exists():
                    tests.append((item.name, str(item)))

    return tests


def parse_sub_test_results(output):
    """
    Parse individual sub-test PASS/FAIL lines from Makefile test output.

    Args:
        output: Combined stdout+stderr from 'make test'

    Returns:
        List of (test_name, passed) tuples
    """
    sub_tests = []
    for line in output.splitlines():
        line = line.strip()
        if line.startswith('PASS:'):
            name = line[5:].strip()
            sub_tests.append((name, True))
        elif line.startswith('FAIL:'):
            name = line[5:].strip()
            sub_tests.append((name, False))
    return sub_tests


def parse_summary(output):
    """
    Parse the 'Results: X passed, Y failed' summary line.

    Args:
        output: Combined stdout+stderr from 'make test'

    Returns:
        (passed, failed) tuple, or (None, None) if not found
    """
    m = re.search(r'Results:\s+(\d+)\s+passed,\s+(\d+)\s+failed', output)
    if m:
        return int(m.group(1)), int(m.group(2))
    return None, None


def run_test(args):
    """
    Build and run a single test suite via 'make clean && make test'.

    Args:
        args: Tuple of (name, path)

    Returns:
        Dictionary with test results
    """
    name, path = args
    start_time = time.time()

    result = {
        'name':      name,
        'status':    'UNKNOWN',
        'duration':  0.0,
        'output':    '',
        'error':     '',
        'sub_tests': [],   # List of (sub_test_name, passed)
        'passed':    0,
        'failed':    0,
    }

    original_dir = os.getcwd()
    os.chdir(path)
    try:
        cmd = 'make clean && make test'

        process = subprocess.run(
            cmd,
            shell=True,
            capture_output=True,
            text=True,
            timeout=120  # 2-minute timeout per test suite
        )

        combined = process.stdout + process.stderr
        result['output'] = combined
        result['error']  = process.stderr

        # Parse sub-test results
        result['sub_tests'] = parse_sub_test_results(combined)

        # Parse summary line
        passed, failed = parse_summary(combined)

        if passed is not None:
            result['passed'] = passed
            result['failed'] = failed
            result['status'] = 'PASS' if failed == 0 else 'FAIL'
        else:
            # Fall back to exit code if no summary line found
            result['status'] = 'PASS' if process.returncode == 0 else 'FAIL'
            result['passed'] = 1 if process.returncode == 0 else 0
            result['failed'] = 0 if process.returncode == 0 else 1

    except subprocess.TimeoutExpired:
        result['status'] = 'TIMEOUT'
        result['error']  = 'Test exceeded 120 second timeout'
    except Exception as e:
        result['status'] = 'ERROR'
        result['error']  = str(e)
    finally:
        os.chdir(original_dir)
        result['duration'] = time.time() - start_time

    return result


def format_table(results):
    """
    Format results as ASCII table.

    Args:
        results: List of result dictionaries

    Returns:
        Formatted table string
    """
    name_width = max(len(r['name']) for r in results)
    name_width = max(name_width, len('Test Suite'))

    lines = []
    lines.append('╔' + '═' * (name_width + 2) + '╦' + '═' * 10 + '╦' + '═' * 14 + '╦' + '═' * 12 + '╗')
    lines.append('║ ' + 'Test Suite'.ljust(name_width) + ' ║ Status   ║ Sub-Tests    ║ Duration   ║')
    lines.append('╠' + '═' * (name_width + 2) + '╬' + '═' * 10 + '╬' + '═' * 14 + '╬' + '═' * 12 + '╣')

    for r in results:
        raw_status = r['status']
        if raw_status == 'PASS':
            status_colored = f"{Colors.GREEN}{raw_status}{Colors.RESET}"
        elif raw_status == 'FAIL':
            status_colored = f"{Colors.RED}{raw_status}{Colors.RESET}"
        elif raw_status == 'TIMEOUT':
            status_colored = f"{Colors.YELLOW}{raw_status}{Colors.RESET}"
        else:
            status_colored = raw_status

        total = r['passed'] + r['failed']
        sub_str = f"{r['passed']}/{total} passed" if total > 0 else "n/a"
        duration_str = f"{r['duration']:.2f}s"

        # Pad status field accounting for ANSI escape codes
        status_pad = 8 + len(status_colored) - len(raw_status)

        lines.append(
            f"║ {r['name'].ljust(name_width)} ║ "
            f"{status_colored.ljust(status_pad)} ║ "
            f"{sub_str.ljust(12)} ║ "
            f"{duration_str.rjust(10)} ║"
        )

    lines.append('╚' + '═' * (name_width + 2) + '╩' + '═' * 10 + '╩' + '═' * 14 + '╩' + '═' * 12 + '╝')

    return '\n'.join(lines)


def write_results_file(results, filename, start_time, end_time, num_cpus):
    """
    Write detailed results to file.
    """
    with open(filename, 'w') as f:
        f.write("=" * 80 + "\n")
        f.write("SystemC-HLS Unit Tests Results\n")
        f.write("=" * 80 + "\n\n")

        f.write(f"Timestamp: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
        f.write(f"Hostname:  {platform.node()}\n")
        f.write(f"CPUs Used: {num_cpus}\n")
        f.write(f"Total Duration: {end_time - start_time:.2f}s\n\n")

        total_passed = sum(r['passed'] for r in results)
        total_failed = sum(r['failed'] for r in results)
        suite_passed = sum(1 for r in results if r['status'] == 'PASS')
        suite_failed = sum(1 for r in results if r['status'] != 'PASS')

        f.write("Summary:\n")
        f.write(f"  Test suites: {suite_passed}/{len(results)} passed\n")
        f.write(f"  Sub-tests:   {total_passed}/{total_passed + total_failed} passed\n\n")

        f.write("=" * 80 + "\n")
        f.write("Detailed Results\n")
        f.write("=" * 80 + "\n\n")

        for r in results:
            f.write(f"Suite:    {r['name']}\n")
            f.write(f"Status:   {r['status']}\n")
            f.write(f"Duration: {r['duration']:.2f}s\n")

            if r['sub_tests']:
                f.write("Sub-tests:\n")
                for sub_name, sub_pass in r['sub_tests']:
                    f.write(f"  {'PASS' if sub_pass else 'FAIL'}: {sub_name}\n")

            if r['status'] != 'PASS':
                f.write("\n--- Output ---\n")
                f.write(r['output'])

            f.write("\n" + "-" * 80 + "\n\n")


def main():
    parser = argparse.ArgumentParser(
        description='Run all unit tests in parallel',
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument('--jobs', '-j', type=int, default=None,
                        help='Number of parallel jobs (default: all CPUs)')
    parser.add_argument('--verbose', '-v', action='store_true',
                        help='Show sub-test details for all suites, not just failures')

    args = parser.parse_args()

    num_cpus = args.jobs if args.jobs else multiprocessing.cpu_count()

    script_dir = Path(__file__).parent

    print(f"{Colors.BOLD}Unit Test Runner{Colors.RESET}")
    print(f"Using {num_cpus} parallel jobs\n")

    tests = discover_tests(script_dir)

    if not tests:
        print(f"{Colors.RED}No test suites found!{Colors.RESET}")
        return 1

    print(f"Found {len(tests)} test suites\n")

    test_args = [(name, path) for name, path in tests]

    start_time = time.time()
    print("Running tests...")

    with concurrent.futures.ProcessPoolExecutor(max_workers=num_cpus) as executor:
        results = list(executor.map(run_test, test_args))

    end_time = time.time()

    results.sort(key=lambda r: r['name'])

    print("\n" + format_table(results))

    # Totals
    total_passed = sum(r['passed'] for r in results)
    total_failed = sum(r['failed'] for r in results)
    suite_failed = sum(1 for r in results if r['status'] != 'PASS')

    print(f"\n{Colors.BOLD}Summary:{Colors.RESET} ", end='')
    if suite_failed == 0:
        print(f"{Colors.GREEN}{len(results)}/{len(results)} suites PASSED "
              f"({total_passed} sub-tests){Colors.RESET}")
    else:
        print(f"{Colors.RED}{suite_failed}/{len(results)} suites FAILED "
              f"({total_failed} sub-tests failed){Colors.RESET}")

    print(f"Total time: {end_time - start_time:.2f}s\n")

    # Write results file
    results_file = script_dir / 'run_results.txt'
    write_results_file(results, results_file, start_time, end_time, num_cpus)
    print(f"Detailed results written to: {results_file}")

    # Show details for failures or in verbose mode
    if args.verbose or suite_failed > 0:
        for r in results:
            if args.verbose or r['status'] != 'PASS':
                header_color = Colors.GREEN if r['status'] == 'PASS' else Colors.RED
                print(f"\n{header_color}{Colors.BOLD}[{r['name']}]{Colors.RESET}")
                if r['sub_tests']:
                    for sub_name, sub_pass in r['sub_tests']:
                        icon = f"{Colors.GREEN}✓{Colors.RESET}" if sub_pass else f"{Colors.RED}✗{Colors.RESET}"
                        print(f"  {icon} {sub_name}")
                if r['status'] != 'PASS':
                    tail = r['output'][-800:] if len(r['output']) > 800 else r['output']
                    print(tail)

    return 1 if suite_failed > 0 else 0


if __name__ == '__main__':
    sys.exit(main())
