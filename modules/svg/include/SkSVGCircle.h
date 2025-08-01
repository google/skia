/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGCircle_DEFINED
#define SkSVGCircle_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/private/base/SkAPI.h"
#include "modules/svg/include/SkSVGNode.h"
#include "modules/svg/include/SkSVGShape.h"
#include "modules/svg/include/SkSVGTypes.h"

#include <tuple>

class SkCanvas;
class SkPaint;
class SkSVGLengthContext;
class SkSVGRenderContext;
struct SkPoint;

class SK_API SkSVGCircle final : public SkSVGShape {
public:
    static sk_sp<SkSVGCircle> Make() { return sk_sp<SkSVGCircle>(new SkSVGCircle()); }

    SVG_ATTR(Cx, SkSVGLength, SkSVGLength(0))
    SVG_ATTR(Cy, SkSVGLength, SkSVGLength(0))
    SVG_ATTR(R , SkSVGLength, SkSVGLength(0))

protected:
    bool parseAndSetAttribute(const char*, const char*) override;

    void onDraw(SkCanvas*, const SkSVGLengthContext&, const SkPaint&,
                SkPathFillType) const override;

    SkPath onAsPath(const SkSVGRenderContext&) const override;

    SkRect onTransformableObjectBoundingBox(const SkSVGRenderContext&) const override;

private:
    SkSVGCircle();

    // resolve and return the center and radius values
    std::tuple<SkPoint, SkScalar> resolve(const SkSVGLengthContext&) const;

    using INHERITED = SkSVGShape;
};

#endif // SkSVGCircle_DEFINED
