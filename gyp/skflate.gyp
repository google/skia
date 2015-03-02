# Target for including SkFlate.
{
  'targets': [
    {
      'target_name': 'skflate',
      'type': 'static_library',
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'zlib.gyp:zlib',
      ],
      'sources': [ '../src/core/SkFlate.cpp' ],
    },
  ],
}
