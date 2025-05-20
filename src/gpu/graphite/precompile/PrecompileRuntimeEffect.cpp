/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/precompile/PrecompileRuntimeEffect.h"

#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileBase.h"
#include "include/gpu/graphite/precompile/PrecompileBlender.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/precompile/PrecompileBaseComplete.h"
#include "src/gpu/graphite/precompile/PrecompileBasePriv.h"

namespace skgpu::graphite {

namespace {

#ifdef SK_DEBUG

bool precompilebase_is_valid_as_child(const PrecompileBase *child) {
    if (!child) {
        return true;
    }

    switch (child->type()) {
        case PrecompileBase::Type::kShader:
        case PrecompileBase::Type::kColorFilter:
        case PrecompileBase::Type::kBlender:
            return true;
        default:
            return false;
    }
}

#endif // SK_DEBUG

int num_options_in_set(const SkSpan<const sk_sp<PrecompileBase>>& optionSet) {
    int numOptions = 0;
    for (const sk_sp<PrecompileBase>& childOption : optionSet) {
        // A missing child will fall back to a passthrough object
        if (childOption) {
            numOptions += childOption->priv().numCombinations();
        } else {
            ++numOptions;
        }
    }

    return numOptions;
}

// Convert from runtime effect type to precompile type
PrecompileBase::Type to_precompile_type(SkRuntimeEffect::ChildType childType) {
    switch(childType) {
        case SkRuntimeEffect::ChildType::kShader:      return PrecompileBase::Type::kShader;
        case SkRuntimeEffect::ChildType::kColorFilter: return PrecompileBase::Type::kColorFilter;
        case SkRuntimeEffect::ChildType::kBlender:     return PrecompileBase::Type::kBlender;
    }
    SkUNREACHABLE;
}

bool children_are_valid(SkRuntimeEffect* effect,
                        SkSpan<const PrecompileChildOptions> childOptions) {
    SkSpan<const SkRuntimeEffect::Child> childInfo = effect->children();
    if (childOptions.size() != childInfo.size()) {
        return false;
    }

    for (size_t i = 0; i < childInfo.size(); ++i) {
        const PrecompileBase::Type expectedType = to_precompile_type(childInfo[i].type);
        for (const sk_sp<PrecompileBase>& childOption : childOptions[i]) {
            if (childOption && expectedType != childOption->type()) {
                return false;
            }
        }
    }

    return true;
}

} // anonymous namespace

template<typename T>
class PrecompileRTEffect : public T {
public:
    PrecompileRTEffect(sk_sp<SkRuntimeEffect> effect,
                       SkSpan<const PrecompileChildOptions> childOptions)
            : fEffect(std::move(effect)) {
        fChildOptions.reserve(childOptions.size());
        for (PrecompileChildOptions c : childOptions) {
            fChildOptions.push_back({ c.begin(), c.end() });
        }

        fNumSlotCombinations.reserve(childOptions.size());
        fNumChildCombinations = 1;
        for (const std::vector<sk_sp<PrecompileBase>>& optionSet : fChildOptions) {
            fNumSlotCombinations.push_back(num_options_in_set(optionSet));
            fNumChildCombinations *= fNumSlotCombinations.back();
        }

        SkASSERT(fChildOptions.size() == fEffect->children().size());
    }

private:
    int numChildCombinations() const override { return fNumChildCombinations; }

    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const override {

        SkASSERT(desiredCombination < this->numCombinations());

        SkSpan<const SkRuntimeEffect::Child> childInfo = fEffect->children();

        if (!RuntimeEffectBlock::BeginBlock(keyContext, builder, gatherer, { fEffect })) {
            RuntimeEffectBlock::AddNoOpEffect(keyContext, builder, gatherer, fEffect.get());
            return;
        }

        int remainingCombinations = desiredCombination;

        for (size_t rowIndex = 0; rowIndex < fChildOptions.size(); ++rowIndex) {
            const std::vector<sk_sp<PrecompileBase>>& slotOptions = fChildOptions[rowIndex];
            int numSlotCombinations = fNumSlotCombinations[rowIndex];

            const int slotOption = remainingCombinations % numSlotCombinations;
            remainingCombinations /= numSlotCombinations;

            auto [option, childOptions] = PrecompileBase::SelectOption(
                    SkSpan<const sk_sp<PrecompileBase>>(slotOptions),
                    slotOption);

            KeyContextForRuntimeEffect childContext(keyContext, fEffect.get(), rowIndex);

            SkASSERT(precompilebase_is_valid_as_child(option.get()));
            if (option) {
                option->priv().addToKey(childContext, builder, gatherer, childOptions);
            } else {
                SkASSERT(childOptions == 0);

                // We don't have a child effect. Substitute in a no-op effect.
                switch (childInfo[rowIndex].type) {
                    case SkRuntimeEffect::ChildType::kShader:
                        // A missing shader returns transparent black
                        SolidColorShaderBlock::AddBlock(childContext, builder, gatherer,
                                                        SK_PMColor4fTRANSPARENT);
                        break;

                    case SkRuntimeEffect::ChildType::kColorFilter:
                        // A "passthrough" shader returns the input color as-is.
                        builder->addBlock(BuiltInCodeSnippetID::kPriorOutput);
                        break;

                    case SkRuntimeEffect::ChildType::kBlender:
                        // A "passthrough" blender performs `blend_src_over(src, dest)`.
                        AddFixedBlendMode(childContext, builder, gatherer, SkBlendMode::kSrcOver);
                        break;
                }
            }
        }

        RuntimeEffectBlock::HandleIntrinsics(keyContext, builder, gatherer, fEffect.get());

        builder->endBlock();
    }

    sk_sp<SkRuntimeEffect> fEffect;
    std::vector<std::vector<sk_sp<PrecompileBase>>> fChildOptions;
    skia_private::TArray<int> fNumSlotCombinations;
    int fNumChildCombinations;
};

sk_sp<PrecompileShader> PrecompileRuntimeEffects::MakePrecompileShader(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const PrecompileChildOptions> childOptions) {
    if (!effect || !effect->allowShader()) {
        return nullptr;
    }

    if (!children_are_valid(effect.get(), childOptions)) {
        return nullptr;
    }

    return sk_make_sp<PrecompileRTEffect<PrecompileShader>>(std::move(effect), childOptions);
}

sk_sp<PrecompileColorFilter> PrecompileRuntimeEffects::MakePrecompileColorFilter(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const PrecompileChildOptions> childOptions) {
    if (!effect || !effect->allowColorFilter()) {
        return nullptr;
    }

    if (!children_are_valid(effect.get(), childOptions)) {
        return nullptr;
    }

    return sk_make_sp<PrecompileRTEffect<PrecompileColorFilter>>(std::move(effect), childOptions);
}

sk_sp<PrecompileBlender> PrecompileRuntimeEffects::MakePrecompileBlender(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const PrecompileChildOptions> childOptions) {
    if (!effect || !effect->allowBlender()) {
        return nullptr;
    }

    if (!children_are_valid(effect.get(), childOptions)) {
        return nullptr;
    }

    return sk_make_sp<PrecompileRTEffect<PrecompileBlender>>(std::move(effect), childOptions);
}

} // namespace skgpu::graphite
