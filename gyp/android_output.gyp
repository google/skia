# GYP file to send android SkDebug to stdout in addition to logcat. To enable,
# include this project as a dependency.
{
  'targets': [
    {
      'target_name': 'android_output',
      'type': 'static_library',
      'sources': [
        '../tools/AndroidSkDebugToStdOut.cpp',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
      ],
    },
  ],
}
