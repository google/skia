#!/usr/bin/env python
# Copyright (c) 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

README = """
Automatically add or remove a specific legacy flag to multiple Skia client repos.

This would only work on Google desktop.

Example usage:
  $ python toggle_legacy_flag.py SK_SUPPORT_LEGACY_SOMETHING \\
      -a /data/android -c ~/chromium/src -g legacyflag

If you only need to add the flag to one repo, for example, Android, please give
only -a (--android-dir) argument:
  $ python toggle_legacy_flag.py SK_SUPPORT_LEGACY_SOMETHING -a /data/android

"""

import os, sys
import argparse
import subprocess
import getpass
from random import randint


ANDROID_TOOLS_DIR = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    'android')


def toggle_android(args):
  sys.path.append(ANDROID_TOOLS_DIR)
  import upload_to_android

  modifier = upload_to_android.AndroidLegacyFlagModifier(args.flag)
  upload_to_android.upload_to_android(args.android_dir, modifier)


def toggle_chromium(args):
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
    exit(1)

  # Update the repository to avoid conflicts
  subprocess.check_call(['git', 'pull'])
  subprocess.check_call(['gclient', 'sync']);

  # Use random number to avoid branch name collision.
  # We'll delete the branch in the end.
  random = randint(1, 10000)
  subprocess.check_call(['git', 'checkout', '-b', 'legacyflag_%d' % random])

  try:
    config_file = os.path.join('skia', 'config', 'SkUserConfig.h')
    with open(config_file) as f:
      lines = f.readlines()

    flag_line = "#define %s\n" % args.flag
    if flag_line in lines:
      index = lines.index(flag_line)
      del lines[index-1 : index +2]
      verb = "Remove"
    else:
      separator = (
        "/////////////////////////"
        " Imported from BUILD.gn and skia_common.gypi\n")
      content = ("#ifndef {0}\n"
                 "#define {0}\n"
                 "#endif\n\n").format(args.flag)
      lines.insert(lines.index(separator), content)
      verb = "Add"

    with open(config_file, 'w') as f:
      for line in lines:
        f.write(line)

    message = "%s %s" % (verb, args.flag)

    subprocess.check_call('git commit -a -m "%s"' % message, shell=True)
    subprocess.check_call('git cl upload -m "%s" -f' % message,
                          shell=True)
  finally:
    subprocess.check_call(['git', 'checkout', 'master'])
    subprocess.check_call(['git', 'branch', '-D', 'legacyflag_%d' % random])


def toggle_google3(args):
  G3_SCRIPT_DIR = os.path.expanduser("~/skia-g3/scripts")
  if not os.path.isdir(G3_SCRIPT_DIR):
    print ("Google3 directory unavailable.\n"
           "Please see "
           "https://sites.google.com/a/google.com/skia/rebaseline#g3_flag "
           "for Google3 setup.")
    exit(1)
  sys.path.append(G3_SCRIPT_DIR)
  import citc_flag

  citc_flag.toggle_google3(args.google3, args.flag)


def main():
  if len(sys.argv) <= 1 or sys.argv[1] == '-h' or sys.argv[1] == '--help':
    print README

  parser = argparse.ArgumentParser()
  parser.add_argument(
      '--android-dir', '-a', required=False,
      help='Directory where an Android checkout will be created (if it does '
           'not already exist). Note: ~1GB space will be used.')
  parser.add_argument(
      '--chromium-dir', '-c', required=False,
      help='Directory of an EXISTING Chromium checkout (e.g., ~/chromium/src)')
  parser.add_argument(
      '--google3', '-g', required=False,
      help='Google3 workspace to be created (if it does not already exist).')
  parser.add_argument('flag', type=str, help='legacy flag name')

  args = parser.parse_args()

  if not args.android_dir and not args.chromium_dir and not args.google3:
    print """
Nothing to do. Please give me at least one of these three arguments:
  -a (--android-dir)
  -c (--chromium-dir)
  -g (--google3)
"""
    exit(1)

  end_message = "CLs generated. Now go review and land them:\n"
  if args.chromium_dir:
    args.chromium_dir = os.path.expanduser(args.chromium_dir)
    toggle_chromium(args)
    end_message += " * https://chromium-review.googlesource.com\n"
  if args.google3:
    toggle_google3(args)
    end_message += " * http://goto.google.com/cl\n"
  if args.android_dir:
    args.android_dir = os.path.expanduser(args.android_dir)
    toggle_android(args)
    end_message += " * http://goto.google.com/androidcl\n"

  print end_message


if __name__ == '__main__':
  main()
