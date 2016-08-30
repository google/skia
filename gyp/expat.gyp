# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Build expat from source.

{
    'targets': [
    {
      'target_name': 'expat',
      'type': 'none',
      'conditions': [
        [ 'skia_android_framework', {
            'dependencies':              [ 'android_deps.gyp:expat_android' ],
            'export_dependent_settings': [ 'android_deps.gyp:expat_android' ],
        },{
            'dependencies':              [ 'expat.gyp:expat_static' ],
            'export_dependent_settings': [ 'expat.gyp:expat_static' ],
        }]
      ]
    },
    {
        'target_name': 'expat_static',
        'type': 'static_library',
        'cflags': [ '-Wno-missing-field-initializers' ],
        'xcode_settings': { 'WARNING_CFLAGS': [ '-Wno-missing-field-initializers', ], },
        'msvs_disabled_warnings': [4244],
        'defines': [
            'HAVE_EXPAT_CONFIG_H',
            'XML_STATIC', # Compile for static linkage.
        ],
        'include_dirs': [
            '../third_party/externals/expat',
        ],
        'sources': [
            '../third_party/externals/expat/lib/xmlparse.c',
            '../third_party/externals/expat/lib/xmlrole.c',
            '../third_party/externals/expat/lib/xmltok.c',
        ],
        'direct_dependent_settings': {
            'include_dirs': [
                '../third_party/externals/expat/lib',
            ],
            'defines': [
                'XML_STATIC',  # Tell dependants to expect static linkage.
            ],
        },
    }]
}
