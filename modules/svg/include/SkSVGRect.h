/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGRect_DEFINED
#define SkSVGRect_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"
#include "modules/svg/include/SkSVGNode.h"
#include "modules/svg/include/SkSVGShape.h"
#include "modules/svg/include/SkSVGTypes.h"
#include "src/base/SkTLazy.h"

class SkCanvas;
class SkPaint;
class SkRRect;
class SkSVGLengthContext;
class SkSVGRenderContext;
enum class SkPathFillType;

class SK_API SkSVGRect final : public SkSVGShape {
public:
    static sk_sp<SkSVGRect> Make() { return sk_sp<SkSVGRect>(new SkSVGRect()); }

    SVG_ATTR(X     , SkSVGLength, SkSVGLength(0))
    SVG_ATTR(Y     , SkSVGLength, SkSVGLength(0))
    SVG_ATTR(Width , SkSVGLength, SkSVGLength(0))
    SVG_ATTR(Height, SkSVGLength, SkSVGLength(0))

    SVG_OPTIONAL_ATTR(Rx, SkSVGLength)
    SVG_OPTIONAL_ATTR(Ry, SkSVGLength)

protected:
    bool parseAndSetAttribute(const char*, const char*) override;

    void onDraw(SkCanvas*, const SkSVGLengthContext&, const SkPaint&,
                SkPathFillType) const override;

    SkPath onAsPath(const SkSVGRenderContext&) const override;

    SkRect onTransformableObjectBoundingBox(const SkSVGRenderContext&) const override;

private:
    SkSVGRect();

    SkRRect resolve(const SkSVGLengthContext&) const;

    using INHERITED = SkSVGShape;
};

#endif // SkSVGRect_DEFINED
