#!/usr/bin/python

# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""
Script to build the shaderc library.
"""

import argparse
import os
import shlex
import shutil
import subprocess
import sys

def main():
  parser = argparse.ArgumentParser(description='Builds shaderc')
  parser.add_argument('-s', '--src-dir', required=True, help=
      'path to shaderc source')
  parser.add_argument('-t', '--build-type', required=True, help=
      'Either Release or Debug')
  parser.add_argument('-a', '--arch-type', required=True, help=
      'Either x86 or x86_64')
  parser.add_argument('-o', '--output-dir', required=True, help=
      'Directory for cmake build')
  parser.add_argument('-p', '--project_type', required=True, help=
      'Project type to use. Must be "ninja", "MSVS2013", or "MSVS2015"')
  args = parser.parse_args()

  args.src_dir = os.path.abspath(args.src_dir)
  args.output_dir = os.path.abspath(args.output_dir)

  if not os.path.isdir(args.src_dir):
    sys.exit(args.src_dir + ' is not a directory.')
    
  if args.build_type != 'Debug' and args.build_type != 'Release':
    sys.exit('Invalid build type: ' + args.build_type);

  if args.arch_type == 'x86':
    vs_arch = ''
  elif args.arch_type == 'x86_64':
    vs_arch = ' Win64'
  else:
    sys.exit('Invalid arch type: ' + args.arch_type);

  if args.project_type == 'ninja':
    generator = 'Ninja'
  elif args.project_type == 'MSVS2013':
    generator = 'Visual Studio 12 2013' + vs_arch
  elif args.project_type == "MSVS2015":
    generator = 'Visual Studio 14 2015' + vs_arch
  else:
    sys.exit('Invalid project type: ' + args.project_type);

  if not os.path.isdir(args.output_dir):
    try:
      os.makedirs(args.output_dir)
    except os.error:
      sys.exit('Error creating output dir ' + args.output_dir)

  try:
    build_type_arg='-DCMAKE_BUILD_TYPE=' + args.build_type
    subprocess.check_call(['cmake', '-G', generator,
        '-DSPIRV_SKIP_EXECUTABLES=ON', '-DSHADERC_ENABLE_SHARED_CRT=ON',
        args.src_dir, build_type_arg], cwd=args.output_dir)
  except subprocess.CalledProcessError as error:
    sys.exit('Error (ret code: {code}) calling "{cmd}" in {dir}'.format(
        code = error.returncode, cmd = error.cmd, dir = args.src_dir))

  try:
    subprocess.check_call(['cmake', '--build', args.output_dir, '--config',
        args.build_type], cwd=args.output_dir)
  except subprocess.CalledProcessError as error:
    sys.exit('Error (ret code: {code}) calling "{cmd}" in {dir}'.format(
        code = error.returncode, cmd = error.cmd, dir = args.src_dir))

if __name__ == '__main__':
  main()

