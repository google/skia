/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGFe_DEFINED
#define SkSVGFe_DEFINED

#include <vector>

#include "modules/svg/include/SkSVGHiddenContainer.h"

class SkImageFilter;
class SkSVGFilterContext;

class SkSVGFe : public SkSVGHiddenContainer {
public:
    static bool IsFilterEffect(const sk_sp<SkSVGNode>& node) {
        return node->tag() == SkSVGTag::kFeTurbulence || node->tag() == SkSVGTag::kFeColorMatrix ||
               node->tag() == SkSVGTag::kFeComposite || node->tag() == SkSVGTag::kFeFlood;
    }

    sk_sp<SkImageFilter> makeImageFilter(const SkSVGRenderContext& ctx,
                                         const SkSVGFilterContext& fctx) const;

    // https://www.w3.org/TR/SVG11/filters.html#FilterPrimitiveSubRegion
    SkRect resolveFilterSubregion(const SkSVGRenderContext&, const SkSVGFilterContext&) const;

    SVG_ATTR(In, SkSVGFeInputType, SkSVGFeInputType(SkSVGFeInputType::Type::kSourceGraphic))
    SVG_ATTR(Result, SkSVGStringType, SkSVGStringType())
    SVG_OPTIONAL_ATTR(X, SkSVGLength)
    SVG_OPTIONAL_ATTR(Y, SkSVGLength)
    SVG_OPTIONAL_ATTR(Width, SkSVGLength)
    SVG_OPTIONAL_ATTR(Height, SkSVGLength)

protected:
    explicit SkSVGFe(SkSVGTag t) : INHERITED(t) {}

    virtual sk_sp<SkImageFilter> onMakeImageFilter(const SkSVGRenderContext&,
                                                   const SkSVGFilterContext&) const = 0;

    virtual std::vector<SkSVGFeInputType> getInputs() const = 0;

    bool parseAndSetAttribute(const char*, const char*) override;

private:
    using INHERITED = SkSVGHiddenContainer;
};

#endif  // SkSVGFe_DEFINED
