# Target for including zlib.
{
  'targets': [
    {
      'target_name': 'zlib',
      'type': 'static_library',
      'dependencies': [
        'skia_lib.gyp:skia_lib',
      ],
      'sources': [
        '../include/core/SkFlate.h',

        '../src/core/SkFlate.cpp',
      ],
      'conditions': [
        [ 'skia_os == "mac"', {
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/usr/lib/libz.dylib',
            ],
          },
          'defines': [ 'SK_ZLIB_INCLUDE=<zlib.h>', ],
        }],
        [ 'skia_os == "ios"', {
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/usr/lib/libz.dylib',
            ],
          },
          'defines': [ 'SK_ZLIB_INCLUDE=<zlib.h>', ],
        }],
        [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris", "android", "nacl"]', {
          'link_settings': { 'libraries': [ '-lz', ], },
          'conditions': [
            [ 'skia_android_framework==0', {
              'defines': [ 'SK_ZLIB_INCLUDE=<zlib.h>', ],
            }],
          ],
        }],
        [ 'skia_android_framework', {
          'include_dirs': [
            'external/zlib',
          ],
        }],
      ],
    },
  ],
}
