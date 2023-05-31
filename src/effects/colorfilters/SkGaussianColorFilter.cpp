/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/effects/colorfilters/SkGaussianColorFilter.h"

#include "include/core/SkColorFilter.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "src/core/SkColorFilterPriv.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"

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

SkGaussianColorFilter::SkGaussianColorFilter() : SkColorFilterBase() {}

bool SkGaussianColorFilter::appendStages(const SkStageRec& rec, bool shaderIsOpaque) const {
    rec.fPipeline->append(SkRasterPipelineOp::gauss_a_to_rgba);
    return true;
}

#if defined(SK_ENABLE_SKVM)
skvm::Color SkGaussianColorFilter::onProgram(skvm::Builder* p,
                                             skvm::Color c,
                                             const SkColorInfo& dst,
                                             skvm::Uniforms*,
                                             SkArenaAlloc*) const {
    // x = 1 - x;
    // exp(-x * x * 4) - 0.018f;
    // ... now approximate with quartic
    //
    skvm::F32 x = p->splat(-2.26661229133605957031f);
    x = c.a * x + 2.89795351028442382812f;
    x = c.a * x + 0.21345567703247070312f;
    x = c.a * x + 0.15489584207534790039f;
    x = c.a * x + 0.00030726194381713867f;
    return {x, x, x, x};
}
#endif

sk_sp<SkFlattenable> SkGaussianColorFilter::CreateProc(SkReadBuffer&) {
    return SkColorFilterPriv::MakeGaussian();
}

#if defined(SK_GRAPHITE)

void SkGaussianColorFilter::addToKey(const skgpu::graphite::KeyContext& keyContext,
                                     skgpu::graphite::PaintParamsKeyBuilder* builder,
                                     skgpu::graphite::PipelineDataGatherer* gatherer) const {
    using namespace skgpu::graphite;

    GaussianColorFilterBlock::BeginBlock(keyContext, builder, gatherer);
    builder->endBlock();
}

#endif

sk_sp<SkColorFilter> SkColorFilterPriv::MakeGaussian() {
    return sk_sp<SkColorFilter>(new SkGaussianColorFilter);
}
