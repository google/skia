#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import argparse
import common
import os
import sys


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--master_name', required=True)
  parser.add_argument('--builder_name', required=True)
  parser.add_argument('--build_number', required=True)
  parser.add_argument('--slave_name', required=True)
  parser.add_argument('--revision', required=True)
  parser.add_argument('--swarm_out_dir', required=True)
  args = parser.parse_args()
  bot = common.BotInfo(args.builder_name, os.path.abspath(args.swarm_out_dir))
  bot.test_steps(args.revision, args.master_name, args.slave_name,
                 args.build_number)


if __name__ == '__main__':
  main()
