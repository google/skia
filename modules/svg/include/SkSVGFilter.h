/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGFilter_DEFINED
#define SkSVGFilter_DEFINED

#include "modules/svg/include/SkSVGHiddenContainer.h"
#include "modules/svg/include/SkSVGTypes.h"

class SkSVGFilter final : public SkSVGHiddenContainer {
public:
    ~SkSVGFilter() override = default;

    static sk_sp<SkSVGFilter> Make() { return sk_sp<SkSVGFilter>(new SkSVGFilter()); }

    sk_sp<SkImageFilter> buildFilterDAG(const SkSVGRenderContext&) const;

    SVG_ATTR(X, SkSVGLength, SkSVGLength(-10, SkSVGLength::Unit::kPercentage))
    SVG_ATTR(Y, SkSVGLength, SkSVGLength(-10, SkSVGLength::Unit::kPercentage))
    SVG_ATTR(Width, SkSVGLength, SkSVGLength(120, SkSVGLength::Unit::kPercentage))
    SVG_ATTR(Height, SkSVGLength, SkSVGLength(120, SkSVGLength::Unit::kPercentage))
    SVG_ATTR(FilterUnits,
             SkSVGObjectBoundingBoxUnits,
             SkSVGObjectBoundingBoxUnits(SkSVGObjectBoundingBoxUnits::Type::kObjectBoundingBox))

private:
    SkSVGFilter() : INHERITED(SkSVGTag::kFilter) {}

    SkRect resolveFilterRegion(const SkSVGRenderContext&) const;

    void onSetAttribute(SkSVGAttribute, const SkSVGValue&) override;

    using INHERITED = SkSVGHiddenContainer;
};

#endif  // SkSVGFilter_DEFINED
