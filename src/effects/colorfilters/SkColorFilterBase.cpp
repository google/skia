/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/effects/colorfilters/SkColorFilterBase.h"

#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h" // IWYU pragma: keep
#include "include/core/SkColorType.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkSurfaceProps.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkAssert.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif

#if defined(SK_ENABLE_SKVM)
#include "src/core/SkVM.h"
#endif

#include <cstddef>

enum class SkBlendMode;

bool SkColorFilterBase::onAsAColorMode(SkColor*, SkBlendMode*) const {
    return false;
}

bool SkColorFilterBase::onAsAColorMatrix(float matrix[20]) const {
    return false;
}

#if defined(SK_ENABLE_SKVM)
skvm::Color SkColorFilterBase::program(skvm::Builder* p, skvm::Color c,
                                       const SkColorInfo& dst,
                                       skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const {
    skvm::F32 original = c.a;
    if ((c = this->onProgram(p,c, dst, uniforms,alloc))) {
        if (this->isAlphaUnchanged()) {
            c.a = original;
        }
        return c;
    }
    //SkDebugf("cannot program %s\n", this->getTypeName());
    return {};
}
#endif

SkPMColor4f SkColorFilterBase::onFilterColor4f(const SkPMColor4f& color,
                                               SkColorSpace* dstCS) const {
    constexpr size_t kEnoughForCommonFilters = 2048;  // big enough for a tiny SkSL program
    SkSTArenaAlloc<kEnoughForCommonFilters> alloc;
    SkRasterPipeline    pipeline(&alloc);
    pipeline.append_constant_color(&alloc, color.vec());
    SkMatrixProvider matrixProvider(SkMatrix::I());
    SkSurfaceProps props{}; // default OK; colorFilters don't render text
    SkStageRec rec = {&pipeline, &alloc, kRGBA_F32_SkColorType, dstCS, color.unpremul(), props};

    if (as_CFB(this)->appendStages(rec, color.fA == 1)) {
        SkPMColor4f dst;
        SkRasterPipeline_MemoryCtx dstPtr = { &dst, 0 };
        pipeline.append(SkRasterPipelineOp::store_f32, &dstPtr);
        pipeline.run(0,0, 1,1);
        return dst;
    }

#if defined(SK_ENABLE_SKVM)
    // This filter doesn't support SkRasterPipeline... try skvm.
    skvm::Builder b;
    skvm::Uniforms uni(b.uniform(), 4);
    SkColor4f uniColor = {color.fR, color.fG, color.fB, color.fA};
    SkColorInfo dstInfo = {kRGBA_F32_SkColorType, kPremul_SkAlphaType, sk_ref_sp(dstCS)};
    if (skvm::Color filtered =
            as_CFB(this)->program(&b, b.uniformColor(uniColor, &uni), dstInfo, &uni, &alloc)) {

        b.store({skvm::PixelFormat::FLOAT, 32,32,32,32, 0,32,64,96},
                b.varying<SkColor4f>(), filtered);

        const bool allow_jit = false;  // We're only filtering one color, no point JITing.
        b.done("filterColor4f", allow_jit).eval(1, uni.buf.data(), &color);
        return color;
    }
#endif

    SkDEBUGFAIL("onFilterColor4f unimplemented for this filter");
    return SkPMColor4f{0,0,0,0};
}

#if defined(SK_GRAPHITE)
void SkColorFilterBase::addToKey(const skgpu::graphite::KeyContext& keyContext,
                                 skgpu::graphite::PaintParamsKeyBuilder* builder,
                                 skgpu::graphite::PipelineDataGatherer* gatherer) const {
    using namespace skgpu::graphite;

    // Return the input color as-is.
    PriorOutputBlock::BeginBlock(keyContext, builder, gatherer);
    builder->endBlock();
}
#endif
