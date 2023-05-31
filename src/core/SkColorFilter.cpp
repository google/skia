/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkColorFilter.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"  // IWYU pragma: keep
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurfaceProps.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkAssert.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkColorFilterBase.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkRuntimeEffectPriv.h"

#include <cstddef>
#include <iterator>

enum class SkBlendMode;
struct SkDeserialProcs;

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif

bool SkColorFilter::asAColorMode(SkColor* color, SkBlendMode* mode) const {
    return as_CFB(this)->onAsAColorMode(color, mode);
}

bool SkColorFilter::asAColorMatrix(float matrix[20]) const {
    return as_CFB(this)->onAsAColorMatrix(matrix);
}

bool SkColorFilter::isAlphaUnchanged() const {
    return as_CFB(this)->onIsAlphaUnchanged();
}

sk_sp<SkColorFilter> SkColorFilter::Deserialize(const void* data, size_t size,
                                                const SkDeserialProcs* procs) {
    return sk_sp<SkColorFilter>(static_cast<SkColorFilter*>(
                                SkFlattenable::Deserialize(
                                kSkColorFilter_Type, data, size, procs).release()));
}

//////////////////////////////////////////////////////////////////////////////////////////////////

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

SkColor SkColorFilter::filterColor(SkColor c) const {
    // This is mostly meaningless. We should phase-out this call entirely.
    SkColorSpace* cs = nullptr;
    return this->filterColor4f(SkColor4f::FromColor(c), cs, cs).toSkColor();
}

SkColor4f SkColorFilter::filterColor4f(const SkColor4f& origSrcColor, SkColorSpace* srcCS,
                                       SkColorSpace* dstCS) const {
    SkPMColor4f color = { origSrcColor.fR, origSrcColor.fG, origSrcColor.fB, origSrcColor.fA };
    SkColorSpaceXformSteps(srcCS, kUnpremul_SkAlphaType,
                           dstCS, kPremul_SkAlphaType).apply(color.vec());

    return as_CFB(this)->onFilterColor4f(color, dstCS).unpremul();
}

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

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkColorFilter> SkColorFilters::Lerp(float weight, sk_sp<SkColorFilter> cf0,
                                                        sk_sp<SkColorFilter> cf1) {
#ifdef SK_ENABLE_SKSL
    if (!cf0 && !cf1) {
        return nullptr;
    }
    if (SkScalarIsNaN(weight)) {
        return nullptr;
    }

    if (cf0 == cf1) {
        return cf0; // or cf1
    }

    if (weight <= 0) {
        return cf0;
    }
    if (weight >= 1) {
        return cf1;
    }

    static const SkRuntimeEffect* effect =
            SkMakeCachedRuntimeEffect(SkRuntimeEffect::MakeForColorFilter,
                                      "uniform colorFilter cf0;"
                                      "uniform colorFilter cf1;"
                                      "uniform half weight;"
                                      "half4 main(half4 color) {"
                                      "return mix(cf0.eval(color), cf1.eval(color), weight);"
                                      "}")
                    .release();
    SkASSERT(effect);

    sk_sp<SkColorFilter> inputs[] = {cf0,cf1};
    return effect->makeColorFilter(SkData::MakeWithCopy(&weight, sizeof(weight)),
                                   inputs, std::size(inputs));
#else
    // TODO(skia:12197)
    return nullptr;
#endif
}

