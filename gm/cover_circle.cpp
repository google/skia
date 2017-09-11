/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkRasterPipeline.h"
#include "../src/jumper/SkJumper.h"

extern bool gSkForceRasterPipelineBlitter;

DEF_SIMPLE_GM(cover_circle, canvas, 256,256) {
    SkPixmap pm;
    if (!canvas->peekPixels(&pm) || !gSkForceRasterPipelineBlitter) {
        SkPaint paint;
        paint.setColor(SK_ColorBLUE);
        canvas->drawCircle(128,128, 64, paint);
        return;
    }

    SkSTArenaAlloc<256> alloc;
    SkRasterPipeline p(&alloc);

    SkJumper_MemoryCtx dst = { pm.writable_addr(), pm.rowBytesAsPixels() };
    SkJumper_CoverCircleCtx cover = { 128.0f, 128.0f, 64.0f };

    p.append_constant_color(&alloc, SkColor4f{1.0f,0.0f,0.0f,1.0f});
#if 0
    p.append(SkRasterPipeline::load_8888_dst, &dst);
    p.append(SkRasterPipeline::lerp_circle, &cover);
    p.append(SkRasterPipeline::srcover);
#else
    p.append(SkRasterPipeline::scale_circle, &cover);
    p.append(SkRasterPipeline::load_8888_dst, &dst);
    p.append(SkRasterPipeline::srcover);
#endif
    p.append(SkRasterPipeline::store_8888, &dst);

    p.run(64,64, 128,128);
}
