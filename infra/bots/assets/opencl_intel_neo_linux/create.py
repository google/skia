#!/usr/bin/env python
#
# Copyright 2018 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create the asset."""


import argparse
import common
import multiprocessing
import os
import shutil
import subprocess
import utils

def create_asset(target_dir):
  """Create the asset."""
  # Check out and build the Intel NEO driver. Following instructions here:
  # https://github.com/intel/compute-runtime/blob/master/documentation/BUILD_Ubuntu.md
  with utils.tmp_dir():
    # Install build deps.
    neo_build_deps = ['ccache', 'flex', 'bison', 'clang-4.0', 'cmake', 'g++',
                      'git', 'patch', 'zlib1g-dev', 'autoconf', 'xutils-dev',
                      'libtool', 'pkg-config', 'libpciaccess-dev']
    apt_get_cmd = ['sudo', 'apt-get', 'install'] + neo_build_deps
    print 'Running "%s"' % ' '.join(apt_get_cmd)
    subprocess.check_call(apt_get_cmd)
    # Check out repos.
    for [repo, branch, local_name] in [
        ['llvm-mirror/clang',             'release_40', 'clang_source'],
        ['intel/opencl-clang',            'master',     'common_clang'],
        ['intel/llvm-patches',            'master',     'llvm_patches'],
        ['llvm-mirror/llvm',              'release_40', 'llvm_source'],
        ['intel/gmmlib',                  'master',     'gmmlib'],
        ['intel/intel-graphics-compiler', 'master',     'igc'],
        ['KhronosGroup/OpenCL-Headers',   'master',     'opencl_headers'],
        ['intel/compute-runtime',         'master',     'neo']
    ]:
      subprocess.check_call(['git', 'clone', '--depth', '1', '--branch', branch,
                             'https://github.com/' + repo, local_name])
    # Configure the build.
    build_dir = os.path.join(os.getcwd(), 'build')
    os.mkdir(build_dir)
    os.chdir(build_dir)
    subprocess.check_call(['cmake', '-DBUILD_TYPE=Release',
                           '-DCMAKE_BUILD_TYPE=Release', '../neo'])
    # Build and package the library.
    subprocess.check_call(['make', '-j%d' % multiprocessing.cpu_count(),
                           'package'])
    # Extract library and move necessary files to target_dir. We ignore the ICD
    # file because it's generated on the bot after we know the path to the CIPD
    # package.
    subprocess.check_call(['dpkg-deb', '--extract',
                           'intel-opencl-1.0-0.x86_64-igdrcl.deb', build_dir])
    lib_dir = os.path.join(build_dir, 'usr', 'local', 'lib')
    for f in os.listdir(lib_dir):
      shutil.move(os.path.join(lib_dir, f), target_dir)


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
