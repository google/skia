#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the SKP asset."""


import argparse
import common
import os
import shutil
import subprocess
import utils


SKIA_TOOLS = os.path.join(common.INFRA_BOTS_DIR, os.pardir, os.pardir, 'tools')


def create_asset(chrome_src_path, browser_executable, target_dir,
                 upload_to_partner_bucket):
  """Create the asset."""
  browser_executable = os.path.realpath(browser_executable)
  chrome_src_path = os.path.realpath(chrome_src_path)
  target_dir = os.path.realpath(target_dir)

  if not os.path.exists(target_dir):
    os.makedirs(target_dir)

  with utils.tmp_dir():
    if os.environ.get('CHROME_HEADLESS'):
      # Start Xvfb if running on a bot.
      try:
        subprocess.Popen(['sudo', 'Xvfb', ':0', '-screen', '0', '1280x1024x24'])
      except Exception:
        # It is ok if the above command fails, it just means that DISPLAY=:0
        # is already up.
        pass

    webpages_playback_cmd = [
      'python', os.path.join(SKIA_TOOLS, 'skp', 'webpages_playback.py'),
      '--page_sets', 'all',
      '--browser_executable', browser_executable,
      '--non-interactive',
      '--output_dir', os.getcwd(),
      '--chrome_src_path', chrome_src_path,
    ]
    if upload_to_partner_bucket:
      webpages_playback_cmd.append('--upload_to_partner_bucket')
    try:
      subprocess.check_call(webpages_playback_cmd)
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
              subprocess.check_call(['kill', '-9', pid])
            except subprocess.CalledProcessError as e:
              print e
          else:
            print 'Refusing to kill self.'
    src = os.path.join(os.getcwd(), 'playback', 'skps')
    for f in os.listdir(src):
      if f.endswith('.skp'):
        shutil.copyfile(os.path.join(src, f), os.path.join(target_dir, f))


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  parser.add_argument('--chrome_src_path', '-c', required=True)
  parser.add_argument('--browser_executable', '-e', required=True)
  parser.add_argument('--upload_to_partner_bucket', action='store_true')
  args = parser.parse_args()
  create_asset(args.chrome_src_path, args.browser_executable, args.target_dir,
               args.upload_to_partner_bucket)


if __name__ == '__main__':
  main()
