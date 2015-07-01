# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# GYP file to build hello world example.
{
  'includes': [
    'apptype_console.gypi',
  ],
  'targets': [
    {
      'target_name': 'HelloWorld',
      'type': 'executable',
      'include_dirs' : [
        '../include/gpu',
      ],
      'sources': [
        '../example/HelloWorld.h',
        '../example/HelloWorld.cpp',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'views.gyp:views',
      ],
    },
  ],
}
