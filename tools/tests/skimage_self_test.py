#!/usr/bin/env python
# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Self-test for skimage.

import os
import subprocess
import sys

class BinaryNotFoundException(Exception):
    def __str__ (self):
        return ("Could not find binary!\n"
                "Did you forget to build the tools project?\n"
                "Self tests failed")

# Find a path to the binary to use. Iterates through a list of possible
# locations the binary may be.
def PickBinaryPath(base_dir):
    POSSIBLE_BINARY_PATHS = [
        'out/Debug/skimage',
        'out/Release/skimage',
        'xcodebuild/Debug/skimage',
        'xcodebuild/Release/skimage',
    ]
    for binary in POSSIBLE_BINARY_PATHS:
        binary_full_path = os.path.join(base_dir, binary)
        if (os.path.exists(binary_full_path)):
            return binary_full_path
    raise BinaryNotFoundException

def main():
    # Use the directory of this file as the out directory
    file_dir = os.path.abspath(os.path.dirname(__file__))

    trunk_dir = os.path.normpath(os.path.join(file_dir, os.pardir, os.pardir))

    # Find the binary
    skimage_binary = PickBinaryPath(trunk_dir)
    print "Running " + skimage_binary

    # Run skimage twice, first to create an expectations file, and then
    # comparing to it.

    # Both commands will run the binary, reading from resources.
    cmd_line = [skimage_binary]
    resources_dir = os.path.join(trunk_dir, 'resources')
    cmd_line.extend(["-r", resources_dir])

    # Create the expectations file
    results_file = os.path.join(file_dir, "skimage", "self_test_results.json")
    create_expectations_cmd = cmd_line + ["--createExpectationsPath",
                                          results_file]
    subprocess.check_call(create_expectations_cmd)

    # Now read from the expectations file
    check_expectations_cmd = cmd_line + ["--readExpectationsPath",
                                         results_file]
    subprocess.check_call(check_expectations_cmd)
    print "Self tests succeeded!"

if __name__ == "__main__":
    main()
