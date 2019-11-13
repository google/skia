#!/usr/bin/python

# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Retrieve the given file from googlesource.com."""


from contextlib import closing

import base64
import sys
import urllib2


def get(repo_url, filepath):
  """Retrieve the contents of the given file from the given googlesource repo.

  Args:
      repo_url: string; URL of the repository from which to retrieve the file.
      filepath: string; path of the file within the repository.

  Return:
      string; the contents of the given file.
  """
  base64_url = '/'.join((repo_url, '+', 'master', filepath)) + '?format=TEXT'
  with closing(urllib2.urlopen(base64_url)) as f:
    return base64.b64decode(f.read())


if __name__ == '__main__':
  if len(sys.argv) != 3:
    print >> sys.stderr, 'Usage: %s <repo_url> <filepath>' % sys.argv[0]
    sys.exit(1)
  sys.stdout.write(get(sys.argv[1], sys.argv[2]))
