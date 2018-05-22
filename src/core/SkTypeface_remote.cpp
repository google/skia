/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypeface_remote.h"
#include "SkPaint.h"
#include "SkRemoteGlyphCache.h"
#include "SkStrikeCache.h"
#include "SkTraceEvent.h"

SkScalerContextProxy::SkScalerContextProxy(sk_sp<SkTypeface> tf,
                                           const SkScalerContextEffects& effects,
                                           const SkDescriptor* desc,
                                           sk_sp<SkStrikeClient::DiscardableHandleManager> manager)
        : SkScalerContext{std::move(tf), effects, desc}, fDiscardableManager{std::move(manager)} {}

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
    TRACE_EVENT1("skia", "generateMetrics", "rec", TRACE_STR_COPY(this->getRec().dump().c_str()));
    SkDebugf("GlyphCacheMiss generateMetrics: %s\n", this->getRec().dump().c_str());

    fDiscardableManager->NotifyCacheMiss(SkStrikeClient::CacheMissType::kGlyphMetrics);
    glyph->zeroMetrics();
}

void SkScalerContextProxy::generateImage(const SkGlyph& glyph) {
    TRACE_EVENT1("skia", "generateImage", "rec", TRACE_STR_COPY(this->getRec().dump().c_str()));
    SkDebugf("GlyphCacheMiss generateImage: %s\n", this->getRec().dump().c_str());

    fDiscardableManager->NotifyCacheMiss(SkStrikeClient::CacheMissType::kGlyphImage);
}

bool SkScalerContextProxy::generatePath(SkGlyphID glyphID, SkPath* path) {
    TRACE_EVENT1("skia", "generatePath", "rec", TRACE_STR_COPY(this->getRec().dump().c_str()));
    SkDebugf("GlyphCacheMiss generatePath: %s\n", this->getRec().dump().c_str());

    fDiscardableManager->NotifyCacheMiss(SkStrikeClient::CacheMissType::kGlyphPath);
    return false;
}

void SkScalerContextProxy::generateFontMetrics(SkPaint::FontMetrics* metrics) {
    TRACE_EVENT1(
            "skia", "generateFontMetrics", "rec", TRACE_STR_COPY(this->getRec().dump().c_str()));
    SkDebugf("GlyphCacheMiss generateFontMetrics: %s\n", this->getRec().dump().c_str());
    SkDEBUGCODE(SkStrikeCache::Dump());

    fDiscardableManager->NotifyCacheMiss(SkStrikeClient::CacheMissType::kFontMetrics);
    sk_bzero(metrics, sizeof(*metrics));
}
