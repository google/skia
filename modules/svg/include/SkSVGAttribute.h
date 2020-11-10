/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGAttribute_DEFINED
#define SkSVGAttribute_DEFINED

#include "modules/svg/include/SkSVGTypes.h"
#include "src/core/SkTLazy.h"

class SkSVGRenderContext;

enum class SkSVGAttribute {
    kClipRule,
    kColor,
    kCx, // <circle>, <ellipse>, <radialGradient>: center x position
    kCy, // <circle>, <ellipse>, <radialGradient>: center y position
    kD,
    kFill,
    kFillOpacity,
    kFillRule,
    kFilter,
    kFilterUnits,
    kFontFamily,
    kFontSize,
    kFontStyle,
    kFontWeight,
    kFx, // <radialGradient>: focal point x position
    kFy, // <radialGradient>: focal point y position
    kGradientUnits,
    kGradientTransform,
    kHeight,
    kHref,
    kOffset,
    kOpacity,
    kPatternTransform,
    kPoints,
    kPreserveAspectRatio,
    kR,  // <circle>, <radialGradient>: radius
    kRx, // <ellipse>,<rect>: horizontal (corner) radius
    kRy, // <ellipse>,<rect>: vertical (corner) radius
    kSpreadMethod,
    kStopColor,
    kStopOpacity,
    kStroke,
    kStrokeDashArray,
    kStrokeDashOffset,
    kStrokeOpacity,
    kStrokeLineCap,
    kStrokeLineJoin,
    kStrokeMiterLimit,
    kStrokeWidth,
    kTransform,
    kText,
    kTextAnchor,
    kViewBox,
    kVisibility,
    kWidth,
    kX,
    kX1, // <line>: first endpoint x
    kX2, // <line>: second endpoint x
    kY,
    kY1, // <line>: first endpoint y
    kY2, // <line>: second endpoint y

    kUnknown,
};

struct SkSVGPresentationAttributes {
    static SkSVGPresentationAttributes MakeInitial();

    // TODO: SkTLazy adds an extra ptr per attribute; refactor to reduce overhead.

    SkTLazy<SkSVGPaint>      fFill;
    SkTLazy<SkSVGNumberType> fFillOpacity;
    SkTLazy<SkSVGFillRule>   fFillRule;
    SkTLazy<SkSVGFillRule>   fClipRule;

    SkTLazy<SkSVGPaint>      fStroke;
    SkTLazy<SkSVGDashArray>  fStrokeDashArray;
    SkTLazy<SkSVGLength>     fStrokeDashOffset;
    SkTLazy<SkSVGLineCap>    fStrokeLineCap;
    SkTLazy<SkSVGLineJoin>   fStrokeLineJoin;
    SkTLazy<SkSVGNumberType> fStrokeMiterLimit;
    SkTLazy<SkSVGNumberType> fStrokeOpacity;
    SkTLazy<SkSVGLength>     fStrokeWidth;

    SkTLazy<SkSVGVisibility> fVisibility;

    SkTLazy<SkSVGColorType>  fColor;

    SkTLazy<SkSVGFontFamily> fFontFamily;
    SkTLazy<SkSVGFontStyle>  fFontStyle;
    SkTLazy<SkSVGFontSize>   fFontSize;
    SkTLazy<SkSVGFontWeight> fFontWeight;
    SkTLazy<SkSVGTextAnchor> fTextAnchor;

    // TODO(tdenniston): add SkSVGStopColor

    // uninherited
    SkTLazy<SkSVGNumberType> fOpacity;
    SkTLazy<SkSVGClip>       fClipPath;
    SkTLazy<SkSVGFilterType> fFilter;
};

#endif // SkSVGAttribute_DEFINED
