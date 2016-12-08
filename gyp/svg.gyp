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
      'target_name': 'svgdom',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'xml.gyp:xml',
      ],
      'include_dirs': [
        '<(skia_include_path)/private',
        '../experimental/svg/model',
      ],
      'sources': [
        '../experimental/svg/model/SkSVGAttribute.h',
        '../experimental/svg/model/SkSVGAttribute.cpp',
        '../experimental/svg/model/SkSVGAttributeParser.h',
        '../experimental/svg/model/SkSVGAttributeParser.cpp',
        '../experimental/svg/model/SkSVGCircle.h',
        '../experimental/svg/model/SkSVGCircle.cpp',
        '../experimental/svg/model/SkSVGClipPath.h',
        '../experimental/svg/model/SkSVGClipPath.cpp',
        '../experimental/svg/model/SkSVGContainer.h',
        '../experimental/svg/model/SkSVGContainer.cpp',
        '../experimental/svg/model/SkSVGDefs.h',
        '../experimental/svg/model/SkSVGDOM.h',
        '../experimental/svg/model/SkSVGDOM.cpp',
        '../experimental/svg/model/SkSVGEllipse.h',
        '../experimental/svg/model/SkSVGEllipse.cpp',
        '../experimental/svg/model/SkSVGG.h',
        '../experimental/svg/model/SkSVGHiddenContainer.h',
        '../experimental/svg/model/SkSVGIDMapper.h',
        '../experimental/svg/model/SkSVGLine.h',
        '../experimental/svg/model/SkSVGLine.cpp',
        '../experimental/svg/model/SkSVGLinearGradient.h',
        '../experimental/svg/model/SkSVGLinearGradient.cpp',
        '../experimental/svg/model/SkSVGNode.h',
        '../experimental/svg/model/SkSVGNode.cpp',
        '../experimental/svg/model/SkSVGPath.h',
        '../experimental/svg/model/SkSVGPath.cpp',
        '../experimental/svg/model/SkSVGPoly.h',
        '../experimental/svg/model/SkSVGPoly.cpp',
        '../experimental/svg/model/SkSVGRect.h',
        '../experimental/svg/model/SkSVGRect.cpp',
        '../experimental/svg/model/SkSVGRenderContext.h',
        '../experimental/svg/model/SkSVGRenderContext.cpp',
        '../experimental/svg/model/SkSVGShape.h',
        '../experimental/svg/model/SkSVGShape.cpp',
        '../experimental/svg/model/SkSVGStop.h',
        '../experimental/svg/model/SkSVGStop.cpp',
        '../experimental/svg/model/SkSVGSVG.h',
        '../experimental/svg/model/SkSVGSVG.cpp',
        '../experimental/svg/model/SkSVGTransformableNode.h',
        '../experimental/svg/model/SkSVGTransformableNode.cpp',
        '../experimental/svg/model/SkSVGTypes.h',
        '../experimental/svg/model/SkSVGValue.h',
        '../experimental/svg/model/SkSVGValue.cpp',

        '../experimental/svg/model/SkPEG.h',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../experimental/svg/model',
        ],
      },
    },

  ],
}
