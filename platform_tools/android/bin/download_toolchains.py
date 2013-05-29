#!/usr/bin/python

"""Download all toolchains for this platform.

This module downloads multiple tgz's.
"""

import download_utils
import sys

url = sys.argv[1]
filepath = sys.argv[2]

try:
  download_utils.SyncURL(url, filepath)
  exit(0)
except download_utils.HashError, e:
  exit(1)

