/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGLinearGradient_DEFINED
#define SkSVGLinearGradient_DEFINED

#include "SkSVGHiddenContainer.h"
#include "SkSVGTypes.h"

class SkSVGLinearGradient : public SkSVGHiddenContainer {
public:
    ~SkSVGLinearGradient() override = default;
    static sk_sp<SkSVGLinearGradient> Make() {
        return sk_sp<SkSVGLinearGradient>(new SkSVGLinearGradient());
    }

    void setHref(const SkSVGStringType&);
    void setGradientTransform(const SkSVGTransformType&);
    void setSpreadMethod(const SkSVGSpreadMethod&);
    void setX1(const SkSVGLength&);
    void setY1(const SkSVGLength&);
    void setX2(const SkSVGLength&);
    void setY2(const SkSVGLength&);

protected:
    bool onAsPaint(const SkSVGRenderContext&, SkPaint*) const override;

    void onSetAttribute(SkSVGAttribute, const SkSVGValue&) override;

private:
    SkSVGLinearGradient();

    void collectColorStops(const SkSVGRenderContext&,
                           SkSTArray<2, SkScalar, true>*,
                           SkSTArray<2, SkColor, true>*) const;

    SkSVGLength fX1 = SkSVGLength(0  , SkSVGLength::Unit::kPercentage);
    SkSVGLength fY1 = SkSVGLength(0  , SkSVGLength::Unit::kPercentage);
    SkSVGLength fX2 = SkSVGLength(100, SkSVGLength::Unit::kPercentage);
    SkSVGLength fY2 = SkSVGLength(0  , SkSVGLength::Unit::kPercentage);

    SkSVGStringType    fHref;
    SkSVGTransformType fGradientTransform = SkSVGTransformType(SkMatrix::I());
    SkSVGSpreadMethod  fSpreadMethod = SkSVGSpreadMethod(SkSVGSpreadMethod::Type::kPad);

    typedef SkSVGHiddenContainer INHERITED;
};

#endif // SkSVGLinearGradient_DEFINED
