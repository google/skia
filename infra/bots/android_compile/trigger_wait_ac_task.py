#!/usr/bin/env python
# Copyright (c) 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Script that triggers and waits for tasks on android-compile.skia.org"""

import base64
import hashlib
import json
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


ANDROID_COMPILE_BUCKET = "android-compile-tasks"

POLLING_FREQUENCY_SECS = 10
DEADLINE_SECS = 60 * 60  # 60 minutes.

INFRA_FAILURE_ERROR_MSG = (
      '\n\n'
      'Your run failed due to infra failures. '
      'It could be due to any of the following:\n\n'
      '* Need to rebase\n\n'
      '* Failure when running "python -c from gn import gn_to_bp"\n\n'
      '* Problem with syncing Android repository.\n\n'
      'See go/skia-android-framework-compile-bot-cloud-logs-errors for the '
      'compile server\'s logs.'
)


class AndroidCompileException(Exception):
  pass


def _CreateTaskDict(options):
  """Creates a dict representation of the requested task."""
  params = {}
  params["issue"] = options.issue
  params["patchset"] = options.patchset
  params["hash"] = options.hash
  return params


def _GetGsUtil():
  gsutil = subprocess.check_output(['which', 'gsutil']).rstrip()
  gsutil_ret = [gsutil]
  if gsutil.endswith('.py'):
    gsutil_ret = ['python', gsutil]
  return gsutil_ret


def _GetGsBucket():
  return 'gs://%s' % ANDROID_COMPILE_BUCKET


# Rename all of this stuff.
def _WriteToStorage(task):
  with utils.tmp_dir():
    json_file = os.path.join(os.getcwd(), _GetTaskFileName(task))
    with open(json_file, 'w') as f:
      json.dump(task, f)
    with open(json_file, 'r') as f:
      subprocess.check_call(
          _GetGsUtil() + ['cp', f.name, '%s/' % _GetGsBucket()])
      print 'Created %s/%s' % (_GetGsBucket(), os.path.basename(f.name))


# TODO(rmistry): Rename all functions!!!!
def _GetTaskFileName(task):
  return '%s-%s.json' % (task["issue"], task["patchset"])


# Checks to see if task already exists in Google storage.
# If the task has completed then the Google storage file is deleted.
def _DoesTaskExistInGs(task):
  gs_file = '%s/%s' % (_GetGsBucket(), _GetTaskFileName(task))
  try:
    output = subprocess.check_output(
        _GetGsUtil() + ['cat', gs_file])
  except subprocess.CalledProcessError:
    print 'Task does not exist in Google storage'
    return False
  taskJSON = json.loads(output)
  if taskJSON.get('done'):
    print 'Task exists in Google storage and has completed.'
    print 'Deleting it so that a new run can be scheduled.'
    subprocess.check_call(_GetGsUtil() + ['rm', gs_file])
    return False
  else:
    print 'Tasks exists in Google storage and is still running.'
    return True


def _TriggerTask(options):
  """Triggers the task by creating a file in Google storage (if necessary).

  Algorithm:
  """
  task = _CreateTaskDict(options)
  # Check to see if file already exists in Google Storage.
  if not _DoesTaskExistInGs(task):
    _WriteToStorage(task)
  return task


def TriggerAndWait(options):
  # Only trigger task if it does not exist in Google Storage yet.

  task = _TriggerTask(options)

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
    gs_file = '%s/%s' % (_GetGsBucket(), _GetTaskFileName(task))
    try:
      output = subprocess.check_output(_GetGsUtil() + ['cat', gs_file])
    except subprocess.CalledProcessError:
      raise AndroidCompileException('The %s file no longer exists.' % gs_file)
    ret = json.loads(output)

    if ret.get("infra_failure"):
      raise AndroidCompileException(INFRA_FAILURE_ERROR_MSG)

    if ret.get("done"):
      if not ret.get("is_master_branch", True):
        print 'The Android Framework Compile bot only works for patches and'
        print 'hashes from the master branch.'
        return 0
      elif ret["withpatch_success"]:
        print 'Your run was successfully completed.'
        print 'With patch logs are here: %s' % ret["withpatch_log"]
        return 0
      elif ret["nopatch_success"]:
        raise AndroidCompileException('The build with the patch failed and the '
               'build without the patch succeeded. This means that the patch '
               'causes Android to fail compilation.\n\n'
               'With patch logs are here: %s\n\n'
               'No patch logs are here: %s\n\n' % (
                   ret["withpatch_log"], ret["nopatch_log"]))
      else:
        print ('Both with patch and no patch builds failed. This means that the'
               ' Android tree is currently broken. Marking this bot as '
               'successful')
        print 'With patch logs are here: %s' % ret["withpatch_log"]
        print 'No patch logs are here: %s' % ret["nopatch_log"]
        return 0

    # Print more informative logs here. withpatch_log and nopatch_log.....
    print 'Task: %s\n' % pretty_task_str(ret)
    time.sleep(POLLING_FREQUENCY_SECS)


def pretty_task_str(task):
  status = "Running withpatch compilation"
  if task.get('withpatch_log'):
    status = "Running nopatch compilation"
  return '[id: %s, checkout: %s, status: %s]' % (
      task.get('task_id'), task.get('checkout'), status)


def main():
  option_parser = optparse.OptionParser()
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
  sys.exit(TriggerAndWait(options))


if __name__ == '__main__':
  main()
