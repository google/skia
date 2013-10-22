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

def test_invalid_file(file_dir, skimage_binary):
    """ Test the return value of skimage when an invalid file is decoded.
        If there is no expectation file, or the file expects a particular
        result, skimage should return nonzero indicating failure.
        If the file has no expectation, or ignore-failure is set to true,
        skimage should return zero indicating success. """
    invalid_file = os.path.join(file_dir, "skimage", "input", "bad-images",
                                "invalid.png")
    # No expectations file:
    args = [skimage_binary, "--readPath", invalid_file]
    result = subprocess.call(args)
    if 0 == result:
      print "'%s' should have reported failure!" % " ".join(args)
      exit(1)

    # Directory holding all expectations files
    expectations_dir = os.path.join(file_dir, "skimage", "input", "bad-images")

    # Expectations file expecting a valid decode:
    incorrect_expectations = os.path.join(expectations_dir,
                                          "incorrect-results.json")
    args = [skimage_binary, "--readPath", invalid_file,
            "--readExpectationsPath", incorrect_expectations]
    result = subprocess.call(args)
    if 0 == result:
      print "'%s' should have reported failure!" % " ".join(args)
      exit(1)

    # Empty expectations:
    empty_expectations = os.path.join(expectations_dir, "empty-results.json")
    output = subprocess.check_output([skimage_binary, "--readPath", invalid_file,
                                      "--readExpectationsPath",
                                      empty_expectations],
                                     stderr=subprocess.STDOUT)
    if not "Missing" in output:
      # Another test (in main()) tests to ensure that "Missing" does not appear
      # in the output. That test could be passed if the output changed so
      # "Missing" never appears. This ensures that an error is not missed if
      # that happens.
      print "skimage output changed! This may cause other self tests to fail!"
      exit(1)

    # Ignore failure:
    ignore_expectations = os.path.join(expectations_dir, "ignore-results.json")
    output = subprocess.check_output([skimage_binary, "--readPath", invalid_file,
                                      "--readExpectationsPath",
                                      ignore_expectations],
                                     stderr=subprocess.STDOUT)
    if not "failures" in output:
      # Another test (in main()) tests to ensure that "failures" does not
      # appear in the output. That test could be passed if the output changed
      # so "failures" never appears. This ensures that an error is not missed
      # if that happens.
      print "skimage output changed! This may cause other self tests to fail!"
      exit(1)

def test_incorrect_expectations(file_dir, skimage_binary):
    """ Test that comparing to incorrect expectations fails, unless
        ignore-failures is set to true. """
    valid_file = os.path.join(file_dir, "skimage", "input",
                                    "images-with-known-hashes",
                                    "1209453360120438698.png")
    expectations_dir = os.path.join(file_dir, "skimage", "input",
                                    "images-with-known-hashes")

    incorrect_results = os.path.join(expectations_dir,
                                     "incorrect-results.json")
    args = [skimage_binary, "--readPath", valid_file, "--readExpectationsPath",
            incorrect_results]
    result = subprocess.call(args)
    if 0 == result:
      print "'%s' should have reported failure!" % " ".join(args)
      exit(1)

    ignore_results = os.path.join(expectations_dir, "ignore-failures.json")
    subprocess.check_call([skimage_binary, "--readPath", valid_file,
                           "--readExpectationsPath", ignore_results])

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
    output = subprocess.check_output([skimage_binary, "--readPath", images_dir,
                                      "--readExpectationsPath",
                                      expectations_path],
                                     stderr=subprocess.STDOUT)

    # Although skimage succeeded, it would have reported success if the file
    # was missing from the expectations file. Consider this a failure, since
    # the expectations file was created from this same image. (It will print
    # "Missing" in this case before listing the missing expectations).
    if "Missing" in output:
      print "Expectations file was missing expectations!"
      print output
      exit(1)

    # Again, skimage would succeed if there were known failures (and print
    # "failures"), but there should be no failures, since the file just
    # created did not include failures to ignore.
    if "failures" in output:
      print "Image failed!"
      print output
      exit(1)


    test_incorrect_expectations(file_dir=file_dir,
                                skimage_binary=skimage_binary)

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

    test_invalid_file(file_dir=file_dir, skimage_binary=skimage_binary)

    # Done with all tests.
    print "Self tests succeeded!"

if __name__ == "__main__":
    main()
