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
#include "src/gpu/effects/GrSkSLFP.h"

GR_FP_SRC_STRING SKSL_OVERDRAW_SRC = R"(
layout(ctype=SkPMColor) uniform half4 color0;
layout(ctype=SkPMColor) uniform half4 color1;
layout(ctype=SkPMColor) uniform half4 color2;
layout(ctype=SkPMColor) uniform half4 color3;
layout(ctype=SkPMColor) uniform half4 color4;
layout(ctype=SkPMColor) uniform half4 color5;

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

    return SkOverdrawColorFilter::Make(colors);
}

void SkOverdrawColorFilter::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkOverdrawColorFilter);
}
#if SK_SUPPORT_GPU

#include "include/private/GrRecordingContext.h"

std::unique_ptr<GrFragmentProcessor> SkOverdrawColorFilter::asFragmentProcessor(
        GrRecordingContext* context, const GrColorSpaceInfo&) const {
    static int overdrawIndex = GrSkSLFP::NewIndex();
    return GrSkSLFP::Make(context, overdrawIndex, "Overdraw", SKSL_OVERDRAW_SRC, fColors,
                          sizeof(fColors));
}

#endif
