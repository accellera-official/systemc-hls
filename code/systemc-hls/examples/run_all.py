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
Parallel Test Runner for the examples

This script runs all examples in parallel and reports results.
It uses all available CPU cores to maximize throughput.

Usage:
    ./run_all.py [options]

Options:
    --jobs N        Number of parallel jobs (default: all CPUs)
    --verbose       Show output for all tests, not just failures
    --help          Show this help message
"""

import os
import sys
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
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    BOLD = '\033[1m'
    RESET = '\033[0m'


def discover_examples(base_dir):
    """
    Find all example directories matching pattern [0-9][0-9]_*

    Args:
        base_dir: Base directory to search in

    Returns:
        List of (name, path) tuples sorted by name
    """
    examples = []
    base_path = Path(base_dir)

    for item in sorted(base_path.iterdir()):
        if item.is_dir() and len(item.name) >= 3:
            # Check if name starts with two digits and underscore
            if item.name[0].isdigit() and item.name[1].isdigit() and item.name[2] == '_':
                examples.append((item.name, str(item)))

    return examples


def run_example(args):
    """
    Build and run a single example

    Args:
        args: Tuple of (name, path)

    Returns:
        Dictionary with test results
    """
    name, path = args
    start_time = time.time()

    result = {
        'name': name,
        'status': 'UNKNOWN',
        'duration': 0.0,
        'output': '',
        'error': ''
    }

    # Change to example directory
    original_dir = os.getcwd()
    os.chdir(path)
    try:
        # Build command - always clean first to ensure fresh compilation
        cmd = 'make clean && make run'

        # Run the command
        process = subprocess.run(
            cmd,
            shell=True,
            capture_output=True,
            text=True,
            timeout=60*10  # 60*10 second timeout per test
        )

        result['output'] = process.stdout
        result['error'] = process.stderr

        # Check exit status
        if process.returncode == 0:
            result['status'] = 'PASS'
        else:
            result['status'] = 'FAIL'

    except subprocess.TimeoutExpired:
        result['status'] = 'TIMEOUT'
        result['error'] = 'Test exceeded 60 second timeout'
    except Exception as e:
        result['status'] = 'ERROR'
        result['error'] = str(e)
    finally:
        os.chdir(original_dir)
        result['duration'] = time.time() - start_time

    return result


def format_table(results):
    """
    Format results as ASCII table

    Args:
        results: List of result dictionaries

    Returns:
        Formatted table string
    """
    # Calculate column widths
    name_width = max(len(r['name']) for r in results)
    name_width = max(name_width, len('Example'))

    # Table header
    lines = []
    lines.append('╔' + '═' * (name_width + 2) + '╦' + '═' * 10 + '╦' + '═' * 12 + '╗')
    lines.append('║ ' + 'Example'.ljust(name_width) + ' ║ Status   ║ Duration   ║')
    lines.append('╠' + '═' * (name_width + 2) + '╬' + '═' * 10 + '╬' + '═' * 12 + '╣')

    # Table rows
    for r in results:
        status_colored = r['status']
        if r['status'] == 'PASS':
            status_colored = f"{Colors.GREEN}{r['status']}{Colors.RESET}"
        elif r['status'] == 'FAIL':
            status_colored = f"{Colors.RED}{r['status']}{Colors.RESET}"
        elif r['status'] == 'TIMEOUT':
            status_colored = f"{Colors.YELLOW}{r['status']}{Colors.RESET}"

        duration_str = f"{r['duration']:.2f}s"
        lines.append(f"║ {r['name'].ljust(name_width)} ║ {status_colored.ljust(8 + len(status_colored) - len(r['status']))} ║ {duration_str.rjust(10)} ║")

    # Table footer
    lines.append('╚' + '═' * (name_width + 2) + '╩' + '═' * 10 + '╩' + '═' * 12 + '╝')

    return '\n'.join(lines)


def write_results_file(results, filename, start_time, end_time, num_cpus):
    """
    Write detailed results to file

    Args:
        results: List of result dictionaries
        filename: Output filename
        start_time: Test suite start time
        end_time: Test suite end time
        num_cpus: Number of CPUs used
    """
    with open(filename, 'w') as f:
        # Header
        f.write("=" * 80 + "\n")
        f.write("SystemC-HLS Examples Test Results\n")
        f.write("=" * 80 + "\n\n")

        # Metadata
        f.write(f"Timestamp: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
        f.write(f"Hostname:  {platform.node()}\n")
        f.write(f"CPUs Used: {num_cpus}\n")
        f.write(f"Total Duration: {end_time - start_time:.2f}s\n\n")

        # Summary
        passed = sum(1 for r in results if r['status'] == 'PASS')
        failed = sum(1 for r in results if r['status'] == 'FAIL')
        timeout = sum(1 for r in results if r['status'] == 'TIMEOUT')
        error = sum(1 for r in results if r['status'] == 'ERROR')

        f.write("Summary:\n")
        f.write(f"  PASSED:  {passed}/{len(results)}\n")
        if failed > 0:
            f.write(f"  FAILED:  {failed}/{len(results)}\n")
        if timeout > 0:
            f.write(f"  TIMEOUT: {timeout}/{len(results)}\n")
        if error > 0:
            f.write(f"  ERROR:   {error}/{len(results)}\n")
        f.write("\n")

        # Detailed results
        f.write("=" * 80 + "\n")
        f.write("Detailed Results\n")
        f.write("=" * 80 + "\n\n")

        for r in results:
            f.write(f"Example: {r['name']}\n")
            f.write(f"Status:  {r['status']}\n")
            f.write(f"Duration: {r['duration']:.2f}s\n")

            # Show output for failed tests or if verbose
            if r['status'] != 'PASS':
                f.write("\n--- Output ---\n")
                f.write(r['output'])
                if r['error']:
                    f.write("\n--- Error ---\n")
                    f.write(r['error'])

            f.write("\n" + "-" * 80 + "\n\n")


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description='Run all SystemC-HLS examples in parallel',
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument('--jobs', '-j', type=int, default=None,
                        help='Number of parallel jobs (default: all CPUs)')
    parser.add_argument('--verbose', '-v', action='store_true',
                        help='Show output for all tests, not just failures')

    args = parser.parse_args()

    # Determine number of CPUs
    num_cpus = args.jobs if args.jobs else multiprocessing.cpu_count()

    # Get script directory
    script_dir = Path(__file__).parent

    print(f"{Colors.BOLD}SystemC-HLS Examples Test Runner{Colors.RESET}")
    print(f"Using {num_cpus} parallel jobs\n")

    # Discover examples
    examples = discover_examples(script_dir)

    if not examples:
        print(f"{Colors.RED}No examples found!{Colors.RESET}")
        return 1

    print(f"Found {len(examples)} examples\n")

    # Prepare arguments for parallel execution
    example_args = [(name, path) for name, path in examples]

    # Run tests in parallel
    start_time = time.time()

    print("Running tests...")
    with concurrent.futures.ProcessPoolExecutor(max_workers=num_cpus) as executor:
        results = list(executor.map(run_example, example_args))

    end_time = time.time()

    # Sort results by name
    results.sort(key=lambda r: r['name'])

    # Display table
    print("\n" + format_table(results))

    # Summary
    passed = sum(1 for r in results if r['status'] == 'PASS')
    failed = sum(1 for r in results if r['status'] == 'FAIL')

    print(f"\n{Colors.BOLD}Summary:{Colors.RESET} ", end='')
    if failed == 0:
        print(f"{Colors.GREEN}{passed}/{len(results)} PASSED{Colors.RESET}")
    else:
        print(f"{Colors.RED}{failed}/{len(results)} FAILED{Colors.RESET}")

    print(f"Total time: {end_time - start_time:.2f}s\n")

    # Write results file
    results_file = script_dir / 'run_results.txt'
    write_results_file(results, results_file, start_time, end_time, num_cpus)
    print(f"Detailed results written to: {results_file}")

    # Show failed test details if any
    if args.verbose or failed > 0:
        print(f"\n{Colors.BOLD}Failed Test Details:{Colors.RESET}")
        for r in results:
            if r['status'] != 'PASS':
                print(f"\n{Colors.RED}[{r['name']}]{Colors.RESET}")
                print(r['output'][-500:] if len(r['output']) > 500 else r['output'])

    # Return non-zero if any failed
    return 1 if failed > 0 else 0


if __name__ == '__main__':
    sys.exit(main())
