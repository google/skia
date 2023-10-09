#!/usr/bin/python
#
# Copyright 2019 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import argparse
import os
import sys

from io import StringIO


parser = argparse.ArgumentParser()
parser.add_argument('-n', '--dry-run', action='store_true',
                    help='Just check there is nothing to rewrite.')
parser.add_argument('sources', nargs='*',
                    help='Source files to rewrite, or all if empty.')
args = parser.parse_args()

roots = [
  'bench',
  'dm',
  'docs',
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

ignorelist = [
  # Don't count our local Vulkan headers as Skia headers;
  # we don't want #include <vulkan/vulkan_foo.h> rewritten to point to them.
  'include/third_party/vulkan',
  # Some node_modules/ files (used by CanvasKit et al) have c++ code which we should ignore.
  'node_modules',
  'include/third_party/skcms',
  'src/gpu/vk/vulkanmemoryallocator',
  # Used by Jetski and Graphite
  'Surface.h',
  # Used by Ganesh and Graphite
  'Device.h',
  # Temporary shims
  'SkEncodedImageFormat.h',
  'SkICC.h',
  # Transitional
  'tools/window',
]

assert '/' in [os.sep, os.altsep]
def fix_path(p):
  return p.replace(os.sep, os.altsep) if os.altsep else p

# Map short name -> absolute path for all Skia headers.
headers = {}
for root in roots:
  for path, _, files in os.walk(root):
    if not any(snippet in fix_path(path) for snippet in ignorelist):
      for file_name in files:
        if file_name.endswith('.h') and not file_name in ignorelist:
          if file_name in headers:
            message = ('Header filename is used more than once!\n- ' + path + '/' + file_name +
                       '\n- ' + headers[file_name])
            assert file_name not in headers, message
          headers[file_name] = os.path.abspath(os.path.join(path, file_name))

def to_rewrite():
  if args.sources:
    for path in args.sources:
      yield path
  else:
    for root in roots:
      for path, _, files in os.walk(root):
        for file_name in files:
          yield os.path.join(path, file_name)

# Rewrite any #includes relative to Skia's top-level directory.
need_rewriting = []
for file_path in to_rewrite():
  if ('/generated/' in file_path or
      'tests/sksl/' in file_path or
      'third_party/skcms' in file_path or
      'modules/skcms' in file_path or
      # transitional
      'jetski' in file_path or
      'tools/window' in file_path or
      file_path.startswith('bazel/rbe') or
      'example/external_client/' in file_path or
      # We intentionally list SkUserConfig.h not from the root in this file.
      file_path == 'include/private/base/SkLoadUserConfig.h'):
    continue
  if (file_path.endswith('.h') or
      file_path.endswith('.c') or
      file_path.endswith('.m') or
      file_path.endswith('.mm') or
      file_path.endswith('.inc') or
      file_path.endswith('.cc') or
      file_path.endswith('.cpp')):
    # Read the whole file into memory.
    lines = open(file_path).readlines()

    # Write it back out again line by line with substitutions for #includes.
    output = StringIO() if args.dry_run else open(file_path, 'w')

    includes = []
    for line in lines:
      parts = line.replace('<', '"').replace('>', '"').split('"')
      if (len(parts) == 3
          and '#' in parts[0]
          and 'include' in parts[0]
          and os.path.basename(parts[1]) in headers):
        header = fix_path(os.path.relpath(headers[os.path.basename(parts[1])], '.'))
        includes.append(parts[0] + '"%s"' % header + parts[2])
      else:
        # deduplicate includes in this block. If a file needs to be included
        # multiple times, the separate includes should go in different blocks.
        includes = sorted(list(set(includes)))
        for inc in includes:
          output.write(inc.strip('\n') + '\n')
        includes = []
        output.write(line.strip('\n') + '\n')
    # Fix any straggling includes, e.g. in a file that only includes something else.
    for inc in sorted(includes):
      output.write(inc.strip('\n') + '\n')
    if args.dry_run and output.getvalue() != open(file_path).read():
      need_rewriting.append(file_path)
      rc = 1
    output.close()

if need_rewriting:
  print('Some files need rewritten #includes:')
  for path in need_rewriting:
    print('\t' + path)
  print('To do this automatically, run')
  print('python3 tools/rewrite_includes.py ' + ' '.join(need_rewriting))
  sys.exit(1)
