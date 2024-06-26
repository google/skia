/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/precompile/PrecompileImageFilter.h"

#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------
PrecompileImageFilter::PrecompileImageFilter(SkSpan<sk_sp<PrecompileImageFilter>> inputs)
        : PrecompileBase(Type::kImageFilter) {
    fInputs.reset(inputs.size());
    for (size_t i = 0; i < inputs.size(); ++i) {
        fInputs[i] = inputs[i];
    }
}

PrecompileImageFilter::~PrecompileImageFilter() = default;

sk_sp<PrecompileColorFilter> PrecompileImageFilter::asAColorFilter() const {
    sk_sp<PrecompileColorFilter> tmp = this->isColorFilterNode();
    if (!tmp) {
        return nullptr;
    }
    SkASSERT(this->countInputs() == 1);
    if (this->getInput(0)) {
        return nullptr;
    }
    // TODO: as in SkImageFilter::asAColorFilter, handle the special case of
    // affectsTransparentBlack. This is tricky for precompilation since we don't,
    // necessarily, have all the parameters of the ColorFilter in order to evaluate
    // filterColor4f(SkColors::kTransparent) - the normal API's implementation.
    return tmp;
}

void PrecompileImageFilter::createPipelines(
        const KeyContext& keyContext,
        PipelineDataGatherer* gatherer,
        const PaintOptions::ProcessCombination& processCombination) {
    // TODO: we will want to mark already visited nodes to prevent loops and track
    // already created Pipelines so we don't over-generate too much (e.g., if a DAG
    // has multiple blurs we don't want to keep trying to create all the blur pipelines).
    this->onCreatePipelines(keyContext, gatherer, processCombination);

    for (const sk_sp<PrecompileImageFilter>& input : fInputs) {
        if (input) {
            input->createPipelines(keyContext, gatherer, processCombination);
        }
    }
}

} // namespace skgpu::graphite
