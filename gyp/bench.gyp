# GYP file to build performance testbench.
#
{
  'includes': [
    'apptype_console.gypi',
  ],
  'targets': [
    {
      'target_name': 'nanobench',
      'type': 'executable',
      'sources': [
        '../gm/gm.cpp',
        '../bench/DecodingBench.cpp',
        '../bench/DecodingSubsetBench.cpp',
        '../bench/GMBench.cpp',
        '../bench/RecordingBench.cpp',
        '../bench/SKPBench.cpp',
        '../bench/nanobench.cpp',
      ],
      'includes': [
        'bench.gypi',
        'gmslides.gypi',
      ],
      'dependencies': [
        'flags.gyp:flags_common',
        'jsoncpp.gyp:jsoncpp',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:crash_handler',
        'tools.gyp:proc_stats',
        'tools.gyp:timer',
      ],
      'conditions': [
        ['skia_android_framework', {
          'libraries': [
            '-lskia',
          ],
        }],
      ],
    },
  ],
}
