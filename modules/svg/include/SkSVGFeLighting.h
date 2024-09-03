/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGFeLighting_DEFINED
#define SkSVGFeLighting_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"
#include "modules/svg/include/SkSVGFe.h"
#include "modules/svg/include/SkSVGNode.h"
#include "modules/svg/include/SkSVGTypes.h"
#include "src/base/SkTLazy.h"

#include <vector>

class SkImageFilter;
class SkSVGFeDistantLight;
class SkSVGFePointLight;
class SkSVGFeSpotLight;
class SkSVGFilterContext;
class SkSVGRenderContext;

class SK_API SkSVGFeLighting : public SkSVGFe {
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

    virtual sk_sp<SkImageFilter> makeDistantLight(const SkSVGRenderContext&,
                                                  const SkSVGFilterContext&,
                                                  const SkSVGFeDistantLight*) const = 0;

    virtual sk_sp<SkImageFilter> makePointLight(const SkSVGRenderContext&,
                                                const SkSVGFilterContext&,
                                                const SkSVGFePointLight*) const = 0;

    virtual sk_sp<SkImageFilter> makeSpotLight(const SkSVGRenderContext&,
                                               const SkSVGFilterContext&,
                                               const SkSVGFeSpotLight*) const = 0;

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

    sk_sp<SkImageFilter> makeDistantLight(const SkSVGRenderContext&,
                                          const SkSVGFilterContext&,
                                          const SkSVGFeDistantLight*) const final;

    sk_sp<SkImageFilter> makePointLight(const SkSVGRenderContext&,
                                        const SkSVGFilterContext&,
                                        const SkSVGFePointLight*) const final;

    sk_sp<SkImageFilter> makeSpotLight(const SkSVGRenderContext&,
                                       const SkSVGFilterContext&,
                                       const SkSVGFeSpotLight*) const final;

private:
    SkSVGFeSpecularLighting() : INHERITED(SkSVGTag::kFeSpecularLighting) {}

    using INHERITED = SkSVGFeLighting;
};

class SkSVGFeDiffuseLighting final : public SkSVGFeLighting {
public:
    static sk_sp<SkSVGFeDiffuseLighting> Make() {
        return sk_sp<SkSVGFeDiffuseLighting>(new SkSVGFeDiffuseLighting());
    }

    SVG_ATTR(DiffuseConstant, SkSVGNumberType, 1)

protected:
    bool parseAndSetAttribute(const char*, const char*) override;

    sk_sp<SkImageFilter> makeDistantLight(const SkSVGRenderContext&,
                                          const SkSVGFilterContext&,
                                          const SkSVGFeDistantLight*) const final;

    sk_sp<SkImageFilter> makePointLight(const SkSVGRenderContext&,
                                        const SkSVGFilterContext&,
                                        const SkSVGFePointLight*) const final;

    sk_sp<SkImageFilter> makeSpotLight(const SkSVGRenderContext&,
                                       const SkSVGFilterContext&,
                                       const SkSVGFeSpotLight*) const final;

private:
    SkSVGFeDiffuseLighting() : INHERITED(SkSVGTag::kFeDiffuseLighting) {}

    using INHERITED = SkSVGFeLighting;
};

#endif  // SkSVGFeLighting_DEFINED
