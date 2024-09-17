/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_precompile_PrecompileImageFilter_DEFINED
#define skgpu_graphite_precompile_PrecompileImageFilter_DEFINED

#include "include/gpu/graphite/precompile/PrecompileBase.h"

#include "include/core/SkBlendMode.h"
#include "include/gpu/graphite/precompile/PaintOptions.h"
#include "include/private/base/SkTemplates.h"

namespace skgpu::graphite {

class PrecompileBlender;
class PrecompileColorFilter;
class PrecompileImageFilterPriv;

/** \class PrecompileImageFilter
    This class corresponds to the SkImageFilter class in the main API.
*/
class SK_API PrecompileImageFilter : public PrecompileBase {
public:
    ~PrecompileImageFilter() override;

    // Provides access to functions that aren't part of the public API.
    PrecompileImageFilterPriv priv();
    const PrecompileImageFilterPriv priv() const;  // NOLINT(readability-const-return-type)

protected:
    PrecompileImageFilter(SkSpan<sk_sp<PrecompileImageFilter>> inputs);

private:
    friend class PaintOptions;  // for createPipelines() access
    friend class PrecompileImageFilterPriv;

    int countInputs() const { return fInputs.count(); }

    const PrecompileImageFilter* getInput(int index) const {
        SkASSERT(index < this->countInputs());
        return fInputs[index].get();
    }

    virtual sk_sp<PrecompileColorFilter> isColorFilterNode() const { return nullptr; }

    sk_sp<PrecompileColorFilter> asAColorFilter() const;

    // The PrecompileImageFilter classes do not use the PrecompileBase::addToKey virtual since
    // they, in general, do not themselves contribute to a given SkPaint/Pipeline but, rather,
    // create separate SkPaints/Pipelines from whole cloth (in onCreatePipelines).
    void addToKey(const KeyContext& /* keyContext */,
                  PaintParamsKeyBuilder* /* builder */,
                  PipelineDataGatherer* /* gatherer */,
                  int /* desiredCombination */) const final {
        SkASSERT(false);
    }

    virtual void onCreatePipelines(const KeyContext&,
                                   PipelineDataGatherer*,
                                   const RenderPassDesc&,
                                   const PaintOptions::ProcessCombination&) const = 0;

    void createPipelines(const KeyContext&,
                         PipelineDataGatherer*,
                         const RenderPassDesc&,
                         const PaintOptions::ProcessCombination&);

    skia_private::AutoSTArray<2, sk_sp<PrecompileImageFilter>> fInputs;
};

//--------------------------------------------------------------------------------------------------
// This is the Precompile correlate to the SkImageFilters class' factories in the main API
//
// Note: In order to make precompilation analysis more tractable we don't allow options for the
// internals of an PrecompileImageFilter nor in the structure of the DAG.
namespace PrecompileImageFilters {

    // This is the Precompile correlate to SkImageFilters::Arithmetic
    SK_API sk_sp<PrecompileImageFilter> Arithmetic(sk_sp<PrecompileImageFilter> background,
                                                   sk_sp<PrecompileImageFilter> foreground);

    // This is the Precompile correlate to SkImageFilters::Blend(SkBlendMode, ...)
    SK_API sk_sp<PrecompileImageFilter> Blend(SkBlendMode bm,
                                              sk_sp<PrecompileImageFilter> background,
                                              sk_sp<PrecompileImageFilter> foreground);

    // This is the Precompile correlate to SkImageFilters::Blend(sk_sp<SkBlender>, ...)
    SK_API sk_sp<PrecompileImageFilter> Blend(sk_sp<PrecompileBlender> blender,
                                              sk_sp<PrecompileImageFilter> background,
                                              sk_sp<PrecompileImageFilter> foreground);

    // This is the Precompile correlate to the two SkImageFilters::Blur factories
    SK_API sk_sp<PrecompileImageFilter> Blur(sk_sp<PrecompileImageFilter> input);

    // This is the Precompile correlate to SkImageFilters::ColorFilter.
    SK_API sk_sp<PrecompileImageFilter> ColorFilter(sk_sp<PrecompileColorFilter> colorFilter,
                                                    sk_sp<PrecompileImageFilter> input);

    // This is the Precompile correlate to SkImageFilters::DisplacementMap
    SK_API sk_sp<PrecompileImageFilter> DisplacementMap(sk_sp<PrecompileImageFilter> input);

    // This is the Precompile correlate to all of SkImageFilters::
    //      DistantLitDiffuse,  PointLitDiffuse,  SpotLitDiffuse
    //      DistantLitSpecular, PointLitSpecular, SpotLitSpecular
    SK_API sk_sp<PrecompileImageFilter> Lighting(sk_sp<PrecompileImageFilter> input);

    // This is the Precompile correlate to SkImageFilters::MatrixConvolution
    SK_API sk_sp<PrecompileImageFilter> MatrixConvolution(sk_sp<PrecompileImageFilter> input);

    // This is the Precompile correlate to SkImageFilters::Erode and SkImageFilters::Dilate
    SK_API sk_sp<PrecompileImageFilter> Morphology(sk_sp<PrecompileImageFilter> input);

} // namespace PrecompileImageFilters

} // namespace skgpu::graphite

#endif // skgpu_graphite_precompile_PrecompileImageFilter_DEFINED
