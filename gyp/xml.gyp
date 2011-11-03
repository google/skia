{
  'includes': [
    'common.gypi',
  ],
  'targets': [
    {
      'target_name': 'xml',
      'type': 'static_library',
      'include_dirs': [
        '../include/config',
        '../include/core',
        '../include/xml',
        '../include/utils',
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
        [ 'skia_os in ["win", "mac", "linux", "freebsd", "openbsd", "solaris", "android"]', {
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

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
