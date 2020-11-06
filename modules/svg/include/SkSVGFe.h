/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGFe_DEFINED
#define SkSVGFe_DEFINED

#include "modules/svg/include/SkSVGHiddenContainer.h"

class SkImageFilter;
class SkSVGFilterContext;

class SkSVGFe : public SkSVGHiddenContainer {
public:
    ~SkSVGFe() override = default;

    static bool IsFilterEffect(const sk_sp<SkSVGNode>& node) {
        return node->tag() == SkSVGTag::kFeTurbulence || node->tag() == SkSVGTag::kFeColorMatrix;
    }

    sk_sp<SkImageFilter> makeImageFilter(const SkSVGRenderContext& ctx,
                                         SkSVGFilterContext* fctx) const;

protected:
    explicit SkSVGFe(SkSVGTag t) : INHERITED(t) {}

    virtual sk_sp<SkImageFilter> onMakeImageFilter(const SkSVGRenderContext&,
                                                   const SkSVGFilterContext&) const = 0;

private:
    using INHERITED = SkSVGHiddenContainer;
};

#endif  // SkSVGFe_DEFINED
