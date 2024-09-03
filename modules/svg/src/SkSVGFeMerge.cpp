/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGFeMerge.h"

#include "include/core/SkImageFilter.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/base/SkTArray.h"
#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGFilterContext.h"

class SkSVGRenderContext;

bool SkSVGFeMergeNode::parseAndSetAttribute(const char* name, const char* value) {
    return INHERITED::parseAndSetAttribute(name, value) ||
           this->setIn(SkSVGAttributeParser::parse<SkSVGFeInputType>("in", name, value));
}

sk_sp<SkImageFilter> SkSVGFeMerge::onMakeImageFilter(const SkSVGRenderContext& ctx,
                                                     const SkSVGFilterContext& fctx) const {
    const SkSVGColorspace colorspace = this->resolveColorspace(ctx, fctx);

    skia_private::STArray<8, sk_sp<SkImageFilter>> merge_node_filters;
    merge_node_filters.reserve(fChildren.size());

    this->forEachChild<SkSVGFeMergeNode>([&](const SkSVGFeMergeNode* child) {
        merge_node_filters.push_back(fctx.resolveInput(ctx, child->getIn(), colorspace));
    });

    return SkImageFilters::Merge(merge_node_filters.data(),
                                 merge_node_filters.size(),
                                 this->resolveFilterSubregion(ctx, fctx));
}

std::vector<SkSVGFeInputType> SkSVGFeMerge::getInputs() const {
    std::vector<SkSVGFeInputType> inputs;
    inputs.reserve(fChildren.size());

    this->forEachChild<SkSVGFeMergeNode>([&](const SkSVGFeMergeNode* child) {
        inputs.push_back(child->getIn());
    });

    return inputs;
}
