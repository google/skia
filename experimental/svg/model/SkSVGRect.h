/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGRect_DEFINED
#define SkSVGRect_DEFINED

#include "experimental/svg/model/SkSVGShape.h"
#include "experimental/svg/model/SkSVGTypes.h"

class SkRRect;

class SkSVGRect final : public SkSVGShape {
public:
    ~SkSVGRect() override = default;
    static sk_sp<SkSVGRect> Make() { return sk_sp<SkSVGRect>(new SkSVGRect()); }

    void setX(const SkSVGLength&);
    void setY(const SkSVGLength&);
    void setWidth(const SkSVGLength&);
    void setHeight(const SkSVGLength&);
    void setRx(const SkSVGLength&);
    void setRy(const SkSVGLength&);

protected:
    void onSetAttribute(SkSVGAttribute, const SkSVGValue&) override;

    void onDraw(SkCanvas*, const SkSVGLengthContext&, const SkPaint&,
                SkPathFillType) const override;

    SkPath onAsPath(const SkSVGRenderContext&) const override;

private:
    SkSVGRect();

    SkRRect resolve(const SkSVGLengthContext&) const;

    SkSVGLength fX      = SkSVGLength(0);
    SkSVGLength fY      = SkSVGLength(0);
    SkSVGLength fWidth  = SkSVGLength(0);
    SkSVGLength fHeight = SkSVGLength(0);

    // The x radius for rounded rects.
    SkSVGLength fRx     = SkSVGLength(0);
    // The y radius for rounded rects.
    SkSVGLength fRy     = SkSVGLength(0);

    typedef SkSVGShape INHERITED;
};

#endif // SkSVGRect_DEFINED
