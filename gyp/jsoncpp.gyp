# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'variables': {
    'skia_warnings_as_errors': 0,
  },
  'targets': [
    {
      'target_name': 'jsoncpp',
      'conditions': [
        ['skia_use_system_json', {
          'type': 'none',
          'direct_dependent_settings': {
            'libraries': [
              'jsoncpp.a',
            ],
          },
        }, {
          'type': 'static_library',
          'defines': [
            'JSON_USE_EXCEPTION=0',
          ],
          'sources': [
            '../third_party/externals/jsoncpp/src/lib_json/json_reader.cpp',
            '../third_party/externals/jsoncpp/src/lib_json/json_value.cpp',
            '../third_party/externals/jsoncpp/src/lib_json/json_writer.cpp',
          ],
          'include_dirs': [
            '../third_party/externals/jsoncpp/include/',
            '../third_party/externals/jsoncpp/src/lib_json/',
          ],
          'direct_dependent_settings': {
            'include_dirs': [
              '../third_party/externals/jsoncpp/include/',
            ],
          },
          'cflags': [
            '-w',
          ],
        }],
      ],
    },
  ],
}
