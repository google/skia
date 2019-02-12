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

DEF_SIMPLE_GM(shaper_arabic,     canvas, 512, 512) { draw(canvas, "arabic"    ); }
DEF_SIMPLE_GM(shaper_armenian,   canvas, 512, 512) { draw(canvas, "armenian"  ); }
DEF_SIMPLE_GM(shaper_balinese,   canvas, 512, 512) { draw(canvas, "balinese"  ); }
DEF_SIMPLE_GM(shaper_bengali,    canvas, 512, 512) { draw(canvas, "bengali"   ); }
DEF_SIMPLE_GM(shaper_buginese,   canvas, 512, 512) { draw(canvas, "buginese"  ); }
DEF_SIMPLE_GM(shaper_cherokee,   canvas, 512, 512) { draw(canvas, "cherokee"  ); }
DEF_SIMPLE_GM(shaper_cyrillic,   canvas, 512, 512) { draw(canvas, "cyrillic"  ); }
DEF_SIMPLE_GM(shaper_devanagari, canvas, 512, 512) { draw(canvas, "devanagari"); }
DEF_SIMPLE_GM(shaper_english,    canvas, 512, 512) { draw(canvas, "english"   ); }
DEF_SIMPLE_GM(shaper_ethiopic,   canvas, 512, 512) { draw(canvas, "ethiopic"  ); }
DEF_SIMPLE_GM(shaper_greek,      canvas, 512, 512) { draw(canvas, "greek"     ); }
DEF_SIMPLE_GM(shaper_han,        canvas, 512, 512) { draw(canvas, "han"       ); }
DEF_SIMPLE_GM(shaper_hangul,     canvas, 512, 512) { draw(canvas, "hangul"    ); }
DEF_SIMPLE_GM(shaper_hebrew,     canvas, 512, 512) { draw(canvas, "hebrew"    ); }
DEF_SIMPLE_GM(shaper_javanese,   canvas, 512, 512) { draw(canvas, "javanese"  ); }
DEF_SIMPLE_GM(shaper_kana,       canvas, 512, 512) { draw(canvas, "kana"      ); }
DEF_SIMPLE_GM(shaper_khmer,      canvas, 512, 512) { draw(canvas, "khmer"     ); }
DEF_SIMPLE_GM(shaper_lao,        canvas, 512, 512) { draw(canvas, "lao"       ); }
DEF_SIMPLE_GM(shaper_mandaic,    canvas, 512, 512) { draw(canvas, "mandaic"   ); }
DEF_SIMPLE_GM(shaper_myanmar,    canvas, 512, 512) { draw(canvas, "myanmar"   ); }
DEF_SIMPLE_GM(shaper_newtailue,  canvas, 512, 512) { draw(canvas, "newtailue" ); }
DEF_SIMPLE_GM(shaper_nko,        canvas, 512, 512) { draw(canvas, "nko"       ); }
DEF_SIMPLE_GM(shaper_sinhala,    canvas, 512, 512) { draw(canvas, "sinhala"   ); }
DEF_SIMPLE_GM(shaper_sundanese,  canvas, 512, 512) { draw(canvas, "sundanese" ); }
DEF_SIMPLE_GM(shaper_syriac,     canvas, 512, 512) { draw(canvas, "syriac"    ); }
DEF_SIMPLE_GM(shaper_taitham,    canvas, 512, 512) { draw(canvas, "taitham"   ); }
DEF_SIMPLE_GM(shaper_tamil,      canvas, 512, 512) { draw(canvas, "tamil"     ); }
DEF_SIMPLE_GM(shaper_thaana,     canvas, 512, 512) { draw(canvas, "thaana"    ); }
DEF_SIMPLE_GM(shaper_thai,       canvas, 512, 512) { draw(canvas, "thai"      ); }
DEF_SIMPLE_GM(shaper_tibetan,    canvas, 512, 512) { draw(canvas, "tibetan"   ); }
DEF_SIMPLE_GM(shaper_tifnagh,    canvas, 512, 512) { draw(canvas, "tifnagh"   ); }
DEF_SIMPLE_GM(shaper_vai,        canvas, 512, 512) { draw(canvas, "vai"       ); }

DEF_SIMPLE_GM(shaper_emoji,      canvas, 512, 512) { draw(canvas, "emoji"     ); }
