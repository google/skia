#!/usr/bin/env python
# Copyright (c) 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Script that triggers and waits for tasks on android-compile.skia.org"""

import base64
import hashlib
import json
import math
import optparse
import os
import requests
import subprocess
import sys
import time

INFRA_BOTS_DIR = os.path.abspath(os.path.realpath(os.path.join(
    os.path.dirname(os.path.abspath(__file__)), os.pardir)))
sys.path.insert(0, INFRA_BOTS_DIR)
import utils


ANDROID_COMPILE_BUCKET = 'android-compile-tasks'

GS_RETRIES = 5
GS_RETRY_WAIT_BASE = 15

POLLING_FREQUENCY_SECS = 10
DEADLINE_SECS = 2* 60 * 60  # 2 hours.

INFRA_FAILURE_ERROR_MSG = (
      '\n\n'
      'Your run failed due to unknown infrastructure failures.\n'
      'Please contact rmistry@ or the trooper from '
      'http://skia-tree-status.appspot.com/trooper\n'
      'Sorry for the inconvenience!\n'
)


class AndroidCompileException(Exception):
  pass


def _create_task_dict(options):
  """Creates a dict representation of the requested task."""
  params = {}
  params['lunch_target'] = options.lunch_target
  params['mmma_targets'] = options.mmma_targets
  params['issue'] = options.issue
  params['patchset'] = options.patchset
  params['hash'] = options.hash
  return params


def _get_gs_bucket():
  """Returns the Google storage bucket with the gs:// prefix."""
  return 'gs://%s' % ANDROID_COMPILE_BUCKET


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
  return '%s-%s-%s.json' % (task['lunch_target'], task['issue'],
                            task['patchset'])


# Checks to see if task already exists in Google storage.
# If the task has completed then the Google storage file is deleted.
def _does_task_exist_in_storage(task):
  """Checks to see if the corresponding file of the task exists in storage.

  If the file exists and the task has already completed then the storage file is
  deleted and False is returned.
  """
  gs_file = '%s/%s' % (_get_gs_bucket(), _get_task_file_name(task))
  try:
    output = subprocess.check_output(['gsutil', 'cat', gs_file])
  except subprocess.CalledProcessError:
    print 'Task does not exist in Google storage'
    return False
  taskJSON = json.loads(output)
  if taskJSON.get('done'):
    print 'Task exists in Google storage and has completed.'
    print 'Deleting it so that a new run can be scheduled.'
    subprocess.check_call(['gsutil', 'rm', gs_file])
    return False
  else:
    print 'Tasks exists in Google storage and is still running.'
    return True


def _trigger_task(options):
  """Triggers a task on the compile server by creating a file in storage."""
  task = _create_task_dict(options)
  # Check to see if file already exists in Google Storage.
  if not _does_task_exist_in_storage(task):
    _write_to_storage(task)
  return task


def trigger_and_wait(options):
  """Triggers a task on the compile server and waits for it to complete."""
  task = _trigger_task(options)
  print 'Android Compile Task for %d/%d has been successfully added to %s.' % (
      options.issue, options.patchset, ANDROID_COMPILE_BUCKET)
  print '%s will be polled every %d seconds.' % (ANDROID_COMPILE_BUCKET,
                                                 POLLING_FREQUENCY_SECS)

  # Now poll the Google storage file till the task completes or till deadline
  # is hit.
  time_started_polling = time.time()
  while True:
    if (time.time() - time_started_polling) > DEADLINE_SECS:
      raise AndroidCompileException(
          'Task did not complete in the deadline of %s seconds.' % (
              DEADLINE_SECS))

    # Get the status of the task.
    gs_file = '%s/%s' % (_get_gs_bucket(), _get_task_file_name(task))

    for retry in range(GS_RETRIES):
      try:
        output = subprocess.check_output(['gsutil', 'cat', gs_file])
      except subprocess.CalledProcessError:
        raise AndroidCompileException('The %s file no longer exists.' % gs_file)
      try:
        ret = json.loads(output)
        break
      except ValueError, e:
        print 'Received output that could not be converted to json: %s' % output
        print e
        if retry == (GS_RETRIES-1):
          print '%d retries did not help' % GS_RETRIES
          raise
        waittime = GS_RETRY_WAIT_BASE * math.pow(2, retry)
        print 'Retry in %d seconds.' % waittime
        time.sleep(waittime)

    if ret.get('infra_failure'):
      if ret.get('error'):
        raise AndroidCompileException('Run failed with:\n\n%s\n' % ret['error'])
      else:
        # Use a general purpose error message.
        raise AndroidCompileException(INFRA_FAILURE_ERROR_MSG)

    if ret.get('done'):
      if not ret.get('is_master_branch', True):
        print 'The Android Framework Compile bot only works for patches and'
        print 'hashes from the master branch.'
        return 0
      elif ret['withpatch_success']:
        print 'Your run was successfully completed.'
        print 'With patch logs are here: %s' % ret['withpatch_log']
        return 0
      elif ret['nopatch_success']:
        raise AndroidCompileException('The build with the patch failed and the '
               'build without the patch succeeded. This means that the patch '
               'causes Android to fail compilation.\n\n'
               'With patch logs are here: %s\n\n'
               'No patch logs are here: %s\n\n'
               'You can force sync of the checkout if needed here: %s\n\n' % (
                   ret['withpatch_log'], ret['nopatch_log'],
                   'https://skia-android-compile.corp.goog/'))
      else:
        print ('Both with patch and no patch builds failed. This means that the'
               ' Android tree is currently broken. Marking this bot as '
               'successful')
        print 'With patch logs are here: %s' % ret['withpatch_log']
        print 'No patch logs are here: %s' % ret['nopatch_log']
        return 0

    # Print status of the task.
    print 'Task: %s\n' % pretty_task_str(ret)
    time.sleep(POLLING_FREQUENCY_SECS)


def pretty_task_str(task):
  status = 'Not picked up by server yet'
  if task.get('task_id'):
    status = 'Running withpatch compilation'
  if task.get('withpatch_log'):
    status = 'Running nopatch compilation'
  return '[id: %s, checkout: %s, status: %s]' % (
      task.get('task_id'), task.get('checkout'), status)


def main():
  option_parser = optparse.OptionParser()
  option_parser.add_option(
      '', '--lunch_target', type=str, default='',
      help='The lunch target the android compile bot should build with.')
  option_parser.add_option(
      '', '--mmma_targets', type=str, default='',
      help='The comma-separated mmma targets the android compile bot should '
           'build.')
  option_parser.add_option(
      '', '--issue', type=int, default=0,
      help='The Gerrit change number to get the patch from.')
  option_parser.add_option(
      '', '--patchset', type=int, default=0,
      help='The Gerrit change patchset to use.')
  option_parser.add_option(
      '', '--hash', type=str, default='',
      help='The Skia repo hash to compile against.')
  options, _ = option_parser.parse_args()
  sys.exit(trigger_and_wait(options))


if __name__ == '__main__':
  main()
