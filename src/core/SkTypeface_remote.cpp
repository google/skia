/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypeface_remote.h"

#include "SkPaint.h"
#include "SkRemoteGlyphCache.h"

SkScalerContextProxy::SkScalerContextProxy(
        sk_sp<SkTypeface> tf,
        const SkScalerContextEffects& effects,
        const SkDescriptor* desc,
        SkStrikeClient* rsc)
    : SkScalerContext{std::move(tf), effects, desc}
    , fClient{rsc} {}

unsigned SkScalerContextProxy::generateGlyphCount()  {
    SK_ABORT("Should never be called.");
    return 0;
}

uint16_t SkScalerContextProxy::generateCharToGlyph(SkUnichar) {
    SK_ABORT("Should never be called.");
    return 0;
}

void  SkScalerContextProxy::generateAdvance(SkGlyph* glyph) {
    this->generateMetrics(glyph);
}

void SkScalerContextProxy::generateMetrics(SkGlyph* glyph) {
    fClient->generateMetricsAndImage(*this->typefaceProxy(), this->getRec(), &fAlloc, glyph);
}

void SkScalerContextProxy::generateImage(const SkGlyph& glyph) {
}

bool SkScalerContextProxy::generatePath(SkGlyphID glyphID, SkPath* path) {
    return fClient->generatePath(*this->typefaceProxy(), this->getRec(), glyphID, path);
}

void SkScalerContextProxy::generateFontMetrics(SkPaint::FontMetrics* metrics) {
    fClient->generateFontMetrics(*this->typefaceProxy(), this->getRec(), metrics);
}

SkTypefaceProxy* SkScalerContextProxy::typefaceProxy() {
    return SkTypefaceProxy::DownCast(this->getTypeface());
}
