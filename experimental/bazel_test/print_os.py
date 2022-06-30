#!/usr/bin/python3

'''
Copyright 2022 Google LLC

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
'''

import os
import platform
import sys

def main():
  print("hello", sys.argv)
  print(os.name, platform.system(), platform.release())
  if len(sys.argv) > 1:
    with open(sys.argv[1], "w") as f:
      f.write("created")


if __name__ == "__main__":
  main()