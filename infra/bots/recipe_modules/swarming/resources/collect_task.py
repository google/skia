#!/usr/bin/env python
# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import json
import logging
import os
import subprocess
import sys


def collect_task(
    collect_cmd, merge_script, build_properties, merge_arguments,
    task_output_dir, output_json):
  """Collect and merge the results of a task.

  This is a relatively thin wrapper script around a `swarming.py collect`
  command and a subsequent results merge to ensure that the recipe system
  treats them as a single step. The results merge can either be the default
  one provided by results_merger or a python script provided as merge_script.

  Args:
    collect_cmd: The `swarming.py collect` command to run. Should not contain
      a --task-output-dir argument.
    merge_script: A merge/postprocessing script that should be run to
      merge the results. This script will be invoked as

        <merge_script> \
            [--build-properties <string JSON>] \
            [merge arguments...] \
            --summary-json <summary json> \
            -o <merged json path> \
            <shard json>...

      where the merge arguments are the contents of merge_arguments_json.
    build_properties: A string containing build information to
      pass to the merge script in JSON form.
    merge_arguments: A string containing additional arguments to pass to
      the merge script in JSON form.
    task_output_dir: A path to a directory in which swarming will write the
      output of the task, including a summary JSON and all of the individual
      shard results.
    output_json: A path to a JSON file to which the merged results should be
      written. The merged results should be in the JSON Results File Format
      (https://www.chromium.org/developers/the-json-test-results-format)
      and may optionally contain a top level "links" field that may contain a
      dict mapping link text to URLs, for a set of links that will be included
      in the buildbot output.
  Returns:
    The exit code of collect_cmd or merge_cmd.
  """
  logging.debug('Using task_output_dir: %r', task_output_dir)
  if os.path.exists(task_output_dir):
    logging.warn('task_output_dir %r already exists!', task_output_dir)
    existing_contents = []
    try:
      for p in os.listdir(task_output_dir):
        existing_contents.append(os.path.join(task_output_dir, p))
    except (OSError, IOError) as e:
      logging.error('Error while examining existing task_output_dir: %s', e)

    logging.warn('task_output_dir existing content: %r', existing_contents)

  collect_cmd.extend(['--task-output-dir', task_output_dir])

  logging.info('collect_cmd: %s', ' '.join(collect_cmd))
  collect_result = subprocess.call(collect_cmd)
  if collect_result != 0:
    logging.warn('collect_cmd had non-zero return code: %s', collect_result)

  task_output_dir_contents = []
  try:
    task_output_dir_contents.extend(
        os.path.join(task_output_dir, p)
        for p in os.listdir(task_output_dir))
  except (OSError, IOError) as e:
    logging.error('Error while processing task_output_dir: %s', e)

  logging.debug('Contents of task_output_dir: %r', task_output_dir_contents)
  if not task_output_dir_contents:
    logging.warn(
        'No files found in task_output_dir: %r',
        task_output_dir)

  task_output_subdirs = (
      p for p in task_output_dir_contents
      if os.path.isdir(p))
  shard_json_files = [
      os.path.join(subdir, 'output.json')
      for subdir in task_output_subdirs]
  extant_shard_json_files = [
      f for f in shard_json_files if os.path.exists(f)]

  if shard_json_files != extant_shard_json_files:
    logging.warn(
        'Expected output.json file missing: %r\nFound: %r\nExpected: %r\n',
        set(shard_json_files) - set(extant_shard_json_files),
        extant_shard_json_files,
        shard_json_files)

  if not extant_shard_json_files:
    logging.warn(
        'No shard json files found in task_output_dir: %r\nFound %r',
        task_output_dir, task_output_dir_contents)

  logging.debug('Found shard_json_files: %r', shard_json_files)

  summary_json_file = os.path.join(task_output_dir, 'summary.json')

  merge_result = 0

  merge_cmd = [sys.executable, merge_script]
  if build_properties:
    merge_cmd.extend(('--build-properties', build_properties))
  if os.path.exists(summary_json_file):
    merge_cmd.extend(('--summary-json', summary_json_file))
  else:
    logging.warn('Summary json file missing: %r', summary_json_file)
  if merge_arguments:
    merge_cmd.extend(json.loads(merge_arguments))
  merge_cmd.extend(('-o', output_json))
  merge_cmd.extend(extant_shard_json_files)

  logging.info('merge_cmd: %s', ' '.join(merge_cmd))
  merge_result = subprocess.call(merge_cmd)
  if merge_result != 0:
    logging.warn('merge_cmd had non-zero return code: %s', merge_result)

  if not os.path.exists(output_json):
    logging.warn(
        'merge_cmd did not create output_json file: %r', output_json)

  return collect_result or merge_result


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('--build-properties')
  parser.add_argument('--merge-additional-args')
  parser.add_argument('--merge-script', required=True)
  parser.add_argument('--task-output-dir', required=True)
  parser.add_argument('-o', '--output-json', required=True)
  parser.add_argument('--verbose', action='store_true')
  parser.add_argument('collect_cmd', nargs='+')

  args = parser.parse_args()
  if args.verbose:
    logging.basicConfig(stream=sys.stdout, level=logging.DEBUG)

  return collect_task(
      args.collect_cmd,
      args.merge_script, args.build_properties, args.merge_additional_args,
      args.task_output_dir, args.output_json)


if __name__ == '__main__':
  sys.exit(main())
