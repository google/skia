# Top-level gyp configuration for Skia.
#
# Projects that use Skia should depend on one or more of the targets
# defined here.
#
# More targets are defined within the gyp/ directory, but those are
# not intended for external use and may change without notice.
#
# Full documentation at http://code.google.com/p/skia/wiki/DocRoot
#
{
  'targets': [
    {
      # Use this target to build everything provided by Skia.
      'target_name': 'all',
      'type': 'none',
      'dependencies': [
        'gyp/bench.gyp:bench',
        'gyp/gm.gyp:gm',
        'gyp/SampleApp.gyp:SampleApp',
        'gyp/tests.gyp:tests',
        'gyp/tools.gyp:tools',
      ],
      'conditions': [
        ['skia_os == "android" and android_make_apk == 1', {
          'dependencies': [
            'gyp/android_system.gyp:SkiaAndroidApp',
          ],
        }],

        # The debugger is not supported for iOS, Android and 32-bit Mac builds.
        ['skia_os != "ios" and skia_os != "android" and (skia_os != "mac" or skia_arch_width == 64)', {
          'dependencies': [ 'gyp/debugger.gyp:debugger' ],
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
