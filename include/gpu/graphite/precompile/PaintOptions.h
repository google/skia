/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_PaintOptions_DEFINED
#define skgpu_graphite_precompile_PaintOptions_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTDArray.h"

#include <functional>

namespace skgpu::graphite {

class PrecompileBlender;
class PrecompileColorFilter;
class PrecompileImageFilter;
class PrecompileMaskFilter;
class PrecompileShader;

enum class Coverage;
enum DrawTypeFlags : uint16_t;
enum class PrecompileImageFilterFlags : uint32_t;

class KeyContext;
class PaintOptionsPriv;
class PaintParamsKeyBuilder;
class PipelineDataGatherer;
struct RenderPassDesc;
class UniquePaintParamsID;

/** \class PaintOptions
    This is the Precompilation analog to SkPaint. It encapsulates a set of options for each
    field of the SkPaint (e.g., colorFilters, imageFilters, etc). Many of the specific details
    of an SkPaint that are irrelevant to the final compiled Pipelines are abstracted away
    (e.g., the SkPaint's color field).

    How Precompilation works in practice is a PaintOptions object is created and a set of options
    for each slot (e.g., shader, blender) are added. When passed to the Precompile() function,
    all the combinations specified by the PaintOptions will be created and precompiled.

    To be concrete, if a PaintOptions object had two shader options and two blender options,
    four combinations would be precompiled.
*/
class SK_API PaintOptions {
public:
    /** Constructs a PaintOptions object with default values. It is equivalent to a default
     *  initialized SkPaint.

        @return  default initialized PaintOptions
    */
    PaintOptions();
    PaintOptions(const PaintOptions&);
    ~PaintOptions();
    PaintOptions& operator=(const PaintOptions&);

    /** Sets the shader options used when generating precompilation combinations.

        This corresponds to SkPaint's setShader() method

        @param shaders  The options used for shading when generating precompilation combinations.
    */
    void setShaders(SkSpan<const sk_sp<PrecompileShader>> shaders);
    SkSpan<const sk_sp<PrecompileShader>> getShaders() const {
        return SkSpan<const sk_sp<PrecompileShader>>(fShaderOptions);
    }

    /** Sets the image filter options used when generating precompilation combinations.

        This corresponds to SkPaint's setImageFilter() method

        @param imageFilters  The options used for image filtering when generating precompilation
                             combinations.
    */
    void setImageFilters(SkSpan<const sk_sp<PrecompileImageFilter>> imageFilters);
    SkSpan<const sk_sp<PrecompileImageFilter>> getImageFilters() const {
        return SkSpan<const sk_sp<PrecompileImageFilter>>(fImageFilterOptions);
    }

    /** Sets the mask filter options used when generating precompilation combinations.

        This corresponds to SkPaint's setMaskFilter() method

        @param maskFilters  The options used for mask filtering when generating precompilation
                            combinations.
    */
    void setMaskFilters(SkSpan<const sk_sp<PrecompileMaskFilter>> maskFilters);
    SkSpan<const sk_sp<PrecompileMaskFilter>> getMaskFilters() const {
        return SkSpan<const sk_sp<PrecompileMaskFilter>>(fMaskFilterOptions);
    }

    /** Sets the color filter options used when generating precompilation combinations.

        This corresponds to SkPaint's setColorFilter() method

        @param colorFilters  The options used for color filtering when generating precompilation
                             combinations.
    */
    void setColorFilters(SkSpan<const sk_sp<PrecompileColorFilter>> colorFilters);
    SkSpan<const sk_sp<PrecompileColorFilter>> getColorFilters() const {
        return SkSpan<const sk_sp<PrecompileColorFilter>>(fColorFilterOptions);
    }

    /** Sets the blend mode options used when generating precompilation combinations.

        This corresponds to SkPaint's setBlendMode() method

        @param blendModes  The options used for blending when generating precompilation
                           combinations.
    */
    void setBlendModes(SkSpan<const SkBlendMode> blendModes);
    SkSpan<const SkBlendMode> getBlendModes() const {
        return SkSpan<const SkBlendMode>(fBlendModeOptions.data(), fBlendModeOptions.size());
    }

    /** Sets the blender options used when generating precompilation combinations.

        This corresponds to SkPaint's setBlender() method

        @param blenders  The options used for blending when generating precompilation combinations.
    */
    void setBlenders(SkSpan<const sk_sp<PrecompileBlender>> blenders);
    SkSpan<const sk_sp<PrecompileBlender>> getBlenders() const {
        return SkSpan<const sk_sp<PrecompileBlender>>(fBlenderOptions);
    }

    /** Sets the dither setting used when generating precompilation combinations

        This corresponds to SkPaint's setDither() method

        @param dither  the dither setting used when generating precompilation combinations.
    */
    void setDither(bool dither) { fDither = dither; }
    bool isDither() const { return fDither; }

    // Provides access to functions that aren't part of the public API.
    PaintOptionsPriv priv();
    const PaintOptionsPriv priv() const;  // NOLINT(readability-const-return-type)

private:
    friend class PaintOptionsPriv;
    friend class PrecompileImageFilter; // for ProcessCombination access
    friend class PrecompileMaskFilter;  // for ProcessCombination access

    void addColorFilter(sk_sp<PrecompileColorFilter> cf);
    void addBlendMode(SkBlendMode bm) {
        fBlendModeOptions.push_back(bm);
    }

    void setClipShaders(SkSpan<const sk_sp<PrecompileShader>> clipShaders);

    int numShaderCombinations() const;
    int numColorFilterCombinations() const;
    int numBlendCombinations() const;
    int numClipShaderCombinations() const;

    int numCombinations() const;
    // 'desiredCombination' must be less than the result of the numCombinations call
    void createKey(const KeyContext&,
                   PaintParamsKeyBuilder*,
                   PipelineDataGatherer*,
                   int desiredCombination,
                   bool addPrimitiveBlender,
                   Coverage coverage) const;

    typedef std::function<void(UniquePaintParamsID id,
                               DrawTypeFlags,
                               bool withPrimitiveBlender,
                               Coverage,
                               const RenderPassDesc&)> ProcessCombination;

    void buildCombinations(const KeyContext&,
                           PipelineDataGatherer*,
                           DrawTypeFlags,
                           bool addPrimitiveBlender,
                           Coverage,
                           const RenderPassDesc&,
                           const ProcessCombination&) const;

    skia_private::TArray<sk_sp<PrecompileShader>> fShaderOptions;
    skia_private::TArray<sk_sp<PrecompileColorFilter>> fColorFilterOptions;
    skia_private::TArray<SkBlendMode> fBlendModeOptions;
    skia_private::TArray<sk_sp<PrecompileBlender>> fBlenderOptions;
    skia_private::TArray<sk_sp<PrecompileShader>> fClipShaderOptions;

    skia_private::TArray<sk_sp<PrecompileImageFilter>> fImageFilterOptions;
    skia_private::TArray<sk_sp<PrecompileMaskFilter>> fMaskFilterOptions;

    bool fDither = false;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_PaintOptions_DEFINED
