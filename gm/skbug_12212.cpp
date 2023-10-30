/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "tools/fonts/FontToolUtils.h"

DEF_SIMPLE_GM_BG(skbug_12212, canvas, 400, 400, SK_ColorCYAN) {
    // Create an Alpha_8 surface to draw into (strangely, with RGB pixel geometry).
    auto imageInfo = SkImageInfo::Make(/*width=*/400, /*height=*/400, kAlpha_8_SkColorType,
                                       kPremul_SkAlphaType);
    SkSurfaceProps props(/*flags=*/0, kRGB_H_SkPixelGeometry);
    sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(
            canvas->recordingContext(), skgpu::Budgeted::kNo, imageInfo, /*sampleCount=*/0, &props);
    if (!surface) {
        surface = SkSurfaces::Raster(imageInfo, &props);
    }

    // Draw text into the surface using LCD antialiasing.
    SkPaint p;
    p.setAntiAlias(true);
    p.setBlendMode(SkBlendMode::kSrc);
    p.setAlpha(0x80);
    SkFont font = ToolUtils::DefaultPortableFont();
    font.setSize(170);
    font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
    auto textBlob = SkTextBlob::MakeFromText("text", /*byteLength=*/4, font);
    surface->getCanvas()->drawTextBlob(textBlob, /*x=*/50, /*y=*/350, p);

    // Draw the surface on our main canvas.
    surface->draw(canvas, /*x=*/0, /*y=*/0);
}
