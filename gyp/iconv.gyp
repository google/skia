# GYP for iconv
#
# NOTE: WINDOWS BUILD ONLY.
# iconv should be native to Mac and Linux.
#
# Based on instructions found on http://www.codeproject.com/Articles/302012/How-to-Build-libiconv-with-Microsoft-Visual-Studio
# See the relevant README.chromium file for more information.

{
  'variables': {
    'skia_warnings_as_errors': 0,
  },
  'targets': [
    {
      'target_name': 'iconv',
      'type': 'static_library',
      'include_dirs' : [
        '../third_party/externals/iconv/lib',
        '../third_party/iconv/config',
      ],
      'sources': [
        '../third_party/externals/iconv/lib/iconv.c',
        '../third_party/externals/iconv/libcharset/lib/localcharset.c',
      ],

      'conditions': [
        ['skia_os == "win"', {
          'include_dirs': [
            '../third_party/iconv/config/windows',
          ],
          'direct_dependent_settings': {
            'include_dirs': [
              '../third_party/iconv/config/windows',
            ],
          },
          'defines': [
            'LIBDIR',
          ],
        }],
      ],
    },
  ],
}
