# Target for including SkFlate.
{
  'targets': [
    {
      'target_name': 'skflate',
      'type': 'static_library',
      'dependencies': [ 'skia_lib.gyp:skia_lib' ],
      'conditions': [
        [ 'skia_android_framework', {
            'dependencies': [ 'zlib.gyp:zlib' ]
        },{
            'dependencies': [ 'zlib.gyp:miniz' ]   # Our bots.
        }],
      ],
      'sources': [ '../src/core/SkFlate.cpp' ],
    },
  ],
}
