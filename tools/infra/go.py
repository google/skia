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
  except (subprocess.CalledProcessError, OSError):
    go_exe = None
  if not go_exe:
    _fail('Unable to find Golang installation; see '
          'https://golang.org/doc/install')
  if not os.environ.get('GOPATH'):
    _fail('GOPATH environment variable is not set; is Golang properly '
          'installed?')
  go_bin = os.path.join(os.environ['GOPATH'], 'bin')
  for entry in os.environ.get('PATH', '').split(os.pathsep):
    if entry == go_bin:
      break
  else:
    _fail('%s not in PATH; is Golang properly installed?' % go_bin)


def get(pkg):
  '''Obtain/update the given package/module via "go get".'''
  check()
  subprocess.check_call(['go', 'get', '-u', pkg])


def update_infra():
  '''Update the local checkout of the Skia infra codebase.'''
  get(INFRA_GO + '/...')


def mod_download(*pkgs):
  '''Run "go mod download" to obtain the given package(s).'''
  check()
  subprocess.check_call(['go', 'mod', 'download']+list(pkgs))


def install(pkg):
  '''"go install" the given package.'''
  check()
  subprocess.check_call(['go', 'install', pkg])
