#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the SKP asset."""


from __future__ import print_function
import argparse
from distutils.dir_util import copy_tree
import os
import shutil
import subprocess
import sys
import tempfile

FILE_DIR = os.path.dirname(os.path.abspath(__file__))
INFRA_BOTS_DIR = os.path.realpath(os.path.join(FILE_DIR, os.pardir, os.pardir))
sys.path.insert(0, INFRA_BOTS_DIR)
import utils


BROWSER_EXECUTABLE_ENV_VAR = 'SKP_BROWSER_EXECUTABLE'
CHROME_SRC_PATH_ENV_VAR = 'SKP_CHROME_SRC_PATH'
UPLOAD_TO_PARTNER_BUCKET_ENV_VAR = 'SKP_UPLOAD_TO_PARTNER_BUCKET'
DM_PATH_ENV_VAR = 'DM_PATH'

SKIA_TOOLS = os.path.join(INFRA_BOTS_DIR, os.pardir, os.pardir, 'tools')
PRIVATE_SKPS_GS = 'gs://skia-skps/private/skps'


def getenv(key):
  val = os.environ.get(key)
  if not val:
    print(('Environment variable %s not set; you should run this via '
           'create_and_upload.py.' % key), file=sys.stderr)
    sys.exit(1)
  return val


def get_flutter_skps(target_dir):
  """Creates SKPs using Flutter's skp_generator tool.

  Documentation is at https://github.com/flutter/tests/tree/master/skp_generator
  """
  with utils.tmp_dir():
    print('Retrieving Flutter SKPs...')
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
    print('Done retrieving Flutter SKPs.')


def create_asset(chrome_src_path, browser_executable, target_dir,
                 upload_to_partner_bucket, dm_path):
  """Create the SKP asset.

  Creates the asset from 3 sources:
  1. From Flutter's skp_generator tool.
  2. The web pages defined in the tools/skp/page_sets/ directory.
  3. Any private SKPs stored in $PRIVATE_SKPS_GS after running dm on
     them (see below).

  The script runs the following cmd on the non-generated SKPs stored in
  $PRIVATE_SKPS_GS -
  `dm --config skp -w newskps/ --skps oldskps/ --src skp`
  The cmd updates the version stored in the SKPs so that the versions in
  them do not eventually become unsupported.
  """
  browser_executable = os.path.realpath(browser_executable)
  chrome_src_path = os.path.realpath(chrome_src_path)
  dm_path = os.path.realpath(dm_path)
  target_dir = os.path.realpath(target_dir)

  if not os.path.exists(target_dir):
    os.makedirs(target_dir)

  # 1. Flutter SKPs
  get_flutter_skps(target_dir)

  # 2. Skia's SKPs from tools/skp/page_sets/
  with utils.tmp_dir():
    if os.environ.get('CHROME_HEADLESS'):
      print('Starting xvfb')
      # Start Xvfb if running on a bot.
      try:
        xvfb_proc = subprocess.Popen([
            'sudo', 'Xvfb', ':0', '-screen', '0', '1280x1024x24'])
      except Exception:
        # It is ok if the above command fails, it just means that DISPLAY=:0
        # is already up.
        xvfb_proc = None

    print('Running webpages_playback to generate SKPs...')
    webpages_playback_cmd = [
      'python', '-u', os.path.join(SKIA_TOOLS, 'skp', 'webpages_playback.py'),
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
      if xvfb_proc:
        try:
          xvfb_proc.kill()
        except OSError as e:
          print('Failed to kill xvfb process via Popen.kill();'
                ' attempting `sudo kill`...')
          try:
            subprocess.check_call(['sudo', 'kill', '-9', str(xvfb_proc.pid)])
          except subprocess.CalledProcessError as e:
            print('Failed to kill xvfb process via `sudo kill`;'
                  'this may cause a hang.')
      # Clean up any leftover browser instances. This can happen if there are
      # telemetry crashes, processes are not always cleaned up appropriately by
      # the webpagereplay and telemetry frameworks.
      procs = subprocess.check_output(['ps', 'ax']).decode()
      for line in procs.splitlines():
        if browser_executable in line:
          print(line)
          pid = line.strip().split(' ')[0]
          if pid != str(os.getpid()) and not 'python' in line:
            print('Kill browser process %s' % str(pid))
            try:
              subprocess.check_call(['kill', '-9', str(pid)])
            except subprocess.CalledProcessError as e:
              print(e)
          else:
            pass
        if 'Xvfb' in line:
          print(line)
          pid = line.strip().split(' ')[0]
          print('Kill Xvfb process %s' % str(pid))
          try:
            subprocess.check_call(['sudo', 'kill', '-9', str(pid)])
          except subprocess.CalledProcessError as e:
            print(e)

    src = os.path.join(os.getcwd(), 'playback', 'skps')
    for f in os.listdir(src):
      if f.endswith('.skp'):
        shutil.copyfile(os.path.join(src, f), os.path.join(target_dir, f))
    print('Done running webpages_playback.')

  # 3. Copy over private SKPs from Google storage into the target_dir.
  old_skps_dir = tempfile.mkdtemp()
  new_skps_dir = tempfile.mkdtemp()
  print('Copying non-generated SKPs from private GCS bucket...')
  subprocess.check_call([
    'gsutil', 'cp', os.path.join(PRIVATE_SKPS_GS, '*'), old_skps_dir])
  print('Updating non-generated SKP versions')
  subprocess.check_call([
      dm_path,
      '--config', 'skp',
      '-w', new_skps_dir,
      '--skps', old_skps_dir,
      '--src', 'skp'])

  # DM creates artifacts in 2 directory level- one for the config and one for
  # the output type. In this case it ends up creating ${temp_dir}/skp/skp/
  for f in os.listdir(os.path.join(new_skps_dir, 'skp', 'skp')):
    if f.endswith('.skp'):
      # Files generated by DM add a suffix based on the output type.
      # For "xyz.skp" DM will create "xyz.skp.skp". We strip out
      # the final ".skp" here.
      shutil.copyfile(
          os.path.join(new_skps_dir, 'skp', 'skp', f),
          os.path.join(target_dir, f.replace('.skp.skp', '.skp')))
  shutil.rmtree(old_skps_dir)
  shutil.rmtree(new_skps_dir)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()

  # Obtain flags from create_and_upload via environment variables, since
  # this script is called via `sk` and not directly.
  chrome_src_path = getenv(CHROME_SRC_PATH_ENV_VAR)
  browser_executable = getenv(BROWSER_EXECUTABLE_ENV_VAR)
  upload_to_partner_bucket = getenv(UPLOAD_TO_PARTNER_BUCKET_ENV_VAR) == '1'
  dm_path = getenv(DM_PATH_ENV_VAR)

  create_asset(chrome_src_path, browser_executable, args.target_dir,
               upload_to_partner_bucket, dm_path)


if __name__ == '__main__':
  main()
