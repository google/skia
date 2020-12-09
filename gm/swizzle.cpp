/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkTypes.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/ops/GrFillRectOp.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

DEF_SIMPLE_GPU_GM(swizzle, ctx, rtCtx, canvas, 512, 512) {
    SkRect bounds = SkRect::MakeIWH(512, 512);

    SkBitmap bmp;
    GetResourceAsBitmap("images/mandrill_512_q075.jpg", &bmp);
    GrBitmapTextureMaker maker(ctx, bmp, GrImageTexGenPolicy::kDraw);
    auto view = maker.view(GrMipmapped::kNo);
    if (!view) {
        return;
    }
    std::unique_ptr<GrFragmentProcessor> imgFP =
        GrTextureEffect::Make(std::move(view), bmp.alphaType(), SkMatrix());
    auto fp = GrFragmentProcessor::SwizzleOutput(std::move(imgFP), GrSwizzle("grb1"));

    GrPaint grPaint;
    grPaint.setColorFragmentProcessor(std::move(fp));

    rtCtx->addDrawOp(GrFillRectOp::MakeNonAARect(ctx, std::move(grPaint), SkMatrix(), bounds));
}
