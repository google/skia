/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGPoly_DEFINED
#define SkSVGPoly_DEFINED

#include "experimental/svg/model/SkSVGShape.h"
#include "include/core/SkPath.h"

// Handles <polygon> and <polyline> elements.
class SkSVGPoly final : public SkSVGShape {
public:
    ~SkSVGPoly() override = default;

    static sk_sp<SkSVGPoly> MakePolygon() {
        return sk_sp<SkSVGPoly>(new SkSVGPoly(SkSVGTag::kPolygon));
    }

    static sk_sp<SkSVGPoly> MakePolyline() {
        return sk_sp<SkSVGPoly>(new SkSVGPoly(SkSVGTag::kPolyline));
    }

    void setPoints(const SkSVGPointsType&);

protected:
    void onSetAttribute(SkSVGAttribute, const SkSVGValue&) override;

    void onDraw(SkCanvas*, const SkSVGLengthContext&, const SkPaint&,
                SkPath::FillType) const override;

    SkPath onAsPath(const SkSVGRenderContext&) const override;

private:
    SkSVGPoly(SkSVGTag);

    mutable SkPath fPath;  // mutated in onDraw(), to apply inherited fill types.

    typedef SkSVGShape INHERITED;
};

#endif // SkSVGPoly_DEFINED
