# Build ALMOST everything provided by Skia; this should be the default target.
#
# This omits the following targets that many developers won't want to build:
# - debugger: this requires QT to build
#
{
  'targets': [
    {
      'target_name': 'most',
      'type': 'none',
      'dependencies': [
        # The minimal set of static libraries for basic Skia functionality.
        'skia_lib.gyp:skia_lib',

        'bench.gyp:bench',
        'gm.gyp:gm',
        'SampleApp.gyp:SampleApp',
        'tests.gyp:tests',
        'tools.gyp:tools',
        'pathops_unittest.gyp:*',
#       'pdfviewer.gyp:pdfviewer',
      ],
      'conditions': [
        ['skia_os == "android"', {
          'dependencies': [ 'android_system.gyp:SampleApp_APK' ],
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
