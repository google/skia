#!/usr/bin/env python
# Copyright (c) 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Script that add a specific legacy flag to multiple Skia client repos.

This would only work on Google desktop.

Example usage:
  $ python add_legacy_flag.py SK_SUPPORT_LEGACY_SOMETHING \
      -a /data/android -c ~/chromium/ -g legacyflag
"""

import os, sys
import argparse
import subprocess
import getpass
from random import randint


ANDROID_TOOLS_DIR = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    'android')

sys.path.append(ANDROID_TOOLS_DIR)


import upload_to_android


def add_to_google3(args):
  print "Run prodaccess to gain access to Google3:"
  subprocess.check_call(['prodaccess'])

  google3_dir = "/google/src/cloud/%s/%s/google3" % (getpass.getuser(),
                                                     args.google3)

  if not os.path.isdir(google3_dir):
    print "Creating citc workspace %s:" % args.google3
    subprocess.check_call(['g4', 'citc', args.google3])

  os.chdir(google3_dir)

  build_file = os.path.join('third_party', 'skia', 'HEAD', 'BUILD')

  # reset any previous change on the same citc client
  subprocess.check_call(['g4', 'revert', build_file])
  subprocess.check_call(['g4', 'sync'])

  with open(build_file) as f:
    lines = f.readlines()

  line_index = lines.index("DEFINES_GOOGLE3 = [\n")
  lines.insert(line_index + 1, "    \"%s\",\n" % args.flag)

  with open(build_file, 'w') as f:
    for line in lines:
      f.write(line)

  subprocess.check_call(['g4', 'mail', '-m', 'benjaminwagner,liyuqian',
                         '--desc', '"Add %s"' % args.flag])


def add_to_android(args):
  repo_binary = upload_to_android.init_work_dir(args.android_dir);

  # Create repo branch.
  subprocess.check_call('%s start flag .' % repo_binary, shell=True)

  # Add flag to SkUserConfigManual.h.
  config_file = os.path.join('include', 'config', 'SkUserConfigManual.h')
  with open(config_file, 'a') as f:
    f.write('\n')
    f.write('#ifndef %s\n'    % args.flag)
    f.write('  #define %s\n'  % args.flag)
    f.write('#endif//%s\n'    % args.flag)
  subprocess.check_call('git add %s' % config_file, shell=True)

  message = ('Add %s\n\n'
             'Test: Presubmit checks will test this change.' % args.flag)

  subprocess.check_call('git commit -m "%s"' % message, shell=True)

  # Upload to Android Gerrit.
  subprocess.check_call('%s upload --verify' % repo_binary, shell=True)

  # Remove repo branch
  subprocess.check_call('%s abandon flag' % repo_binary, shell=True)


def add_to_chromium(args):
  os.chdir(args.chromium_dir)

  branch = subprocess.check_output(['git', 'rev-parse', '--abbrev-ref', 'HEAD'])
  branch = branch.strip()

  EXPECTED_STASH_OUT = "No local changes to save"
  stash_output = subprocess.check_output(['git', 'stash']).strip()

  if branch != "master" or stash_output != EXPECTED_STASH_OUT:
    print ("Please checkout a clean master branch at your chromium repo (%s) "
        "before running this script") % args.chromium_dir
    if stash_output != EXPECTED_STASH_OUT:
      subprocess.check_call(['git', 'stash', 'pop'])
    exit()

  # Use random number to avoid branch name colission.
  # We'll delete the branch in the end.
  random = randint(1, 10000)
  subprocess.check_call(['git', 'checkout', '-b', 'legacyflag_%d' % random])

  config_file = os.path.join('skia', 'config', 'SkUserConfig.h')
  with open(config_file) as f:
    lines = f.readlines()

  separator = (
    "///////////////////////// Imported from BUILD.gn and skia_common.gypi\n")
  index = lines.index(separator)

  lines.insert(index, "#ifndef {0}\n#define {0}\n#endif\n\n".format(args.flag))

  with open(config_file, 'w') as f:
    for line in lines:
      f.write(line)

  subprocess.check_call('git commit -a -m "Add %s"' % args.flag, shell=True)
  subprocess.check_call('git cl upload -m "Add %s" -f' % args.flag, shell=True)

  subprocess.check_call(['git', 'checkout', 'master'])
  subprocess.check_call(['git', 'branch', '-D', 'legacyflag_%d' % random])


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument(
      '--android-dir', '-a', required=True,
      help='Directory where an Android checkout will be created (if it does '
           'not already exist). Note: ~1GB space will be used.')
  parser.add_argument(
      '--chromium-dir', '-c', required=True,
      help='Directory of an EXISTING Chromium checkout (e.g., ~/chromium/src')
  parser.add_argument(
      '--google3', '-g', required=True,
      help='Google3 workspace to be created (if it does not already exist).')
  parser.add_argument('flag', type=str, help='legacy flag name')

  args = parser.parse_args()
  args.android_dir = os.path.expanduser(args.android_dir)
  args.chromium_dir = os.path.expanduser(args.chromium_dir)

  add_to_chromium(args)
  add_to_android(args)
  add_to_google3(args)


if __name__ == '__main__':
  main()
