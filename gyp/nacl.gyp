# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# Common entry point for all Skia executables running in NaCl
{
  'targets': [
    {
      'target_name': 'nacl_interface',
      'type': 'static_library',
      'dependencies': [
        'skia_lib.gyp:skia_lib',
      ],
      'include_dirs': [
        # For SkThreadUtils.h
        '../src/utils',
      ],
      'sources': [
        '../platform_tools/nacl/src/nacl_interface.cpp',
      ],
    },
  ],
}
