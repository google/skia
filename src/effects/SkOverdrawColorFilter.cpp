/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkOverdrawColorFilter.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"

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

#ifdef SK_PMCOLOR_IS_BGRA
static uint32_t swizzle_rb(uint32_t c) {
    return SkColorSetARGB(SkColorGetA(c), SkColorGetB(c), SkColorGetG(c), SkColorGetR(c));
}
#endif

sk_sp<SkColorFilter> SkOverdrawColorFilter::Make(const uint32_t rgba[kNumColors]) {
    SkPMColor pm[kNumColors];
    for (int i = 0; i < kNumColors; ++i) {
#ifdef SK_PMCOLOR_IS_BGRA
        pm[i] = swizzle_rb(rgba[i]);
#else
        pm[i] = rgba[i];
#endif
    }
    return sk_sp<SkColorFilter>(new SkOverdrawColorFilter(pm));
}

sk_sp<SkColorFilter> SkOverdrawColorFilter::MakeWithSkColors(const SkColor colors[kNumColors]) {
    SkPMColor pm[kNumColors];
    for (int i = 0; i < kNumColors; ++i) {
        pm[i] = SkPreMultiplyColor(colors[i]);
    }
    return sk_sp<SkColorFilter>(new SkOverdrawColorFilter(pm));
}

SkOverdrawColorFilter::SkOverdrawColorFilter(const SkPMColor colors[kNumColors]) {
    memcpy(fColors, colors, kNumColors * sizeof(SkPMColor));
}

bool SkOverdrawColorFilter::onAppendStages(const SkStageRec& rec, bool shader_is_opaque) const {
    struct Ctx : public SkRasterPipeline_CallbackCtx {
        const SkPMColor* colors;
    };
    // TODO: do we care about transforming to dstCS?
    auto ctx = rec.fAlloc->make<Ctx>();
    ctx->colors = fColors;
    ctx->fn = [](SkRasterPipeline_CallbackCtx* arg, int active_pixels) {
        auto ctx = (Ctx*)arg;
        auto pixels = (SkPMColor4f*)ctx->rgba;
        for (int i = 0; i < active_pixels; i++) {
            uint8_t alpha = (int)(pixels[i].fA * 255);
            if (alpha >= kNumColors) {
                alpha = kNumColors - 1;
            }
            pixels[i] = SkPMColor4f::FromPMColor(ctx->colors[alpha]);
        }
    };
    rec.fPipeline->append(SkRasterPipeline::callback, ctx);
    return true;
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
    SkPMColor4f* floatColors = reinterpret_cast<SkPMColor4f*>(inputs->writable_data());
    for (int i = 0; i < kNumColors; ++i) {
        floatColors[i] = SkPMColor4f::FromPMColor(fColors[i]);
    }
    return GrSkSLFP::Make(context, effect, "Overdraw", std::move(inputs));
}

#endif
