# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'targets': [
    {
      'target_name': 'svg',
      'product_name': 'skia_svg',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'xml.gyp:*',
      ],
      'include_dirs': [
        '../include/private',
        '../include/svg',
        '../src/core',
      ],
      'sources': [
        '<(skia_include_path)/svg/SkSVGCanvas.h',

        '<(skia_src_path)/svg/SkSVGCanvas.cpp',
        '<(skia_src_path)/svg/SkSVGDevice.cpp',
        '<(skia_src_path)/svg/SkSVGDevice.h',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/svg',
        ],
      },
    },
    {
      'target_name': 'svg_parser',
      'product_name': 'skia_svg_parser',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'xml.gyp:*',
      ],
      'include_dirs': [
        '../include/svg/parser',
      ],
      'sources': [
        '../include/svg/parser/SkSVGAttribute.h',
        '../include/svg/parser/SkSVGBase.h',
        '../include/svg/parser/SkSVGPaintState.h',
        '../include/svg/parser/SkSVGParser.h',
        '../include/svg/parser/SkSVGTypes.h',

        '../src/svg/parser/SkSVGCircle.cpp',
        '../src/svg/parser/SkSVGCircle.h',
        '../src/svg/parser/SkSVGClipPath.cpp',
        '../src/svg/parser/SkSVGClipPath.h',
        '../src/svg/parser/SkSVGDefs.cpp',
        '../src/svg/parser/SkSVGDefs.h',
        '../src/svg/parser/SkSVGElements.cpp',
        '../src/svg/parser/SkSVGElements.h',
        '../src/svg/parser/SkSVGEllipse.cpp',
        '../src/svg/parser/SkSVGEllipse.h',
        '../src/svg/parser/SkSVGFeColorMatrix.cpp',
        '../src/svg/parser/SkSVGFeColorMatrix.h',
        '../src/svg/parser/SkSVGFilter.cpp',
        '../src/svg/parser/SkSVGFilter.h',
        '../src/svg/parser/SkSVGG.cpp',
        '../src/svg/parser/SkSVGG.h',
        '../src/svg/parser/SkSVGGradient.cpp',
        '../src/svg/parser/SkSVGGradient.h',
        '../src/svg/parser/SkSVGGroup.cpp',
        '../src/svg/parser/SkSVGGroup.h',
        '../src/svg/parser/SkSVGImage.cpp',
        '../src/svg/parser/SkSVGImage.h',
        '../src/svg/parser/SkSVGLine.cpp',
        '../src/svg/parser/SkSVGLine.h',
        '../src/svg/parser/SkSVGLinearGradient.cpp',
        '../src/svg/parser/SkSVGLinearGradient.h',
        '../src/svg/parser/SkSVGMask.cpp',
        '../src/svg/parser/SkSVGMask.h',
        '../src/svg/parser/SkSVGMetadata.cpp',
        '../src/svg/parser/SkSVGMetadata.h',
        '../src/svg/parser/SkSVGPaintState.cpp',
        '../src/svg/parser/SkSVGParser.cpp',
        '../src/svg/parser/SkSVGPath.cpp',
        '../src/svg/parser/SkSVGPath.h',
        '../src/svg/parser/SkSVGPolygon.cpp',
        '../src/svg/parser/SkSVGPolygon.h',
        '../src/svg/parser/SkSVGPolyline.cpp',
        '../src/svg/parser/SkSVGPolyline.h',
        '../src/svg/parser/SkSVGRadialGradient.cpp',
        '../src/svg/parser/SkSVGRadialGradient.h',
        '../src/svg/parser/SkSVGRect.cpp',
        '../src/svg/parser/SkSVGRect.h',
        '../src/svg/parser/SkSVGStop.cpp',
        '../src/svg/parser/SkSVGStop.h',
        '../src/svg/parser/SkSVGSVG.cpp',
        '../src/svg/parser/SkSVGSVG.h',
        '../src/svg/parser/SkSVGSymbol.cpp',
        '../src/svg/parser/SkSVGSymbol.h',
        '../src/svg/parser/SkSVGText.cpp',
        '../src/svg/parser/SkSVGText.h',
        '../src/svg/parser/SkSVGUse.cpp',
      ],
      'sources!' : [
          '../src/svg/parser/SkSVG.cpp', # doesn't compile, maybe this is test code?
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/svg/parser',
        ],
      },
    },
  ],
}
