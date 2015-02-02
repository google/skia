# Build ALMOST everything provided by Skia; this should be the default target.
#
# This omits the following targets that many developers won't want to build:
# - debugger: this requires QT to build
#
{
  'variables': {
    'skia_skip_gui%': 0,
  },
  'targets': [
    {
      'target_name': 'most',
      'type': 'none',
      'dependencies': [
        # The minimal set of static libraries for basic Skia functionality.
        'skia_lib.gyp:skia_lib',

        'bench.gyp:*',
        'example.gyp:HelloWorld',
        'SampleApp.gyp:SampleApp',
        'tools.gyp:tools',
        'pathops_unittest.gyp:*',
        'pathops_skpclip.gyp:*',
#       'pdfviewer.gyp:pdfviewer',
        'dm.gyp:dm',
      ],
      'conditions': [
        [ 'skia_gpu == 0 or skia_os == "android"', {
          'dependencies!': [
            'example.gyp:HelloWorld',
          ],
        }],
        ['skia_os == "android"', {
          'dependencies': [ 'android_system.gyp:SampleApp_APK' ],
        }],
        ['skia_os == "ios"', {
          'dependencies!': [
            'example.gyp:HelloWorld',
            'SampleApp.gyp:SampleApp',
          ],
          'dependencies': ['iOSShell.gyp:iOSShell' ],
        }],
        ['skia_os == "mac" or skia_os == "linux"', {
          'dependencies': [ 'nanomsg.gyp:*' ],
        }],
        [ 'skia_skip_gui',
          {
            'dependencies!': [
              'example.gyp:HelloWorld',
              'SampleApp.gyp:SampleApp',
            ]
          }
        ]
      ],
    },
  ],
}
