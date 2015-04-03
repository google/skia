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
      'target_name': 'libpng',
      'type': 'none',
      'conditions': [
        [ 'skia_os == "android"',
          {
            'dependencies': [
              'android_deps.gyp:png',
            ],
            'export_dependent_settings': [
              'android_deps.gyp:png',
            ],
          }, {  # skia_os != "android"
            'dependencies': [
              'libpng.gyp:libpng_static',
            ],
            'export_dependent_settings': [
              'libpng.gyp:libpng_static',
            ],
          }
        ]
      ]
    },
    {
      'target_name': 'libpng_static',
      'type': 'static_library',
      'standalone_static_library': 1,
      'include_dirs': [
        # Needed for generated pnglibconf.h and pngprefix.h
        '../third_party/libpng',
        '../third_party/externals/libpng',
      ],
      'dependencies': [
        'zlib.gyp:zlib',
      ],
      'export_dependent_settings': [
        'zlib.gyp:zlib',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../third_party/externals/libpng',
          # Needed for generated pnglibconf.h and pngprefix.h
          '../third_party/libpng',
        ],
        'defines': [
          'SKIA_PNG_PREFIXED',
        ],
      },
      'cflags': [
        '-w',
        '-fvisibility=hidden',
      ],
      'sources': [
        '../third_party/externals/libpng/png.c',
        '../third_party/externals/libpng/pngerror.c',
        '../third_party/externals/libpng/pngget.c',
        '../third_party/externals/libpng/pngmem.c',
        '../third_party/externals/libpng/pngpread.c',
        '../third_party/externals/libpng/pngread.c',
        '../third_party/externals/libpng/pngrio.c',
        '../third_party/externals/libpng/pngrtran.c',
        '../third_party/externals/libpng/pngrutil.c',
        '../third_party/externals/libpng/pngset.c',
        '../third_party/externals/libpng/pngtrans.c',
        '../third_party/externals/libpng/pngwio.c',
        '../third_party/externals/libpng/pngwrite.c',
        '../third_party/externals/libpng/pngwtran.c',
        '../third_party/externals/libpng/pngwutil.c',
      ],
      'conditions': [
        [ 'skia_os == "ios"', {
          # explicitly disable looking for NEON on iOS builds
          'defines': [
            'PNG_ARM_NEON_OPT=0',
          ],
        }, { # skia_os != "ios" 
          'dependencies': [
            'libpng.gyp:libpng_static_neon',
          ],
        }],
      ],
    },
    {
      'target_name': 'libpng_static_neon',
      'type': 'static_library',
      'include_dirs': [
        # Needed for generated pnglibconf.h and pngprefix.h
        '../third_party/libpng',
        '../third_party/externals/libpng',
      ],
      'dependencies': [
        'zlib.gyp:zlib',
      ],
     'sources': [
        '../third_party/externals/libpng/arm/arm_init.c',
        '../third_party/externals/libpng/arm/filter_neon.S',
        '../third_party/externals/libpng/arm/filter_neon_intrinsics.c',
      ],
      'conditions': [
        ['arm_neon_optional', {
          'cflags': [
            '-mfpu=neon',
          ],
        }],
      ],
    }
  ]
}
