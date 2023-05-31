/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkGaussianColorFilter_DEFINED
#define SkGaussianColorFilter_DEFINED

#include "include/core/SkFlattenable.h"
#include "include/core/SkTypes.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"

class SkReadBuffer;
class SkWriteBuffer;
struct SkStageRec;

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"

namespace skgpu::graphite {
class PipelineDataGatherer;
}
#endif

#if defined(SK_ENABLE_SKVM)
#include "src/core/SkVM.h"
class SkArenaAlloc;
class SkColorInfo;
#endif

/**
 * Remaps the input color's alpha to a Gaussian ramp and then outputs premul white using the
 * remapped alpha.
 */
class SkGaussianColorFilter final : public SkColorFilterBase {
public:
    SkGaussianColorFilter();

    bool appendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;

    SkColorFilterBase::Type type() const override { return SkColorFilterBase::Type::kGaussian; }

#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif

protected:
    void flatten(SkWriteBuffer&) const override {}

#if defined(SK_ENABLE_SKVM)
    skvm::Color onProgram(skvm::Builder* p,
                          skvm::Color c,
                          const SkColorInfo& dst,
                          skvm::Uniforms*,
                          SkArenaAlloc*) const override;
#endif

private:
    SK_FLATTENABLE_HOOKS(SkGaussianColorFilter)
};

#endif
