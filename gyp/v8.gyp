# GYP file to build a V8 sample.
{
  'targets': [
    {
      'target_name': 'SkV8Example',
      'type': 'executable',
      'mac_bundle' : 1,
      'include_dirs' : [
        '../tools/flags',
        '../../../v8/include',
        ],
      'includes': [],
       'sources': [
         '../experimental/SkV8Example/SkV8Example.h',
         '../experimental/SkV8Example/SkV8Example.cpp',
       ],
       'dependencies': [
         'skia_lib.gyp:skia_lib',
         'views.gyp:views',
         'xml.gyp:xml',
       ],

        'link_settings': {
          'libraries': [

#            'd:/src/v8/build/Debug/lib/v8_base.ia32.lib',
#            'd:/src/v8/build/Debug/lib/v8_snapshot.lib',

#            'd:/src/v8/build/Debug/lib/icuuc.lib',
#            'd:/src/v8/build/Debug/lib/icui18n.lib',

#            'Ws2_32.lib',
#            'Winmm.lib',

            '-lpthread',
            '-lrt',
            '../../../v8/out/native/obj.target/tools/gyp/libv8_base.x64.a',
          '../../../v8/out/native/obj.target/tools/gyp/libv8_snapshot.a',

          '../../../v8/out/native/obj.target/third_party/icu/libicudata.a',
          '../../../v8/out/native/obj.target/third_party/icu/libicui18n.a',
          '../../../v8/out/native/obj.target/third_party/icu/libicuuc.a',

          '../../../v8/out/native/obj.target/icudata/third_party/icu/linux/icudt46l_dat.o',
            ],
        },
       'conditions' : [
         [ 'skia_gpu == 1', {
           'include_dirs' : [
             '../src/gpu',  #gl/GrGLUtil.h
             ]
         }],
        [ 'skia_os == "win"', {
         'sources' : [
           '../src/views/win/SkOSWindow_Win.cpp',
           '../src/views/win/skia_win.cpp',
           ],
          },
        ],

        [ 'skia_os == "mac"', {
          'sources': [

          '../src/views/mac/SampleAppDelegate.h',
          '../src/views/mac/SampleAppDelegate.mm',
          '../src/views/mac/SkEventNotifier.mm',
          '../src/views/mac/skia_mac.mm',
          '../src/views/mac/SkNSView.h',
          '../src/views/mac/SkNSView.mm',
          '../src/views/mac/SkOptionsTableView.h',
          '../src/views/mac/SkOptionsTableView.mm',
          '../src/views/mac/SkOSWindow_Mac.mm',
          '../src/views/mac/SkTextFieldCell.h',
          '../src/views/mac/SkTextFieldCell.m',
          ],
        'include_dirs' : [
          '../src/views/mac/'
          ],
        'xcode_settings' : {
          'INFOPLIST_FILE' : '../experimental/SkiaExamples/SkiaExamples-Info.plist',
        },
        'mac_bundle_resources' : [
          '../experimental/SkiaExamples/SkiaExamples.xib'
          ],
        }
      ],
     ],
    }
  ],
}
