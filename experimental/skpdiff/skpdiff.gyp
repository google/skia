# GYP file to build skpdiff.
#
# To build on Linux:
#  ./gyp_skia skpdiff.gyp && make skpdiff
#
{
  'targets': [
    {
      'target_name': 'skpdiff',
      'type': 'executable',
      'sources': [
        'main.cpp',
        'SkDiffContext.cpp',
        'SkImageDiffer.cpp',
        'SkPMetric.cpp',
        'skpdiff_util.cpp',
        '../../tools/flags/SkCommandLineFlags.cpp',
      ],
      'include_dirs': [
        '../../tools/flags'
      ],
      'dependencies': [
        '../../gyp/skia_lib.gyp:skia_lib',
      ],
      'cflags': [
        '-O3',
      ],
      'conditions': [
        [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris", "chromeos"]', {
          'link_settings': {
            'libraries': [
              '-lrt',
            ],
          },
        }],
        ['skia_opencl', {
          'sources': [
            'SkCLImageDiffer.cpp',
            'SkDifferentPixelsMetric_opencl.cpp',
          ],
          'conditions': [
            [ 'skia_os == "mac"', {
              'link_settings': {
                'libraries': [
                  '$(SDKROOT)/System/Library/Frameworks/OpenCL.framework',
                ]
              }
            }, {
              'link_settings': {
                'libraries': [
                  '-lOpenCL',
                ],
              },
            }],
          ],
        }, { # !skia_opencl
          'sources': [
            'SkDifferentPixelsMetric_cpu.cpp',
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
