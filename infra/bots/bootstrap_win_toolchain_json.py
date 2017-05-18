#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Resolve the path placeholders in the win_toolchain.json file."""


import argparse
import win_toolchain_utils


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--win_toolchain_json', required=True)
  parser.add_argument('--depot_tools_parent_dir', required=True)
  args = parser.parse_args()
  win_toolchain_utils.resolve(args.win_toolchain_json,
                              args.depot_tools_parent_dir)


if __name__ == '__main__':
  main()
