#!/usr/bin/env python
#
# Copyright 2019 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import os
import subprocess
import sys


INFRA_GO = 'go.skia.org/infra'
WHICH = 'where' if sys.platform == 'win32' else 'which'


def check():
  '''Verify that golang is properly installed. If not, exit with an error.'''
  def _fail(msg):
    print >> sys.stderr, msg
    sys.exit(1)

  try:
    go_exe = subprocess.check_output([WHICH, 'go'])
  except:
    pass
  if not go_exe:
    _fail('Unable to find Golang installation; see '
          'https://golang.org/doc/install')
  if not os.environ.get('GOROOT'):
    _fail('GOROOT environment variable is not set; is Golang properly '
          'installed?')


def get(url):
  '''Clone or update the given repo URL via "go get".'''
  check()
  subprocess.check_call(['go', 'get', '-u', url])


def update_infra():
  '''Update the local checkout of the Skia infra codebase.'''
  get(INFRA_GO + '/...')
