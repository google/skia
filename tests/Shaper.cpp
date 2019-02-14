// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "SkTypes.h"

#ifdef SK_USING_SKSHAPER

#include "Test.h"

#include "Resources.h"
#include "SkCanvas.h"
#include "SkFontMgr.h"
#include "SkPDFDocument.h"
#include "SkPictureRecorder.h"
#include "SkRect.h"
#include "SkShaper.h"
#include "SkSurface.h"

#include <cstdio>

#include <memory>

DEF_TEST(ShaperSimple, r) {
    static constexpr char kText[] = "HПривет, мир!";
    sk_sp<SkFontMgr> fontManager = SkFontMgr::RefDefault();
    if (!fontManager) {
        ERRORF(r, "assert(SkFontMgr::RefDefault())");
        return;
    }
    SkFont font(fontManager->legacyMakeTypeface("Noto Sans", SkFontStyle()), 18);
    SkShaper shaper(font.refTypefaceOrDefault());
    if (!shaper.good()) {
        ERRORF(r, "assert(shaper.good())");
        return;
    }
    SkTextBlobBuilderRunHandler handler(kText);
    shaper.shape(&handler, font, kText, strlen(kText), true, {0, 0}, 504);
    auto blob = handler.makeBlob();
    REPORTER_ASSERT(r, blob);
}


static unsigned cluster_text_len(const uint32_t* clusters, int index,
                                 int glyphCount, uint32_t max) {
    uint32_t cluster = clusters[index];
    SkASSERT(max > cluster);
    uint32_t next = max;
    for (int i = 0; i < glyphCount; ++i) {
        if (clusters[i] > cluster && clusters[i] < next) {
            next = clusters[i];
        }
    }
    SkASSERT(next > cluster);
    return SkToUInt(next - cluster);
}

namespace {
}
struct TextRunHandler final : public SkShaper::RunHandler {
    const char* fText = nullptr;
    const char* fStop = nullptr;
    SkTextBlobBuilderRunHandler fTextBlobBuilderRunHandler;
    SkShaper::RunHandler::Buffer fCurrentBuffer;
    SkFont fCurrentFont;
    FILE* fOutput = nullptr;
    SkRect fCanvasBounds;
    int fCurrentGlyphCount = 0;

    TextRunHandler(const char* text, FILE* o, SkISize s)
        : fText(text), fTextBlobBuilderRunHandler(text), fOutput(o)
        , fCanvasBounds{0, 0, (float)s.width(), (float)s.height()} {}

    SkShaper::RunHandler::Buffer newRunBuffer(const RunInfo& runInfo,
                                              const SkFont& font,
                                              int glyphCount,
                                              SkSpan<const char> utf8) override {
        fCurrentBuffer = fTextBlobBuilderRunHandler.newRunBuffer(runInfo,
                                                                 font,
                                                                 glyphCount,
                                                                 utf8);
        SkASSERT(utf8.size() > 0);
        fStop = utf8.end();
        fCurrentFont = font;
        fCurrentGlyphCount = glyphCount;
        return fCurrentBuffer;
    }
    void commitRun() override {
        if (fOutput) {
            std::unique_ptr<SkRect[]> bounds(new SkRect[fCurrentGlyphCount]);
            fCurrentFont.getBounds(fCurrentBuffer.glyphs, fCurrentGlyphCount,
                                    bounds.get(), nullptr);
            for (int i = 0; i < fCurrentGlyphCount; ++i) {
                SkRect b = bounds[i];
                SkPoint pos = fCurrentBuffer.positions[i];
                b.offset(pos);
                if (b.isEmpty()) {
                    if (!fCanvasBounds.contains(pos.x(), pos.y())) {
                        continue;
                    }
                } else {
                    if (!fCanvasBounds.intersects(b)) {
                        continue;
                    }
                } 
                unsigned len = cluster_text_len(fCurrentBuffer.clusters, i,
                                                fCurrentGlyphCount, SkToU32(fStop - fText));
                fprintf(fOutput, "%g,%g,%g,%g,\"%.*s\"\n",
                        b.left(), b.top(), b.right(), b.bottom(),
                        (int)len, fText + fCurrentBuffer.clusters[i]);
            }

        }
        fTextBlobBuilderRunHandler.commitRun();
    }
    void commitLine() override {
        fTextBlobBuilderRunHandler.commitLine();
    }
};
//}

// Kind of like Python's readlines(), but without any allocation.
// Calls f() on each line, includeing trailing newline.
// F is [](const char*, size_t) -> void
template <typename F>
static void readlines(const void* data, size_t size, F f) {
    const char* start = (const char*)data;
    const char* end = start + size;
    for (const char* ptr = start; ptr < end;) {
        while (*ptr++ != '\n' && ptr < end) {}
        f(start, (size_t)(ptr - start));
        start = ptr;
    }
}


static void encode_to_file(const SkImage* img, const char* path) {
    if (path) {
        SkASSERT(img);
        FILE* o = fopen(path, "wb");
        SkASSERT(o);
        if (sk_sp<SkData> d = img->encodeToData()) {
            fwrite(d->data(), d->size(), 1, o);
        }
        fclose(o);
    }
}
static void make_pdf(SkPicture* picture, SkWStream* dst, SkISize size) {
    if (auto doc = SkPDF::MakeDocument(dst)) {
        picture->playback(doc->beginPage((float)size.width(), (float)size.height()));
    }
}

DEF_TEST(ShaperTextExample, reporter) {
    #if 1//def SK_TEST_SHAPER_TEXT_LOCATION_OUTPUT
    // If you compile Skia with `extra_cflags=["-DSK_TEST_SHAPER_TEXT_LOCATION_OUTPUT"]` in
    // your `args.gn` file, then the locations of each glyph, along with the
    // text string that produced the glyph will show up in the text file.
    // The image will go to the png file.
    constexpr char kTextOutput[]  = "/tmp/ShaperTextExample.txt";
    constexpr char kImageOutput[] = "/tmp/ShaperTextExample.png";
    constexpr char kPDFOutput[]   = "/tmp/ShaperTextExample.pdf";
    #else
    const char* const kTextOutput  = nullptr; 
    const char* const kImageOutput = nullptr;
    const char* const kPDFOutput   = nullptr;
    #endif
    constexpr char kTypefaceName[] = "Noto Sans";
    constexpr char kInputTextResource[] = "text/emoji.txt";
    constexpr SkISize kSize = {612, 792};
    constexpr SkPoint kOrigin = {4, 4};
    constexpr float kWidth = kSize.fWidth - 8;
    constexpr float kTextSize = 18;

    sk_sp<SkData> data = GetResourceAsData(kInputTextResource);
    if (!data) {
        return;
    }
    sk_sp<SkFontMgr> fontManager = SkFontMgr::RefDefault();
    if (!fontManager) {
        SkDebugf("no default font mugger?");
        return;
    }
    SkFont font(fontManager->legacyMakeTypeface(kTypefaceName, SkFontStyle()), kTextSize);
    SkPaint paint;
    SkPoint xy = kOrigin;
    FILE* output = kTextOutput ? fopen(kTextOutput, "w") : nullptr;
    if (output) {
        fputs("\"Left\",\"Top\",\"Right\",\"Bottom\",\"Text\"\n", output);
    }
    SkPictureRecorder pictureRecorder;
    SkCanvas* canvas = pictureRecorder.beginRecording(kSize.width(), kSize.height());

    readlines(data->data(), data->size(), [&](const char* text, size_t len) {
        while (len > 0 && text[len-1] == '\n') { --len; }
        SkShaper shaper(nullptr);
        TextRunHandler handler(text, output, kSize);
        SkPoint p = shaper.shape(&handler, font, text, len, true, xy, kWidth);
        canvas->drawTextBlob(handler.fTextBlobBuilderRunHandler.makeBlob(), 0, 0, SkPaint());
        xy.fY = p.y();
    });

    if (output) {
        fclose(output);
    }
    sk_sp<SkPicture> picture = pictureRecorder.finishRecordingAsPicture();
    auto s = SkSurface::MakeRasterN32Premul(kSize.width(), kSize.height());
    s->getCanvas()->clear(0xFFFFFFFF);
    picture->playback(s->getCanvas());
    encode_to_file(s->makeImageSnapshot().get(), kImageOutput);

    if (kPDFOutput) {
        SkFILEWStream pdf(kPDFOutput);
        make_pdf(picture.get(), &pdf, kSize);
    } else {
        SkNullWStream nstr;
        make_pdf(picture.get(), &nstr, kSize);
    }
}

#endif
