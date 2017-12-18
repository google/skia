#! /usr/bin/env python2
# Copyright 2017 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import StringIO
import os
import sys
import sysopen
import tarfile
import tempfile
import zlib

if __name__ == '__main__':
  if len(sys.argv) != 2:
    print 'usage: %s FILE.ab\n' % sys.argv[0]
    exit (1)
  with open(sys.argv[1], 'rb') as f:
    f.read(24)
    t = tarfile.open(fileobj=StringIO.StringIO(zlib.decompress(f.read())))
    d = tempfile.mkdtemp(prefix='skqp_')
    t.extractall(d)
    p = os.path.join(d, 'apps/org.skia.skqp/f/skqp_report/report.html')
    assert os.path.isfile(p)
    print p
    sysopen.sysopen(p)



