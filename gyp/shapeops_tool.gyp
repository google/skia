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

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
