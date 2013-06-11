# Copyright 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This is a copy of ../third_party/externals/libjpeg/libjpeg.gyp , modified
# such that all source paths point into that directory.
# See http://code.google.com/p/skia/issues/detail?id=543 ('wrap libjpeg.gyp
# from Chrome's libjpeg port, rather than making our own copy') for a better
# long-term solution.

{
  'variables': {
    'use_system_libjpeg%': 0,
    'skia_warnings_as_errors': 0,
  },
  'conditions': [
    ['skia_os == "android"', {
      'targets': [
        {
          'target_name': 'libjpeg',
          'type': 'none',
          'dependencies': [
            'android_deps.gyp:jpeg',
          ],
          'export_dependent_settings': [
            'android_deps.gyp:jpeg',
          ],
        },
      ],
    }, { # skia_os != android
      'conditions': [
          ['use_system_libjpeg==0', {
          'targets': [
            {
              'target_name': 'libjpeg',
              'type': 'static_library',
              'sources': [
                # we currently build skia's version of libjpeg-turbo without
                # SIMD optimizations for simplicity
                '../third_party/externals/libjpeg/jsimd_none.c',

                '../third_party/externals/libjpeg/jcapimin.c',
                '../third_party/externals/libjpeg/jcapistd.c',
                '../third_party/externals/libjpeg/jccoefct.c',
                '../third_party/externals/libjpeg/jccolor.c',
                '../third_party/externals/libjpeg/jcdctmgr.c',
                '../third_party/externals/libjpeg/jchuff.c',
                '../third_party/externals/libjpeg/jchuff.h',
                '../third_party/externals/libjpeg/jcinit.c',
                '../third_party/externals/libjpeg/jcmainct.c',
                '../third_party/externals/libjpeg/jcmarker.c',
                '../third_party/externals/libjpeg/jcmaster.c',
                '../third_party/externals/libjpeg/jcomapi.c',
                '../third_party/externals/libjpeg/jconfig.h',
                '../third_party/externals/libjpeg/jcparam.c',
                '../third_party/externals/libjpeg/jcphuff.c',
                '../third_party/externals/libjpeg/jcprepct.c',
                '../third_party/externals/libjpeg/jcsample.c',
                '../third_party/externals/libjpeg/jdapimin.c',
                '../third_party/externals/libjpeg/jdapistd.c',
                '../third_party/externals/libjpeg/jdatadst.c',
                '../third_party/externals/libjpeg/jdatasrc.c',
                '../third_party/externals/libjpeg/jdcoefct.c',
                '../third_party/externals/libjpeg/jdcolor.c',
                '../third_party/externals/libjpeg/jdct.h',
                '../third_party/externals/libjpeg/jddctmgr.c',
                '../third_party/externals/libjpeg/jdhuff.c',
                '../third_party/externals/libjpeg/jdhuff.h',
                '../third_party/externals/libjpeg/jdinput.c',
                '../third_party/externals/libjpeg/jdmainct.c',
                '../third_party/externals/libjpeg/jdmarker.c',
                '../third_party/externals/libjpeg/jdmaster.c',
                '../third_party/externals/libjpeg/jdmerge.c',
                '../third_party/externals/libjpeg/jdphuff.c',
                '../third_party/externals/libjpeg/jdpostct.c',
                '../third_party/externals/libjpeg/jdsample.c',
                '../third_party/externals/libjpeg/jerror.c',
                '../third_party/externals/libjpeg/jerror.h',
                '../third_party/externals/libjpeg/jfdctflt.c',
                '../third_party/externals/libjpeg/jfdctfst.c',
                '../third_party/externals/libjpeg/jfdctint.c',
                '../third_party/externals/libjpeg/jidctflt.c',
                '../third_party/externals/libjpeg/jidctfst.c',
                '../third_party/externals/libjpeg/jidctint.c',
                '../third_party/externals/libjpeg/jidctred.c',
                '../third_party/externals/libjpeg/jinclude.h',
                '../third_party/externals/libjpeg/jmemmgr.c',
                '../third_party/externals/libjpeg/jmemnobs.c',
                '../third_party/externals/libjpeg/jmemsys.h',
                '../third_party/externals/libjpeg/jmorecfg.h',
                '../third_party/externals/libjpeg/jpegint.h',
                '../third_party/externals/libjpeg/jpeglib.h',
                '../third_party/externals/libjpeg/jpeglibmangler.h',
                '../third_party/externals/libjpeg/jquant1.c',
                '../third_party/externals/libjpeg/jquant2.c',
                '../third_party/externals/libjpeg/jutils.c',
                '../third_party/externals/libjpeg/jversion.h',
              ],
              'direct_dependent_settings': {
                'include_dirs': [
                  '../third_party/externals/libjpeg',
                ],
              },
              'conditions': [
                [ 'skia_os != "win"', {
                  'product_name': 'jpeg',
                  'cflags': [
                   '-Wno-main', # supresses warnings about naming things "main"
                  ],
                }],
              ],
            },
          ],
        }, {
          'targets': [
            {
              'target_name': 'libjpeg',
              'type': 'none',
              'direct_dependent_settings': {
                'defines': [
                  'USE_SYSTEM_LIBJPEG',
                ],
              },
              'link_settings': {
                'libraries': [
                  '-ljpeg',
                ],
              },
            }
          ],
        }],
      ],
    }],
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
