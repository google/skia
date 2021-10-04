/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPaint.h"
#include "src/core/SkRemoteGlyphCache.h"
#include "src/core/SkScalerCache.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkTraceEvent.h"
#include "src/core/SkTypeface_remote.h"

SkScalerContextProxy::SkScalerContextProxy(sk_sp<SkTypeface> tf,
                                           const SkScalerContextEffects& effects,
                                           const SkDescriptor* desc,
                                           sk_sp<SkStrikeClient::DiscardableHandleManager> manager)
        : SkScalerContext{std::move(tf), effects, desc}
        , fDiscardableManager{std::move(manager)} {}

bool SkScalerContextProxy::generateAdvance(SkGlyph* glyph) {
    return false;
}

void SkScalerContextProxy::generateMetrics(SkGlyph* glyph) {
    TRACE_EVENT1("skia", "generateMetrics", "rec", TRACE_STR_COPY(this->getRec().dump().c_str()));
    if (this->getProxyTypeface()->isLogging()) {
        SkDebugf("GlyphCacheMiss generateMetrics: %s\n", this->getRec().dump().c_str());
    }

    glyph->fMaskFormat = fRec.fMaskFormat;
    glyph->zeroMetrics();
    fDiscardableManager->notifyCacheMiss(
            SkStrikeClient::CacheMissType::kGlyphMetrics, fRec.fTextSize);
}

void SkScalerContextProxy::generateImage(const SkGlyph& glyph) {
    TRACE_EVENT1("skia", "generateImage", "rec", TRACE_STR_COPY(this->getRec().dump().c_str()));
    if (this->getProxyTypeface()->isLogging()) {
        SkDebugf("GlyphCacheMiss generateImage: %s\n", this->getRec().dump().c_str());
    }

    // There is no desperation search here, because if there was an image to be found it was
    // copied over with the metrics search.
    fDiscardableManager->notifyCacheMiss(
            SkStrikeClient::CacheMissType::kGlyphImage, fRec.fTextSize);
}

bool SkScalerContextProxy::generatePath(SkGlyphID glyphID, SkPath* path) {
    TRACE_EVENT1("skia", "generatePath", "rec", TRACE_STR_COPY(this->getRec().dump().c_str()));
    if (this->getProxyTypeface()->isLogging()) {
        SkDebugf("GlyphCacheMiss generatePath: %s\n", this->getRec().dump().c_str());
    }

    fDiscardableManager->notifyCacheMiss(
            SkStrikeClient::CacheMissType::kGlyphPath, fRec.fTextSize);
    return false;
}

void SkScalerContextProxy::generateFontMetrics(SkFontMetrics* metrics) {
    TRACE_EVENT1(
            "skia", "generateFontMetrics", "rec", TRACE_STR_COPY(this->getRec().dump().c_str()));
    if (this->getProxyTypeface()->isLogging()) {
        SkDebugf("GlyphCacheMiss generateFontMetrics: %s\n", this->getRec().dump().c_str());
    }

    // Font metrics aren't really used for render, so just zero out the data and return.
    fDiscardableManager->notifyCacheMiss(
            SkStrikeClient::CacheMissType::kFontMetrics, fRec.fTextSize);
    sk_bzero(metrics, sizeof(*metrics));
}

SkTypefaceProxy* SkScalerContextProxy::getProxyTypeface() const {
    return (SkTypefaceProxy*)this->getTypeface();
}
