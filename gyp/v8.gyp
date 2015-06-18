# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# GYP file to build a V8 sample.
{
  'targets': [
    {
      'target_name': 'SkV8Example',
      'type': 'executable',
      'mac_bundle' : 1,
      'include_dirs' : [
        '../third_party/externals/v8/include',
        '../third_party/externals/v8',
      ],
      'sources': [
        '../experimental/SkV8Example/DrawingMethods.cpp',
        '../experimental/SkV8Example/DrawingMethods.h',
        '../experimental/SkV8Example/Global.cpp',
        '../experimental/SkV8Example/Global.h',
        '../experimental/SkV8Example/JsContext.cpp',
        '../experimental/SkV8Example/JsContext.h',
        '../experimental/SkV8Example/Path2DBuilder.cpp',
        '../experimental/SkV8Example/Path2DBuilder.h',
        '../experimental/SkV8Example/Path2D.cpp',
        '../experimental/SkV8Example/Path2D.h',
        '../experimental/SkV8Example/SkV8Example.cpp',
        '../experimental/SkV8Example/SkV8Example.h',
      ],
      'dependencies': [
        'flags.gyp:flags',
        'skia_lib.gyp:skia_lib',
        'views.gyp:views',
        'xml.gyp:xml',
      ],
      'link_settings': {
        'libraries': [

#         'd:/src/v8/build/Debug/lib/v8_base.ia32.lib',
#         'd:/src/v8/build/Debug/lib/v8_snapshot.lib',
#         'd:/src/v8/build/Debug/lib/icuuc.lib',
#         'd:/src/v8/build/Debug/lib/icui18n.lib',
#         'Ws2_32.lib',
#         'Winmm.lib',

          '-lpthread',
          '-lrt',
          '../../third_party/externals/v8/out/native/obj.target/tools/gyp/libv8_base.a',
          '../../third_party/externals/v8/out/native/obj.target/tools/gyp/libv8_libbase.a',
          '../../third_party/externals/v8/out/native/obj.target/tools/gyp/libv8_snapshot.a',
          '../../third_party/externals/v8/out/native/obj.target/tools/gyp/libv8_libplatform.a',
          '../../third_party/externals/v8/out/native/obj.target/third_party/icu/libicudata.a',
          '../../third_party/externals/v8/out/native/obj.target/third_party/icu/libicui18n.a',
          '../../third_party/externals/v8/out/native/obj.target/third_party/icu/libicuuc.a',
          '../../third_party/externals/v8/out/native/obj.target/icudata/third_party/icu/linux/icudtl_dat.o',
        ],
      },
      'conditions' : [
        [ 'skia_gpu == 1', {
          'include_dirs' : [
            '../src/gpu',
          ]
        }],
      ],
    }
  ],
}
