# GYP file to build a the webtry sample.
{
  'targets': [
    {
      'target_name': 'webtry',
      'type': 'executable',
      'include_dirs' : [
        '../src/core',
        '../src/images',
        '../src/pathops',
        ],
       'sources': [
         '../experimental/webtry/result.cpp',
         '../experimental/webtry/main.cpp',
       ],
       'dependencies': [
         'flags.gyp:flags',
         'skia_lib.gyp:skia_lib',
         'images.gyp:images',
       ],
    },
  ],
}
