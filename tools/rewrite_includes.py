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
    'third_party/etc1',
    'third_party/gif',
    'tools'
  ]

# Don't count our local Vulkan headers as Skia headers;
# we don't want #include <vulkan/vulkan_foo.h> rewritten to point to them.
blacklist = ['include/third_party/vulkan']

# Map short name -> absolute path for all Skia headers.
headers = {}
for root in roots:
  for path, _, files in os.walk(root):
    if not any(snippet in path for snippet in blacklist):
      for file_name in files:
        if file_name.endswith('.h'):
          if file_name in headers:
            print path, file_name, headers[file_name]
          assert file_name not in headers
          headers[file_name] = os.path.abspath(os.path.join(path, file_name))

# Rewrite any #includes relative to Skia's top-level directory.
for root in roots:
  for path, _, files in os.walk(root):
    if 'generated' in path:
      continue
    for file_name in files:
      if (file_name.endswith('.h') or
          file_name.endswith('.c') or
          file_name.endswith('.m') or
          file_name.endswith('.mm') or
          file_name.endswith('.inc') or
          file_name.endswith('.fp') or
          file_name.endswith('.cc') or
          file_name.endswith('.cpp')):
        # Read the whole file into memory.
        file_path = os.path.join(path, file_name)
        lines = open(file_path).readlines()

        # Write it back out again line by line with substitutions for #includes.
        with open(file_path, 'w') as output:
          includes = []

          for line in lines:
            parts = line.replace('<', '"').replace('>', '"').split('"')
            if (len(parts) == 3
                and '#' in parts[0]
                and 'include' in parts[0]
                and os.path.basename(parts[1]) in headers):
              header = headers[os.path.basename(parts[1])]
              includes.append(parts[0] +
                              '"%s"' % os.path.relpath(header, '.') +
                              parts[2])
            else:
              for inc in sorted(includes):
                print >>output, inc.strip('\n')
              includes = []
              print >>output, line.strip('\n')

