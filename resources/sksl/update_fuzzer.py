#!/usr/bin/env python3
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
with tempfile.NamedTemporaryFile(suffix='primary.zip', delete=False, mode='w') as pathToZip:
    with zipfile.ZipFile(pathToZip.name, 'w', zipfile.ZIP_DEFLATED) as archive:
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
                    with open('%s/%s' % (root, file), 'rb') as skslFile:
                        # Read the SkSL text as input.
                        inputSkSL = skslFile.read()
                        # Copy the SkSL into our zip archive.
                        archive.writestr(pathInZip, inputSkSL)

    try:
        # Upload our zip file to cloud storage.
        output = subprocess.check_output(
                ['gsutil', 'cp', pathToZip.name,
                 'gs://skia-fuzzer/oss-fuzz/sksl_seed_corpus.zip'],
                 stderr=subprocess.STDOUT)

        # Make the uploaded file world-readable.
        output = subprocess.check_output(
                ['gsutil', 'acl', 'ch', '-u', 'AllUsers:R',
                 'gs://skia-fuzzer/oss-fuzz/sksl_seed_corpus.zip'],
                stderr=subprocess.STDOUT)

    except subprocess.CalledProcessError as err:
        # Report the error.
        print("### Unable to upload fuzzer corpus to Google Cloud:")
        print("    " + "\n    ".join(err.output.splitlines()))
        print("\nPlease read the notes at the top of update_fuzzer.py for next steps.\n")
        sys.exit(err.returncode)
