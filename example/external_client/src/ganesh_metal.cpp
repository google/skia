/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/mtl/GrMtlDirectContext.h"
#include "include/gpu/ganesh/mtl/GrMtlBackendContext.h"
#include "include/encode/SkJpegEncoder.h"

#include "metal_context_helper.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <name.jpeg>\n", argv[0]);
        return 1;
    }

    SkFILEWStream output(argv[1]);
    if (!output.isValid()) {
        printf("Cannot open output file %s\n", argv[1]);
        return 1;
    }

    GrMtlBackendContext backendContext = GetMetalContext();
    sk_sp<GrDirectContext> ctx = GrDirectContexts::MakeMetal(backendContext);
    if (!ctx) {
        printf("Could not make metal context\n");
        return 1;
    }
    printf("Context made, now to make the surface\n");

    SkImageInfo imageInfo =
            SkImageInfo::Make(200, 400, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> surface =
            SkSurfaces::RenderTarget(ctx.get(), skgpu::Budgeted::kYes, imageInfo);
    if (!surface) {
        printf("Could not make surface from Metal DirectContext\n");
        return 1;
    }

    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorCYAN);
    SkRRect rrect = SkRRect::MakeRectXY(SkRect::MakeLTRB(10, 20, 50, 70), 10, 10);

    SkPaint paint;
    paint.setColor(SK_ColorMAGENTA);
    paint.setAntiAlias(true);

    canvas->drawRRect(rrect, paint);

    ctx->flush();

    printf("Drew to surface, now doing readback\n");
    sk_sp<SkImage> img = surface->makeImageSnapshot();
    sk_sp<SkData> jpeg = SkJpegEncoder::Encode(ctx.get(), img.get(), {});
    if (!jpeg) {
        printf("Readback of pixels (or encoding) failed\n");
        return 1;
    }
    output.write(jpeg->data(), jpeg->size());
    output.fsync();
    return 0;
}
