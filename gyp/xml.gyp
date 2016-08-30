# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'targets': [
    {
      'target_name': 'xml',
      'product_name': 'skia_xml',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'expat.gyp:expat',
      ],
      'include_dirs': [
        '<(skia_include_path)/private',
        '<(skia_include_path)/xml',
      ],
      'sources': [
        '<(skia_include_path)/xml/SkDOM.h',
        '<(skia_include_path)/xml/SkXMLParser.h',
        '<(skia_include_path)/xml/SkXMLWriter.h',

        '<(skia_src_path)/xml/SkDOM.cpp',
        '<(skia_src_path)/xml/SkXMLParser.cpp',
        '<(skia_src_path)/xml/SkXMLWriter.cpp',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/xml',
        ],
        'defines' : [
          'SK_XML',
        ],
      },
    },
  ],
}
