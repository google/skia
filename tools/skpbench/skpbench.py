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
__argparse.add_argument('--adb_binary', default='adb',
  help="The name of the adb binary to use.")
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
__argparse.add_argument('--pr',
  help="comma- or space-separated list of GPU path renderers, including: "
       "[[~]all [~]default [~]dashline [~]msaa [~]aaconvex "
       "[~]aalinearizing [~]small [~]tess]")
__argparse.add_argument('--cc',
  action='store_true', help="allow coverage counting shortcuts to render paths")
__argparse.add_argument('--nocache',
  action='store_true', help="disable caching of path mask textures")
__argparse.add_argument('--allPathsVolatile',
  action='store_true',
  help="Causes all GPU paths to be processed as if 'setIsVolatile' had been called.")
__argparse.add_argument('-c', '--config',
  default='gl', help="comma- or space-separated list of GPU configs")
__argparse.add_argument('-a', '--resultsfile',
  help="optional file to append results into")
__argparse.add_argument('--ddl',
  action='store_true', help="record the skp into DDLs before rendering")
__argparse.add_argument('--lock-clocks',
  action='store_true', help="Put device in benchmarking mode (locked clocks, no other processes)")
__argparse.add_argument('--clock-speed',
  type=float, default=66.0, help="A number between 0 and 100 indicating how fast to lock the CPU and GPU clock."
  "Valid speeds are chosen from their respective available frequencies list.")
__argparse.add_argument('--ddlNumRecordingThreads',
  type=int, default=0,
  help="number of DDL recording threads (0=num_cores)")
__argparse.add_argument('--ddlTilingWidthHeight',
  type=int, default=0, help="number of tiles along one edge when in DDL mode")
__argparse.add_argument('--dontReduceOpsTaskSplitting',
  action='store_true', help="don't reorder GPU tasks to reduce render target swaps")
__argparse.add_argument('--gpuThreads',
  type=int, default=-1,
  help="Create this many extra threads to assist with GPU work, including"
       " software path rendering. Defaults to two.")
__argparse.add_argument('--internalSamples',
  type=int, default=-1,
  help="Number of samples for internal draws that use MSAA.")
__argparse.add_argument('srcs',
  nargs='+',
  help=".skp files or directories to expand for .skp files, and/or .svg files")
__argparse.add_argument('--gpuResourceCacheLimit',
  type=int, default=-1,
  help="Maximum number of bytes to use for budgeted GPU resources.")

FLAGS = __argparse.parse_args()
if FLAGS.adb:
  import _adb_path as _path
  _path.init(FLAGS.device_serial, FLAGS.adb_binary)
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
  if FLAGS.pr:
    ARGV.extend(['--pr'] + re.split(r'[ ,]', FLAGS.pr))
  if FLAGS.cc:
    ARGV.extend(['--cc', 'true'])
  if FLAGS.nocache:
    ARGV.extend(['--cachePathMasks', 'false'])
  if FLAGS.allPathsVolatile:
    ARGV.extend(['--allPathsVolatile', 'true'])
  if FLAGS.gpuThreads != -1:
    ARGV.extend(['--gpuThreads', str(FLAGS.gpuThreads)])
  if FLAGS.internalSamples != -1:
    ARGV.extend(['--internalSamples', str(FLAGS.internalSamples)])

  # DDL parameters
  if FLAGS.ddl:
    ARGV.extend(['--ddl', 'true'])
  if FLAGS.ddlNumRecordingThreads:
    ARGV.extend(['--ddlNumRecordingThreads',
                 str(FLAGS.ddlNumRecordingThreads)])
  if FLAGS.ddlTilingWidthHeight:
    ARGV.extend(['--ddlTilingWidthHeight', str(FLAGS.ddlTilingWidthHeight)])

  if FLAGS.dontReduceOpsTaskSplitting:
    ARGV.extend(['--dontReduceOpsTaskSplitting'])

  if FLAGS.gpuResourceCacheLimit:
    ARGV.extend(['--gpuResourceCacheLimit', str(FLAGS.gpuResourceCacheLimit)])

  if FLAGS.adb:
    if FLAGS.device_serial is None:
      ARGV[:0] = [FLAGS.adb_binary, 'shell']
    else:
      ARGV[:0] = [FLAGS.adb_binary, '-s', FLAGS.device_serial, 'shell']

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
                              '--src', 'warmup']
    dump_commandline_if_verbose(commandline)
    output = subprocess.check_output(commandline, stderr=subprocess.STDOUT)

    # validate the warmup run output.
    for line in output.decode('utf-8').split('\n'):
      match = BenchResult.match(line.rstrip())
      if match and match.bench == 'warmup':
        return
    raise Exception('Invalid warmup output:\n%s' % output)

  def __init__(self, src, config, max_stddev, best_result=None):
    self.src = src
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
                               '--src', self.src,
                               '--suppressHeader', 'true']
    if FLAGS.write_path:
      pngfile = _path.join(FLAGS.write_path, self.config,
                           _path.basename(self.src) + '.png')
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

def run_benchmarks(configs, srcs, hardware, resultsfile=None):
  hasheader = False
  benches = collections.deque([(src, config, FLAGS.max_stddev)
                               for src in srcs
                               for config in configs])
  while benches:
    try:
      with hardware:
        SKPBench.run_warmup(hardware.warmup_time, configs[0])
        if not hasheader:
          emit_result(SKPBench.get_header(), resultsfile)
          hasheader = True
        while benches:
          benchargs = benches.popleft()
          with SKPBench(*benchargs) as skpbench:
            try:
              skpbench.execute(hardware)
              if skpbench.best_result:
                emit_result(skpbench.best_result.format(FLAGS.suffix),
                            resultsfile)
              else:
                print("WARNING: no result for %s with config %s" %
                      (skpbench.src, skpbench.config), file=sys.stderr)

            except StddevException:
              retry_max_stddev = skpbench.max_stddev * math.sqrt(2)
              if FLAGS.verbosity >= 1:
                print("stddev is too high for %s/%s (%s%%, max=%.2f%%), "
                      "re-queuing with max=%.2f%%." %
                      (skpbench.best_result.config, skpbench.best_result.bench,
                       skpbench.best_result.stddev, skpbench.max_stddev,
                       retry_max_stddev),
                      file=sys.stderr)
              benches.append((skpbench.src, skpbench.config, retry_max_stddev,
                              skpbench.best_result))

            except HardwareException as exception:
              skpbench.terminate()
              if FLAGS.verbosity >= 4:
                hardware.print_debug_diagnostics()
              if FLAGS.verbosity >= 1:
                print("%s; rebooting and taking a %i second nap..." %
                      (exception.message, exception.sleeptime), file=sys.stderr)
              benches.appendleft(benchargs) # retry the same bench next time.
              raise # wake hw up from benchmarking mode before the nap.

    except HardwareException as exception:
      time.sleep(exception.sleeptime)

def main():
  # Delimiter is ',' or ' ', skip if nested inside parens (e.g. gpu(a=b,c=d)).
  DELIMITER = r'[, ](?!(?:[^(]*\([^)]*\))*[^()]*\))'
  configs = re.split(DELIMITER, FLAGS.config)
  srcs = _path.find_skps(FLAGS.srcs)
  assert srcs


  if FLAGS.adb:
    adb = Adb(FLAGS.device_serial, FLAGS.adb_binary,
              echo=(FLAGS.verbosity >= 5))
    from _hardware_android import HardwareAndroid

    model = adb.check('getprop ro.product.model').strip()
    if model == 'Pixel C':
      from _hardware_pixel_c import HardwarePixelC
      hardware = HardwarePixelC(adb)
    elif model == 'Pixel' or model == "Pixel XL":
      from _hardware_pixel import HardwarePixel
      hardware = HardwarePixel(adb)
    elif model == 'Pixel 2':
      from _hardware_pixel2 import HardwarePixel2
      hardware = HardwarePixel2(adb)
    elif model == 'Nexus 6P':
      from _hardware_nexus_6p import HardwareNexus6P
      hardware = HardwareNexus6P(adb)
    else:
      print("WARNING: %s: don't know how to monitor this hardware; results "
            "may be unreliable." % model, file=sys.stderr)
      hardware = HardwareAndroid(adb)

    if FLAGS.lock_clocks:
      hardware.__enter__()
      print("Entered benchmarking mode, not running benchmarks. Reboot to restore.");
      return;

    if FLAGS.clock_speed:
      hardware.setDesiredClock(FLAGS.clock_speed)
  else:
    hardware = Hardware()

  if FLAGS.resultsfile:
    with open(FLAGS.resultsfile, mode='a+') as resultsfile:
      run_benchmarks(configs, srcs, hardware, resultsfile=resultsfile)
  else:
    run_benchmarks(configs, srcs, hardware)


if __name__ == '__main__':
  main()
