/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkOverdrawColorFilter.h"
#include "SkPM4f.h"
#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "../jumper/SkJumper.h"

void SkOverdrawColorFilter::onAppendStages(SkRasterPipeline* p,
                                           SkColorSpace* dstCS,
                                           SkArenaAlloc* alloc,
                                           bool shader_is_opaque) const {
    struct Ctx : public SkJumper_CallbackCtx {
        const SkPMColor* colors;
    };
    // TODO: do we care about transforming to dstCS?
    auto ctx = alloc->make<Ctx>();
    ctx->colors = fColors;
    ctx->fn = [](SkJumper_CallbackCtx* arg, int active_pixels) {
        auto ctx = (Ctx*)arg;
        auto pixels = (SkPM4f*)ctx->rgba;
        for (int i = 0; i < active_pixels; i++) {
            uint8_t alpha = (int)(pixels[i].a() * 255);
            if (alpha >= kNumColors) {
                alpha = kNumColors - 1;
            }
            pixels[i] = SkPM4f::FromPMColor(ctx->colors[alpha]);
        }
    };
    p->append(SkRasterPipeline::callback, ctx);
}

void SkOverdrawColorFilter::toString(SkString* str) const {
    str->append("SkOverdrawColorFilter (");
    for (int i = 0; i < kNumColors; i++) {
        str->appendf("%d: %x\n", i, fColors[i]);
    }
    str->append(")");
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

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkOverdrawColorFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkOverdrawColorFilter)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
#if SK_SUPPORT_GPU

#include "effects/GrOverdrawFragmentProcessor.h"

std::unique_ptr<GrFragmentProcessor> SkOverdrawColorFilter::asFragmentProcessor(
        GrContext*, const GrColorSpaceInfo&) const {
    return GrOverdrawFragmentProcessor::Make(fColors[0], fColors[1], fColors[2], fColors[3],
                                             fColors[4], fColors[5]);
}

#endif
