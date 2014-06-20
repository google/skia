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
        'etc1.gyp:libetc1',
        'flags.gyp:flags',
        'jsoncpp.gyp:jsoncpp',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:crash_handler',
        'tools.gyp:resources',
        'tools.gyp:timer',
      ],
      'sources': [
        '../bench/BenchLogger.cpp',
        '../bench/BenchLogger.h',
        '../bench/GMBench.cpp',
        '../bench/GMBench.h',
        '../bench/ResultsWriter.cpp',
        '../bench/benchmain.cpp',
        '../tools/sk_tool_utils.cpp',
      ],
      'conditions': [
        ['skia_gpu == 1',
          {
            'include_dirs' : [
              '../src/gpu',
            ],
            'dependencies': [
              'gputest.gyp:skgputest',
            ],
          },
        ],
        ['skia_android_framework == 1',
          {
            'libraries': [
              '-lskia',
            ],
          },
        ],
      ],
      'includes': [
        'bench.gypi',
        'gmslides.gypi',
      ],
    },
  ],
}
