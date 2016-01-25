# Copyright 2016 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
{
'targets': [{
  'target_name': 'piex-selector',
  'type': 'none',
  'conditions': [
    [ 'skia_android_framework', {
        'dependencies': [ 'android_deps.gyp:piex' ],
        'export_dependent_settings': [ 'android_deps.gyp:piex' ],
      }, {
        'dependencies': [ 'piex' ],
        'export_dependent_settings': [ 'piex' ],
    }]
  ]
},{
  'target_name': 'piex',
  'type': 'static_library',
  'sources': [
    '../third_party/externals/piex/src/piex.cc',
    '../third_party/externals/piex/src/tiff_parser.cc',
  ],
  'variables': {
    'headers': [
      '../third_party/externals/piex/src/piex.h',
      '../third_party/externals/piex/src/piex_types.h',
      '../third_party/externals/piex/src/tiff_parser.h',
    ],
  },
  'include_dirs': ['../third_party/externals/piex'],
  'cflags': [
    '-Wsign-compare',
    '-w',
  ],
  'msvs_settings': {
    'VCCLCompilerTool': {
      'WarningLevel': '0',
    },
  },
  'xcode_settings': {
    'WARNING_CFLAGS': ['-w'],
  },
  'dependencies': [
    'binary_parse',
    'image_type_recognition',
    'tiff_directory',
  ],
  'direct_dependent_settings': {
    'include_dirs': [
      '../third_party/externals/piex',
    ],
  },
}, {
  'target_name': 'binary_parse',
  'type': 'static_library',
  'sources': [
      '../third_party/externals/piex/src/binary_parse/cached_paged_byte_array.cc',
      '../third_party/externals/piex/src/binary_parse/range_checked_byte_ptr.cc',
  ],
  'variables': {
    'headers': [
      '../third_party/externals/piex/src/binary_parse/cached_paged_byte_array.h',
      '../third_party/externals/piex/src/binary_parse/range_checked_byte_ptr.h',
    ],
  },
  'include_dirs': ['../third_party/externals/piex'],
  'cflags': ['-Wsign-compare'],
}, {
  'target_name': 'image_type_recognition',
  'type': 'static_library',
  'sources': [
    '../third_party/externals/piex/src/image_type_recognition/image_type_recognition_lite.cc',
  ],
  'variables': {
    'headers': ['../third_party/externals/piex/src/image_type_recognition/image_type_recognition_lite.h'],
  },
  'include_dirs': ['../third_party/externals/piex'],
  'cflags': ['-Wsign-compare'],
  'dependencies': ['binary_parse'],
}, {
  'target_name': 'tiff_directory',
  'type': 'static_library',
  'sources': [
    '../third_party/externals/piex/src/tiff_directory/tiff_directory.cc',
  ],
  'variables': {
    'headers': ['../third_party/externals/piex/src/tiff_directory/tiff_directory.h'],
  },
  'include_dirs': ['../third_party/externals/piex'],
  'dependencies': ['binary_parse'],
}],
}
