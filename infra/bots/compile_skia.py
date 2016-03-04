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
import utils


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--builder_name', required=True)
  parser.add_argument('--swarm_out_dir', required=True)
  args = parser.parse_args()
  with utils.print_timings():
    bot = common.BotInfo(args.builder_name, os.path.abspath(args.swarm_out_dir))
    bot.compile_steps()


if __name__ == '__main__':
  main()
