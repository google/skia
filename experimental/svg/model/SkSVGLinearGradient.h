/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGLinearGradient_DEFINED
#define SkSVGLinearGradient_DEFINED

#include "SkSVGGradient.h"
#include "SkSVGTypes.h"

class SkSVGLinearGradient : public SkSVGGradient {
public:
    ~SkSVGLinearGradient() override = default;
    static sk_sp<SkSVGLinearGradient> Make() {
        return sk_sp<SkSVGLinearGradient>(new SkSVGLinearGradient());
    }

    void setX1(const SkSVGLength&);
    void setY1(const SkSVGLength&);
    void setX2(const SkSVGLength&);
    void setY2(const SkSVGLength&);

protected:
    void onSetAttribute(SkSVGAttribute, const SkSVGValue&) override;

    sk_sp<SkShader> onMakeShader(const SkSVGRenderContext&,
                                 const SkColor*, const SkScalar*, int count,
                                 SkShader::TileMode, const SkMatrix&) const override;
private:
    SkSVGLinearGradient();

    void collectColorStops(const SkSVGRenderContext&,
                           SkSTArray<2, SkScalar, true>*,
                           SkSTArray<2, SkColor, true>*) const;

    SkSVGLength fX1 = SkSVGLength(0  , SkSVGLength::Unit::kPercentage);
    SkSVGLength fY1 = SkSVGLength(0  , SkSVGLength::Unit::kPercentage);
    SkSVGLength fX2 = SkSVGLength(100, SkSVGLength::Unit::kPercentage);
    SkSVGLength fY2 = SkSVGLength(0  , SkSVGLength::Unit::kPercentage);

    typedef SkSVGGradient INHERITED;
};

#endif // SkSVGLinearGradient_DEFINED
