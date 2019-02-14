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
struct TextRunHandler final : public SkShaper::RunHandler {
    SkTextBlobBuilderRunHandler fBlobBuilderHandler;
    const char* fText = nullptr;
    const char* fStop = nullptr;
    SkShaper::RunHandler::Buffer fCurrentBuffer = {nullptr, nullptr, nullptr};
    int fCurrentGlyphCount = 0;
    SkFont fCurrentFont;
    FILE* fOutput = nullptr;
    SkRect fCanvasBounds;

    TextRunHandler(const char* text, FILE* o, SkISize s)
        : fBlobBuilderHandler(text)
        , fText(text)
        , fOutput(o)
        , fCanvasBounds{0, 0, (float)s.width(), (float)s.height()} {}

    SkShaper::RunHandler::Buffer newRunBuffer(const RunInfo& runInfo,
                                              const SkFont& font,
                                              int glyphCount,
                                              SkSpan<const char> utf8) override {
        fStop = utf8.end();
        fCurrentFont = font;
        fCurrentGlyphCount = glyphCount;
        fCurrentBuffer = fBlobBuilderHandler.newRunBuffer(runInfo, font, glyphCount, utf8);
        return fCurrentBuffer;
    }
    void commitRun() override {
        if (fOutput) {
            std::unique_ptr<SkRect[]> bounds(new SkRect[fCurrentGlyphCount]);
            fCurrentFont.getBounds(fCurrentBuffer.glyphs, fCurrentGlyphCount,
                                    bounds.get(), nullptr);
            for (int i = 0; i < fCurrentGlyphCount; ++i) {
                SkPoint pos = fCurrentBuffer.positions[i];
                SkRect b = bounds[i];
                b.offset(pos);
                if (b.isEmpty() ? !fCanvasBounds.contains(pos.x(), pos.y())
                                : !fCanvasBounds.intersects(b)) {
                    continue;
                }
                unsigned len = cluster_text_len(fCurrentBuffer.clusters, i,
                                                fCurrentGlyphCount, SkToU32(fStop - fText));
                fprintf(fOutput, "%g,%g,%g,%g,\"%.*s\"\n",
                        b.left(), b.top(), b.right(), b.bottom(),
                        (int)len, fText + fCurrentBuffer.clusters[i]);
            }

        }
        fBlobBuilderHandler.commitRun();
    }
    void commitLine() override { fBlobBuilderHandler.commitLine(); }
};
}

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
        SkFILEWStream dst(path);
        if (dst.isValid()) {
            if (sk_sp<SkData> d = img->encodeToData()) {
                dst.write(d->data(), d->size());
            }
        }
    }
}
static void make_pdf(SkPicture* picture, SkWStream* dst, SkISize size) {
    if (auto doc = SkPDF::MakeDocument(dst)) {
        picture->playback(doc->beginPage((float)size.width(), (float)size.height()));
    }
}

static void shape_and_keep_glyph_bounds(const char* textOutput,
                                        const char* imageOutput,
                                        const char* pdfOutput,
                                        const char* typefaceName,
                                        float textSize,
                                        const char* inputTextResource,
                                        SkISize pageSize,
                                        float margin) {
    const float textWidth = pageSize.fWidth - 2 * margin;
    sk_sp<SkData> data = GetResourceAsData(inputTextResource);
    if (!data) {
        return;
    }
    sk_sp<SkFontMgr> fontManager = SkFontMgr::RefDefault();
    if (!fontManager) {
        SkDebugf("no default font mugger?");
        return;
    }
    SkFont font(fontManager->legacyMakeTypeface(typefaceName, SkFontStyle()), textSize);
    SkPaint paint;
    SkPoint xy = {margin, margin};
    FILE* output = textOutput ? fopen(textOutput, "w") : nullptr;
    if (output) {
        fputs("\"Left\",\"Top\",\"Right\",\"Bottom\",\"Text\"\n", output);
    }
    SkPictureRecorder pictureRecorder;
    SkCanvas* canvas = pictureRecorder.beginRecording(pageSize.width(), pageSize.height());

    readlines(data->data(), data->size(), [&](const char* text, size_t len) {
        while (len > 0 && text[len-1] == '\n') { --len; }
        SkShaper shaper;
        TextRunHandler handler(text, output, pageSize);
        SkPoint p = shaper.shape(&handler, font, text, len, true, xy, textWidth);
        canvas->drawTextBlob(handler.fBlobBuilderHandler.makeBlob(), 0, 0, SkPaint());
        xy.fY = p.y();
    });

    if (output) {
        fclose(output);
    }
    sk_sp<SkPicture> picture = pictureRecorder.finishRecordingAsPicture();
    auto s = SkSurface::MakeRasterN32Premul(pageSize.width(), pageSize.height());
    s->getCanvas()->clear(SK_ColorWHITE);
    picture->playback(s->getCanvas());
    encode_to_file(s->makeImageSnapshot().get(), imageOutput);

    if (pdfOutput) {
        SkFILEWStream pdf(pdfOutput);
        make_pdf(picture.get(), &pdf, pageSize);
    } else {
        SkNullWStream nstr;
        make_pdf(picture.get(), &nstr, pageSize);
    }
}

DEF_TEST(ShaperTextExample, reporter) {
    constexpr char kTypefaceName[] = "Noto Sans";
    constexpr float kTextSize = 18;
    constexpr char kInputTextResource[] = "text/emoji.txt";
    constexpr SkISize kSize = {612, 792};
    constexpr float kMargin = 4;
    #if defined(SK_TEST_SHAPER_TEXT_LOCATION_OUTPUT)
    // If you compile Skia with `extra_cflags=["-DSK_TEST_SHAPER_TEXT_LOCATION_OUTPUT"]` in
    // your `args.gn` file, then the locations of each glyph, along with the
    // text string that produced the glyph will show up in the text file.
    // The image will go to the png file.  You also get a PDF for printing.
    shape_and_keep_glyph_bounds("/tmp/ShaperTextExample.txt",
                                "/tmp/ShaperTextExample.png",
                                "/tmp/ShaperTextExample.pdf",
                                kTypefaceName,
                                kTextSize,
                                kInputTextResource,
                                kSize,
                                kMargin);
    #else
    shape_and_keep_glyph_bounds(nullptr,
                                nullptr,
                                nullptr,
                                kTypefaceName,
                                kTextSize,
                                kInputTextResource,
                                kSize,
                                kMargin);
    #endif
}
#endif  // SK_USING_SKSHAPER
