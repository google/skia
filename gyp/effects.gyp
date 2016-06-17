# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# Gyp file for effects
{
  'targets': [
    {
      'target_name': 'effects',
      'product_name': 'skia_effects',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'core.gyp:*',
        'images.gyp:*',
        'utils.gyp:utils',
      ],
      'includes': [
        'effects.gypi',
      ],
      'include_dirs': [
        '../include/effects',
        '../include/client/android',
        '../include/private',
        '../src/effects',
        '../src/opts',
        '../src/core',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/effects',
          '../include/client/android',
        ],
      },
      'sources': [
        'effects.gypi', # Makes the gypi appear in IDEs (but does not modify the build).
      ],
      'conditions': [
        ['skia_gpu == 1', {
          'include_dirs': [
            '../include/gpu',
            '../src/gpu',
          ],
        }],
      ],
    },
  ],
}
