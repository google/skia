#!/usr/bin/env python3
# Copyright 2026 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import os
import shutil
import subprocess
import sys

OPTS_FILE = "src/opts/SkRasterPipeline_opts.h"
SKOPTS_FILE = "src/core/SkOpts.cpp"
BUILD_DIR = "out/Release"
BIN_TEMP_DIR = os.path.join(BUILD_DIR, "nanobenches")

def run_cmd(args, check=True):
    """Utility to run a shell command and print outputs on failure."""
    try:
        result = subprocess.run(args, capture_output=True, text=True, check=check)
        return result.stdout.strip()
    except subprocess.CalledProcessError as e:
        print(f"Command failed: {' '.join(args)}")
        print(f"Stdout:\n{e.stdout}")
        print(f"Stderr:\n{e.stderr}")
        if check:
            sys.exit(e.returncode)
        raise e

def check_git_status(baseline):
    """Ensures there are no uncommitted changes to the opts file before we start."""
    print(f"Checking Git status of {OPTS_FILE}...")
    status = subprocess.run(["git", "diff", "--exit-code", OPTS_FILE], capture_output=True)
    if status.returncode != 0:
        print(f"ERROR: You have uncommitted changes in {OPTS_FILE}!")
        print("Please commit your changes before running this script so Git can safely manage the branches.")
        sys.exit(1)

def backup_and_revert_skopts():
    """Reads SkOpts.cpp so we can restore it exactly when done."""
    with open(SKOPTS_FILE, "r") as f:
        return f.read()

def restore_skopts(original_content):
    """Restores the original content of SkOpts.cpp."""
    with open(SKOPTS_FILE, "w") as f:
        f.write(original_content)

def configure_skopts(mode):
    """Modifies SkOpts.cpp to isolate a specific architecture: ml4, ml3, or sse2."""
    with open(SKOPTS_FILE, "r") as f:
        content = f.read()

    # Find the Init_* calls we want to block
    # Note: under sse2, we disable both ml4 and ml3. Under ml3, we disable ml4. Under ml4, we leave both.
    if mode == "sse2":
        content = content.replace("if (SkCpu::Supports(SkX64::ML4)) { Init_ml4(); }", "// if (SkCpu::Supports(SkX64::ML4)) { Init_ml4(); }")
        content = content.replace("if (SkCpu::Supports(SkX64::ML3)) { Init_ml3(); }", "// if (SkCpu::Supports(SkX64::ML3)) { Init_ml3(); }")
    elif mode == "ml3":
        content = content.replace("if (SkCpu::Supports(SkX64::ML4)) { Init_ml4(); }", "// if (SkCpu::Supports(SkX64::ML4)) { Init_ml4(); }")

    with open(SKOPTS_FILE, "w") as f:
        f.write(content)

def compile_binaries(baseline):
    """Orchestrates the checkout, modification, and compilation of all 6 modes."""
    os.makedirs(BIN_TEMP_DIR, exist_ok=True)
    original_skopts = backup_and_revert_skopts()

    try:
        # Step 1: Compile the 3 Control (Baseline) versions
        print(f"\n--- Checking out Control baseline ({baseline}) ---")
        run_cmd(["git", "checkout", baseline, "--", OPTS_FILE])

        for target in ["ml4", "ml3", "sse2"]:
            print(f"\nConfiguring SkOpts.cpp for Control {target.upper()}...")
            restore_skopts(original_skopts)
            configure_skopts(target)

            print(f"Compiling nanobench_control_{target}...")
            run_cmd(["ninja", "-C", BUILD_DIR, "nanobench"])

            dest = os.path.join(BIN_TEMP_DIR, f"nanobench_{target}_control")
            shutil.copy(os.path.join(BUILD_DIR, "nanobench"), dest)
            print(f"Saved binary to {dest}")

        # Step 2: Compile the 3 Optimized (With Changes) versions
        print("\n--- Checking out HEAD (Optimized) ---")
        run_cmd(["git", "checkout", "HEAD", "--", OPTS_FILE])

        for target in ["ml4", "ml3", "sse2"]:
            print(f"\nConfiguring SkOpts.cpp for Optimized {target.upper()}...")
            restore_skopts(original_skopts)
            configure_skopts(target)

            print(f"Compiling nanobench_with_changes_{target}...")
            run_cmd(["ninja", "-C", BUILD_DIR, "nanobench"])

            dest = os.path.join(BIN_TEMP_DIR, f"nanobench_{target}_with_changes")
            shutil.copy(os.path.join(BUILD_DIR, "nanobench"), dest)
            print(f"Saved binary to {dest}")

    finally:
        # Cleanup: Revert SkOpts.cpp and opts back to clean working branch state
        print("\nCleaning up working tree files...")
        restore_skopts(original_skopts)
        run_cmd(["git", "checkout", "HEAD", "--", OPTS_FILE])
        run_cmd(["git", "checkout", "HEAD", "--", SKOPTS_FILE])

def run_benchmarks(bench_args):
    """Runs all 6 saved binaries and prints outputs directly to stdout."""
    targets = ["ml4", "ml3", "sse2"]
    types = ["control", "with_changes"]

    for t in targets:
        for run_type in types:
            binary_name = f"nanobench_{t}_{run_type}"
            binary_path = os.path.join(BIN_TEMP_DIR, binary_name)

            if not os.path.exists(binary_path):
                print(f"ERROR: Binary {binary_name} not found in {BIN_TEMP_DIR}. Did you run --compile?")
                sys.exit(1)

            print(f"\n======================================================================")
            print(f" Running {binary_name}...")

            # Build full run command
            cmd = [binary_path] + bench_args

            # Run and print output directly to stdout
            result = subprocess.run(cmd, text=True)
            if result.returncode != 0:
                print(f"ERROR running benchmark {binary_name}!")
                sys.exit(result.returncode)

def main():
    parser = argparse.ArgumentParser(description="Automate compiling and running of SkRasterPipeline microbenchmarks.")
    parser.add_argument("--compile", action="store_true", help="Compile and cache control and optimized binaries.")
    parser.add_argument("--run", action="store_true", help="Run the compiled benchmark binaries directly to stdout.")
    parser.add_argument("--baseline", default="origin/main", help="Git reference for the baseline. Defaults to origin/main.")

    # Capture all remaining arguments to forward to nanobench when running
    args, remaining = parser.parse_known_args()

    # Fail if neither --compile nor --run is specified explicitly
    if not args.compile and not args.run:
        print("ERROR: You must explicitly specify either --compile or --run")
        print("\nUsage examples:")
        print("  1. Compile all target binaries:")
        print("     python3 tools/raster_pipeline/run_benchmarks.py --compile")
        print("\n  2. Run the benchmarks when the machine is quiet:")
        print("     python3 tools/raster_pipeline/run_benchmarks.py --run --match skrp_gather --samples 5 --ms 0")
        sys.exit(1)

    if args.compile:
        check_git_status(args.baseline)
        compile_binaries(args.baseline)

    if args.run:
        # Default bench arguments if none were provided by the user
        bench_args = remaining if remaining else ["--match", "skrp_"]
        run_benchmarks(bench_args)

if __name__ == "__main__":
    main()
