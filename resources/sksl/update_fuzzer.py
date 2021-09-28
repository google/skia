#!/usr/bin/env python
#
# Copyright 2021 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This tool updates the OSS-Fuzz corpus using Google Cloud's 'gsutil' tool.

# You will need to be given access to the Google Storage fuzzer repo (at
# gs://skia-fuzzer/oss-fuzz/) by the Skia Infra team.

# You will also need to set up credentials for gsutil on your machine by running:
#   gcloud auth login

import os
import subprocess
import tempfile
import zipfile

# Locate this script in the file system.
startDir = os.path.dirname(os.path.abspath(__file__))
fileNum = 1

# Prepare two scratch zip files, one for the input data as-is and another with 256-byte padding.
with tempfile.NamedTemporaryFile(suffix='primary.zip', delete=False, mode='w') as pathToPrimaryZip:
    with tempfile.NamedTemporaryFile(suffix='pad.zip', delete=False, mode='w') as pathToPaddedZip:
        with zipfile.ZipFile(pathToPrimaryZip.name, 'w', zipfile.ZIP_DEFLATED) as primaryArchive:
            with zipfile.ZipFile(pathToPaddedZip.name, 'w', zipfile.ZIP_DEFLATED) as paddedArchive:
                # Iterate over every file in this directory and use it to assemble our corpus.
                for root, dirs, files in os.walk(startDir):
                    for file in files:
                        # Exclude files that won't be useful fuzzer inputs.
                        if (not file.startswith('.')        # Hidden
                            and not file.endswith('.py')    # Python
                            and not file.endswith('.test')  # ES2 conformance script
                            and not file.endswith('.txt')): # Text
                            # Prepend a number to each output filename to guarantee uniqueness.
                            pathInZip = '%d_%s' % (fileNum, file)
                            fileNum += 1
                            with open('%s/%s' % (root, file), 'r') as skslFile:
                                # Read the SkSL text as input.
                                inputSkSL = skslFile.read()
                                # In the primary archive, write the input SkSL as-is.
                                primaryArchive.writestr(pathInZip, inputSkSL)
                                # In the padded archive, write the input SkSL with 256 bonus bytes.
                                paddedSkSL = inputSkSL + ("/" * 256)
                                paddedArchive.writestr(pathInZip, paddedSkSL)

    try:
        # Upload both zip files to cloud storage.
        output = subprocess.check_output(
                ['gsutil', 'cp', pathToPrimaryZip.name,
                 'gs://skia-fuzzer/oss-fuzz/sksl_seed_corpus.zip'],
                 stderr=subprocess.STDOUT)

        output = subprocess.check_output(
                ['gsutil', 'cp', pathToPaddedZip.name,
                 'gs://skia-fuzzer/oss-fuzz/sksl_with_256_padding_seed_corpus.zip'],
                stderr=subprocess.STDOUT)

        # Make the uploaded files world-readable.
        output = subprocess.check_output(
                ['gsutil', 'acl', 'ch', '-u', 'AllUsers:R',
                 'gs://skia-fuzzer/oss-fuzz/sksl_seed_corpus.zip'],
                stderr=subprocess.STDOUT)

        output = subprocess.check_output(
                ['gsutil', 'acl', 'ch', '-u', 'AllUsers:R',
                 'gs://skia-fuzzer/oss-fuzz/sksl_with_256_padding_seed_corpus.zip'],
                stderr=subprocess.STDOUT)

    except subprocess.CalledProcessError as err:
        # Report the error.
        print("### Unable to upload fuzzer corpus to Google Cloud:")
        print("    " + "\n    ".join(err.output.splitlines()))
        print("\nPlease read the notes at the top of update_fuzzer.py for next steps.\n")
        sys.exit(err.returncode)
