// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "sk_shaper_utils.h"

#include "SkCanvas.h"
#include "SkFontMetrics.h"
#include "SkShaper.h"
#include "SkStream.h"

static sk_sp<SkTextBlob> shape(const char* text, size_t byteLength, const SkFont& font) {
    sk_sp<SkTypeface> typeface = font.refTypefaceOrDefault();
    if (!typeface) {
       return nullptr;
    }
    int index;
    std::unique_ptr<SkStreamAsset> asset(typeface->openStream(&index));
    if (!asset) {
        return nullptr;
    }
    asset = nullptr;
    SkShaper shaper(std::move(typeface));
    if (!shaper.good()) {
        return nullptr;
    }
    SkFontMetrics metrics;
    font.getMetrics(&metrics);
    SkTextBlobBuilderRunHandler handler;
    shaper.shape(&handler, font, text, byteLength, true, {0, metrics.fAscent}, FLT_MAX);
    return handler.makeBlob();
}

void SkDrawShapedUTF8(SkCanvas* canvas, const char* text, size_t byteLength,
                      SkScalar x, SkScalar y, const SkFont& font,
                      const SkPaint& paint) {
    if (!canvas || !byteLength || !text) { return; }
    auto blob = shape(text, byteLength, font);
    if (!blob) {
        blob = SkTextBlob::MakeFromText(text, byteLength, font, kUTF8_SkTextEncoding);
    }
    canvas->drawTextBlob(blob.get(), x, y, paint);
}
