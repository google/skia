#!/usr/bin/env python
# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Run the webpages_playback automation script."""


import os
import subprocess
import sys

sys.path.insert(0, os.getcwd())

from common.py.utils import gs_utils
from common.py.utils import shell_utils


SKP_VERSION_FILE = 'SKP_VERSION'


def _get_skp_version():
  """Find an unused SKP version."""
  current_skp_version = None
  with open(SKP_VERSION_FILE) as f:
    current_skp_version = int(f.read().rstrip())

  # Find the first SKP version which has no uploaded SKPs.
  new_version = current_skp_version + 1
  while True:
    gs_path = 'playback_%d/skps' % new_version
    if not gs_utils.GSUtils().does_storage_object_exist('chromium-skia-gm',
                                                        gs_path):
      return new_version
    new_version += 1


def main(chrome_src_path, browser_executable):
  browser_executable = os.path.realpath(browser_executable)
  skp_version = _get_skp_version()
  print 'SKP_VERSION=%d' % skp_version

  if os.environ.get('CHROME_HEADLESS'):
    # Start Xvfb if running on a bot.
    try:
      shell_utils.run('sudo Xvfb :0 -screen 0 1280x1024x24 &', shell=True)
    except Exception:
      # It is ok if the above command fails, it just means that DISPLAY=:0
      # is already up.
      pass

  upload_dir = 'playback_%d' % skp_version
  webpages_playback_cmd = [
    'python', os.path.join(os.path.dirname(os.path.realpath(__file__)),
                           'webpages_playback.py'),
    '--page_sets', 'all',
    '--browser_executable', browser_executable,
    '--non-interactive',
    '--upload',
    '--alternate_upload_dir', upload_dir,
    '--chrome_src_path', chrome_src_path,
  ]

  try:
    shell_utils.run(webpages_playback_cmd)
  finally:
    # Clean up any leftover browser instances. This can happen if there are
    # telemetry crashes, processes are not always cleaned up appropriately by
    # the webpagereplay and telemetry frameworks.
    procs = subprocess.check_output(['ps', 'ax'])
    for line in procs.splitlines():
      if browser_executable in line:
        pid = line.strip().split(' ')[0]
        if pid != str(os.getpid()) and not 'python' in line:
          try:
            shell_utils.run(['kill', '-9', pid])
          except shell_utils.CommandFailedException as e:
            print e
        else:
          print 'Refusing to kill self.'

  print 'writing %s: %s' % (SKP_VERSION_FILE, skp_version)
  with open(SKP_VERSION_FILE, 'w') as f:
    f.write(str(skp_version))


if '__main__' == __name__:
  if len(sys.argv) != 3:
    print >> sys.stderr, 'USAGE: %s <chrome src path> <browser executable>'
    sys.exit(1)
  main(*sys.argv[1:])
