# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [{
    'target_name': 'zlib',
    'type': 'none',
    'link_settings': { 'libraries': [ '-lz' ] },
    'direct_dependent_settings': {
      'conditions': [
        [ 'skia_android_framework', { 'include_dirs': [ 'external/zlib' ] }]
      ],
    },
  }],
}
