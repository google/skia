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
        'SkImageDiffer.cpp',
        'SkCLImageDiffer.cpp',
        'skpdiff_util.cpp',
      ],
      'dependencies': [
        '../../gyp/skia_lib.gyp:skia_lib',
      ],
      'link_settings': {
        'libraries': [
          '-lOpenCL',
        ],
      },
    },
  ],
  'conditions': [
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
