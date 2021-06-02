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


BRANCH = "release/10.x"


def create_asset(target_dir):
  # CMake will sometimes barf if we pass it a relative path.
  target_dir = os.path.abspath(target_dir)

  # Build Clang, lld, compiler-rt (sanitizer support) and libc++.
  os.chdir(tempfile.mkdtemp())
  subprocess.check_call(["git", "clone", "--depth", "1", "-b", BRANCH,
                         "https://llvm.googlesource.com/llvm-project"])
  os.chdir("llvm-project")
  os.mkdir("out")
  os.chdir("out")
  subprocess.check_call(["cmake", "../llvm", "-G", "Ninja",
                         "-DCMAKE_BUILD_TYPE=MinSizeRel",
                         "-DCMAKE_INSTALL_PREFIX=" + target_dir,
                         "-DLLVM_ENABLE_PROJECTS=clang;clang-tools-extra;" +
                             "compiler-rt;libcxx;libcxxabi;lld",
                         "-DLLVM_INSTALL_TOOLCHAIN_ONLY=ON",
                         "-DLLVM_ENABLE_TERMINFO=OFF"])
  subprocess.check_call(["ninja", "install"])

  # Copy a couple extra files we need.
  subprocess.check_call(["cp", "bin/llvm-symbolizer", target_dir + "/bin"])
  subprocess.check_call(["cp", "bin/llvm-profdata", target_dir + "/bin"])
  subprocess.check_call(["cp", "bin/llvm-cov", target_dir + "/bin"])
  libstdcpp = subprocess.check_output(["c++",
                                       "-print-file-name=libstdc++.so.6"])
  subprocess.check_call(["cp", libstdcpp.strip(), target_dir + "/lib"])

  # Finally, build libc++ for TSAN and MSAN bots using the Clang we just built.
  for (short,full) in [('tsan','Thread'), ('msan','MemoryWithOrigins')]:
    os.mkdir("../{}_out".format(short))
    os.chdir("../{}_out".format(short))
    subprocess.check_call(
        ["cmake", "../llvm", "-G", "Ninja",
         "-DCMAKE_BUILD_TYPE=MinSizeRel",
         "-DCMAKE_C_COMPILER="   + target_dir + "/bin/clang",
         "-DCMAKE_CXX_COMPILER=" + target_dir + "/bin/clang++",
         "-DLLVM_ENABLE_PROJECTS=libcxx;libcxxabi",
         "-DLLVM_USE_SANITIZER={}".format(full)])
    subprocess.check_call(["ninja", "cxx"])
    subprocess.check_call(["cp", "-r", "lib",  target_dir + "/" + short])


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--target_dir', '-t', required=True)
  args = parser.parse_args()
  create_asset(args.target_dir)


if __name__ == '__main__':
  main()
