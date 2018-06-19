#!/usr/bin/env python
# Copyright (c) 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Script that triggers and waits for tasks on android-compile.skia.org"""

import base64
import hashlib
import json
import optparse
import requests
import sys
import time


ANDROID_COMPILE_HOST = "https://android-compile.skia.org"
ANDROID_COMPILE_REGISTER_POST_URI = ANDROID_COMPILE_HOST + "/_/register"
ANDROID_COMPILE_TASK_STATUS_URI = ANDROID_COMPILE_HOST + "/get_task_status"
GCE_WEBHOOK_SALT_METADATA_URI = (
    "http://metadata/computeMetadata/v1/project/attributes/"
    "ac_webhook_request_salt")


POLLING_FREQUENCY_SECS = 60  # 1 minute.
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


def _CreateTaskJSON(options):
  """Creates a JSON representation of the requested task."""
  params = {}
  params["issue"] = options.issue
  params["patchset"] = options.patchset
  params["hash"] = options.hash
  return json.dumps(params)


def _GetWebhookSaltFromMetadata():
  """Gets webhook_request_salt from GCE's metadata server."""
  headers = {"Metadata-Flavor": "Google"}
  resp = requests.get(GCE_WEBHOOK_SALT_METADATA_URI, headers=headers)
  if resp.status_code != 200:
      raise AndroidCompileException(
          'Return code from %s was %s' % (GCE_WEBHOOK_SALT_METADATA_URI,
                                          resp.status_code))
  return base64.standard_b64decode(resp.text)


def _GetAuthHeaders(data, options):
  m = hashlib.sha512()
  if data:
    m.update(data)
  m.update('notverysecret' if options.local else _GetWebhookSaltFromMetadata())
  encoded = base64.standard_b64encode(m.digest())
  return {
      "Content-type": "application/x-www-form-urlencoded",
      "Accept": "application/json",
      "X-Webhook-Auth-Hash": encoded}


def _TriggerTask(options):
  """Triggers the task on Android Compile and returns the new task's ID."""
  task = _CreateTaskJSON(options)
  headers = _GetAuthHeaders(task, options)
  resp = requests.post(ANDROID_COMPILE_REGISTER_POST_URI, task, headers=headers)

  if resp.status_code != 200:
    raise AndroidCompileException(
        'Return code from %s was %s' % (ANDROID_COMPILE_REGISTER_POST_URI,
                                        resp.status_code))
  try:
    ret = json.loads(resp.text)
  except ValueError, e:
    raise AndroidCompileException(
        'Did not get a JSON response from %s: %s' % (
            ANDROID_COMPILE_REGISTER_POST_URI, e))
  return ret["taskID"]


def TriggerAndWait(options):
  task_id = _TriggerTask(options)
  task_str = '[id: %d, issue: %d, patchset: %d, hash: %s]' % (
      task_id, options.issue, options.patchset, options.hash)

  print
  print 'Task %s has been successfully scheduled on %s.' % (
      task_str, ANDROID_COMPILE_HOST)
  print
  print 'The server will be polled every %d seconds.' % POLLING_FREQUENCY_SECS
  print

  headers = _GetAuthHeaders('', options)
  # Now poll the server till the task completes or till deadline is hit.
  time_started_polling = time.time()
  while True:
    if (time.time() - time_started_polling) > DEADLINE_SECS:
      raise AndroidCompileException(
          'Task did not complete in the deadline of %s seconds.' % (
              DEADLINE_SECS))

    # Get the status of the task the trybot added.
    get_url = '%s?task=%s' % (ANDROID_COMPILE_TASK_STATUS_URI, task_id)
    resp = requests.get(get_url, headers=headers)
    if resp.status_code != 200:
      raise AndroidCompileException(
          'Return code from %s was %s' % (ANDROID_COMPILE_TASK_STATUS_URI,
                                          resp.status_code))
    try:
      ret = json.loads(resp.text)
    except ValueError, e:
      raise AndroidCompileException(
          'Did not get a JSON response from %s: %s' % (get_url, e))

    if ret["infra_failure"]:
      raise AndroidCompileException(INFRA_FAILURE_ERROR_MSG)

    if ret["done"]:
      print
      print
      if not ret.get("is_master_branch", True):
        print 'The Android Framework Compile bot only works for patches and'
        print 'hashes from the master branch.'
        print
        return 0
      elif ret["withpatch_success"]:
        print 'Your run was successfully completed.'
        print
        print 'With patch logs are here: %s' % ret["withpatch_log"]
        print
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
        print
        print 'With patch logs are here: %s' % ret["withpatch_log"]
        print 'No patch logs are here: %s' % ret["nopatch_log"]
        return 0

    print '.'
    time.sleep(POLLING_FREQUENCY_SECS)


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
  option_parser.add_option(
      '', '--local', default=False, action='store_true',
      help='Uses a dummy metadata salt if this flag is true else it tries to '
           'get the salt from GCE metadata.')
  options, _ = option_parser.parse_args()
  sys.exit(TriggerAndWait(options))


if __name__ == '__main__':
  main()
