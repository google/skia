/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/svg/model/SkSVGAttribute.h"

SkSVGPresentationAttributes SkSVGPresentationAttributes::MakeInitial() {
    SkSVGPresentationAttributes result;

    result.fFill.set(SkSVGPaint(SkSVGColorType(SK_ColorBLACK)));
    result.fFillOpacity.set(SkSVGNumberType(1));
    result.fFillRule.set(SkSVGFillRule(SkSVGFillRule::Type::kNonZero));
    result.fClipRule.set(SkSVGFillRule(SkSVGFillRule::Type::kNonZero));

    result.fStroke.set(SkSVGPaint(SkSVGPaint::Type::kNone));
    result.fStrokeDashArray.set(SkSVGDashArray(SkSVGDashArray::Type::kNone));
    result.fStrokeDashOffset.set(SkSVGLength(0));
    result.fStrokeLineCap.set(SkSVGLineCap(SkSVGLineCap::Type::kButt));
    result.fStrokeLineJoin.set(SkSVGLineJoin(SkSVGLineJoin::Type::kMiter));
    result.fStrokeMiterLimit.set(SkSVGNumberType(4));
    result.fStrokeOpacity.set(SkSVGNumberType(1));
    result.fStrokeWidth.set(SkSVGLength(1));

    result.fVisibility.set(SkSVGVisibility(SkSVGVisibility::Type::kVisible));

    return result;
}
