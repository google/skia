# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'hb_directory': '../third_party/externals/harfbuzz',
  },
  'targets': [
    {
      'target_name': 'harfbuzz',
      'type': 'static_library',
      'defines': [
        'HAVE_OT',
        'HAVE_ICU',
        'HAVE_ICU_BUILTIN',
        'HB_NO_MT',
      ],
      'sources': [
        '<!@(python find.py <(hb_directory)/src "hb-*.c*")',
      ],
      'sources!': [
        '<(hb_directory)/src/hb-directwrite.cc',
        '<(hb_directory)/src/hb-ft.cc',
        '<(hb_directory)/src/hb-glib.cc',
        '<(hb_directory)/src/hb-gobject-structs.cc',
        '<(hb_directory)/src/hb-graphite2.cc',
        '<(hb_directory)/src/hb-ucdn.cc',
        '<(hb_directory)/src/hb-uniscribe.cc',
      ],
      'include_dirs': [
        '<(hb_directory)/src',
        '../third_party/harfbuzz',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(hb_directory)/src',
          '../third_party/harfbuzz',
        ],
      },
      'dependencies': [ 'icu.gyp:icuuc', ],
      'cflags': [ '-w', ],
      'msvs_settings': {
        'VCCLCompilerTool': {
          'WarningLevel': '0',
          'AdditionalOptions': [ '/wd4189', ],
        },
      },
      'xcode_settings': { 'WARNING_CFLAGS': [ '-w', ], },
      'conditions': [
        [ 'skia_os == "mac"',
          {
            'defines': [ 'HAVE_CORETEXT', ],
          }, {
            'sources!': [ '<(hb_directory)/src/hb-coretext.cc', ],
          }
        ],
      ],
    },
  ],
}
