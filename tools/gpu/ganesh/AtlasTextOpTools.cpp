/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/ganesh/AtlasTextOpTools.h"

#include "include/core/SkFont.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrikeCache.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/text/GlyphRun.h"
#include "src/text/gpu/TextBlob.h"
#include "tools/text/gpu/TextBlobTools.h"

#if defined(GR_TEST_UTILS)
#include "src/base/SkRandom.h"
#include "src/gpu/ganesh/GrDrawOpTest.h"
#endif

namespace skgpu::ganesh {

GrOp::Owner AtlasTextOpTools::CreateOp(skgpu::ganesh::SurfaceDrawContext* sdc,
                                       const SkPaint& skPaint,
                                       const SkFont& font,
                                       const SkMatrix& ctm,
                                       const char* text,
                                       int x,
                                       int y) {
    size_t textLen = (int)strlen(text);

    SkMatrix drawMatrix = ctm;
    drawMatrix.preTranslate(x, y);
    auto drawOrigin = SkPoint::Make(x, y);
    sktext::GlyphRunBuilder builder;
    auto glyphRunList = builder.textToGlyphRunList(font, skPaint, text, textLen, drawOrigin);
    if (glyphRunList.empty()) {
        return nullptr;
    }

    auto rContext = sdc->recordingContext();
    sktext::gpu::SDFTControl control =
            rContext->priv().getSDFTControl(sdc->surfaceProps().isUseDeviceIndependentFonts());

    SkStrikeDeviceInfo strikeDeviceInfo{
            sdc->surfaceProps(), SkScalerContextFlags::kBoostContrast, &control};

    sk_sp<sktext::gpu::TextBlob> blob =
            sktext::gpu::TextBlob::Make(glyphRunList,
                                        skPaint,
                                        drawMatrix,
                                        strikeDeviceInfo,
                                        SkStrikeCache::GlobalStrikeCache());

    const sktext::gpu::AtlasSubRun* subRun = sktext::gpu::TextBlobTools::FirstSubRun(blob.get());
    if (!subRun) {
        return nullptr;
    }

    GrOp::Owner op;
    std::tie(std::ignore, op) =
            subRun->makeAtlasTextOp(nullptr, ctm, glyphRunList.origin(), skPaint, blob, sdc);
    return op;
}

}  // namespace skgpu::ganesh

#if defined(GR_TEST_UTILS)
GR_DRAW_OP_TEST_DEFINE(AtlasTextOp) {
    SkMatrix ctm = GrTest::TestMatrixInvertible(random);

    SkPaint skPaint;
    skPaint.setColor(random->nextU());

    SkFont font;
    if (random->nextBool()) {
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
    } else {
        font.setEdging(random->nextBool() ? SkFont::Edging::kAntiAlias : SkFont::Edging::kAlias);
    }
    font.setSubpixel(random->nextBool());

    const char* text = "The quick brown fox jumps over the lazy dog.";

    // create some random x/y offsets, including negative offsets
    static const int kMaxTrans = 1024;
    int xPos = (random->nextU() % 2) * 2 - 1;
    int yPos = (random->nextU() % 2) * 2 - 1;
    int xInt = (random->nextU() % kMaxTrans) * xPos;
    int yInt = (random->nextU() % kMaxTrans) * yPos;

    return skgpu::ganesh::AtlasTextOpTools::CreateOp(sdc, skPaint, font, ctm, text, xInt, yInt);
}
#endif
