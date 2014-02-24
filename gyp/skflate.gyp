# Target for including SkFlate.
{
  'targets': [
    {
      'target_name': 'skflate',
      'type': 'static_library',
      'dependencies': [
        'skia_lib.gyp:skia_lib',
      ],
      'conditions': [
        # When zlib is not availible on a system,
        # SkFlate::HaveFlate will just return false.
        [ 'skia_os != "win"',
          {
            'dependencies': [
              'zlib.gyp:zlib',
            ],
          }
        ],
      ],
      'sources': [
        '../src/core/SkFlate.cpp',
      ],
    },
  ],
}
