# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'includes': [
    'common_variables.gypi',
  ],
  'variables': {
    'component%': 'static_library',
    'icu_directory': '../third_party/externals/icu'
  },
  'targets': [
    {
      'target_name': 'icuuc',
      'type': '<(component)',
      'sources': [
        '<!@(python find.py ../third_party/externals/icu/source/common "*.c*")'
      ],
      'defines': [
        'U_COMMON_IMPLEMENTATION',
        'U_HIDE_DATA_SYMBOL',
        'U_USING_ICU_NAMESPACE=0',
        'HAVE_DLOPEN=0',
        'UCONFIG_NO_NON_HTML5_CONVERSION=1',
      ],
      'include_dirs': [ '<(icu_directory)/source/common', ],
      'direct_dependent_settings': {
        'defines': [
          'U_USING_ICU_NAMESPACE=0',
          'U_ENABLE_DYLOAD=0',
        ],
        'include_dirs': [ '<(icu_directory)/source/common', ],
        'conditions': [
          [
            'component=="static_library"', {
              'defines': [
                'U_STATIC_IMPLEMENTATION',
              ],
            }
          ],
        ],
      },
      'cflags': [ '-w' ],
      'cflags_cc': [ '-frtti', ],
      'conditions': [
        [
          'component=="static_library"', {
            'defines': [ 'U_STATIC_IMPLEMENTATION', ],
          }
        ],
        [
          'OS == "win"', {
            'sources': [
              '<(icu_directory)/source/stubdata/stubdata.c',
            ],
            'copies': [
              {
                'destination': '<(PRODUCT_DIR)',
                'files': [ '<(icu_directory)/windows/icudt.dll', ],
              },
            ],
            'msvs_disabled_warnings': [4005, 4068, 4244, 4355, 4996, 4267],
            'msvs_settings': {
              'VCCLCompilerTool': {
                'AdditionalOptions': [ '/EHsc', ],
              },
            },
            'configurations': {
              'Debug': {
                'msvs_settings': {
                  'VCCLCompilerTool': {
                    'RuntimeTypeInfo': 'true', # /GR
                  },
                },
              },
              'Release': {
                'msvs_settings': {
                  'VCCLCompilerTool': {
                    'RuntimeTypeInfo': 'true', # /GR
                  },
                },
              },
            },
            'all_dependent_settings': {
              'msvs_settings': {
                'VCLinkerTool': {
                  'AdditionalDependencies': [
                    'advapi32.lib',
                  ],
                },
              },
            },
          }
        ],
        [
          'OS == "win" and skia_clang_build', {
            'msvs_settings': {
              'VCCLCompilerTool': {
                'AdditionalOptions': [
                  # See http://bugs.icu-project.org/trac/ticket/11122
                  '-Wno-inline-new-delete',
                  '-Wno-implicit-exception-spec-mismatch',
                ],
              },
            },
          }
        ],
        [
          'skia_os == "android"', {
            'sources': [ '<(icu_directory)/android/icudtl_dat.S', ],
          }
        ],
        [
          'skia_os == "linux"', {
            'sources': [ '<(icu_directory)/linux/icudtl_dat.S', ],
          }
        ],
        [
          'skia_os == "mac"', {
            'sources': [ '<(icu_directory)/mac/icudtl_dat.S', ],
            'xcode_settings': {
              'GCC_ENABLE_CPP_RTTI': 'YES',  # -frtti
              'WARNING_CFLAGS': [ '-w' ],
            },
          }
        ],
      ], # conditions
    },
  ], # targets
}
