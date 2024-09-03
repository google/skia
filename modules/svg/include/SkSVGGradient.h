/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGGradient_DEFINED
#define SkSVGGradient_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/private/base/SkAPI.h"
#include "include/private/base/SkTArray.h"
#include "modules/svg/include/SkSVGHiddenContainer.h"
#include "modules/svg/include/SkSVGNode.h"
#include "modules/svg/include/SkSVGTypes.h"

class SkPaint;
class SkSVGRenderContext;
class SkSVGStop;
class SkShader;
enum class SkTileMode;

class SK_API SkSVGGradient : public SkSVGHiddenContainer {
public:
    SVG_ATTR(Href, SkSVGIRI, SkSVGIRI())
    SVG_ATTR(GradientTransform, SkSVGTransformType, SkSVGTransformType(SkMatrix::I()))
    SVG_ATTR(SpreadMethod, SkSVGSpreadMethod, SkSVGSpreadMethod(SkSVGSpreadMethod::Type::kPad))
    SVG_ATTR(GradientUnits,
             SkSVGObjectBoundingBoxUnits,
             SkSVGObjectBoundingBoxUnits(SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox))

protected:
    explicit SkSVGGradient(SkSVGTag t) : INHERITED(t) {}

    bool parseAndSetAttribute(const char*, const char*) override;

    bool onAsPaint(const SkSVGRenderContext&, SkPaint*) const final;

    virtual sk_sp<SkShader> onMakeShader(const SkSVGRenderContext&,
                                         const SkColor4f*, const SkScalar*, int count,
                                         SkTileMode, const SkMatrix& localMatrix) const = 0;

private:
    using StopPositionArray = skia_private::STArray<2, SkScalar , true>;
    using    StopColorArray = skia_private::STArray<2, SkColor4f, true>;
    void collectColorStops(const SkSVGRenderContext&, StopPositionArray*, StopColorArray*) const;
    SkColor4f resolveStopColor(const SkSVGRenderContext&, const SkSVGStop&) const;

    using INHERITED = SkSVGHiddenContainer;
};

#endif // SkSVGGradient_DEFINED
