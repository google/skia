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

enum class PrecompileImageFilterFlags : uint32_t {
    kNone              = 0x00,
    kBlur              = 0x01,
    kDisplacement      = 0x02,
    kLighting          = 0x04,
    kMatrixConvolution = 0x08,
    kMorphology        = 0x10,
};
SK_MAKE_BITMASK_OPS(PrecompileImageFilterFlags)

//--------------------------------------------------------------------------------------------------
class PaintOptionsPriv;
class PrecompileImageFilter;

class PaintOptions {
public:
    void setShaders(SkSpan<const sk_sp<PrecompileShader>> shaders) {
        fShaderOptions.assign(shaders.begin(), shaders.end());
    }

    void setImageFilters(SkSpan<const sk_sp<PrecompileImageFilter>> imageFilters) {
        fImageFilterOptions.assign(imageFilters.begin(), imageFilters.end());
    }

    void setImageFilterFlags(SkEnumBitMask<PrecompileImageFilterFlags> flags) {
        fImageFilterFlags = flags;
    }

    void setMaskFilters(SkSpan<const sk_sp<PrecompileMaskFilter>> maskFilters) {
        for (const sk_sp<PrecompileMaskFilter>& mf : maskFilters) {
            if (mf) {
                // Currently Graphite only supports BlurMaskFilters which are implemented
                // via BlurImageFiltering
                fImageFilterFlags |= PrecompileImageFilterFlags::kBlur;
                break;
            }
        }
    }

    void setColorFilters(SkSpan<const sk_sp<PrecompileColorFilter>> colorFilters) {
        fColorFilterOptions.assign(colorFilters.begin(), colorFilters.end());
    }
    SkSpan<const sk_sp<PrecompileColorFilter>> colorFilters() const {
        return SkSpan<const sk_sp<PrecompileColorFilter>>(fColorFilterOptions);
    }

    void setBlendModes(SkSpan<const SkBlendMode> blendModes) {
        fBlendModeOptions.clear();
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
    void addBlendMode(SkBlendMode bm) {
        fBlendModeOptions.push_back(bm);
    }

    SkSpan<const SkBlendMode> blendModes() const {
        return SkSpan<const SkBlendMode>(fBlendModeOptions.data(),
                                         fBlendModeOptions.size());
    }
    SkSpan<const sk_sp<PrecompileBlender>> blenders() const {
        return SkSpan<const sk_sp<PrecompileBlender>>(fBlenderOptions);
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
    std::vector<sk_sp<PrecompileColorFilter>> fColorFilterOptions;
    SkTDArray<SkBlendMode> fBlendModeOptions;
    skia_private::TArray<sk_sp<PrecompileBlender>> fBlenderOptions;
    std::vector<sk_sp<PrecompileShader>> fClipShaderOptions;

    SkEnumBitMask<PrecompileImageFilterFlags> fImageFilterFlags = PrecompileImageFilterFlags::kNone;
    std::vector<sk_sp<PrecompileImageFilter>> fImageFilterOptions;

    bool fDither = false;
};

class PrecompileImageFilter : public PrecompileBase {
protected:
    PrecompileImageFilter(SkSpan<sk_sp<PrecompileImageFilter>> inputs)
            : PrecompileBase(Type::kImageFilter) {
        fInputs.reset(inputs.size());
        for (int i = 0; i < (int) inputs.size(); ++i) {
            fInputs[i] = inputs[i];
        }
    }

private:
    friend class PaintOptions;  // for createPipelines() access

    // The PrecompileImageFilter classes do not use the PrecompileBase::addToKey virtual since
    // they, in general, do not themselves contribute to a given SkPaint/Pipeline but, rather,
    // create separate SkPaints/Pipelines from whole cloth (in onCreatePipelines).
    void addToKey(const KeyContext& keyContext,
                  PaintParamsKeyBuilder* builder,
                  PipelineDataGatherer* gatherer,
                  int desiredCombination) const final {
        SkASSERT(false);
    }

    virtual SkSpan<const sk_sp<PrecompileColorFilter>> onColorFilterOptions() const { return {}; }

    int countInputs() const { return fInputs.count(); }

    const PrecompileImageFilter* getInput(int index) const {
        SkASSERT(index < this->countInputs());
        return fInputs[index].get();
    }

    SkSpan<const sk_sp<PrecompileColorFilter>> colorFilterOptions() const {
        SkASSERT(this->countInputs() == 1);
        if (this->getInput(0)) {
            return {};
        }

        SkSpan<const sk_sp<PrecompileColorFilter>> tmp = this->onColorFilterOptions();
        if (tmp.empty()) {
            return {};
        }
        // TODO: as in SkImageFilter::asAColorFilter, handle the special case of
        // affectsTransparentBlack. This is tricky for precompilation since we don't,
        // necessarily, have all the parameters of the ColorFilter in order to evaluate
        // filterColor4f(SkColors::kTransparent) - the normal API's implementation.
        return tmp;
    }

    virtual void onCreatePipelines(const KeyContext&,
                                   PipelineDataGatherer*,
                                   const PaintOptions::ProcessCombination&) const = 0;

    void createPipelines(const KeyContext& keyContext,
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

    skia_private::AutoSTArray<2, sk_sp<PrecompileImageFilter>> fInputs;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Precompile_DEFINED
