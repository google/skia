#!/usr/bin/env python
# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Self-test for skimage.

import filecmp
import os
import subprocess
import sys
import tempfile

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

# Quit early if two files have different content.
def DieIfFilesMismatch(expected, actual):
    if not filecmp.cmp(expected, actual):
        print 'Error: file mismatch! expected=%s , actual=%s' % (
            expected, actual)
        exit(1)

def main():
    # Use the directory of this file as the out directory
    file_dir = os.path.abspath(os.path.dirname(__file__))

    trunk_dir = os.path.normpath(os.path.join(file_dir, os.pardir, os.pardir))

    # Find the binary
    skimage_binary = PickBinaryPath(trunk_dir)
    print "Running " + skimage_binary

    # Generate an expectations file from known images.
    images_dir = os.path.join(file_dir, "skimage", "input",
                              "images-with-known-hashes")
    expectations_path = os.path.join(file_dir, "skimage", "output-actual",
                                     "create-expectations", "expectations.json")
    subprocess.check_call([skimage_binary, "--readPath", images_dir,
                           "--createExpectationsPath", expectations_path])

    # Make sure the expectations file was generated correctly.
    golden_expectations = os.path.join(file_dir, "skimage", "output-expected",
                                       "create-expectations",
                                       "expectations.json")
    DieIfFilesMismatch(expected=golden_expectations, actual=expectations_path)

    # Tell skimage to read back the expectations file it just wrote, and
    # confirm that the images in images_dir match it.
    subprocess.check_call([skimage_binary, "--readPath", images_dir,
                           "--readExpectationsPath", expectations_path])

    # TODO(scroggo): Add a test that compares expectations and image files that
    # are known to NOT match, and make sure it returns an error.

    # Generate an expectations file from an empty directory.
    empty_dir = tempfile.mkdtemp()
    expectations_path = os.path.join(file_dir, "skimage", "output-actual",
                                     "empty-dir", "expectations.json")
    subprocess.check_call([skimage_binary, "--readPath", empty_dir,
                           "--createExpectationsPath", expectations_path])
    golden_expectations = os.path.join(file_dir, "skimage", "output-expected",
                                       "empty-dir", "expectations.json")
    DieIfFilesMismatch(expected=golden_expectations, actual=expectations_path)
    os.rmdir(empty_dir)

    # Generate an expectations file from a nonexistent directory.
    expectations_path = os.path.join(file_dir, "skimage", "output-actual",
                                     "nonexistent-dir", "expectations.json")
    subprocess.check_call([skimage_binary, "--readPath", "/nonexistent/dir",
                           "--createExpectationsPath", expectations_path])
    golden_expectations = os.path.join(file_dir, "skimage", "output-expected",
                                       "nonexistent-dir", "expectations.json")
    DieIfFilesMismatch(expected=golden_expectations, actual=expectations_path)

    # Done with all tests.
    print "Self tests succeeded!"

if __name__ == "__main__":
    main()
