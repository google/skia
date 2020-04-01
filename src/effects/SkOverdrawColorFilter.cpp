/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkUnPreMultiply.h"
#include "include/effects/SkOverdrawColorFilter.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkVM.h"

#if SK_SUPPORT_GPU
#include "include/effects/SkRuntimeEffect.h"
#include "src/gpu/effects/GrSkSLFP.h"

GR_FP_SRC_STRING SKSL_OVERDRAW_SRC = R"(
uniform half4 color0;
uniform half4 color1;
uniform half4 color2;
uniform half4 color3;
uniform half4 color4;
uniform half4 color5;

void main(inout half4 color) {
    half alpha = 255.0 * color.a;
    if (alpha < 0.5) {
        color = color0;
    } else if (alpha < 1.5) {
        color = color1;
    } else if (alpha < 2.5) {
        color = color2;
    } else if (alpha < 3.5) {
        color = color3;
    } else if (alpha < 4.5) {
        color = color4;
    } else {
        color = color5;
    }
}
)";
#endif

static void convert_to_pm4f(SkPMColor4f dst[], const SkColor src[]) {
    for (int i = 0; i < SkOverdrawColorFilter::kNumColors; ++i) {
        dst[i] = SkColor4f::FromColor(src[i]).premul();
    }
}

bool SkOverdrawColorFilter::onAppendStages(const SkStageRec& rec, bool shader_is_opaque) const {
    struct Ctx : public SkRasterPipeline_CallbackCtx {
        SkPMColor4f colors[kNumColors];
    };
    // TODO: do we care about transforming to dstCS?
    auto ctx = rec.fAlloc->make<Ctx>();
    convert_to_pm4f(ctx->colors, fColors);

    ctx->fn = [](SkRasterPipeline_CallbackCtx* arg, int active_pixels) {
        auto ctx = (Ctx*)arg;
        auto pixels = (SkPMColor4f*)ctx->rgba;
        for (int i = 0; i < active_pixels; i++) {
            uint8_t alpha = (int)(pixels[i].fA * 255);
            if (alpha >= kNumColors) {
                alpha = kNumColors - 1;
            }
            pixels[i] = ctx->colors[alpha];
        }
    };
    rec.fPipeline->append(SkRasterPipeline::callback, ctx);
    return true;
}

skvm::Color SkOverdrawColorFilter::onProgram(skvm::Builder* p, skvm::Color c,
                                             SkColorSpace* /*dstCS*/, skvm::Uniforms* uniforms,
                                             SkArenaAlloc* alloc) const {
    skvm::I32 index = min(to_unorm(8,c.a), kNumColors - 1);
    c = unpack_8888(gather32(uniforms->pushPtr(fColors), index));
    std::swap(c.r, c.b);  // The SkColors in fColors are BGRA, but we want RGBA
    return c;
}

void SkOverdrawColorFilter::flatten(SkWriteBuffer& buffer) const {
    buffer.writeByteArray(fColors, kNumColors * sizeof(SkPMColor));
}

sk_sp<SkFlattenable> SkOverdrawColorFilter::CreateProc(SkReadBuffer& buffer) {
    SkPMColor colors[kNumColors];
    size_t size = buffer.getArrayCount();
    if (!buffer.validate(size == sizeof(colors))) {
        return nullptr;
    }
    if (!buffer.readByteArray(colors, sizeof(colors))) {
        return nullptr;
    }

    return sk_sp<SkColorFilter>(new SkOverdrawColorFilter(colors));
}

void SkOverdrawColorFilter::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkOverdrawColorFilter);
}
#if SK_SUPPORT_GPU

#include "include/private/GrRecordingContext.h"

std::unique_ptr<GrFragmentProcessor> SkOverdrawColorFilter::asFragmentProcessor(
        GrRecordingContext* context, const GrColorInfo&) const {
    static auto effect = std::get<0>(SkRuntimeEffect::Make(SkString(SKSL_OVERDRAW_SRC)));
    SkASSERT(effect->inputSize() == (kNumColors * sizeof(SkPMColor4f)));

    auto inputs = SkData::MakeUninitialized(kNumColors * sizeof(SkPMColor4f));
    convert_to_pm4f(reinterpret_cast<SkPMColor4f*>(inputs->writable_data()), fColors);

    return GrSkSLFP::Make(context, effect, "Overdraw", std::move(inputs));
}

#endif
