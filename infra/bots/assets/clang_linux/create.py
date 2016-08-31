#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Create a Clang toolchain for Linux hosts."""


import argparse
import os
import subprocess
import tempfile

REPO = "https://llvm.googlesource.com/"
BRANCH = "release_39"

def create_asset(target_dir):
  os.chdir(tempfile.mkdtemp())
  subprocess.check_call(["git", "clone", "-b", BRANCH, REPO + "llvm"])
  os.chdir("llvm/tools")
  subprocess.check_call(["git", "clone", "-b", BRANCH, REPO + "clang"])
  os.chdir("../projects")
  subprocess.check_call(["git", "clone", "-b", BRANCH, REPO + "compiler-rt"])
  subprocess.check_call(["git", "clone", "-b", BRANCH, REPO + "libcxx"])
  subprocess.check_call(["git", "clone", "-b", BRANCH, REPO + "libcxxabi"])
  os.chdir("..")
  os.mkdir("out")
  os.chdir("out")
  subprocess.check_call(["cmake", "..", "-G", "Ninja",
                         "-DCMAKE_BUILD_TYPE=MinSizeRel",
                         "-DCMAKE_INSTALL_PREFIX=" + target_dir,
                         "-DLLVM_INSTALL_TOOLCHAIN_ONLY=ON",
                         "-DLLVM_ENABLE_TERMINFO=OFF"])
  subprocess.check_call(["cmake", "--build", "."])
  subprocess.check_call(["cmake", "--build", ".", "--target", "install"])
  subprocess.check_call(["cp", "bin/llvm-symbolizer", target_dir + "/bin"])

  libstdcpp = subprocess.check_output(["c++",
                                       "-print-file-name=libstdc++.so.6"])
  subprocess.check_call(["cp", libstdcpp.strip(), target_dir + "/lib"])


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
