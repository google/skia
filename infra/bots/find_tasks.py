#!/usr/bin/env python
#
# Copyright 2024 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


'''Simple tool for finding tasks in tasks.json which match search terms.

Example usage:

Find tasks with dimension "os:Mac-14.5"
$ find_tasks.py ^os:Mac-14.5$

Find tasks with "ANGLE" in dimensions or name:
$ find_tasks.py ANGLE

Find tasks with dimension "os:Mac-14.5" and "ANGLE" in dimensions or name:
$ find_tasks.py ^os:Mac-14.5$ ANGLE
'''


import json
import os
import re
import sys


# match_dimensions returns true iff the given search term matches at least one
# of the task's dimensions.
def match_dimensions(term, task):
  for dim in task['dimensions']:
    if re.search(term, dim):
      return True
  return False


# match_name returns true iff the given search term matches the task's name.
def match_name(term, name):
  return re.search(term, name)


# match_task returns true iff all search terms match some part of the task.
def match_task(terms, name, task):
  for term in terms:
    if not (match_name(term, name)
        or match_dimensions(term, task)):
      return False
  return True


def main(terms):
  dir = os.path.dirname(os.path.realpath(__file__))
  tasks_json = os.path.join(dir, 'tasks.json')
  with open(tasks_json) as f:
    taskCfg = json.load(f)
  for name, task in taskCfg['tasks'].items():
    if match_task(terms, name, task):
      print(name)


if __name__ == '__main__':
  main(sys.argv[1:])
