/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/FactoryFunctions.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/Precompile.h"
#include "src/gpu/graphite/PrecompileBasePriv.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------
sk_sp<PrecompileShader> PrecompileShader::makeWithLocalMatrix() {
    if (this->priv().isALocalMatrixShader()) {
        // SkShader::makeWithLocalMatrix collapses chains of localMatrix shaders so we need to
        // follow suit here
        return sk_ref_sp(this);
    }

    return PrecompileShaders::LocalMatrix(sk_ref_sp(this));
}

sk_sp<PrecompileShader> PrecompileShader::makeWithColorFilter(sk_sp<PrecompileColorFilter> cf) {
    if (!cf) {
        return sk_ref_sp(this);
    }

    return PrecompileShaders::ColorFilter(sk_ref_sp(this), std::move(cf));
}

//--------------------------------------------------------------------------------------------------
int PaintOptions::numShaderCombinations() const {
    int numShaderCombinations = 0;
    for (const sk_sp<PrecompileShader>& s : fShaderOptions) {
        numShaderCombinations += s->numCombinations();
    }

    // If no shader option is specified we will add a solid color shader option
    return numShaderCombinations ? numShaderCombinations : 1;
}

int PaintOptions::numMaskFilterCombinations() const {
    int numMaskFilterCombinations = 0;
    for (const sk_sp<PrecompileMaskFilter>& mf : fMaskFilterOptions) {
        numMaskFilterCombinations += mf->numCombinations();
    }

    // If no mask filter options are specified we will use the geometry's coverage
    return numMaskFilterCombinations ? numMaskFilterCombinations : 1;
}

int PaintOptions::numColorFilterCombinations() const {
    int numColorFilterCombinations = 0;
    for (const sk_sp<PrecompileColorFilter>& cf : fColorFilterOptions) {
        numColorFilterCombinations += cf->numCombinations();
    }

    // If no color filter options are specified we will use the unmodified result color
    return numColorFilterCombinations ? numColorFilterCombinations : 1;
}

int PaintOptions::numBlendModeCombinations() const {
    bool bmBased = false;
    int numBlendCombos = 0;
    for (auto b: fBlenderOptions) {
        if (b->asBlendMode().has_value()) {
            bmBased = true;
        } else {
            numBlendCombos += b->numChildCombinations();
        }
    }

    if (bmBased || !numBlendCombos) {
        // If numBlendCombos is zero we will fallback to kSrcOver blending
        ++numBlendCombos;
    }

    return numBlendCombos;
}

int PaintOptions::numCombinations() const {
    // TODO: we need to handle ImageFilters separately
    return this->numShaderCombinations() *
           this->numMaskFilterCombinations() *
           this->numColorFilterCombinations() *
           this->numBlendModeCombinations();
}

void PaintOptions::createKey(const KeyContext& keyContext,
                             int desiredCombination,
                             PaintParamsKeyBuilder* keyBuilder,
                             bool addPrimitiveBlender) const {
    SkDEBUGCODE(keyBuilder->checkReset();)
    SkASSERT(desiredCombination < this->numCombinations());

    const int numBlendModeCombos = this->numBlendModeCombinations();
    const int numColorFilterCombinations = this->numColorFilterCombinations();
    const int numMaskFilterCombinations = this->numMaskFilterCombinations();

    const int desiredBlendCombination = desiredCombination % numBlendModeCombos;
    int remainingCombinations = desiredCombination / numBlendModeCombos;

    const int desiredColorFilterCombination = remainingCombinations % numColorFilterCombinations;
    remainingCombinations /= numColorFilterCombinations;

    const int desiredMaskFilterCombination = remainingCombinations % numMaskFilterCombinations;
    remainingCombinations /= numMaskFilterCombinations;

    const int desiredShaderCombination = remainingCombinations;
    SkASSERT(desiredShaderCombination < this->numShaderCombinations());

    // TODO: eliminate this block for the Paint's color when it isn't needed
    SolidColorShaderBlock::BeginBlock(keyContext, keyBuilder, /* gatherer= */ nullptr,
                                      {1, 0, 0, 1});
    keyBuilder->endBlock();

    if (!fShaderOptions.empty()) {
        PrecompileBase::AddToKey(keyContext, keyBuilder, fShaderOptions, desiredShaderCombination);
    }

    if (addPrimitiveBlender) {
        PrimitiveBlendModeBlock::BeginBlock(keyContext, keyBuilder, /* gatherer= */ nullptr,
                                            SkBlendMode::kSrcOver);
        keyBuilder->endBlock();
    }

    PrecompileBase::AddToKey(keyContext, keyBuilder, fMaskFilterOptions,
                             desiredMaskFilterCombination);
    PrecompileBase::AddToKey(keyContext, keyBuilder, fColorFilterOptions,
                             desiredColorFilterCombination);

    if (fBlenderOptions.empty()) {
        BlendModeBlock::BeginBlock(keyContext, keyBuilder, /* gatherer= */ nullptr,
                                   SkBlendMode::kSrcOver);
        keyBuilder->endBlock();
    } else {
        PrecompileBase::AddToKey(keyContext, keyBuilder, fBlenderOptions, desiredBlendCombination);
    }
}

void PaintOptions::buildCombinations(
        const KeyContext& keyContext,
        bool addPrimitiveBlender,
        const std::function<void(UniquePaintParamsID)>& processCombination) const {

    PaintParamsKeyBuilder builder(keyContext.dict());

    int numCombinations = this->numCombinations();
    for (int i = 0; i < numCombinations; ++i) {
        this->createKey(keyContext, i, &builder, addPrimitiveBlender);

        // The 'findOrCreate' calls lockAsKey on builder and then destroys the returned
        // PaintParamsKey. This serves to reset the builder.
        auto entry = keyContext.dict()->findOrCreate(&builder);

        processCombination(entry->uniqueID());
    }
}

} // namespace skgpu::graphite
