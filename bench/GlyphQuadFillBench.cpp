/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFont.h"
#include "include/core/SkTypeface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/base/SkUTF.h"
#include "src/base/SkUtils.h"
#include "src/core/SkStrikeCache.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/text/GlyphRun.h"
#include "src/text/gpu/StrikeCache.h"
#include "src/text/gpu/TextBlob.h"
#include "src/utils/SkTestCanvas.h"
#include "tools/fonts/FontToolUtils.h"

// From Project Guttenberg. This is UTF-8 text.
static const char* gText =
        "Call me Ishmael.  Some years ago--never mind how long precisely";

class FillBench {};
template <> class SkTestCanvas<FillBench> {
public:
    static SkDevice* GetDevice(SkCanvas* canvas) {
        return canvas->topDevice();
    }
};

class DirectMaskGlyphVertexFillBenchmark : public Benchmark {
    bool isSuitableFor(Backend backend) override {
        return backend == kGPU_Backend;
    }

    const char* onGetName() override {
        return "DirectMaskGlyphVertexFillBenchmark";
    }

    void onPerCanvasPreDraw(SkCanvas* canvas) override {
        auto typeface = ToolUtils::CreateTestTypeface("monospace", SkFontStyle());
        SkFont font(typeface);

        SkMatrix view = SkMatrix::I();
        size_t len = strlen(gText);
        sktext::GlyphRunBuilder builder;
        SkPaint paint;
        auto glyphRunList = builder.textToGlyphRunList(font, paint, gText, len, {100, 100});
        SkASSERT_RELEASE(!glyphRunList.empty());
        auto device = SkTestCanvas<FillBench>::GetDevice(canvas);
        SkMatrix drawMatrix = view;
        const SkPoint drawOrigin = glyphRunList.origin();
        drawMatrix.preTranslate(drawOrigin.x(), drawOrigin.y());
        fBlob = sktext::gpu::TextBlob::Make(glyphRunList,
                                            paint,
                                            drawMatrix,
                                            device->strikeDeviceInfo(),
                                            SkStrikeCache::GlobalStrikeCache());

        const sktext::gpu::AtlasSubRun* subRun = fBlob->testingOnlyFirstSubRun();
        SkASSERT_RELEASE(subRun);
        subRun->testingOnly_packedGlyphIDToGlyph(&fCache);
        fVertices.reset(new char[subRun->vertexStride(drawMatrix) * subRun->glyphCount() * 4]);
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        const sktext::gpu::AtlasSubRun* subRun = fBlob->testingOnlyFirstSubRun();
        SkASSERT_RELEASE(subRun);

        SkIRect clip = SkIRect::MakeEmpty();
        SkPaint paint;
        GrColor grColor = SkColorToPremulGrColor(paint.getColor());
        SkMatrix positionMatrix = SkMatrix::Translate(100, 100);

        for (int loop = 0; loop < loops; loop++) {
            subRun->fillVertexData(fVertices.get(), 0, subRun->glyphCount(),
                                   grColor, positionMatrix, {0, 0}, clip);
        }
    }

private:
    sk_sp<sktext::gpu::TextBlob> fBlob;
    sktext::gpu::StrikeCache fCache;
    std::unique_ptr<char[]> fVertices;
};

DEF_BENCH(return new DirectMaskGlyphVertexFillBenchmark{});
