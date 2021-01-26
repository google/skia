/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGFeLighting_DEFINED
#define SkSVGFeLighting_DEFINED

#include "modules/svg/include/SkSVGFe.h"
#include "modules/svg/include/SkSVGTypes.h"

class SkSVGFePointLight;

class SkSVGFeLighting : public SkSVGFe {
public:
    struct KernelUnitLength {
        SkSVGNumberType fDx;
        SkSVGNumberType fDy;
    };

    SVG_ATTR(SurfaceScale, SkSVGNumberType, 1)
    SVG_OPTIONAL_ATTR(KernelUnitLength, KernelUnitLength)

protected:
    explicit SkSVGFeLighting(SkSVGTag t) : INHERITED(t) {}

    std::vector<SkSVGFeInputType> getInputs() const final { return {this->getIn()}; }

    bool parseAndSetAttribute(const char*, const char*) override;

    sk_sp<SkImageFilter> onMakeImageFilter(const SkSVGRenderContext&,
                                           const SkSVGFilterContext&) const final;

    virtual sk_sp<SkImageFilter> makePointLight(const SkSVGRenderContext&,
                                                const SkSVGFilterContext&,
                                                const SkSVGFePointLight*) const = 0;

    SkColor resolveLightingColor(const SkSVGRenderContext&) const;

    SkPoint3 resolveXYZ(const SkSVGRenderContext&,
                        const SkSVGFilterContext&,
                        SkSVGNumberType,
                        SkSVGNumberType,
                        SkSVGNumberType) const;

private:
    using INHERITED = SkSVGFe;
};

class SkSVGFeSpecularLighting final : public SkSVGFeLighting {
public:
    static sk_sp<SkSVGFeSpecularLighting> Make() {
        return sk_sp<SkSVGFeSpecularLighting>(new SkSVGFeSpecularLighting());
    }

    SVG_ATTR(SpecularConstant, SkSVGNumberType, 1)
    SVG_ATTR(SpecularExponent, SkSVGNumberType, 1)

protected:
    bool parseAndSetAttribute(const char*, const char*) override;

    sk_sp<SkImageFilter> makePointLight(const SkSVGRenderContext&,
                                        const SkSVGFilterContext&,
                                        const SkSVGFePointLight*) const final;

private:
    SkSVGFeSpecularLighting() : INHERITED(SkSVGTag::kFeSpecularLighting) {}

    using INHERITED = SkSVGFeLighting;
};

#endif  // SkSVGFeLighting_DEFINED
