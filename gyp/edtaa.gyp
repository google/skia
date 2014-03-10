#
# Copyright 2013 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#

{
  'targets': [
    {
      'target_name': 'edtaa',
      'type': 'none',
      'conditions': [
        [ 'skia_distancefield_fonts', {
          'type': 'static_library',
          'sources': [
            '../third_party/edtaa/edtaa3func.cpp',
          ],
          'include_dirs': [
            '../third_party/edtaa/',
          ],
          'all_dependent_settings': {
            'include_dirs': [
              '../third_party/edtaa/',
            ],
          },
        }],
      ],
    },
  ],
}
