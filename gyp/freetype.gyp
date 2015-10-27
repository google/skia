# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# Target for building freetype.
{
  'targets': [
    {
      'target_name': 'freetype',
      'type': 'none',
      'conditions': [
        [ 'skia_freetype_static',
          {
            'dependencies': [
              'freetype_static'
            ],
            'export_dependent_settings': [
              'freetype_static'
            ],
            'conditions': [
              [ 'skia_os == "android"',
                {
                  'direct_dependent_settings': {
                    'defines': [
                      # Android provides at least FreeType 2.4.0
                      'SK_FONTHOST_FREETYPE_RUNTIME_VERSION=0x020400',
                      'SK_CAN_USE_DLOPEN=0',
                    ],
                  }
                }
              ]
            ],
          }, { # (not skia_freetype_static)
            # dynamic linking depends on the OS:
            'conditions': [
              [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris", "chromeos"]',
                {
                  'direct_dependent_settings': {
                    'include_dirs' : [
                      '/usr/include/freetype2',
                    ],
                    'link_settings': {
                      'libraries': [
                        '-lfreetype',
                      ],
                    },
                    'defines': [
                      # Skia's FreeType port requires at least FreeType 2.3.8
                      # for building and at runtime.
                      'SK_FONTHOST_FREETYPE_RUNTIME_VERSION=0x020308',
                      'SK_CAN_USE_DLOPEN=1',
                    ],
                  }
                },
              ],
              [ 'skia_android_framework',
                {
                  'direct_dependent_settings': {
                    'defines': [
                      # Android provides at least FreeType 2.4.0
                      'SK_FONTHOST_FREETYPE_RUNTIME_VERSION=0x020400',
                      'SK_CAN_USE_DLOPEN=0',
                    ],
                  },
                  'include_dirs': [
                    'external/expat/lib',
                    'external/freetype/include',
                  ],
                  'libraries': [
                    '-lft2',
                  ],
                }
              ],
            ],
          }
        ],
      ],
    },
    {
      'target_name': 'freetype_static',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        # we are dependent upon PNG for color emoji glyphs
        'libpng.gyp:libpng',
      ],
      'includes': [
        # TODO: merge this back in here?
        'freetype.gypi',
      ],
      'conditions': [
        [ 'skia_os == "android"', {
          # These flags are used by the Android OS.  They are probably overkill
          # for Skia, but we add them for consistency.
          'cflags': [
            '-W',
            '-Wall',
            '-fPIC',
            '-DPIC',
            '-DDARWIN_NO_CARBON',
            '-DFT2_BUILD_LIBRARY',
            '-O2',
          ],
        }],
      ],
    },
  ],
}
