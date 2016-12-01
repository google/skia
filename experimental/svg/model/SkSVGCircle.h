/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGCircle_DEFINED
#define SkSVGCircle_DEFINED

#include "SkSVGShape.h"
#include "SkSVGTypes.h"

class SkSVGCircle final : public SkSVGShape {
public:
    virtual ~SkSVGCircle() = default;
    static sk_sp<SkSVGCircle> Make() { return sk_sp<SkSVGCircle>(new SkSVGCircle()); }

    void setCx(const SkSVGLength&);
    void setCy(const SkSVGLength&);
    void setR(const SkSVGLength&);

protected:
    void onSetAttribute(SkSVGAttribute, const SkSVGValue&) override;

    void onDraw(SkCanvas*, const SkSVGLengthContext&, const SkPaint&,
                SkPath::FillType) const override;

private:
    SkSVGCircle();

    SkSVGLength fCx = SkSVGLength(0);
    SkSVGLength fCy = SkSVGLength(0);
    SkSVGLength fR  = SkSVGLength(0);

    typedef SkSVGShape INHERITED;
};

#endif // SkSVGCircle_DEFINED
