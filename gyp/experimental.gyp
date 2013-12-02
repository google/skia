# GYP file to build experimental directory.
{
  'targets': [
    {
      'target_name': 'experimental',
      'type': 'static_library',
      'include_dirs': [
        '../include/config',
        '../include/core',
      ],
      'sources': [
        '../experimental/SkSetPoly3To3.cpp',
        '../experimental/SkSetPoly3To3_A.cpp',
        '../experimental/SkSetPoly3To3_D.cpp',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../experimental',
        ],
      },
    },
    {
      'target_name': 'SkiaExamples',
      'type': 'executable',
      'mac_bundle' : 1,
      'include_dirs' : [
        '../tools/flags',
        ],
      'includes': [],
       'sources': [
         '../experimental/SkiaExamples/SkExample.h',
         '../experimental/SkiaExamples/SkExample.cpp',
         '../experimental/SkiaExamples/HelloSkiaExample.cpp',
       ],
       'dependencies': [
         'skia_lib.gyp:skia_lib',
         'views.gyp:views',
         'xml.gyp:xml',
         'flags.gyp:flags'
       ],
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

# SkiaExamples specific files
          '../experimental/SkiaExamples/SkiaExamples-Info.plist',
          '../experimental/SkiaExamples/SkExampleNSView.h',
          '../experimental/SkiaExamples/SkExampleNSView.mm',

# Mac files
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
        'link_settings': {
        },
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
