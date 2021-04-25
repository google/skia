#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the SKP asset."""


from __future__ import print_function
import argparse
import common
from distutils.dir_util import copy_tree
import os
import shutil
import subprocess
import utils


SKIA_TOOLS = os.path.join(common.INFRA_BOTS_DIR, os.pardir, os.pardir, 'tools')

PRIVATE_SKPS_GS = 'gs://skia-skps/private/skps'


def get_flutter_skps(target_dir):
  """Creates SKPs using Flutter's skp_generator tool.

  Documentation is at https://github.com/flutter/tests/tree/master/skp_generator
  """
  with utils.tmp_dir():
    utils.git_clone('https://github.com/flutter/tests.git', '.')
    os.chdir('skp_generator')
    subprocess.check_call(['bash', 'build.sh'])
    # Fix invalid SKP file names.
    for f in os.listdir('skps'):
      original_file_name = os.path.splitext(f)[0]
      new_file_name = ''.join([x if x.isalnum() else "_"
                               for x in original_file_name])
      if new_file_name != original_file_name:
        os.rename(os.path.join('skps', f),
                  os.path.join('skps', new_file_name + '.skp'))
    copy_tree('skps', target_dir)


def create_asset(chrome_src_path, browser_executable, target_dir,
                 upload_to_partner_bucket):
  """Create the SKP asset.

  Creates the asset from 3 sources:
  1. From Flutter's skp_generator tool.
  2. The web pages defined in the tools/skp/page_sets/ directory.
  3. Any private SKPs stored in $PRIVATE_SKPS_GS
  """
  browser_executable = os.path.realpath(browser_executable)
  chrome_src_path = os.path.realpath(chrome_src_path)
  target_dir = os.path.realpath(target_dir)

  if not os.path.exists(target_dir):
    os.makedirs(target_dir)

  # 1. Flutter SKPs
  get_flutter_skps(target_dir)

  # 2. Skia's SKPs from tools/skp/page_sets/
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
    print('Running webpages_playback command:\n$ %s' %
        ' '.join(webpages_playback_cmd))
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
              print(e)
          else:
            print('Refusing to kill self.')
    src = os.path.join(os.getcwd(), 'playback', 'skps')
    for f in os.listdir(src):
      if f.endswith('.skp'):
        shutil.copyfile(os.path.join(src, f), os.path.join(target_dir, f))

  # 3. Copy over private SKPs from Google storage into the target_dir.
  subprocess.call([
        'gsutil', 'cp', os.path.join(PRIVATE_SKPS_GS, '*'), target_dir])


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
