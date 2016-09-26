# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'variables': {
    'includes': [ 'skia_sources.gypi' ]
  },
  'targets': [
    {
      'target_name': 'sksl',
      'type': 'static_library',
      'standalone_static_library': 1,
      'sources': [ '<@(sksl_sources)' ],
      'include_dirs': [ '<@(sksl_include_dirs)' ],
      'defines': [
        'SKIA'
      ],
      'all_dependent_settings': {
        'include_dirs': [
          '<(skia_src_path)/sksl',
        ],
      },
    },
  ],
}
