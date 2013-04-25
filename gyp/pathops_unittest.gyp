# GYP file to build unit tests.
{
  'includes': [
    'apptype_console.gypi',
  ],
  'targets': [
    {
      'target_name': 'pathops_unittest',
      'type': 'executable',
      'suppress_wildcard': '1',
      'include_dirs' : [
        '../src/core',
        '../src/effects',
        '../src/lazy',
        '../src/pathops',
        '../src/pdf',
        '../src/pipe/utils',
        '../src/utils',
        '../tools/',
      ],
      'includes': [
        'pathops_unittest.gypi',
      ],
      'sources': [
        '../tests/Test.cpp',
        '../tests/skia_test.cpp',
        '../tests/Test.h',
      ],
      'dependencies': [
        'skia_base_libs.gyp:skia_base_libs',
        'effects.gyp:effects',
        'flags.gyp:flags',
        'images.gyp:images',
        'utils.gyp:utils',
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
