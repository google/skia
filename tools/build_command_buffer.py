#!/usr/bin/python

# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""
Script to build the command buffer shared library and copy it to Skia tree
"""


import argparse
import os
import shlex
import shutil
import subprocess
import sys


def main():
  parser = argparse.ArgumentParser(description=('Builds command_buffer_gles2 '
                                                'library and copies it'))
  parser.add_argument('-c', '--chrome-dir', required=True, help=
      'path to Chromium checkout (directory containing .gclient)')
  parser.add_argument('-o', '--output-dir', required=True,
      help='path to copy the command buffer shared library to')
  parser.add_argument('--make-output-dir', default=False, action='store_true',
      help='Makes the output directory if it does not already exist.')
  parser.add_argument('-t', '--chrome-build-type', default='Release',
      help='Type of build for the command buffer (e.g. Debug or Release). The '
            'output dir to build will be <chrome-dir>/out/<chrome-build-type> '
            'and must already be initialized by gn.')
  parser.add_argument('--extra-ninja-args', default='',
      help=('Extra arguments to pass to ninja when building the command '
            'buffer shared library'))
  parser.add_argument('--chrome-revision', default='origin/lkgr',
      help='Revision (hash, branch, tag) of Chromium to use.')
  parser.add_argument('--no-sync', action='store_true', default=False,
      help='Don\'t run git fetch or gclient sync in the Chromium tree')
  parser.add_argument('--no-hooks', action='store_true', default=False,
      help='Don\'t run gclient runhooks in the Chromium tree. Implies '
           '--no-sync')
  args = parser.parse_args()

  args.chrome_dir = os.path.abspath(args.chrome_dir)
  args.output_dir = os.path.abspath(args.output_dir)

  if args.no_hooks:
     args.no_sync = True

  if os.path.isfile(args.chrome_dir):
    sys.exit(args.chrome_dir + ' exists but is a file.')

  if os.path.isfile(args.output_dir):
    sys.exit(args.output_dir + ' exists but is a file.')

  chrome_src_dir = os.path.join(args.chrome_dir, 'src')

  if not os.path.isdir(chrome_src_dir):
    sys.exit(chrome_src_dir + ' is not a directory.')

  if os.path.isfile(args.output_dir):
    sys.exit(args.output_dir + ' exists but is a file.')
  elif not os.path.isdir(args.output_dir):
    if args.make_output_dir:
      os.makedirs(args.output_dir)
    else:
      sys.exit(args.output_dir + ' does not exist (specify --make-output-dir '
          'to create).')

  chrome_target_dir_rel = os.path.join('out', args.chrome_build_type)

  # The command buffer shared library will have a different name on Linux,
  # Mac, and Windows. Also, on Linux it will be in a 'lib' subdirectory and
  # needs to be placed in a 'lib' subdirectory of the directory containing the
  # Skia executable. Also, the name of the gclient executable we call out to has
  # a .bat file extension on Windows.
  platform = sys.platform
  if platform == 'cygwin':
    platform = 'win32'

  shared_lib_name = 'libcommand_buffer_gles2.so'
  shared_lib_subdir = 'lib'
  gclient = 'gclient'
  if platform == 'darwin':
    shared_lib_name = 'libcommand_buffer_gles2.dylib'
    shared_lib_subdir = ''
  elif platform == 'win32':
    shared_lib_name = 'command_buffer_gles2.dll'
    shared_lib_subdir = ''
    gclient = 'gclient.bat'

  if not args.no_sync:
    try:
      subprocess.check_call(['git', 'fetch'], cwd=chrome_src_dir)
    except subprocess.CalledProcessError as error:
      sys.exit('Error (ret code: %s) calling "%s" in %s' % (error.returncode,
          error.cmd, chrome_src_dir))

    try:
      subprocess.check_call(['git', 'checkout', args.chrome_revision],
          cwd=chrome_src_dir)
    except subprocess.CalledProcessError as error:
      sys.exit('Error (ret code: %s) calling "%s" in %s' % (error.returncode,
          error.cmd, chrome_src_dir))

    try:
      os.environ['GYP_GENERATORS'] = 'ninja'
      subprocess.check_call([gclient, 'sync', '--reset', '--force',
                             '--nohooks'],
          cwd=chrome_src_dir)
    except subprocess.CalledProcessError as error:
      sys.exit('Error (ret code: %s) calling "%s" in %s' % (error.returncode,
          error.cmd, chrome_src_dir))

  if not args.no_hooks:
    try:
      subprocess.check_call([gclient, 'runhooks'], cwd=chrome_src_dir)
    except subprocess.CalledProcessError as error:
      sys.exit('Error (ret code: %s) calling "%s" in %s' % (
          error.returncode, error.cmd, chrome_src_dir))

  gn = 'gn'
  platform = 'linux64'
  if sys.platform == 'darwin':
    platform = 'mac'
  elif sys.platform == 'win32':
    platform = 'win'
    gn = 'gn.exe'
  gn = os.path.join(chrome_src_dir, 'buildtools', platform, gn)
  try:
    subprocess.check_call([gn, 'gen', chrome_target_dir_rel,
                           '--args=is_component_build=false'],
                          cwd=chrome_src_dir)
  except subprocess.CalledProcessError as error:
    sys.exit('Error (ret code: %s) calling "%s" in %s' % (
        error.returncode, error.cmd, chrome_src_dir))

  try:
    subprocess.check_call(['ninja'] + shlex.split(args.extra_ninja_args) +
        ['-C', chrome_target_dir_rel, 'command_buffer_gles2'],
        cwd=chrome_src_dir)
  except subprocess.CalledProcessError as error:
    sys.exit('Error (ret code: %s) calling "%s" in %s' % (error.returncode,
        error.cmd, chrome_src_dir))

  shared_lib_src_dir = os.path.join(chrome_src_dir, chrome_target_dir_rel)
  shared_lib_dst_dir = os.path.join(args.output_dir, shared_lib_subdir)
  # Make the subdir for the dst if does not exist
  if shared_lib_subdir and not os.path.isdir(shared_lib_dst_dir):
    os.mkdir(shared_lib_dst_dir)

  shared_lib_src = os.path.join(shared_lib_src_dir, shared_lib_name)
  shared_lib_dst = os.path.join(shared_lib_dst_dir, shared_lib_name)

  if not os.path.isfile(shared_lib_src):
    sys.exit('Command buffer shared library not at expected location: ' +
        shared_lib_src)

  shutil.copy2(shared_lib_src, shared_lib_dst)

  if not os.path.isfile(shared_lib_dst):
    sys.exit('Command buffer library not copied to ' + shared_lib_dst)

  print('Command buffer library copied to ' + shared_lib_dst)


if __name__ == '__main__':
  main()

