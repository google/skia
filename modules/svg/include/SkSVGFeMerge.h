/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGFeMerge_DEFINED
#define SkSVGFeMerge_DEFINED

#include "modules/svg/include/SkSVGFe.h"
#include "modules/svg/include/SkSVGHiddenContainer.h"
#include "modules/svg/include/SkSVGNode.h"
#include "modules/svg/include/SkSVGTypes.h"

// https://www.w3.org/TR/SVG11/filters.html#feMergeNodeElement
class SK_API SkSVGFeMergeNode : public SkSVGHiddenContainer {
public:
    static constexpr SkSVGTag tag = SkSVGTag::kFeMergeNode;

    static sk_sp<SkSVGFeMergeNode> Make() {
        return sk_sp<SkSVGFeMergeNode>(new SkSVGFeMergeNode());
    }

    SVG_ATTR(In, SkSVGFeInputType, SkSVGFeInputType())

protected:
    bool parseAndSetAttribute(const char*, const char*) override;

private:
    SkSVGFeMergeNode() : INHERITED(tag) {}

    using INHERITED = SkSVGHiddenContainer;
};

// https://www.w3.org/TR/SVG11/filters.html#feMergeElement
class SK_API SkSVGFeMerge : public SkSVGFe {
public:
    static constexpr SkSVGTag tag = SkSVGTag::kFeMerge;

    static sk_sp<SkSVGFeMerge> Make() { return sk_sp<SkSVGFeMerge>(new SkSVGFeMerge()); }

protected:
    sk_sp<SkImageFilter> onMakeImageFilter(const SkSVGRenderContext&,
                                           const SkSVGFilterContext&) const override;

    std::vector<SkSVGFeInputType> getInputs() const override;

private:
    SkSVGFeMerge() : INHERITED(tag) {}

    using INHERITED = SkSVGFe;
};

#endif //  SkSVGFeMerge_DEFINED
