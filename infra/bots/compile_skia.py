#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import common
import sys


def main():
  if len(sys.argv) != 3:
    print >> sys.stderr, 'Usage: compile_skia.py <builder name> <out-dir>'
    sys.exit(1)
  bot = common.BotInfo(sys.argv[1], 'fake-slave', sys.argv[2])
  for t in bot.build_targets:
    bot.flavor.compile(t)


if __name__ == '__main__':
  main()
