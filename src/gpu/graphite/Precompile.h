/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Precompile_DEFINED
#define skgpu_graphite_Precompile_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTDArray.h"

#include <functional>
#include <optional>
#include <vector>

class SkRuntimeEffect;

namespace skgpu::graphite {

enum class Coverage;
class KeyContext;
class PipelineDataGatherer;
class PrecompileBasePriv;
class UniquePaintParamsID;

// Create the Pipelines specified by 'options' by combining the shading portion w/ the specified
// 'drawTypes' and a stock set of RenderPass descriptors (e.g., kDepth+msaa, kDepthStencil+msaa)
void PrecompileCombinations(Context* context,
                            const PaintOptions& options,
                            const KeyContext& keyContext,
                            DrawTypeFlags drawTypes,
                            bool withPrimitiveBlender,
                            Coverage coverage);

class PrecompileBase : public SkRefCnt {
public:
    enum class Type {
        kBlender,
        kColorFilter,
        kImageFilter,
        kMaskFilter,
        kShader,
        // TODO: add others: kDrawable, kPathEffect (?!)
    };

    PrecompileBase(Type type) : fType(type) {}

    Type type() const { return fType; }

    // TODO: Maybe convert these two to be parameters passed into PrecompileBase from all the
    // derived classes and then make them non-virtual.
    virtual int numIntrinsicCombinations() const { return 1; }
    virtual int numChildCombinations() const { return 1; }

    int numCombinations() const {
        return this->numIntrinsicCombinations() * this->numChildCombinations();
    }

    // Provides access to functions that aren't part of the public API.
    PrecompileBasePriv priv();
    const PrecompileBasePriv priv() const;  // NOLINT(readability-const-return-type)

protected:
    // This returns the desired option along with the child options.
    template<typename T>
    static std::pair<sk_sp<T>, int> SelectOption(SkSpan<const sk_sp<T>> options, int desiredOption);

    // In general, derived classes should use AddToKey to select the desired child option from
    // a vector and then have it added to the key with its reduced/nested child option.
    template<typename T>
    static void AddToKey(const KeyContext&,
                         PaintParamsKeyBuilder*,
                         PipelineDataGatherer*,
                         SkSpan<const sk_sp<T>> options,
                         int desiredOption);

private:
    friend class PaintOptions;
    friend class PrecompileBasePriv;

    virtual bool isALocalMatrixShader() const { return false; }

    virtual void addToKey(const KeyContext&,
                          PaintParamsKeyBuilder*,
                          PipelineDataGatherer*,
                          int desiredCombination) const = 0;

    Type fType;
};

//--------------------------------------------------------------------------------------------------

template<typename T>
std::pair<sk_sp<T>, int> PrecompileBase::SelectOption(SkSpan<const sk_sp<T>> options,
                                                      int desiredOption) {
    for (const sk_sp<T>& option : options) {
        if (desiredOption < (option ? option->numCombinations() : 1)) {
            return { option, desiredOption };
        }
        desiredOption -= option ? option->numCombinations() : 1;
    }
    return { nullptr, 0 };
}

template<typename T>
void PrecompileBase::AddToKey(const KeyContext& keyContext,
                              PaintParamsKeyBuilder* builder,
                              PipelineDataGatherer* gatherer,
                              SkSpan<const sk_sp<T>> options,
                              int desiredOption) {
    auto [option, childOptions] = SelectOption(options, desiredOption);
    if (option) {
        option->priv().addToKey(keyContext, builder, gatherer, childOptions);
    }
}

//--------------------------------------------------------------------------------------------------
class PrecompileColorFilter;

class PrecompileShader : public PrecompileBase {
public:
    PrecompileShader() : PrecompileBase(Type::kShader) {}

    virtual bool isConstant(int desiredCombination) const { return false; }

    sk_sp<PrecompileShader> makeWithLocalMatrix();

    sk_sp<PrecompileShader> makeWithColorFilter(sk_sp<PrecompileColorFilter>);

    sk_sp<PrecompileShader> makeWithWorkingColorSpace(sk_sp<SkColorSpace>);

    sk_sp<PrecompileShader> makeWithCTM();
};

class PrecompileMaskFilter : public PrecompileBase {
public:
    PrecompileMaskFilter() : PrecompileBase(Type::kMaskFilter) {}
};

class PrecompileColorFilter : public PrecompileBase {
public:
    PrecompileColorFilter() : PrecompileBase(Type::kColorFilter) {}

    sk_sp<PrecompileColorFilter> makeComposed(sk_sp<PrecompileColorFilter> inner) const;
};

class PrecompileBlender : public PrecompileBase {
public:
    PrecompileBlender() : PrecompileBase(Type::kBlender) {}

    virtual std::optional<SkBlendMode> asBlendMode() const { return {}; }

    static sk_sp<PrecompileBlender> Mode(SkBlendMode);
};

enum class PrecompileImageFilters : uint32_t {
    kNone = 0x0,
    kBlur = 0x1,
};
SK_MAKE_BITMASK_OPS(PrecompileImageFilters)

//--------------------------------------------------------------------------------------------------
class PaintOptionsPriv;

class PaintOptions {
public:
    void setShaders(SkSpan<const sk_sp<PrecompileShader>> shaders) {
        fShaderOptions.assign(shaders.begin(), shaders.end());
    }

    void setMaskFilters(SkSpan<const sk_sp<PrecompileMaskFilter>> maskFilters) {
        fMaskFilterOptions.assign(maskFilters.begin(), maskFilters.end());
        if (fMaskFilterOptions.size()) {
            // Currently Graphite only supports BlurMaskFilters which are implemented
            // via BlurImageFiltering
            fImageFilterOptions |= PrecompileImageFilters::kBlur;
        }
    }

    void setColorFilters(SkSpan<const sk_sp<PrecompileColorFilter>> colorFilters) {
        fColorFilterOptions.assign(colorFilters.begin(), colorFilters.end());
    }

    void setImageFilters(SkEnumBitMask<PrecompileImageFilters> options) {
        fImageFilterOptions = options;
    }

    void setBlendModes(SkSpan<SkBlendMode> blendModes) {
        fBlendModeOptions.append(blendModes.size(), blendModes.data());
    }
    void setBlenders(SkSpan<const sk_sp<PrecompileBlender>> blenders) {
        for (const sk_sp<PrecompileBlender>& b: blenders) {
            if (b->asBlendMode().has_value()) {
                fBlendModeOptions.push_back(b->asBlendMode().value());
            } else {
                fBlenderOptions.push_back(b);
            }
        }
    }

    void setClipShaders(SkSpan<const sk_sp<PrecompileShader>> clipShaders);

    void setDither(bool dither) { fDither = dither; }

    typedef std::function<void(UniquePaintParamsID id,
                               DrawTypeFlags,
                               bool withPrimitiveBlender,
                               Coverage)> ProcessCombination;

    // Provides access to functions that aren't part of the public API.
    PaintOptionsPriv priv();
    const PaintOptionsPriv priv() const;  // NOLINT(readability-const-return-type)

private:
    friend class PaintOptionsPriv;

    int numShaderCombinations() const;
    int numMaskFilterCombinations() const;
    int numColorFilterCombinations() const;
    // TODO: need to decompose imagefilters into component draws
    int numBlendModeCombinations() const;
    int numClipShaderCombinations() const;

    int numCombinations() const;
    // 'desiredCombination' must be less than the result of the numCombinations call
    void createKey(const KeyContext&,
                   PaintParamsKeyBuilder*,
                   PipelineDataGatherer*,
                   int desiredCombination,
                   bool addPrimitiveBlender,
                   Coverage coverage) const;

    void buildCombinations(
        const KeyContext&,
        PipelineDataGatherer*,
        DrawTypeFlags drawTypes,
        bool addPrimitiveBlender,
        Coverage coverage,
        const ProcessCombination& processCombination) const;

    std::vector<sk_sp<PrecompileShader>> fShaderOptions;
    std::vector<sk_sp<PrecompileMaskFilter>> fMaskFilterOptions;
    std::vector<sk_sp<PrecompileColorFilter>> fColorFilterOptions;
    SkEnumBitMask<PrecompileImageFilters> fImageFilterOptions = PrecompileImageFilters::kNone;
    SkTDArray<SkBlendMode> fBlendModeOptions;
    skia_private::TArray<sk_sp<PrecompileBlender>> fBlenderOptions;
    std::vector<sk_sp<PrecompileShader>> fClipShaderOptions;
    bool fDither = false;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Precompile_DEFINED
