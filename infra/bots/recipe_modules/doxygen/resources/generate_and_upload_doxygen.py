#!/usr/bin/env python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Generate Doxygen documentation."""


import datetime
import os
import shutil
import subprocess
import sys


DOXYFILE_BASENAME = 'Doxyfile'  # must match name of Doxyfile in skia root
DOXYGEN_BINARY = 'doxygen'
WORKDIR = os.path.join(os.pardir, 'doxygen_workdir')
DOXYGEN_CONFIG_DIR = os.path.join(WORKDIR, 'doxygen-config')
DOXYGEN_WORKING_DIR = os.path.join(WORKDIR, 'doxygen')
DOXYGEN_GS_PATH = '/'.join(['gs://skia-doc', 'doxygen'])

IFRAME_FOOTER_TEMPLATE = """
<html><body><address style="text-align: right;"><small>
Generated at %s for skia
by <a href="http://www.doxygen.org/index.html">doxygen</a>
%s </small></address></body></html>
"""


def recreate_dir(path):
  """Delete and recreate the directory."""
  try:
    shutil.rmtree(path)
  except OSError:
    if os.path.exists(path):
      raise Exception('Could not remove %s' % path)
  os.makedirs(path)


def generate_and_upload_doxygen():
  """Generate Doxygen."""
  # Create empty dir and add static_footer.txt
  recreate_dir(DOXYGEN_WORKING_DIR)
  static_footer_path = os.path.join(DOXYGEN_WORKING_DIR, 'static_footer.txt')
  shutil.copyfile(os.path.join('tools', 'doxygen_footer.txt'),
                  static_footer_path)

  # Make copy of doxygen config file, overriding any necessary configs,
  # and run doxygen.
  recreate_dir(DOXYGEN_CONFIG_DIR)
  modified_doxyfile = os.path.join(DOXYGEN_CONFIG_DIR, DOXYFILE_BASENAME)
  with open(DOXYFILE_BASENAME, 'r') as reader:
    with open(modified_doxyfile, 'w') as writer:
      shutil.copyfileobj(reader, writer)
      writer.write('OUTPUT_DIRECTORY = %s\n' % DOXYGEN_WORKING_DIR)
      writer.write('HTML_FOOTER = %s\n' % static_footer_path)
  subprocess.check_call([DOXYGEN_BINARY, modified_doxyfile])

  # Create iframe_footer.html
  with open(os.path.join(DOXYGEN_WORKING_DIR, 'iframe_footer.html'), 'w') as f:
    f.write(IFRAME_FOOTER_TEMPLATE % (
        datetime.datetime.now().isoformat(' '),
        subprocess.check_output([DOXYGEN_BINARY, '--version']).rstrip()))

  # Upload.
  cmd = ['gsutil', 'cp', '-a', 'public-read', '-R',
         DOXYGEN_WORKING_DIR, DOXYGEN_GS_PATH]
  subprocess.check_call(cmd)


if '__main__' == __name__:
  generate_and_upload_doxygen()

