#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Run all infrastructure-related tests."""


import os
import subprocess
import sys


INFRA_BOTS_DIR = os.path.dirname(os.path.realpath(__file__))
SKIA_DIR = os.path.abspath(os.path.join(INFRA_BOTS_DIR, os.pardir, os.pardir))


def test(cmd, cwd):
  try:
    subprocess.check_output(cmd, cwd=cwd, stderr=subprocess.STDOUT)
  except subprocess.CalledProcessError as e:
    return e.output


def python_unit_tests(train):
  if train:
    return None
  return test(
      ['python', '-m', 'unittest', 'discover', '-s', '.', '-p', '*_test.py'],
      INFRA_BOTS_DIR)


def recipe_test(train):
  cmd = [
      'python', os.path.join(INFRA_BOTS_DIR, 'recipes.py'), 'test']
  if train:
    cmd.append('train')
  else:
    cmd.append('run')
  return test(cmd, SKIA_DIR)


def gen_tasks_test(train):
  cmd = ['go', 'run', 'gen_tasks.go']
  if not train:
    cmd.append('--test')
  try:
    output = test(cmd, INFRA_BOTS_DIR)
  except OSError:
    return ('Failed to run "%s"; do you have Go installed on your machine?'
            % ' '.join(cmd))
  if output and 'cannot find package "go.skia.org/infra' in output:
    return ('Failed to run gen_tests.go:\n\n%s\nMaybe you need to run:\n\n'
            '$ go get -u go.skia.org/infra/...' % output)
  return output


def main():
  train = False
  if '--train' in sys.argv:
    train = True

  tests = (
      python_unit_tests,
      recipe_test,
      gen_tasks_test,
  )
  errs = []
  for t in tests:
    err = t(train)
    if err:
      errs.append(err)

  if len(errs) > 0:
    print >> sys.stderr, 'Test failures:\n'
    for err in errs:
      print >> sys.stderr, '=============================='
      print >> sys.stderr, err
      print >> sys.stderr, '=============================='
    sys.exit(1)

  if train:
    print 'Trained tests successfully.'
  else:
    print 'All tests passed!'


if __name__ == '__main__':
  main()
