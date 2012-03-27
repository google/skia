
{
  'includes': [
    'common.gypi',
  ],
  'targets': [
    {
      'target_name': 'shapeops_demo',
      'type': 'executable',
      'mac_bundle' : 1,
      'include_dirs' : [
        '../experimental/SimpleCocoaApp', # needed to get SimpleApp.h
      ],
      'sources': [
        '../experimental/Intersection/DataTypes.cpp',
        '../experimental/Intersection/EdgeWalker.cpp',
        '../experimental/Intersection/EdgeWalker_TestUtility.cpp',
        '../experimental/Intersection/LineIntersection.cpp',
        '../experimental/Intersection/LineParameterization.cpp',
        '../experimental/Intersection/LineUtilities.cpp',
        '../experimental/Intersection/EdgeDemoApp.mm',
      ],
      'dependencies': [
        'core.gyp:core',
        'effects.gyp:effects',
        'images.gyp:images',
        'ports.gyp:ports',
        'views.gyp:views',
        'utils.gyp:utils',
        'animator.gyp:animator',
        'xml.gyp:xml',
        'svg.gyp:svg',
        'experimental.gyp:experimental',
        'gpu.gyp:gr',
        'gpu.gyp:skgr',
        'pdf.gyp:pdf',
      ],
      'conditions' : [
       [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris"]', {
        }],
        [ 'skia_os == "win"', {
        }],
        [ 'skia_os == "mac"', {
          'sources': [
            
            # Mac files
            '../src/utils/mac/SkEventNotifier.h',
            '../src/utils/mac/SkEventNotifier.mm',
            '../src/utils/mac/skia_mac.mm',
            '../src/utils/mac/SkNSView.h',
            '../src/utils/mac/SkNSView.mm',
            '../src/utils/mac/SkOptionsTableView.h',
            '../src/utils/mac/SkOptionsTableView.mm',
            '../src/utils/mac/SkOSWindow_Mac.mm',
            '../src/utils/mac/SkTextFieldCell.h',
            '../src/utils/mac/SkTextFieldCell.m',
          ],
          'libraries': [
            '$(SDKROOT)/System/Library/Frameworks/QuartzCore.framework',
            '$(SDKROOT)/System/Library/Frameworks/OpenGL.framework',
          ],
          'xcode_settings' : {
            'INFOPLIST_FILE' : '../experimental/Intersection/EdgeDemoApp-Info.plist',
          },
          'mac_bundle_resources' : [
            '../experimental/Intersection/EdgeDemoApp.xib',
          ],
        }],
      ],
      'msvs_settings': {
        'VCLinkerTool': {
          'SubSystem': '2',
          'AdditionalDependencies': [
            'd3d9.lib',
          ],
        },
      },
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
