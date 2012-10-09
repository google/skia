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
        'core.gyp:core',
        'effects.gyp:effects',
        'experimental.gyp:experimental',
        'images.gyp:images',
        'ports.gyp:ports',
        'pdf.gyp:pdf',
        'utils.gyp:utils',
      ],
      'conditions': [
        [ 'skia_gpu == 1', {
          'include_dirs': [
            '../src/gpu',
          ],
          'dependencies': [
            'gpu.gyp:gr',
            'gpu.gyp:skgr',
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
