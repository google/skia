# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This arguably should go in libjpeg-turbo.gyp.  But if we put it there, gyp
# gets overeager and starts trying to parse all of the targets in that file.
# And it will fail to parse a target it doesn't need anyway when we are
# building for the android framework.  So we will add this file as another
# layer of indirection.
{
  'targets': [
    {
      'target_name': 'libjpeg-turbo-selector',
      'type': 'none',
      'conditions': [
        [ 'skia_android_framework', {
            'dependencies':              [ 'android_deps.gyp:libjpeg-turbo' ],
            'export_dependent_settings': [ 'android_deps.gyp:libjpeg-turbo' ],
        },{
            'dependencies':              [ 'libjpeg-turbo.gyp:libjpeg-turbo' ],
            'export_dependent_settings': [ 'libjpeg-turbo.gyp:libjpeg-turbo' ],
        }]
      ]
    },
  ]
}
