# GYP file to build unit tests.
{
  'includes': [
    'apptype_console.gypi',
    'common.gypi',
  ],
  'targets': [
    {
      'target_name': 'addTest',
      'type': 'executable',
      'include_dirs' : [
        '../src/core',
      ],
      'sources': [
        '../experimental/Intersection/AddTestOutput/main.cpp',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'experimental.gyp:experimental',
        'pdf.gyp:pdf',
      ],
      'conditions': [
        [ 'skia_gpu == 1', {
          'include_dirs': [
            '../src/gpu',
          ],
        }],
      ],
    },
  ],
}
