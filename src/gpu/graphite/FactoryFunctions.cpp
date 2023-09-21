/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/FactoryFunctions.h"

#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/Blend.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParams.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/Precompile.h"
#include "src/gpu/graphite/PrecompileBasePriv.h"
#include "src/shaders/SkShaderBase.h"

namespace skgpu::graphite {

//--------------------------------------------------------------------------------------------------
class PrecompileBlendModeBlender : public PrecompileBlender {
public:
    PrecompileBlendModeBlender(SkBlendMode blendMode) : fBlendMode(blendMode) {}

    std::optional<SkBlendMode> asBlendMode() const final { return fBlendMode; }

private:
    void addToKey(const KeyContext& keyContext,
                  int desiredCombination,
                  PaintParamsKeyBuilder* builder) const override {
        SkASSERT(desiredCombination == 0); // The blend mode blender only ever has one combination

        AddModeBlend(keyContext, builder, /* gatherer= */ nullptr, fBlendMode);
    }


    SkBlendMode fBlendMode;
};

sk_sp<PrecompileBlender> PrecompileBlender::Mode(SkBlendMode blendMode) {
    return sk_make_sp<PrecompileBlendModeBlender>(blendMode);
}

//--------------------------------------------------------------------------------------------------
class PrecompileColorShader : public PrecompileShader {
public:
    PrecompileColorShader() {}

    bool isConstant() const override { return true; }

private:
    void addToKey(const KeyContext& keyContext,
                  int desiredCombination,
                  PaintParamsKeyBuilder* builder) const override {

        SkASSERT(desiredCombination == 0); // The color shader only ever has one combination

        constexpr SkPMColor4f kUnusedColor = { 1, 0, 0, 1 };

        SolidColorShaderBlock::BeginBlock(keyContext, builder, /* gatherer= */ nullptr,
                                          kUnusedColor); // color isn't used w/o a gatherer
        builder->endBlock();
    }

};

sk_sp<PrecompileShader> PrecompileShaders::Color() {
    return sk_make_sp<PrecompileColorShader>();
}

//--------------------------------------------------------------------------------------------------
class PrecompileBlendShader : public PrecompileShader {
public:
    PrecompileBlendShader(SkSpan<const sk_sp<PrecompileBlender>> runtimeBlendEffects,
                          SkSpan<const sk_sp<PrecompileShader>> dsts,
                          SkSpan<const sk_sp<PrecompileShader>> srcs,
                          bool needsPorterDuffBased,
                          bool needsSeparableMode)
            : fRuntimeBlendEffects(runtimeBlendEffects.begin(), runtimeBlendEffects.end())
            , fDstOptions(dsts.begin(), dsts.end())
            , fSrcOptions(srcs.begin(), srcs.end()) {

        fNumBlenderCombos = 0;
        for (auto rt : fRuntimeBlendEffects) {
            fNumBlenderCombos += rt->numCombinations();
        }
        if (needsPorterDuffBased) {
            ++fNumBlenderCombos;
        }
        if (needsSeparableMode) {
            ++fNumBlenderCombos;
        }

        SkASSERT(fNumBlenderCombos >= 1);

        fNumDstCombos = 0;
        for (auto d : fDstOptions) {
            fNumDstCombos += d->numCombinations();
        }

        fNumSrcCombos = 0;
        for (auto s : fSrcOptions) {
            fNumSrcCombos += s->numCombinations();
        }

        if (needsPorterDuffBased) {
            fPorterDuffIndex = 0;
            if (needsSeparableMode) {
                fSeparableModeIndex = 1;
                if (!fRuntimeBlendEffects.empty()) {
                    fBlenderIndex = 2;
                }
            } else if (!fRuntimeBlendEffects.empty()) {
                fBlenderIndex = 1;
            }
        } else if (needsSeparableMode) {
            fSeparableModeIndex = 0;
            if (!fRuntimeBlendEffects.empty()) {
                fBlenderIndex = 1;
            }
        } else {
            SkASSERT(!fRuntimeBlendEffects.empty());
            fBlenderIndex = 0;
        }
    }

private:
    int numChildCombinations() const override {
        return fNumBlenderCombos * fNumDstCombos * fNumSrcCombos;
    }

    void addToKey(const KeyContext& keyContext,
                  int desiredCombination,
                  PaintParamsKeyBuilder* builder) const override {
        SkASSERT(desiredCombination < this->numCombinations());

        const int desiredDstCombination = desiredCombination % fNumDstCombos;
        int remainingCombinations = desiredCombination / fNumDstCombos;

        const int desiredSrcCombination = remainingCombinations % fNumSrcCombos;
        remainingCombinations /= fNumSrcCombos;

        int desiredBlendCombination = remainingCombinations;
        SkASSERT(desiredBlendCombination < fNumBlenderCombos);

        if (desiredBlendCombination == fPorterDuffIndex ||
            desiredBlendCombination == fSeparableModeIndex) {
            BlendShaderBlock::BeginBlock(keyContext, builder, /* gatherer= */ nullptr);

        } else {
            // TODO: share this with the copy over in SkComposeShader.cpp. For now, the block ID is
            // determined by a hash of the code so both copies will generate the same key and
            // the SkPaint vs. PaintOptions key match testing will trigger if they get out of
            // sync.
            static SkRuntimeEffect* sBlendEffect = SkMakeRuntimeEffect(
                    SkRuntimeEffect::MakeForShader,
                    "uniform shader s, d;"
                    "uniform blender b;"
                    "half4 main(float2 xy) {"
                        "return b.eval(s.eval(xy), d.eval(xy));"
                    "}"
            );
            RuntimeEffectBlock::BeginBlock(keyContext, builder, /* gatherer= */ nullptr,
                                           { sk_ref_sp(sBlendEffect) });
            SkASSERT(desiredBlendCombination >= fBlenderIndex);
            desiredBlendCombination -= fBlenderIndex;
        }

        AddToKey<PrecompileShader>(keyContext, builder, fSrcOptions, desiredSrcCombination);
        AddToKey<PrecompileShader>(keyContext, builder, fDstOptions, desiredDstCombination);

        if (desiredBlendCombination == fPorterDuffIndex) {
            CoeffBlenderBlock::BeginBlock(keyContext, builder, /* gatherer= */ nullptr,
                                          {}); // coeffs aren't used
            builder->endBlock();
        } else if (desiredBlendCombination == fSeparableModeIndex) {
            BlendModeBlenderBlock::BeginBlock(keyContext, builder, /* gatherer= */ nullptr,
                                              SkBlendMode::kOverlay); // the blendmode is unused
            builder->endBlock();
        } else {
            AddToKey<PrecompileBlender>(keyContext, builder, fRuntimeBlendEffects,
                                        desiredBlendCombination);
        }

        builder->endBlock();  // BlendShaderBlock or RuntimeEffectBlock
    }

    std::vector<sk_sp<PrecompileBlender>> fRuntimeBlendEffects;
    std::vector<sk_sp<PrecompileShader>> fDstOptions;
    std::vector<sk_sp<PrecompileShader>> fSrcOptions;

    int fNumBlenderCombos;
    int fNumDstCombos;
    int fNumSrcCombos;

    int fPorterDuffIndex = -1;
    int fSeparableModeIndex = -1;
    int fBlenderIndex = -1;
};

sk_sp<PrecompileShader> PrecompileShaders::Blend(
        SkSpan<const sk_sp<PrecompileBlender>> blenders,
        SkSpan<const sk_sp<PrecompileShader>> dsts,
        SkSpan<const sk_sp<PrecompileShader>> srcs) {
    std::vector<sk_sp<PrecompileBlender>> tmp;
    tmp.reserve(blenders.size());

    bool needsPorterDuffBased = false;
    bool needsBlendModeBased = false;

    for (auto b : blenders) {
        if (!b) {
            needsPorterDuffBased = true; // fall back to kSrcOver
        } else if (b->asBlendMode().has_value()) {
            SkBlendMode bm = b->asBlendMode().value();

            SkSpan<const float> coeffs = skgpu::GetPorterDuffBlendConstants(bm);
            if (!coeffs.empty()) {
                needsPorterDuffBased = true;
            } else {
                needsBlendModeBased = true;
            }
        } else {
            tmp.push_back(b);
        }
    }

    if (!needsPorterDuffBased && !needsBlendModeBased && tmp.empty()) {
        needsPorterDuffBased = true; // fallback to kSrcOver
    }

    return sk_make_sp<PrecompileBlendShader>(SkSpan<const sk_sp<PrecompileBlender>>(tmp),
                                             dsts, srcs,
                                             needsPorterDuffBased, needsBlendModeBased);
}

sk_sp<PrecompileShader> PrecompileShaders::Blend(
        SkSpan<SkBlendMode> blendModes,
        SkSpan<const sk_sp<PrecompileShader>> dsts,
        SkSpan<const sk_sp<PrecompileShader>> srcs) {

    bool needsPorterDuffBased = false;
    bool needsBlendModeBased = false;

    for (SkBlendMode bm : blendModes) {
        SkSpan<const float> porterDuffConstants = skgpu::GetPorterDuffBlendConstants(bm);
        if (!porterDuffConstants.empty()) {
            needsPorterDuffBased = true;
        } else {
            needsBlendModeBased = true;
        }
    }

    if (!needsPorterDuffBased && !needsBlendModeBased) {
        needsPorterDuffBased = true; // fallback to kSrcOver
    }

    return sk_make_sp<PrecompileBlendShader>(SkSpan<const sk_sp<PrecompileBlender>>(),
                                             dsts, srcs,
                                             needsPorterDuffBased, needsBlendModeBased);
}

//--------------------------------------------------------------------------------------------------
class PrecompileImageShader : public PrecompileShader {
public:
    PrecompileImageShader() {}

private:
    int numIntrinsicCombinations() const override {
        return 2; // cubic and non-cubic sampling
    }

    void addToKey(const KeyContext& keyContext,
                  int desiredCombination,
                  PaintParamsKeyBuilder* builder) const override {
        if (desiredCombination == 0) {
            ImageShaderBlock::BeginBlock(keyContext, builder,
                                        /* gatherer= */ nullptr, /* imgData= */ nullptr);
        } else {
            ImageShaderBlock::BeginCubicBlock(keyContext, builder,
                                              /* gatherer= */ nullptr, /* imgData= */ nullptr);
        }
        builder->endBlock();
    }
};

sk_sp<PrecompileShader> PrecompileShaders::Image() {
    return sk_make_sp<PrecompileImageShader>();
}

//--------------------------------------------------------------------------------------------------
class PrecompileGradientShader : public PrecompileShader {
public:
    PrecompileGradientShader(SkShaderBase::GradientType type) : fType(type) {}

private:
    /*
     * The gradients currently have two specializations based on the number of stops.
     */
    inline static constexpr int kNumStopVariants = 2;
    inline static constexpr int kStopVariants[kNumStopVariants] = { 4, 8 };

    int numIntrinsicCombinations() const override {
        return kNumStopVariants;
    }

    void addToKey(const KeyContext& keyContext,
                  int desiredCombination,
                  PaintParamsKeyBuilder* builder) const override {
        const int intrinsicCombination = desiredCombination / this->numChildCombinations();
        SkDEBUGCODE(int childCombination = desiredCombination % this->numChildCombinations();)
        SkASSERT(intrinsicCombination < kNumStopVariants);
        SkASSERT(childCombination == 0);

        // Only the type and number of stops are accessed when there is no gatherer
        GradientShaderBlocks::GradientData gradData(fType, kStopVariants[intrinsicCombination]);

        // TODO: we may need SkLocalMatrixShader-wrapped versions too
        Compose(keyContext, builder, /* gatherer= */ nullptr,
                /* addInnerToKey= */ [&]() -> void {
                    GradientShaderBlocks::BeginBlock(keyContext, builder, /* gatherer= */ nullptr,
                                                     gradData);
                    builder->endBlock();
                },
                /* addOuterToKey= */  [&]() -> void {
                    ColorSpaceTransformBlock::BeginBlock(keyContext, builder,
                                                         /* gatherer= */ nullptr,
                                                         /* data= */ nullptr);
                    builder->endBlock();
                });
    }

    SkShaderBase::GradientType fType;
};

sk_sp<PrecompileShader> PrecompileShaders::LinearGradient() {
    return sk_make_sp<PrecompileGradientShader>(SkShaderBase::GradientType::kLinear);
}

sk_sp<PrecompileShader> PrecompileShaders::RadialGradient() {
    return sk_make_sp<PrecompileGradientShader>(SkShaderBase::GradientType::kRadial);
}

sk_sp<PrecompileShader> PrecompileShaders::SweepGradient() {
    return sk_make_sp<PrecompileGradientShader>(SkShaderBase::GradientType::kSweep);
}

sk_sp<PrecompileShader> PrecompileShaders::TwoPointConicalGradient() {
    return sk_make_sp<PrecompileGradientShader>(SkShaderBase::GradientType::kConical);
}

//--------------------------------------------------------------------------------------------------
class PrecompileLocalMatrixShader : public PrecompileShader {
public:
    PrecompileLocalMatrixShader(sk_sp<PrecompileShader> wrapped) : fWrapped(std::move(wrapped)) {}

private:
    bool isALocalMatrixShader() const override { return true; }

    int numChildCombinations() const override {
        return fWrapped->numChildCombinations();
    }

    void addToKey(const KeyContext& keyContext,
                  int desiredCombination,
                  PaintParamsKeyBuilder* builder) const override {
        SkASSERT(desiredCombination < fWrapped->numCombinations());

        LocalMatrixShaderBlock::BeginBlock(keyContext, builder,
                                           /* gatherer= */ nullptr, /* lmShaderData= */ nullptr);

        fWrapped->priv().addToKey(keyContext, desiredCombination, builder);

        builder->endBlock();
    }

    sk_sp<PrecompileShader> fWrapped;
};

sk_sp<PrecompileShader> PrecompileShaders::LocalMatrix(sk_sp<PrecompileShader> wrapped) {
    return sk_make_sp<PrecompileLocalMatrixShader>(std::move(wrapped));
}

//--------------------------------------------------------------------------------------------------
class PrecompileColorFilterShader : public PrecompileShader {
public:
    PrecompileColorFilterShader(sk_sp<PrecompileShader> shader, sk_sp<PrecompileColorFilter> cf)
            : fShader(std::move(shader))
            , fColorFilter(std::move(cf)) {}

private:
    int numChildCombinations() const override {
        const int numShaderCombos = fShader->numCombinations();
        const int numColorFilterCombos = fColorFilter->numCombinations();

        return numShaderCombos * numColorFilterCombos;
    }

    void addToKey(const KeyContext& keyContext,
                  int desiredCombination,
                  PaintParamsKeyBuilder* builder) const override {

        SkASSERT(desiredCombination < this->numCombinations());

        const int numShaderCombos = fShader->numCombinations();
        SkDEBUGCODE(int numColorFilterCombos = fColorFilter->numCombinations();)

        int desiredShaderCombination = desiredCombination % numShaderCombos;
        int desiredColorFilterCombination = desiredCombination / numShaderCombos;
        SkASSERT(desiredColorFilterCombination < numColorFilterCombos);

        Compose(keyContext, builder, /* gatherer= */ nullptr,
                /* addInnerToKey= */ [&]() -> void {
                    fShader->priv().addToKey(keyContext, desiredShaderCombination, builder);
                },
                /* addOuterToKey= */ [&]() -> void {
                    fColorFilter->priv().addToKey(keyContext, desiredColorFilterCombination,
                                                  builder);
                });
    }

    sk_sp<PrecompileShader> fShader;
    sk_sp<PrecompileColorFilter> fColorFilter;
};

sk_sp<PrecompileShader> PrecompileShaders::ColorFilter(sk_sp<PrecompileShader> shader,
                                                       sk_sp<PrecompileColorFilter> cf) {
    return sk_make_sp<PrecompileColorFilterShader>(std::move(shader), std::move(cf));
}

//--------------------------------------------------------------------------------------------------
class PrecompileBlurMaskFilter : public PrecompileMaskFilter {
public:
    PrecompileBlurMaskFilter() {}

private:
    void addToKey(const KeyContext& keyContext,
                  int desiredCombination,
                  PaintParamsKeyBuilder* builder) const override {
        SkASSERT(desiredCombination == 0);

        // TODO: need to add a BlurMaskFilter Block. This is somewhat blocked on figuring out
        // what we're going to do with the Blur system.
    }
};

sk_sp<PrecompileMaskFilter> PrecompileMaskFilters::Blur() {
    return sk_make_sp<PrecompileBlurMaskFilter>();
}

//--------------------------------------------------------------------------------------------------
class PrecompileBlendModeColorFilter : public PrecompileColorFilter {
public:
    PrecompileBlendModeColorFilter() {}

private:
    void addToKey(const KeyContext& keyContext,
                  int desiredCombination,
                  PaintParamsKeyBuilder* builder) const override {
        SkASSERT(desiredCombination == 0);

        // Here, kSrcOver and the white color are just a stand-ins for some later blend mode
        // and color.
        AddBlendModeColorFilter(keyContext, builder, /* gatherer= */ nullptr,
                                SkBlendMode::kSrcOver, SK_PMColor4fWHITE);
    }
};

sk_sp<PrecompileColorFilter> PrecompileColorFilters::Blend() {
    return sk_make_sp<PrecompileBlendModeColorFilter>();
}

//--------------------------------------------------------------------------------------------------
class PrecompileMatrixColorFilter : public PrecompileColorFilter {
public:
    PrecompileMatrixColorFilter() {}

private:
    void addToKey(const KeyContext& keyContext,
                  int desiredCombination,
                  PaintParamsKeyBuilder* builder) const override {
        SkASSERT(desiredCombination == 0);

        MatrixColorFilterBlock::BeginBlock(keyContext, builder,
                                           /* gatherer= */ nullptr,
                                           /* matrixCFData= */ nullptr);
        builder->endBlock();
    }
};

sk_sp<PrecompileColorFilter> PrecompileColorFilters::Matrix() {
    return sk_make_sp<PrecompileMatrixColorFilter>();
}

sk_sp<PrecompileColorFilter> PrecompileColorFilters::HSLAMatrix() {
    return sk_make_sp<PrecompileMatrixColorFilter>();
}

//--------------------------------------------------------------------------------------------------
// TODO: need to figure out how we're going to decompose ImageFilters
sk_sp<PrecompileImageFilter> PrecompileImageFilters::Blur() {
    return nullptr; // sk_make_sp<PrecompileImageFilter>();
}

sk_sp<PrecompileImageFilter> PrecompileImageFilters::Image() {
    return nullptr; // sk_make_sp<PrecompileImageFilter>();
}

//--------------------------------------------------------------------------------------------------
PrecompileChildPtr::PrecompileChildPtr(sk_sp<PrecompileShader> s) : fChild(std::move(s)) {}
PrecompileChildPtr::PrecompileChildPtr(sk_sp<PrecompileColorFilter> cf)
        : fChild(std::move(cf)) {
}
PrecompileChildPtr::PrecompileChildPtr(sk_sp<PrecompileBlender> b) : fChild(std::move(b)) {}

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

} // anonymous namespace

PrecompileChildPtr::PrecompileChildPtr(sk_sp<PrecompileBase> child)
        : fChild(std::move(child)) {
    SkASSERT(precompilebase_is_valid_as_child(fChild.get()));
}

std::optional<SkRuntimeEffect::ChildType> PrecompileChildPtr::type() const {
    if (fChild) {
        switch (fChild->type()) {
            case PrecompileBase::Type::kShader:
                return SkRuntimeEffect::ChildType::kShader;
            case PrecompileBase::Type::kColorFilter:
                return SkRuntimeEffect::ChildType::kColorFilter;
            case PrecompileBase::Type::kBlender:
                return SkRuntimeEffect::ChildType::kBlender;
            default:
                break;
        }
    }
    return std::nullopt;
}

PrecompileShader* PrecompileChildPtr::shader() const {
    return (fChild && fChild->type() == PrecompileBase::Type::kShader)
           ? static_cast<PrecompileShader*>(fChild.get())
           : nullptr;
}

PrecompileColorFilter* PrecompileChildPtr::colorFilter() const {
    return (fChild && fChild->type() == PrecompileBase::Type::kColorFilter)
           ? static_cast<PrecompileColorFilter*>(fChild.get())
           : nullptr;
}

PrecompileBlender* PrecompileChildPtr::blender() const {
    return (fChild && fChild->type() == PrecompileBase::Type::kBlender)
           ? static_cast<PrecompileBlender*>(fChild.get())
           : nullptr;
}

//--------------------------------------------------------------------------------------------------
namespace {

int num_options_in_set(const std::vector<PrecompileChildPtr>& optionSet) {
    int numOptions = 1;
    for (const PrecompileChildPtr& childOption : optionSet) {
        // A missing child will fall back to a passthrough object
        if (childOption.base()) {
            numOptions *= childOption.base()->numCombinations();
        }
    }

    return numOptions;
}

// This is the precompile correlate to SkRuntimeEffect.cpp's add_children_to_key
void add_children_to_key(const KeyContext& keyContext,
                         int desiredCombination,
                         PaintParamsKeyBuilder* builder,
                         const std::vector<PrecompileChildPtr>& optionSet,
                         SkSpan<const SkRuntimeEffect::Child> childInfo) {
    using ChildType = SkRuntimeEffect::ChildType;

    SkASSERT(optionSet.size() == childInfo.size());

    int remainingCombinations = desiredCombination;

    for (size_t index = 0; index < optionSet.size(); ++index) {
        const PrecompileChildPtr& childOption = optionSet[index];

        const int numChildCombos = childOption.base() ? childOption.base()->numCombinations()
                                                      : 1;
        const int curCombo = remainingCombinations % numChildCombos;
        remainingCombinations /= numChildCombos;

        std::optional<ChildType> type = childOption.type();
        if (type == ChildType::kShader) {
            childOption.shader()->priv().addToKey(keyContext, curCombo, builder);
        } else if (type == ChildType::kColorFilter) {
            childOption.colorFilter()->priv().addToKey(keyContext, curCombo, builder);
        } else if (type == ChildType::kBlender) {
            childOption.blender()->priv().addToKey(keyContext, curCombo, builder);
        } else {
            SkASSERT(curCombo == 0);

            // We don't have a child effect. Substitute in a no-op effect.
            switch (childInfo[index].type) {
                case ChildType::kShader:
                case ChildType::kColorFilter:
                    // A "passthrough" shader returns the input color as-is.
                    PriorOutputBlock::BeginBlock(keyContext, builder, /* gatherer= */ nullptr);
                    builder->endBlock();
                    break;

                case ChildType::kBlender:
                    // A "passthrough" blender performs `blend_src_over(src, dest)`.
                    BlendModeBlenderBlock::BeginBlock(
                            keyContext, builder, /* gatherer= */ nullptr, SkBlendMode::kSrcOver);
                    builder->endBlock();
                    break;
            }
        }
    }
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
    }

private:
    int numChildCombinations() const override {
        int numOptions = 0;
        for (const std::vector<PrecompileChildPtr>& optionSet : fChildOptions) {
            numOptions += num_options_in_set(optionSet);
        }

        return numOptions ? numOptions : 1;
    }

    void addToKey(const KeyContext& keyContext,
                  int desiredCombination,
                  PaintParamsKeyBuilder* builder) const override {

        SkASSERT(desiredCombination < this->numCombinations());

        SkSpan<const SkRuntimeEffect::Child> childInfo = fEffect->children();

        RuntimeEffectBlock::BeginBlock(keyContext, builder, /* gatherer= */ nullptr, { fEffect });

        for (const std::vector<PrecompileChildPtr>& optionSet : fChildOptions) {
            int numOptionsInSet = num_options_in_set(optionSet);

            if (desiredCombination < numOptionsInSet) {
                add_children_to_key(keyContext, desiredCombination, builder, optionSet, childInfo);
                break;
            }

            desiredCombination -= numOptionsInSet;
        }

        builder->endBlock();
    }

    sk_sp<SkRuntimeEffect> fEffect;
    std::vector<std::vector<PrecompileChildPtr>> fChildOptions;
};

sk_sp<PrecompileShader> MakePrecompileShader(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const PrecompileChildOptions> childOptions) {
    // TODO: check that 'effect' has the kAllowShader_Flag bit set and:
    //  for each entry in childOptions:
    //    all the SkPrecompileChildPtrs have the same type as the corresponding child in the effect
    return sk_make_sp<PrecompileRTEffect<PrecompileShader>>(std::move(effect), childOptions);
}

sk_sp<PrecompileColorFilter> MakePrecompileColorFilter(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const PrecompileChildOptions> childOptions) {
    // TODO: check that 'effect' has the kAllowColorFilter_Flag bit set and:
    //  for each entry in childOptions:
    //    all the SkPrecompileChildPtrs have the same type as the corresponding child in the effect
    return sk_make_sp<PrecompileRTEffect<PrecompileColorFilter>>(std::move(effect),
                                                                 childOptions);
}

sk_sp<PrecompileBlender> MakePrecompileBlender(
        sk_sp<SkRuntimeEffect> effect,
        SkSpan<const PrecompileChildOptions> childOptions) {
    // TODO: check that 'effect' has the kAllowBlender_Flag bit set and:
    //  for each entry in childOptions:
    //    all the SkPrecompileChildPtrs have the same type as the corresponding child in the effect
    return sk_make_sp<PrecompileRTEffect<PrecompileBlender>>(std::move(effect), childOptions);
}

} // namespace skgpu::graphite

//--------------------------------------------------------------------------------------------------
