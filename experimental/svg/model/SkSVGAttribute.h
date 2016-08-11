/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGAttribute_DEFINED
#define SkSVGAttribute_DEFINED

#include "SkSVGTypes.h"
#include "SkTLazy.h"

class SkSVGRenderContext;

enum class SkSVGAttribute {
    kD,
    kFill,
    kFillOpacity,
    kHeight,
    kRx,
    kRy,
    kStroke,
    kStrokeOpacity,
    kStrokeLineCap,
    kStrokeLineJoin,
    kStrokeWidth,
    kTransform,
    kViewBox,
    kWidth,
    kX,
    kY,

    kUnknown,
};

struct SkSVGPresentationAttributes {
    static SkSVGPresentationAttributes MakeInitial();

    // TODO: SkTLazy adds an extra ptr per attribute; refactor to reduce overhead.

    SkTLazy<SkSVGPaint>      fFill;
    SkTLazy<SkSVGNumberType> fFillOpacity;

    SkTLazy<SkSVGPaint>      fStroke;
    SkTLazy<SkSVGLineCap>    fStrokeLineCap;
    SkTLazy<SkSVGLineJoin>   fStrokeLineJoin;
    SkTLazy<SkSVGNumberType> fStrokeOpacity;
    SkTLazy<SkSVGLength>     fStrokeWidth;
};

#endif // SkSVGAttribute_DEFINED
