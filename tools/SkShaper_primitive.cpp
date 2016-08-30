/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkShaper.h"
#include "SkStream.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"

struct SkShaper::Impl {
    sk_sp<SkTypeface> fTypeface;
};

SkShaper::SkShaper(sk_sp<SkTypeface> tf) : fImpl(new Impl) {
    fImpl->fTypeface = tf ? std::move(tf) : SkTypeface::MakeDefault();
}

SkShaper::~SkShaper() {}

bool SkShaper::good() const { return true; }

SkScalar SkShaper::shape(SkTextBlobBuilder* builder,
                         const SkPaint& srcPaint,
                         const char* utf8text,
                         size_t textBytes,
                         SkPoint point) const {
    SkPaint paint(srcPaint);
    paint.setTypeface(fImpl->fTypeface);
    paint.setTextEncoding(SkPaint::kUTF8_TextEncoding);
    int glyphCount = paint.countText(utf8text, textBytes);
    if (glyphCount <= 0) {
        return 0;
    }
    SkRect bounds;
    (void)paint.measureText(utf8text, textBytes, &bounds);
    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    const SkTextBlobBuilder::RunBuffer& runBuffer = builder->allocRunPosH(
            paint, glyphCount, point.y(), &bounds);
    paint.setTextEncoding(SkPaint::kUTF8_TextEncoding);
    (void)paint.textToGlyphs(utf8text, textBytes, runBuffer.glyphs);
    (void)paint.getTextWidths(utf8text, textBytes, runBuffer.pos);
    SkScalar x = point.x();
    for (int i = 0; i < glyphCount; ++i) {
        SkScalar w = runBuffer.pos[i];
        runBuffer.pos[i] = x;
        x += w;
    }
    return (SkScalar)x;
}
