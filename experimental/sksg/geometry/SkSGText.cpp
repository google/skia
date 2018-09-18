/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGText.h"

#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkTArray.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"

namespace sksg {

sk_sp<Text> Text::Make(sk_sp<SkTypeface> tf, const SkString& text) {
    return sk_sp<Text>(new Text(std::move(tf), text));
}

Text::Text(sk_sp<SkTypeface> tf, const SkString& text)
    : fTypeface(std::move(tf))
    , fText(text) {}

Text::~Text() = default;

SkRect Text::onRevalidate(InvalidationController*, const SkMatrix&) {
    // TODO: we could potentially track invals which don't require rebuilding the blob.

    SkPaint font;
    font.setFlags(fFlags);
    font.setTypeface(fTypeface);
    font.setTextSize(fSize);
    font.setTextScaleX(fScaleX);
    font.setTextSkewX(fSkewX);
    font.setTextAlign(fAlign);
    font.setHinting(fHinting);

    // First, convert to glyphIDs.
    font.setTextEncoding(SkPaint::kUTF8_TextEncoding);
    SkSTArray<256, SkGlyphID, true> glyphs;
    glyphs.reset(font.textToGlyphs(fText.c_str(), fText.size(), nullptr));
    SkAssertResult(font.textToGlyphs(fText.c_str(), fText.size(), glyphs.begin()) == glyphs.count());
    font.setTextEncoding(SkPaint::kGlyphID_TextEncoding);

    // Next, build the cached blob.
    SkTextBlobBuilder builder;
    const auto& buf = builder.allocRun(font, glyphs.count(), 0, 0, nullptr);
    if (!buf.glyphs) {
        fBlob.reset();
        return SkRect::MakeEmpty();
    }

    memcpy(buf.glyphs, glyphs.begin(), glyphs.count() * sizeof(SkGlyphID));

    fBlob = builder.make();
    return fBlob
        ? fBlob->bounds().makeOffset(fPosition.x(), fPosition.y())
        : SkRect::MakeEmpty();
}

void Text::onDraw(SkCanvas* canvas, const SkPaint& paint) const {
    canvas->drawTextBlob(fBlob, fPosition.x(), fPosition.y(), paint);
}

SkPath Text::onAsPath() const {
    // TODO
    return SkPath();
}

void Text::onClip(SkCanvas* canvas, bool antiAlias) const {
    canvas->clipPath(this->asPath(), antiAlias);
}

} // namespace sksg
