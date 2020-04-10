/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGRadialGradient_DEFINED
#define SkSVGRadialGradient_DEFINED

#include "experimental/svg/model/SkSVGGradient.h"
#include "experimental/svg/model/SkSVGTypes.h"

class SkSVGRadialGradient final : public SkSVGGradient {
public:
    ~SkSVGRadialGradient() override = default;
    static sk_sp<SkSVGRadialGradient> Make() {
        return sk_sp<SkSVGRadialGradient>(new SkSVGRadialGradient());
    }

    void setCx(const SkSVGLength&);
    void setCy(const SkSVGLength&);
    void setR(const SkSVGLength&);
    void setFx(const SkSVGLength&);
    void setFy(const SkSVGLength&);

protected:
    void onSetAttribute(SkSVGAttribute, const SkSVGValue&) override;

    sk_sp<SkShader> onMakeShader(const SkSVGRenderContext&,
                                 const SkColor*, const SkScalar*, int count,
                                 SkTileMode, const SkMatrix&) const override;
private:
    SkSVGRadialGradient();

    SkSVGLength fCx = SkSVGLength(50, SkSVGLength::Unit::kPercentage);
    SkSVGLength fCy = SkSVGLength(50, SkSVGLength::Unit::kPercentage);
    SkSVGLength fR  = SkSVGLength(50, SkSVGLength::Unit::kPercentage);
    SkTLazy<SkSVGLength> fFx;
    SkTLazy<SkSVGLength> fFy;


   typedef SkSVGGradient INHERITED;
};

#endif // SkSVGRadialGradient_DEFINED
