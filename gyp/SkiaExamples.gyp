{
  'targets' : [
  {
    'target_name': 'SkiaExamples',
      'type': 'executable',
      'mac_bundle' : 1,
      'include_dirs' : [],
      'includes': [],
      'sources': [
        '../experimental/SkiaExamples/HelloSkiaExample.cpp',
        '../experimental/SkiaExamples/BaseExample.h',
        '../experimental/SkiaExamples/BaseExample.cpp',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
      'views.gyp:views',
      'xml.gyp:xml',
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

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
