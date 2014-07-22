# GYP for "dm" (Diamond Master, a.k.a Dungeon master, a.k.a GM 2).
{
    'includes': [ 'apptype_console.gypi' ],

    'targets': [{
        'target_name': 'dm',
        'type': 'executable',
        'includes': [
          'dm.gypi',
        ],
        'conditions': [
          ['skia_android_framework', {
              'libraries': [ '-lskia' ],
          }],
          ['skia_poppler_enabled', {
              'sources':      [ '../src/utils/SkPDFRasterizer.cpp' ],
              'defines':      [ 'SK_BUILD_POPPLER' ],
              'dependencies': [ 'poppler.gyp:*' ],
          }],
        ],
    }]
}
