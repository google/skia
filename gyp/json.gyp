# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'targets': [
    {
      'target_name': 'json',
      'product_name': 'skia_json',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'core.gyp:core',
        'jsoncpp.gyp:jsoncpp',
      ],
      'include_dirs': [
        '../include/core',
        '../include/effects',
        '../include/private',
        '../include/utils',
        '../src/core',
      ],
      'sources': [
        '../tools/json/SkJSONCanvas.cpp',
        '../tools/json/SkJSONRenderer.cpp',
      ],
    },
  ],
}
