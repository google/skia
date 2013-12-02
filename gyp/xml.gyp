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
        '../include/xml',
      ],
      'sources': [
        '../include/xml/SkBML_WXMLParser.h',
        '../include/xml/SkBML_XMLParser.h',
        '../include/xml/SkDOM.h',
        '../include/xml/SkJS.h',
        '../include/xml/SkXMLParser.h',
        '../include/xml/SkXMLWriter.h',

        '../src/xml/SkBML_Verbs.h',
        '../src/xml/SkBML_XMLParser.cpp',
        '../src/xml/SkDOM.cpp',
        '../src/xml/SkJS.cpp',
        '../src/xml/SkJSDisplayable.cpp',
        '../src/xml/SkXMLParser.cpp',
        '../src/xml/SkXMLPullParser.cpp',
        '../src/xml/SkXMLWriter.cpp',
      ],
      'sources!': [
          '../src/xml/SkXMLPullParser.cpp', #if 0 around class decl in header
      ],
      'conditions': [
        [ 'skia_os in ["win", "mac", "linux", "freebsd", "openbsd", "solaris", "android", "ios", "nacl", "chromeos"]', {
          'sources!': [
            # no jsapi.h by default on system
            '../include/xml/SkJS.h',
            '../src/xml/SkJS.cpp',
            '../src/xml/SkJSDisplayable.cpp',
          ],
        }],
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/xml',
        ],
      },
    },
  ],
}
