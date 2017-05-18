#!/usr/bin/env python

# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from _adb import Adb
from _benchresult import BenchResult
from _hardware import HardwareException, Hardware
from argparse import ArgumentParser
from multiprocessing import Queue
from threading import Thread, Timer
import collections
import glob
import math
import re
import subprocess
import sys
import time

__argparse = ArgumentParser(description="""

Executes the skpbench binary with various configs and skps.

Also monitors the output in order to filter out and re-run results that have an
unacceptable stddev.

""")

__argparse.add_argument('skpbench',
  help="path to the skpbench binary")
__argparse.add_argument('--adb',
  action='store_true', help="execute skpbench over adb")
__argparse.add_argument('-s', '--device-serial',
  help="if using adb, ID of the specific device to target "
       "(only required if more than 1 device is attached)")
__argparse.add_argument('-m', '--max-stddev',
  type=float, default=4,
  help="initial max allowable relative standard deviation")
__argparse.add_argument('-x', '--suffix',
  help="suffix to append on config (e.g. '_before', '_after')")
__argparse.add_argument('-w','--write-path',
  help="directory to save .png proofs to disk.")
__argparse.add_argument('-v','--verbosity',
  type=int, default=1, help="level of verbosity (0=none to 5=debug)")
__argparse.add_argument('-d', '--duration',
  type=int, help="number of milliseconds to run each benchmark")
__argparse.add_argument('-l', '--sample-ms',
  type=int, help="duration of a sample (minimum)")
__argparse.add_argument('--gpu',
  action='store_true',
  help="perform timing on the gpu clock instead of cpu (gpu work only)")
__argparse.add_argument('--fps',
  action='store_true', help="use fps instead of ms")
__argparse.add_argument('-c', '--config',
  default='gl', help="comma- or space-separated list of GPU configs")
__argparse.add_argument('-a', '--resultsfile',
  help="optional file to append results into")
__argparse.add_argument('skps',
  nargs='+',
  help=".skp files or directories to expand for .skp files")

FLAGS = __argparse.parse_args()
if FLAGS.adb:
  import _adb_path as _path
  _path.init(FLAGS.device_serial)
else:
  import _os_path as _path

def dump_commandline_if_verbose(commandline):
  if FLAGS.verbosity >= 5:
    quoted = ['\'%s\'' % re.sub(r'([\\\'])', r'\\\1', x) for x in commandline]
    print(' '.join(quoted), file=sys.stderr)


class StddevException(Exception):
  pass

class Message:
  READLINE = 0,
  POLL_HARDWARE = 1,
  EXIT = 2
  def __init__(self, message, value=None):
    self.message = message
    self.value = value

class SubprocessMonitor(Thread):
  def __init__(self, queue, proc):
    self._queue = queue
    self._proc = proc
    Thread.__init__(self)

  def run(self):
    """Runs on the background thread."""
    for line in iter(self._proc.stdout.readline, b''):
      self._queue.put(Message(Message.READLINE, line.decode('utf-8').rstrip()))
    self._queue.put(Message(Message.EXIT))

class SKPBench:
  ARGV = [FLAGS.skpbench, '--verbosity', str(FLAGS.verbosity)]
  if FLAGS.duration:
    ARGV.extend(['--duration', str(FLAGS.duration)])
  if FLAGS.sample_ms:
    ARGV.extend(['--sampleMs', str(FLAGS.sample_ms)])
  if FLAGS.gpu:
    ARGV.extend(['--gpuClock', 'true'])
  if FLAGS.fps:
    ARGV.extend(['--fps', 'true'])
  if FLAGS.adb:
    if FLAGS.device_serial is None:
      ARGV[:0] = ['adb', 'shell']
    else:
      ARGV[:0] = ['adb', '-s', FLAGS.device_serial, 'shell']

  @classmethod
  def get_header(cls, outfile=sys.stdout):
    commandline = cls.ARGV + ['--duration', '0']
    dump_commandline_if_verbose(commandline)
    out = subprocess.check_output(commandline, stderr=subprocess.STDOUT)
    return out.rstrip()

  @classmethod
  def run_warmup(cls, warmup_time, config):
    if not warmup_time:
      return
    print('running %i second warmup...' % warmup_time, file=sys.stderr)
    commandline = cls.ARGV + ['--duration', str(warmup_time * 1000),
                              '--config', config,
                              '--skp', 'warmup']
    dump_commandline_if_verbose(commandline)
    output = subprocess.check_output(commandline, stderr=subprocess.STDOUT)

    # validate the warmup run output.
    for line in output.decode('utf-8').split('\n'):
      match = BenchResult.match(line.rstrip())
      if match and match.bench == 'warmup':
        return
    raise Exception('Invalid warmup output:\n%s' % output)

  def __init__(self, skp, config, max_stddev, best_result=None):
    self.skp = skp
    self.config = config
    self.max_stddev = max_stddev
    self.best_result = best_result
    self._queue = Queue()
    self._proc = None
    self._monitor = None
    self._hw_poll_timer = None

  def __enter__(self):
    return self

  def __exit__(self, exception_type, exception_value, traceback):
    if self._proc:
      self.terminate()
    if self._hw_poll_timer:
      self._hw_poll_timer.cancel()

  def execute(self, hardware):
    hardware.sanity_check()
    self._schedule_hardware_poll()

    commandline = self.ARGV + ['--config', self.config,
                               '--skp', self.skp,
                               '--suppressHeader', 'true']
    if FLAGS.write_path:
      pngfile = _path.join(FLAGS.write_path, self.config,
                           _path.basename(self.skp) + '.png')
      commandline.extend(['--png', pngfile])
    dump_commandline_if_verbose(commandline)
    self._proc = subprocess.Popen(commandline, stdout=subprocess.PIPE,
                                  stderr=subprocess.STDOUT)
    self._monitor = SubprocessMonitor(self._queue, self._proc)
    self._monitor.start()

    while True:
      message = self._queue.get()
      if message.message == Message.READLINE:
        result = BenchResult.match(message.value)
        if result:
          hardware.sanity_check()
          self._process_result(result)
        elif hardware.filter_line(message.value):
          print(message.value, file=sys.stderr)
        continue
      if message.message == Message.POLL_HARDWARE:
        hardware.sanity_check()
        self._schedule_hardware_poll()
        continue
      if message.message == Message.EXIT:
        self._monitor.join()
        self._proc.wait()
        if self._proc.returncode != 0:
          raise Exception("skpbench exited with nonzero exit code %i" %
                          self._proc.returncode)
        self._proc = None
        break

  def _schedule_hardware_poll(self):
    if self._hw_poll_timer:
      self._hw_poll_timer.cancel()
    self._hw_poll_timer = \
      Timer(1, lambda: self._queue.put(Message(Message.POLL_HARDWARE)))
    self._hw_poll_timer.start()

  def _process_result(self, result):
    if not self.best_result or result.stddev <= self.best_result.stddev:
      self.best_result = result
    elif FLAGS.verbosity >= 2:
      print("reusing previous result for %s/%s with lower stddev "
            "(%s%% instead of %s%%)." %
            (result.config, result.bench, self.best_result.stddev,
             result.stddev), file=sys.stderr)
    if self.max_stddev and self.best_result.stddev > self.max_stddev:
      raise StddevException()

  def terminate(self):
    if self._proc:
      self._proc.terminate()
      self._monitor.join()
      self._proc.wait()
      self._proc = None

def emit_result(line, resultsfile=None):
  print(line)
  sys.stdout.flush()
  if resultsfile:
    print(line, file=resultsfile)
    resultsfile.flush()

def run_benchmarks(configs, skps, hardware, resultsfile=None):
  emit_result(SKPBench.get_header(), resultsfile)
  benches = collections.deque([(skp, config, FLAGS.max_stddev)
                               for skp in skps
                               for config in configs])
  while benches:
    benchargs = benches.popleft()
    with SKPBench(*benchargs) as skpbench:
      try:
        skpbench.execute(hardware)
        if skpbench.best_result:
          emit_result(skpbench.best_result.format(FLAGS.suffix), resultsfile)
        else:
          print("WARNING: no result for %s with config %s" %
                (skpbench.skp, skpbench.config), file=sys.stderr)

      except StddevException:
        retry_max_stddev = skpbench.max_stddev * math.sqrt(2)
        if FLAGS.verbosity >= 1:
          print("stddev is too high for %s/%s (%s%%, max=%.2f%%), "
                "re-queuing with max=%.2f%%." %
                (skpbench.best_result.config, skpbench.best_result.bench,
                 skpbench.best_result.stddev, skpbench.max_stddev,
                 retry_max_stddev),
                file=sys.stderr)
        benches.append((skpbench.skp, skpbench.config, retry_max_stddev,
                        skpbench.best_result))

      except HardwareException as exception:
        skpbench.terminate()
        if FLAGS.verbosity >= 4:
          hardware.print_debug_diagnostics()
        if FLAGS.verbosity >= 1:
          print("%s; taking a %i second nap..." %
                (exception.message, exception.sleeptime), file=sys.stderr)
        benches.appendleft(benchargs) # retry the same bench next time.
        hardware.sleep(exception.sleeptime)
        if FLAGS.verbosity >= 4:
          hardware.print_debug_diagnostics()
        SKPBench.run_warmup(hardware.warmup_time, configs[0])

def main():
  # Delimiter is ',' or ' ', skip if nested inside parens (e.g. gpu(a=b,c=d)).
  DELIMITER = r'[, ](?!(?:[^(]*\([^)]*\))*[^()]*\))'
  configs = re.split(DELIMITER, FLAGS.config)
  skps = _path.find_skps(FLAGS.skps)

  if FLAGS.adb:
    adb = Adb(FLAGS.device_serial, echo=(FLAGS.verbosity >= 5))
    model = adb.check('getprop ro.product.model').strip()
    if model == 'Pixel C':
      from _hardware_pixel_c import HardwarePixelC
      hardware = HardwarePixelC(adb)
    elif model == 'Nexus 6P':
      from _hardware_nexus_6p import HardwareNexus6P
      hardware = HardwareNexus6P(adb)
    else:
      from _hardware_android import HardwareAndroid
      print("WARNING: %s: don't know how to monitor this hardware; results "
            "may be unreliable." % model, file=sys.stderr)
      hardware = HardwareAndroid(adb)
  else:
    hardware = Hardware()

  with hardware:
    SKPBench.run_warmup(hardware.warmup_time, configs[0])
    if FLAGS.resultsfile:
      with open(FLAGS.resultsfile, mode='a+') as resultsfile:
        run_benchmarks(configs, skps, hardware, resultsfile=resultsfile)
    else:
      run_benchmarks(configs, skps, hardware)


if __name__ == '__main__':
  main()
