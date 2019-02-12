// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "gm.h"

#include "Resources.h"
#include "SkCanvas.h"
#include "SkFontMetrics.h"
#include "SkShaper.h"
#include "SkStream.h"

// Shaper will fail if a typeface has no stream to inspect.
static bool has_stream(const SkTypeface* typeface) {
    int index;
    std::unique_ptr<SkStreamAsset> asset(typeface->openStream(&index));
    return asset != nullptr;
}

static sk_sp<SkTextBlob> shape(const char* text, size_t byteLength,
                               const SkFont& font, SkScalar width) {
    sk_sp<SkTypeface> typeface = font.refTypefaceOrDefault();
    if (!typeface || !has_stream(typeface.get())) {
       return nullptr;
    }
    SkShaper shaper(std::move(typeface));
    if (!shaper.good()) {
        return nullptr;
    }
    SkFontMetrics metrics;
    font.getMetrics(&metrics);
    SkTextBlobBuilderRunHandler handler(text);
    shaper.shape(&handler, font, text, byteLength, true, {0, metrics.fAscent}, width);
    return handler.makeBlob();
}

static SkScalar draw_shaped(SkCanvas* canvas, const char* text, size_t byteLength,
                            SkScalar x, SkScalar y, const SkFont& font,
                            const SkPaint& paint, SkScalar width) {
    if (!canvas || !byteLength || !text) { return 0; }
    auto blob = shape(text, byteLength, font, width);
    if (!blob) {
        blob = SkTextBlob::MakeFromText(text, byteLength, font, kUTF8_SkTextEncoding);
    }
    canvas->drawTextBlob(blob.get(), x, y, paint);
    return blob->bounds().bottom();
}


// Kind of like Python's readlines(), but without any allocation.
// Calls f() on each line.
// F is [](const char*, size_t) -> void
template <typename F>
static void readlines(const void* data, size_t size, F f) {
    const char* start = (const char*)data;
    const char* end = start + size;
    const char* ptr = start;
    while (ptr < end) {
        while (*ptr++ != '\n' && ptr < end) {}
        size_t len = ptr - start;
        f(start, len);
        start = ptr;
    }
}

static void draw(SkCanvas* canvas, const char* name) {
    sk_sp<SkData> data = GetResourceAsData(SkStringPrintf("text/%s.txt", name).c_str());
    if (!data) { return; }
    SkFont font(nullptr, 18);
    SkPaint paint;
    float x = 4;
    float y = 4 + font.getSpacing();
    y += draw_shaped(canvas, name, strlen(name), x, y, font, paint, 504) + font.getSpacing();
    readlines(data->data(), data->size(), [&](const char* text, size_t len) {
        while (len > 0 && text[len-1] == '\n') { --len; }
        y += draw_shaped(canvas, text, len, x, y, font, paint, 504) + font.getSpacing();
    });
}

#define M(X) DEF_SIMPLE_GM(shaper_ ## X, c, 512, 512) { draw(c, #X); }
M(arabic)
M(armenian)
M(balinese)
M(bengali)
M(buginese)
M(cherokee)
M(cyrillic)
M(devanagari)
M(english)
M(ethiopic)
M(greek)
M(han)
M(hangul)
M(hebrew)
M(javanese)
M(kana)
M(khmer)
M(lao)
M(mandaic)
M(myanmar)
M(newtailue)
M(nko)
M(sinhala)
M(sundanese)
M(syriac)
M(taitham)
M(tamil)
M(thaana)
M(thai)
M(tibetan)
M(tifnagh)
M(vai)
M(emoji)
#undef M
