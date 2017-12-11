#! /usr/bin/env python

# Copyright 2017 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import glob
import os
import re
import sys

import sysopen

if len(sys.argv) == 2:
  if not os.path.isdir(sys.argv[1]):
    exit(1)
  os.chdir(sys.argv[1])

head = '''<!doctype html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>SkQP Report</title>
<style>
img { max-width:48%; border:1px green solid; }
</style>
</head>
<body>
<h1>SkQP Report</h1>
<hr>
'''

reg = re.compile('="../../')
with open('report.html', 'w') as o:
  o.write(head)
  for x in glob.iglob('*/*/report.html'):
    with open(x, 'r') as f:
      o.write(reg.sub('="', f.read()))
  o.write('</body>\n</html>\n')

sysopen.sysopen('report.html')
