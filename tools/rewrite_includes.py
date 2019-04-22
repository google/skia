#!/usr/bin/python2
#
# Copyright 2019 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os

roots = [
    'bench',
    'dm',
    'docs',
    'example',
    'experimental',
    'fuzz',
    'gm',
    'include',
    'modules',
    'platform_tools/android/apps',
    'samplecode',
    'src',
    'tests',
    'third_party/gif',
    'tools'
  ]

# Map short name -> absolute path for all Skia headers.
headers = {}
for root in roots:
  for path, dirs, files in os.walk(root):
    for file_name in files:
      if file_name.endswith('.h'):
        if file_name in headers:
          print path, file_name, headers[file_name]
        assert file_name not in headers
        headers[file_name] = os.path.abspath(os.path.join(path, file_name))

# Rewrite any #includes of short names outside their directory.
for root in roots:
  for path, dirs, files in os.walk(root):
    for file_name in files:
      if (file_name.endswith('.h') or
          file_name.endswith('.c') or
          file_name.endswith('.inc') or
          file_name.endswith('.cc') or
          file_name.endswith('.cpp')):
        # Read the whole file into memory.
        file_path = os.path.join(path, file_name)
        lines = open(file_path).readlines()

        # Write it back out again line by line with substitutions for #includes.
        with open(file_path, 'w') as output:
          for line in lines:
            parts = line.split('"')
            if (len(parts) == 3
                and '#' in parts[0]
                and 'include' in parts[0]
                and os.path.basename(parts[1]) in headers):

              header_path = headers[os.path.basename(parts[1])]

              # Generally we want to see relative #includes.
              include = os.path.relpath(header_path, path)

              # If the relative path would need to back up,
              # instead use the path from the top.
              # This is most important to make sure include/ can be relocated.
              if '..' in include:
                include = os.path.relpath(header_path, '.')

              print >>output, parts[0] + '"%s"' % include
            else:
              print >>output, line.strip('\n')
