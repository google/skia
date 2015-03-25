# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# ANGLE is the Windows-specific translator from OGL ES to D3D

{
  'targets': [
    {
      'target_name': 'angle',
      'type': 'none',
      'conditions': [
        [ 'skia_angle', {
          'direct_dependent_settings': {
            'include_dirs': [
              '../third_party/externals/angle2/include',
            ],
          },
          'dependencies': [
            '../third_party/externals/angle2/src/angle.gyp:libEGL',
            '../third_party/externals/angle2/src/angle.gyp:libGLESv2',
          ],
        }],
      ],
    },
  ],
}
