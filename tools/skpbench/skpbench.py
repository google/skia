#!/usr/bin/env python

# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from _benchresult import BenchResult
from argparse import ArgumentParser
from queue import Queue
from threading import Thread
import collections
import glob
import math
import re
import subprocess
import sys

__argparse = ArgumentParser(description="""

Executes the skpbench binary with various configs and skps.

Also monitors the output in order to filter out and re-run results that have an
unacceptable stddev.

""")

__argparse.add_argument('--adb',
    action='store_true', help='execute skpbench over adb')
__argparse.add_argument('-s', '--device-serial',
    help='if using adb, id of the specific device to target')
__argparse.add_argument('-p', '--path',
    help='directory to execute ./skpbench from')
__argparse.add_argument('-m', '--max-stddev',
    type=float, default=4,
    help='initial max allowable relative standard deviation')
__argparse.add_argument('-x', '--suffix',
    help='suffix to append on config (e.g. "_before", "_after")')
__argparse.add_argument('-w','--write-path',
    help='directory to save .png proofs to disk.')
__argparse.add_argument('-v','--verbosity',
    type=int, default=0, help='level of verbosity (0=none to 5=debug)')
__argparse.add_argument('-n', '--samples',
    type=int, help='number of samples to collect for each bench')
__argparse.add_argument('-d', '--sample-ms',
    type=int, help='duration of each sample')
__argparse.add_argument('--fps',
    action='store_true', help='use fps instead of ms')
__argparse.add_argument('-c', '--config',
    default='gpu', help='comma- or space-separated list of GPU configs')
__argparse.add_argument('skps',
    nargs='+',
    help='.skp files or directories to expand for .skp files')

FLAGS = __argparse.parse_args()
if FLAGS.adb:
  import _adb_path as _path
  _path.set_device_serial(FLAGS.device_serial)
else:
  import _os_path as _path


class StddevException(Exception):
  pass

class Message:
  READLINE = 0,
  EXIT = 1
  def __init__(self, message, value=None):
    self.message = message
    self.value = value

class SKPBench(Thread):
  ARGV = ['skpbench', '--verbosity', str(FLAGS.verbosity)]
  if FLAGS.samples:
    ARGV.extend(['--samples', str(FLAGS.samples)])
  if FLAGS.sample_ms:
    ARGV.extend(['--sampleMs', str(FLAGS.sample_ms)])
  if FLAGS.fps:
    ARGV.extend(['--fps', 'true'])
  if FLAGS.path:
    ARGV[0] = _path.join(FLAGS.path, ARGV[0])
  if FLAGS.adb:
    if FLAGS.device_serial is None:
      ARGV = ['adb', 'shell'] + ARGV
    else:
      ARGV = ['adb', '-s', FLAGS.device_serial, 'shell'] + ARGV

  @classmethod
  def print_header(cls):
    subprocess.call(cls.ARGV + ['--samples', '0'])

  def __init__(self, skp, config, max_stddev, best_result=None):
    self.skp = skp
    self.config = config
    self.max_stddev = max_stddev
    self.best_result = best_result
    self._queue = Queue()
    Thread.__init__(self)

  def execute(self):
    self.start()
    while True:
      message = self._queue.get()
      if message.message == Message.READLINE:
        result = BenchResult.match(message.value)
        if result:
          self.__process_result(result)
        else:
          print(message.value)
        sys.stdout.flush()
        continue
      if message.message == Message.EXIT:
        self.join()
        break

  def __process_result(self, result):
    if not self.best_result or result.stddev <= self.best_result.stddev:
      self.best_result = result
    elif FLAGS.verbosity >= 1:
      print('NOTE: reusing previous result for %s/%s with lower stddev '
            '(%s%% instead of %s%%).' %
            (result.config, result.bench, self.best_result.stddev,
             result.stddev), file=sys.stderr)
    if self.max_stddev and self.best_result.stddev > self.max_stddev:
      raise StddevException()
    self.best_result.print_values(config_suffix=FLAGS.suffix)

  def run(self):
    """Called on the background thread.

    Launches and reads output from an skpbench process.

    """
    commandline = self.ARGV + ['--config', self.config,
                               '--skp', self.skp,
                               '--suppressHeader', 'true']
    if (FLAGS.write_path):
      pngfile = _path.join(FLAGS.write_path, self.config,
                           _path.basename(self.skp) + '.png')
      commandline.extend(['--png', pngfile])
    if (FLAGS.verbosity >= 3):
      print(' '.join(commandline), file=sys.stderr)
    proc = subprocess.Popen(commandline, stdout=subprocess.PIPE)
    for line in iter(proc.stdout.readline, b''):
      self._queue.put(Message(Message.READLINE, line.decode('utf-8').rstrip()))
    proc.wait()
    self._queue.put(Message(Message.EXIT, proc.returncode))


def main():
  SKPBench.print_header()

  # Delimiter is "," or " ", skip if nested inside parens (e.g. gpu(a=b,c=d)).
  DELIMITER = r'[, ](?!(?:[^(]*\([^)]*\))*[^()]*\))'
  configs = re.split(DELIMITER, FLAGS.config)
  skps = _path.find_skps(FLAGS.skps)

  benches = collections.deque([(skp, config, FLAGS.max_stddev)
                               for skp in skps
                               for config in configs])
  while benches:
    benchargs = benches.popleft()
    skpbench = SKPBench(*benchargs)
    try:
      skpbench.execute()

    except StddevException:
      retry_max_stddev = skpbench.max_stddev * math.sqrt(2)
      if FLAGS.verbosity >= 1:
        print('NOTE: stddev too high for %s/%s (%s%%; max=%.2f%%). '
              'Re-queuing with max=%.2f%%.' %
              (skpbench.best_result.config, skpbench.best_result.bench,
               skpbench.best_result.stddev, skpbench.max_stddev,
               retry_max_stddev),
              file=sys.stderr)
      benches.append((skpbench.skp, skpbench.config, retry_max_stddev,
                      skpbench.best_result))


if __name__ == '__main__':
  main()
