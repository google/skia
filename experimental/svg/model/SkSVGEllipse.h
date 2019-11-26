/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGEllipse_DEFINED
#define SkSVGEllipse_DEFINED

#include "experimental/svg/model/SkSVGShape.h"
#include "experimental/svg/model/SkSVGTypes.h"

struct SkRect;

class SkSVGEllipse final : public SkSVGShape {
public:
    ~SkSVGEllipse() override = default;
    static sk_sp<SkSVGEllipse> Make() { return sk_sp<SkSVGEllipse>(new SkSVGEllipse()); }

    void setCx(const SkSVGLength&);
    void setCy(const SkSVGLength&);
    void setRx(const SkSVGLength&);
    void setRy(const SkSVGLength&);

protected:
    void onSetAttribute(SkSVGAttribute, const SkSVGValue&) override;

    void onDraw(SkCanvas*, const SkSVGLengthContext&, const SkPaint&,
                SkPath::FillType) const override;

    SkPath onAsPath(const SkSVGRenderContext&) const override;

private:
    SkSVGEllipse();

    SkRect resolve(const SkSVGLengthContext&) const;

    SkSVGLength fCx = SkSVGLength(0);
    SkSVGLength fCy = SkSVGLength(0);
    SkSVGLength fRx = SkSVGLength(0);
    SkSVGLength fRy = SkSVGLength(0);

    typedef SkSVGShape INHERITED;
};

#endif // SkSVGEllipse_DEFINED
