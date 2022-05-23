/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCombinationBuilder.h"

#include "include/private/SkTHash.h"
#include "src/core/SkKeyContext.h"
#include "src/core/SkKeyHelpers.h"
#include "src/core/SkShaderCodeDictionary.h"

#ifdef SK_GRAPHITE_ENABLED
#include "src/gpu/graphite/ContextPriv.h"
#endif

//--------------------------------------------------------------------------------------------------
class SkPaintCombinations {
public:
    SkPaintCombinations() {}

    // SkBlenders
    void add(SkBlendMode bm) { fBlendModes.add((uint32_t) bm); }
    void add(SkBlenderID id) { fBlendModes.add(id.asUInt()); }
    int numBlendModes() const { return fBlendModes.count(); }

    // SkShaders
    void add(const SkShaderCombo& shaderCombo) { fShaders.push_back(shaderCombo); }

private:
    friend class SkCombinationBuilder;     // for iterators

    std::vector<SkShaderCombo> fShaders;
    SkTHashSet<uint32_t>       fBlendModes;
};

//--------------------------------------------------------------------------------------------------
#ifdef SK_GRAPHITE_ENABLED
SkCombinationBuilder::SkCombinationBuilder(skgpu::graphite::Context* context)
        : fDictionary(context->priv().shaderCodeDictionary()) {
    fArena = std::make_unique<SkArenaAllocWithReset>(64);
    this->reset();
}
#else
SkCombinationBuilder::SkCombinationBuilder(SkShaderCodeDictionary* dict)
        : fDictionary(dict) {
    fArena = std::make_unique<SkArenaAllocWithReset>(64);
    this->reset();
}
#endif

void SkCombinationBuilder::add(SkShaderCombo shaderCombo) {
    fCombinations->add(shaderCombo);
}

void SkCombinationBuilder::add(SkBlendMode bm) {
    SkASSERT(fDictionary->isValidID((int) bm));

    fCombinations->add(bm);
}

void SkCombinationBuilder::add(SkBlendMode rangeStart, SkBlendMode rangeEnd) {
    for (int i = (int)rangeStart; i <= (int) rangeEnd; ++i) {
        this->add((SkBlendMode) i);
    }
}

void SkCombinationBuilder::add(BlendModeGroup group) {
    switch (group) {
        case BlendModeGroup::kPorterDuff:
            this->add(SkBlendMode::kClear, SkBlendMode::kScreen);
            break;
        case BlendModeGroup::kAdvanced:
            this->add(SkBlendMode::kOverlay, SkBlendMode::kMultiply);
            break;
        case BlendModeGroup::kColorAware:
            this->add(SkBlendMode::kHue, SkBlendMode::kLuminosity);
            break;
        case BlendModeGroup::kAll:
            this->add(SkBlendMode::kClear, SkBlendMode::kLastMode);
            break;
    }
}

void SkCombinationBuilder::add(SkBlenderID id) {
    SkASSERT(fDictionary->isValidID(id.asUInt()));

    fCombinations->add(id);
}

void SkCombinationBuilder::reset() {
    fArena->reset();
    fCombinations = fArena->make<SkPaintCombinations>();
}

void SkCombinationBuilder::buildCombinations(
        SkShaderCodeDictionary* dict,
        const std::function<void(SkUniquePaintParamsID)>& func) const {
    SkKeyContext keyContext(dict);
    SkPaintParamsKeyBuilder builder(dict, SkBackend::kGraphite);

    for (uint32_t bmVal : fCombinations->fBlendModes) {
        SkBlendMode bm;
        if (bmVal < kSkBlendModeCount) {
            bm = (SkBlendMode) bmVal;
        } else {
            // TODO: add creation of PaintParamKey fragments from runtime effect SkBlenders
            continue;
        }

        for (const SkShaderCombo& shaderCombo : fCombinations->fShaders) {
            for (auto shaderType: shaderCombo.fTypes) {
                for (auto tm: shaderCombo.fTileModes) {
                    // TODO: expand CreateKey to take either an SkBlendMode or an SkBlendID
                    auto uniqueID = CreateKey(keyContext, &builder, shaderType, tm, bm);

                    func(uniqueID);
                }
            }
        }
    }
}
