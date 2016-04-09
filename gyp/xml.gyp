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
      ],
      'include_dirs': [
        '../include/private',
        '../include/xml',
      ],
      'sources': [
        '../include/xml/SkDOM.h',
        '../include/xml/SkXMLParser.h',
        '../include/xml/SkXMLWriter.h',

        '../src/xml/SkDOM.cpp',
        '../src/xml/SkXMLParser.cpp',
        '../src/xml/SkXMLPullParser.cpp',
        '../src/xml/SkXMLWriter.cpp',
      ],
      'sources!': [
          '../src/xml/SkXMLPullParser.cpp', #if 0 around class decl in header
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/xml',
        ],
      },
    },
  ],
}
