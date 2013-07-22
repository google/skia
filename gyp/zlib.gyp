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
          'defines': [ 'SK_ZLIB_INCLUDE=<zlib.h>', ],
        }],
      ],
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
