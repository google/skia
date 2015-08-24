# Copyright 2014 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'skia_warnings_as_errors': 0,
  },
  'targets': [
    {
      'target_name': 'giflib',
      'conditions': [
        [ 'skia_android_framework == 0',
          {
            'type': 'static_library',
            'include_dirs': [
              '../third_party/externals/giflib',
            ],
            'dependencies': [
            ],
            'direct_dependent_settings': {
              'include_dirs': [
                '../third_party/externals/giflib',
              ],
            },
            'cflags': [
              '-w',
            ],
            'xcode_settings': {
              'WARNING_CFLAGS': [
                '-w'
              ],
            },
            'msvs_settings': {
              'VCCLCompilerTool': {
                'AdditionalOptions': [
                  '/wd4996',
                  '/wd4018',
                  '/wd4267',
                ],
              },
            },
            'sources': [
              '../third_party/externals/giflib/dgif_lib.c',
              '../third_party/externals/giflib/gifalloc.c',
              '../third_party/externals/giflib/gif_err.c',
            ],
            'conditions' : [
              [ 'skia_os == "win"', {
                  'include_dirs': [
                    # Used to include a dummy unistd.h file for windows
                    '../third_party/giflib',
                  ],
                },
              ],
            ],
          }, { # skia_android_framework
            'type': 'none',
            'direct_dependent_settings': {
              'libraries' : [
                'libgif.a',
              ],
              'include_dirs': [
                'external/giflib',
              ]
            }
          }
        ]
      ]
    }
  ]
}
