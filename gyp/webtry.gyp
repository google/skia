# GYP file to build a the webtry sample.
{
  'targets': [
    {
      'target_name': 'webtry',
      'type': 'executable',
      'mac_bundle' : 1,
      'include_dirs' : [
        '../src/core',
        '../src/images',
        ],
       'sources': [
         '../experimental/webtry/result.cpp',
       ],
       'dependencies': [
         'flags.gyp:flags',
         'skia_lib.gyp:skia_lib',
         'images.gyp:images',
       ],
    },
  ],
}
