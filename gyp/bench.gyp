# GYP file to build performance testbench.
#
{
  'includes': [
    'apptype_console.gypi',
  ],
  'targets': [
    {
      'target_name': 'bench',
      'type': 'executable',
      'dependencies': [
        'flags.gyp:flags',
        'jsoncpp.gyp:jsoncpp',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:crash_handler',
        'tools.gyp:timer',
      ],
      'sources': [
        '../bench/BenchLogger.cpp',
        '../bench/BenchLogger.h',
        '../bench/GMBench.cpp',
        '../bench/GMBench.h',
        '../bench/ResultsWriter.cpp',
        '../bench/benchmain.cpp',
      ],
      'conditions': [
        ['skia_android_framework == 1', {
          'libraries': [ '-lskia' ],
        }],
      ],
      'includes': [
        'bench.gypi',
        'gmslides.gypi',
      ],
    },
    {
      'target_name': 'nanobench',
      'type': 'executable',
      'sources': [
        '../bench/nanobench.cpp',
      ],
      'includes': [ 'bench.gypi' ],
      'dependencies': [
        'flags.gyp:flags',
        'tools.gyp:crash_handler',
        'tools.gyp:timer',
      ],
    },
  ],
}
