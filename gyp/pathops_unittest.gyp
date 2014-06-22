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
      ],
      'includes': [
        'pathops_unittest.gypi',
      ],
      'sources': [
        '../tests/PathOpsAngleIdeas.cpp',
        '../tests/PathOpsCubicLineIntersectionIdeas.cpp',
        '../tests/PathOpsDebug.cpp',
        '../tests/PathOpsOpLoopThreadedTest.cpp',
        '../tests/PathOpsSkpClipTest.cpp',
        '../tests/Test.cpp',
        '../tests/Test.h',
        '../tests/skia_test.cpp',
      ],
      'dependencies': [
        'flags.gyp:flags',
        'skia_lib.gyp:skia_lib',
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
