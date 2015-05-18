# Copyright (c) 2011 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'targets': [
    {
      'target_name': 'sfntly',
      'type': 'static_library',
      'includes': [
        'common_conditions.gypi',
        'common_variables.gypi',
      ],
      'variables': {
        'sfntly_src_path': '../third_party/externals/sfntly',
      },
      'direct_dependent_settings': {
        'include_dirs': [ '<(sfntly_src_path)/sample/chromium', ],
        'defines': [
          'SK_SFNTLY_SUBSETTER="font_subsetter.h"',
        ],
      },
      'sources': [
        '<(sfntly_src_path)/sample/chromium/font_subsetter.cc',
        '<(sfntly_src_path)/sample/chromium/subsetter_impl.cc',
        '<!@(python find.py ../third_party/externals/sfntly/sfntly "*.c*")'
      ],
      'include_dirs': [
        '<(sfntly_src_path)',
      ],
      'defines': [
        'SFNTLY_NO_EXCEPTION',
      ],
      'dependencies' : [
        'icu.gyp:icuuc',
      ],
      'conditions': [
        [ 'skia_os == "win"',
          {
            'defines': [ 'WIN32', ],
            'msvs_settings': {
              'VCCLCompilerTool': {
                'AdditionalOptions': [ '/EHsc' ],
              },
            },
          }
        ],
        [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris", "chromeos"]',
          { 'link_settings': { 'libraries': [ '-lpthread', ], }, },
        ],
      ],
      # TODO(jschuh): http://crbug.com/167187
      'msvs_disabled_warnings': [ 4267, 4244 ],
    },
  ]
}
