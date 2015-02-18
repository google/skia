# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [{
    'target_name': 'zlib',
    'type': 'none',
    'direct_dependent_settings': {
      'conditions': [
        [ 'skia_android_framework', { 'include_dirs': [ 'external/zlib' ] }],
        [ 'skia_os == "mac" or skia_os == "ios"', {
            # XCode needs and explicit file path, not a logical name like -lz.
            'link_settings': { 'libraries': [ '$(SDKROOT)/usr/lib/libz.dylib' ] },
        },{
            'link_settings': { 'libraries': [ '-lz' ] },
        }]
      ],
    },
  }],
}
