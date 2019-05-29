#!/usr/bin/env python
# Copyright (c) 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Script that triggers and waits for g3 compile tasks."""

import json
import math
import optparse
import os
import subprocess
import sys
import time

INFRA_BOTS_DIR = os.path.abspath(os.path.realpath(os.path.join(
    os.path.dirname(os.path.abspath(__file__)), os.pardir)))
sys.path.insert(0, INFRA_BOTS_DIR)
import utils


G3_COMPILE_BUCKET = 'g3-compile-tasks'

GS_RETRIES = 5
GS_RETRY_WAIT_BASE = 15

POLLING_FREQUENCY_SECS = 10
DEADLINE_SECS = 60 * 60  # 60 minutes.

INFRA_FAILURE_ERROR_MSG = (
    '\n\n'
    'Your run failed due to unknown infrastructure failures.\n'
    'Please contact rmistry@ or the trooper from '
    'http://skia-tree-status.appspot.com/trooper\n'
    'Sorry for the inconvenience!\n'
)
MISSING_APPROVAL_ERROR_MSG = (
    '\n\n'
    'To run the G3 tryjob, changes must be either owned and authored by \n'
    'Googlers or approved (Code-Review+1) by Googlers.\n'
)
MERGE_CONFLICT_ERROR_MSG = (
    '\n\n'
    'G3 tryjob failed because the change is causing a merge conflict when \n'
    'applying it to the Skia hash in G3.\n'
)


class G3CompileException(Exception):
  pass


def _create_task_dict(options):
  """Creates a dict representation of the requested task."""
  params = {}
  params['issue'] = options.issue
  params['patchset'] = options.patchset
  return params


def _get_gs_bucket():
  """Returns the Google storage bucket with the gs:// prefix."""
  return 'gs://%s' % G3_COMPILE_BUCKET


def _write_to_storage(task):
  """Writes the specified compile task to Google storage."""
  with utils.tmp_dir():
    json_file = os.path.join(os.getcwd(), _get_task_file_name(task))
    with open(json_file, 'w') as f:
      json.dump(task, f)
    subprocess.check_call(['gsutil', 'cp', json_file, '%s/' % _get_gs_bucket()])
    print 'Created %s/%s' % (_get_gs_bucket(), os.path.basename(json_file))


def _get_task_file_name(task):
  """Returns the file name of the compile task. Eg: ${issue}-${patchset}.json"""
  return '%s-%s.json' % (task['issue'], task['patchset'])


def _does_running_task_exist_in_storage(task):
  """Checks to see if the task file exists in storage and is running."""
  gs_file = '%s/%s' % (_get_gs_bucket(), _get_task_file_name(task))
  try:
    # Read without exponential backoff because it is unlikely that the file
    # already exists and we do not want to waste minutes every time.
    taskJSON = _read_from_storage(gs_file, use_expo_retries=False)
  except (subprocess.CalledProcessError, ValueError):
    return False
  if taskJSON.get('status'):
    print 'Task exists in Google storage and has completed.'
    return False
  print 'Task exists in Google storage and is still running.'
  return True


def _trigger_task(options):
  """Triggers a g3 compile task by creating a file in storage."""
  task = _create_task_dict(options)
  # Check to see if the task is already running in Google Storage.
  if not _does_running_task_exist_in_storage(task):
    _write_to_storage(task)
  return task


def _read_from_storage(gs_file, use_expo_retries=True):
  """Returns the contents of the specified file from storage."""
  num_retries = GS_RETRIES if use_expo_retries else 1
  for retry in range(num_retries):
    try:
      output = subprocess.check_output(['gsutil', 'cat', gs_file])
      ret = json.loads(output)
      return ret
    except (subprocess.CalledProcessError, ValueError), e:
      print 'Error when reading and loading %s: %s' % (gs_file, e)
      if retry == (num_retries-1):
        print '%d retries did not help' % num_retries
        raise
      waittime = GS_RETRY_WAIT_BASE * math.pow(2, retry)
      print 'Retry in %d seconds.' % waittime
      time.sleep(waittime)
      continue


def trigger_and_wait(options):
  """Triggers a g3 compile task and waits for it to complete."""
  task = _trigger_task(options)
  print 'G3 Compile Task for %d/%d has been successfully added to %s.' % (
      options.issue, options.patchset, G3_COMPILE_BUCKET)
  print '%s will be polled every %d seconds.' % (G3_COMPILE_BUCKET,
                                                 POLLING_FREQUENCY_SECS)

  # Now poll the Google storage file till the task completes or till deadline
  # is hit.
  time_started_polling = time.time()
  while True:
    if (time.time() - time_started_polling) > DEADLINE_SECS:
      raise G3CompileException(
          'Task did not complete in the deadline of %s seconds.' % (
              DEADLINE_SECS))

    # Get the status of the task.
    gs_file = '%s/%s' % (_get_gs_bucket(), _get_task_file_name(task))
    ret = _read_from_storage(gs_file)

    if ret.get('status'):
      # The task is done, delete the file.
      subprocess.check_call(['gsutil', 'rm', gs_file])

      # Now either raise an Exception or return success based on the status.
      if ret['status'] == 'exception':
        if ret.get('error'):
          raise G3CompileException('Run failed with:\n\n%s\n' % ret['error'])
        else:
          # Use a general purpose error message.
          raise G3CompileException(INFRA_FAILURE_ERROR_MSG)
      elif ret['status'] == 'missing_approval':
          raise G3CompileException(MISSING_APPROVAL_ERROR_MSG)
      elif ret['status'] == 'merge_conflict':
          raise G3CompileException(MERGE_CONFLICT_ERROR_MSG)
      elif ret['status'] == 'failure':
        raise G3CompileException('\n\nRun failed G3 TAP: cl/%s' % ret['cl'])
      elif ret['status'] == 'success':
        print '\n\nRun passed G3 TAP: cl/%s' % ret['cl']
        return 0
      else:
        # Not sure what happened here. Use a general purpose error message.
        raise G3CompileException(INFRA_FAILURE_ERROR_MSG)

    # Print status of the task.
    print 'Task: %s\n' % pretty_task_str(ret)
    time.sleep(POLLING_FREQUENCY_SECS)


def pretty_task_str(task):
  if task.get('result'):
    status = task['result']
  else:
    status = 'Task not started yet'
  return '[status: %s, cl: %s]' % (status, task.get('cl'))


def main():
  option_parser = optparse.OptionParser()
  option_parser.add_option(
      '', '--issue', type=int, default=0,
      help='The Gerrit change number to get the patch from.')
  option_parser.add_option(
      '', '--patchset', type=int, default=0,
      help='The Gerrit change patchset to use.')
  options, _ = option_parser.parse_args()
  sys.exit(trigger_and_wait(options))


if __name__ == '__main__':
  main()
