# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# TODO: This file was copied from the external dependency
# third_party/externals/jsoncpp-chromium/jsoncpp.gyp , at revision 125399,
# with directory paths modified to work at this level.
#
# It would be better for us to depend on that gypfile within the external
# dependency, but so far we have been unable to make that work reliably.
# See https://code.google.com/p/skia/issues/detail?id=1023
# and https://code.google.com/p/skia/source/detail?r=7115

{
  'targets': [
    {
      'target_name': 'jsoncpp',
      'type': 'static_library',
      'defines': [
        'JSON_USE_EXCEPTION=0',
      ],
      'sources': [
        '../third_party/externals/jsoncpp/include/json/assertions.h',
        '../third_party/externals/jsoncpp/include/json/autolink.h',
        '../third_party/externals/jsoncpp/include/json/config.h',
        '../third_party/externals/jsoncpp/include/json/features.h',
        '../third_party/externals/jsoncpp/include/json/forwards.h',
        '../third_party/externals/jsoncpp/include/json/json.h',
        '../third_party/externals/jsoncpp/include/json/reader.h',
        '../third_party/externals/jsoncpp-chromium/overrides/include/json/value.h',
        '../third_party/externals/jsoncpp/include/json/writer.h',
        '../third_party/externals/jsoncpp/src/lib_json/json_batchallocator.h',
        '../third_party/externals/jsoncpp/src/lib_json/json_reader.cpp',
        '../third_party/externals/jsoncpp/src/lib_json/json_tool.h',
        '../third_party/externals/jsoncpp-chromium/overrides/src/lib_json/json_value.cpp',
        '../third_party/externals/jsoncpp/src/lib_json/json_writer.cpp',
      ],
      'include_dirs': [
        '../third_party/externals/jsoncpp-chromium/overrides/include/',
        '../third_party/externals/jsoncpp/include/',
        '../third_party/externals/jsoncpp/src/lib_json/',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../third_party/externals/jsoncpp-chromium/overrides/include/',
          '../third_party/externals/jsoncpp/include/',
        ],
      },
      'conditions': [
        [ 'skia_os == "mac"', {
          'xcode_settings': {
            'OTHER_CPLUSPLUSFLAGS!': [
              '-Werror',
            ]
          },
        }],
        [ 'skia_os == "win"', {
          'msvs_settings': {
            'VCCLCompilerTool': {
              'WarnAsError': 'false',
            },
          },
        }],
        [ 'skia_os == "android"', {
          'cflags!': [
            '-Wall',
            '-Werror',
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
