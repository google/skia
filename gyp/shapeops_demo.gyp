
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
        '../experimental/Intersection/ConvexHull.cpp',
        '../experimental/Intersection/CubeRoot.cpp',
        '../experimental/Intersection/CubicBezierClip.cpp',
        '../experimental/Intersection/CubicBounds.cpp',
        '../experimental/Intersection/CubicIntersection.cpp',
        '../experimental/Intersection/CubicReduceOrder.cpp',
        '../experimental/Intersection/CubicSubDivide.cpp',
        '../experimental/Intersection/CubicToQuadratics.cpp',
        '../experimental/Intersection/CubicUtilities.cpp',
        '../experimental/Intersection/DataTypes.cpp',
        '../experimental/Intersection/EdgeDemo.cpp',
        '../experimental/Intersection/EdgeDemoApp.mm',
        '../experimental/Intersection/EdgeWalker.cpp',
        '../experimental/Intersection/EdgeWalker_TestUtility.cpp',
        '../experimental/Intersection/Extrema.cpp',
        '../experimental/Intersection/Intersections.cpp',
        '../experimental/Intersection/LineCubicIntersection.cpp',
        '../experimental/Intersection/LineIntersection.cpp',
        '../experimental/Intersection/LineParameterization.cpp',
        '../experimental/Intersection/LineQuadraticIntersection.cpp',
        '../experimental/Intersection/LineUtilities.cpp',
        '../experimental/Intersection/QuadraticBezierClip.cpp',
        '../experimental/Intersection/QuadraticBounds.cpp',
        '../experimental/Intersection/QuadraticIntersection.cpp',
        '../experimental/Intersection/QuadraticImplicit.cpp',
        '../experimental/Intersection/QuadraticLineSegments.cpp',
        '../experimental/Intersection/QuadraticParameterization.cpp',
        '../experimental/Intersection/QuadraticReduceOrder.cpp',
        '../experimental/Intersection/QuadraticSubDivide.cpp',
        '../experimental/Intersection/QuadraticUtilities.cpp',
        '../experimental/Intersection/QuarticRoot.cpp',
        '../experimental/Intersection/ShapeOps.cpp',
        '../experimental/Intersection/Simplify.cpp',
        '../experimental/Intersection/TriangleUtilities.cpp',
        '../experimental/Intersection/CubicParameterization.cpp',
        '../experimental/Intersection/CubicReduceOrder.cpp',
        '../experimental/Intersection/CubicSubDivide.cpp',
        '../experimental/Intersection/CubicUtilities.h',
        '../experimental/Intersection/CurveIntersection.h',
        '../experimental/Intersection/DataTypes.h',
        '../experimental/Intersection/EdgeDemo.h',
        '../experimental/Intersection/Extrema.h',
        '../experimental/Intersection/Intersections.h',
        '../experimental/Intersection/IntersectionUtilities.h',
        '../experimental/Intersection/LineIntersection.h',
        '../experimental/Intersection/LineParameters.h',
        '../experimental/Intersection/LineUtilities.h',
        '../experimental/Intersection/QuadraticLineSegments.h',
        '../experimental/Intersection/QuadraticParameterization.h',
        '../experimental/Intersection/QuadraticUtilities.h',
        '../experimental/Intersection/QuarticRoot.h',
        '../experimental/Intersection/ShapeOps.h',
        '../experimental/Intersection/Simplify.h',
        '../experimental/Intersection/TSearch.h',
        '../experimental/Intersection/TriangleUtilities.h',
     ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'views.gyp:views',
        'xml.gyp:xml',
      ],
      'conditions' : [
        [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris", "chromeos"]', {
        }],
        [ 'skia_os == "win"', {
        }],
        [ 'skia_os == "mac"', {
          'sources': [

            # Mac files
            '../src/views/mac/SkEventNotifier.h',
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
