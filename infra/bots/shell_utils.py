#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

""" This module contains tools for running commands in a shell. """

import datetime
import os
import Queue
import select
import subprocess
import sys
import threading
import time

if 'nt' in os.name:
  import ctypes


DEFAULT_SECS_BETWEEN_ATTEMPTS = 10
POLL_MILLIS = 250
VERBOSE = True


class CommandFailedException(Exception):
  """Exception which gets raised when a command fails."""

  def __init__(self, output, *args):
    """Initialize the CommandFailedException.

    Args:
        output: string; output from the command.
    """
    Exception.__init__(self, *args)
    self._output = output

  @property
  def output(self):
    """Output from the command."""
    return self._output


class TimeoutException(CommandFailedException):
  """CommandFailedException which gets raised when a subprocess exceeds its
  timeout. """
  pass


def run_async(cmd, echo=None, shell=False):
  """ Run 'cmd' in a subprocess, returning a Popen class instance referring to
  that process.  (Non-blocking) """
  if echo is None:
    echo = VERBOSE
  if echo:
    print ' '.join(cmd) if isinstance(cmd, list) else cmd
  if 'nt' in os.name:
    # Windows has a bad habit of opening a dialog when a console program
    # crashes, rather than just letting it crash.  Therefore, when a program
    # crashes on Windows, we don't find out until the build step times out.
    # This code prevents the dialog from appearing, so that we find out
    # immediately and don't waste time waiting around.
    SEM_NOGPFAULTERRORBOX = 0x0002
    ctypes.windll.kernel32.SetErrorMode(SEM_NOGPFAULTERRORBOX)
    flags = 0x8000000 # CREATE_NO_WINDOW
  else:
    flags = 0
  return subprocess.Popen(cmd, shell=shell, stderr=subprocess.STDOUT,
                          stdout=subprocess.PIPE, creationflags=flags,
                          bufsize=1)


class EnqueueThread(threading.Thread):
  """ Reads and enqueues lines from a file. """
  def __init__(self, file_obj, queue):
    threading.Thread.__init__(self)
    self._file = file_obj
    self._queue = queue
    self._stopped = False

  def run(self):
    if sys.platform.startswith('linux'):
      # Use a polling object to avoid the blocking call to readline().
      poll = select.poll()
      poll.register(self._file, select.POLLIN)
      while not self._stopped:
        has_output = poll.poll(POLL_MILLIS)
        if has_output:
          line = self._file.readline()
          if line == '':
            self._stopped = True
          self._queue.put(line)
    else:
      # Only Unix supports polling objects, so just read from the file,
      # Python-style.
      for line in iter(self._file.readline, ''):
        self._queue.put(line)
        if self._stopped:
          break

  def stop(self):
    self._stopped = True


def log_process_in_real_time(proc, echo=None, timeout=None, log_file=None,
                             halt_on_output=None, print_timestamps=True):
  """ Log the output of proc in real time until it completes. Return a tuple
  containing the exit code of proc and the contents of stdout.

  proc: an instance of Popen referring to a running subprocess.
  echo: boolean indicating whether to print the output received from proc.stdout
  timeout: number of seconds allotted for the process to run. Raises a
      TimeoutException if the run time exceeds the timeout.
  log_file: an open file for writing output
  halt_on_output: string; kill the process and return if this string is found
      in the output stream from the process.
  print_timestamps: boolean indicating whether a formatted timestamp should be
      prepended to each line of output.
  """
  if echo is None:
    echo = VERBOSE
  stdout_queue = Queue.Queue()
  log_thread = EnqueueThread(proc.stdout, stdout_queue)
  log_thread.start()
  try:
    all_output = []
    t_0 = time.time()
    while True:
      code = proc.poll()
      try:
        output = stdout_queue.get_nowait()
        all_output.append(output)
        if output and print_timestamps:
          timestamp = datetime.datetime.now().strftime('%H:%M:%S.%f')
          output = ''.join(['[%s] %s\n' % (timestamp, line)
                            for line in output.splitlines()])
        if echo:
          sys.stdout.write(output)
          sys.stdout.flush()
        if log_file:
          log_file.write(output)
          log_file.flush()
        if halt_on_output and halt_on_output in output:
          proc.terminate()
          break
      except Queue.Empty:
        if code != None: # proc has finished running
          break
        time.sleep(0.5)
      if timeout and time.time() - t_0 > timeout:
        proc.terminate()
        raise TimeoutException(
            ''.join(all_output),
            'Subprocess exceeded timeout of %ds' % timeout)
  finally:
    log_thread.stop()
    log_thread.join()
  return (code, ''.join(all_output))


def log_process_after_completion(proc, echo=None, timeout=None,
                                 log_file=None):
  """ Wait for proc to complete and return a tuple containing the exit code of
  proc and the contents of stdout. Unlike log_process_in_real_time, does not
  attempt to read stdout from proc in real time.

  proc: an instance of Popen referring to a running subprocess.
  echo: boolean indicating whether to print the output received from proc.stdout
  timeout: number of seconds allotted for the process to run. Raises a
      TimeoutException if the run time exceeds the timeout.
  log_file: an open file for writing outout
  """
  if echo is None:
    echo = VERBOSE
  t_0 = time.time()
  code = None
  while code is None:
    if timeout and time.time() - t_0 > timeout:
      raise TimeoutException(
          proc.communicate()[0],
          'Subprocess exceeded timeout of %ds' % timeout)
    time.sleep(0.5)
    code = proc.poll()
  output = proc.communicate()[0]
  if echo:
    print output
  if log_file:
    log_file.write(output)
    log_file.flush()
  return (code, output)


def run(cmd, echo=None, shell=False, timeout=None, print_timestamps=True,
        log_in_real_time=True):
  """ Run 'cmd' in a shell and return the combined contents of stdout and
  stderr (Blocking).  Throws an exception if the command exits non-zero.

  cmd: list of strings (or single string, iff shell==True) indicating the
      command to run
  echo: boolean indicating whether we should print the command and log output
  shell: boolean indicating whether we are using advanced shell features. Use
      only when absolutely necessary, since this allows a lot more freedom which
      could be exploited by malicious code. See the warning here:
      http://docs.python.org/library/subprocess.html#popen-constructor
  timeout: optional, integer indicating the maximum elapsed time in seconds
  print_timestamps: boolean indicating whether a formatted timestamp should be
      prepended to each line of output. Unused if echo or log_in_real_time is
      False.
  log_in_real_time: boolean indicating whether to read stdout from the
      subprocess in real time instead of when the process finishes. If echo is
      False, we never log in real time, even if log_in_real_time is True.
  """
  if echo is None:
    echo = VERBOSE
  proc = run_async(cmd, echo=echo, shell=shell)
  # If we're not printing the output, we don't care if the output shows up in
  # real time, so don't bother.
  if log_in_real_time and echo:
    (returncode, output) = log_process_in_real_time(proc, echo=echo,
        timeout=timeout, print_timestamps=print_timestamps)
  else:
    (returncode, output) = log_process_after_completion(proc, echo=echo,
                                                        timeout=timeout)
  if returncode != 0:
    raise CommandFailedException(
        output,
        'Command failed with code %d: %s' % (returncode, cmd))
  return output


def run_retry(cmd, echo=None, shell=False, attempts=1,
              secs_between_attempts=DEFAULT_SECS_BETWEEN_ATTEMPTS,
              timeout=None, print_timestamps=True):
  """ Wrapper for run() which makes multiple attempts until either the command
  succeeds or the maximum number of attempts is reached. """
  if echo is None:
    echo = VERBOSE
  attempt = 1
  while True:
    try:
      return run(cmd, echo=echo, shell=shell, timeout=timeout,
                 print_timestamps=print_timestamps)
    except CommandFailedException:
      if attempt >= attempts:
        raise
    print 'Command failed. Retrying in %d seconds...' % secs_between_attempts
    time.sleep(secs_between_attempts)
    attempt += 1
