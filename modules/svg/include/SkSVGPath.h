/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGPath_DEFINED
#define SkSVGPath_DEFINED

#include "include/core/SkPath.h"
#include "modules/svg/include/SkSVGShape.h"

class SkSVGPath final : public SkSVGShape {
public:
    static sk_sp<SkSVGPath> Make() { return sk_sp<SkSVGPath>(new SkSVGPath()); }

    void setPath(const SkPath& path) { fPath = path; }

protected:
    void onSetAttribute(SkSVGAttribute, const SkSVGValue&) override;

    void onDraw(SkCanvas*, const SkSVGLengthContext&, const SkPaint&,
                SkPathFillType) const override;

    SkPath onAsPath(const SkSVGRenderContext&) const override;

private:
    SkSVGPath();

    mutable SkPath fPath; // mutated in onDraw(), to apply inherited fill types.

    using INHERITED = SkSVGShape;
};

#endif // SkSVGPath_DEFINED
