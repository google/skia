/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkFont.h"
#include "include/core/SkTypeface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkUtils.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/text/GrTextBlob.h"
#include "src/utils/SkUTF.h"


// From Project Guttenberg. This is UTF-8 text.
static const char* atext =
        "Call me Ishmael.  Some years ago--never mind how long precisely";

class DirectMaskGlyphVertexFill : public Benchmark {
    bool isSuitableFor(Backend backend) override {
        return backend == kGPU_Backend;
    }

    const char* onGetName() override {
        return "DirectMaskGlyphVertexFill";
    }

    void onPerCanvasPreDraw(SkCanvas* canvas) override {
        auto typeface = SkTypeface::MakeFromName("monospace", SkFontStyle());
        SkFont font(typeface);

        size_t len = strlen(atext);
        SkGlyphRunBuilder builder;
        builder.drawTextUTF8(SkPaint{}, font, atext, len, {100,100});
        auto glyphRunList = builder.useGlyphRunList();
        SkASSERT(!glyphRunList.empty());
        fBlob = GrTextBlob::Make(glyphRunList, SkMatrix::I());

        SkSurfaceProps props{SkSurfaceProps::kLegacyFontHost_InitType};
        auto colorSpace = SkColorSpace::MakeSRGB();
        SkGlyphRunListPainter painter{props, kUnknown_SkColorType,
                                      colorSpace.get(), SkStrikeCache::GlobalStrikeCache()};

        GrSDFTOptions options = canvas->recordingContext()->priv().SDFTOptions();
        painter.processGlyphRunList(
                glyphRunList, SkMatrix::I(), props, false, options, fBlob.get());

        SkASSERT(fBlob->subRunList().head() != nullptr);
        GrAtlasSubRun* subRun = static_cast<GrAtlasSubRun*>(fBlob->subRunList().head());
        subRun->testing_convertToGrGlyph(&fCache);
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        GrAtlasSubRun* subRun = static_cast<GrAtlasSubRun*>(fBlob->subRunList().head());

        GrColor color = 0;
        SkIRect clip = SkIRect::MakeEmpty();

        char* vertices = new char[subRun->vertexStride() * subRun->glyphCount() * 4];
        for (int loop = 0; loop < loops; loop++) {
            subRun->fillVertexData(vertices, 0, subRun->glyphCount(),
                    color, SkMatrix::I(), {100, 100}, clip);
        }
    }

private:
    sk_sp<GrTextBlob> fBlob;
    GrStrikeCache fCache;
};

DEF_BENCH(return new DirectMaskGlyphVertexFill{});
