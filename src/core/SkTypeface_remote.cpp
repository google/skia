/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPaint.h"
#include "src/core/SkRemoteGlyphCache.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkTraceEvent.h"
#include "src/core/SkTypeface_remote.h"

SkScalerContextProxy::SkScalerContextProxy(sk_sp<SkTypeface> tf,
                                           const SkScalerContextEffects& effects,
                                           const SkDescriptor* desc,
                                           sk_sp<SkStrikeClient::DiscardableHandleManager> manager)
        : SkScalerContext{std::move(tf), effects, desc}
        , fDiscardableManager{std::move(manager)} {}

void SkScalerContextProxy::initCache(SkStrike* cache, SkStrikeCache* strikeCache) {
    SkASSERT(fCache == nullptr);
    SkASSERT(cache != nullptr);

    fCache = cache;
    fStrikeCache = strikeCache;
}

unsigned SkScalerContextProxy::generateGlyphCount()  {
    SK_ABORT("Should never be called.");
    return 0;
}

bool SkScalerContextProxy::generateAdvance(SkGlyph* glyph) {
    return false;
}

void SkScalerContextProxy::generateMetrics(SkGlyph* glyph) {
    TRACE_EVENT1("skia", "generateMetrics", "rec", TRACE_STR_COPY(this->getRec().dump().c_str()));
    if (this->getProxyTypeface()->isLogging()) {
        SkDebugf("GlyphCacheMiss generateMetrics: %s\n", this->getRec().dump().c_str());
    }

    glyph->fMaskFormat = fRec.fMaskFormat;

    // Since the scaler context is being called, we don't have the needed data. Try to find a
    // fallback before failing.
    if (fCache && fCache->belongsToCache(glyph)) {
        // First check the original cache, in case there is a sub-pixel pos mismatch.
        if (const SkGlyph* from =
                    fCache->getCachedGlyphAnySubPix(glyph->getGlyphID(), glyph->getPackedID())) {
            fCache->mergeGlyphAndImage(glyph->getPackedID(), *from);
            fDiscardableManager->notifyCacheMiss(
                    SkStrikeClient::CacheMissType::kGlyphMetricsFallback);
            return;
        }

        // Now check other caches for a desc mismatch.
        if (fStrikeCache->desperationSearchForImage(fCache->getDescriptor(), glyph, fCache)) {
            fDiscardableManager->notifyCacheMiss(
                    SkStrikeClient::CacheMissType::kGlyphMetricsFallback);
            return;
        }
    }

    glyph->zeroMetrics();
    fDiscardableManager->notifyCacheMiss(SkStrikeClient::CacheMissType::kGlyphMetrics);
}

void SkScalerContextProxy::generateImage(const SkGlyph& glyph) {
    TRACE_EVENT1("skia", "generateImage", "rec", TRACE_STR_COPY(this->getRec().dump().c_str()));
    if (this->getProxyTypeface()->isLogging()) {
        SkDebugf("GlyphCacheMiss generateImage: %s\n", this->getRec().dump().c_str());
    }

    // There is no desperation search here, because if there was an image to be found it was
    // copied over with the metrics search.
    fDiscardableManager->notifyCacheMiss(SkStrikeClient::CacheMissType::kGlyphImage);
}

bool SkScalerContextProxy::generatePath(SkGlyphID glyphID, SkPath* path) {
    TRACE_EVENT1("skia", "generatePath", "rec", TRACE_STR_COPY(this->getRec().dump().c_str()));
    if (this->getProxyTypeface()->isLogging()) {
        SkDebugf("GlyphCacheMiss generatePath: %s\n", this->getRec().dump().c_str());
    }

    // Since the scaler context is being called, we don't have the needed data. Try to find a
    // fallback before failing.
    auto desc = SkScalerContext::DescriptorGivenRecAndEffects(this->getRec(), this->getEffects());
    bool foundPath = fStrikeCache && fStrikeCache->desperationSearchForPath(*desc, glyphID, path);
    fDiscardableManager->notifyCacheMiss(foundPath
                                                 ? SkStrikeClient::CacheMissType::kGlyphPathFallback
                                                 : SkStrikeClient::CacheMissType::kGlyphPath);
    return foundPath;
}

void SkScalerContextProxy::generateFontMetrics(SkFontMetrics* metrics) {
    TRACE_EVENT1(
            "skia", "generateFontMetrics", "rec", TRACE_STR_COPY(this->getRec().dump().c_str()));
    if (this->getProxyTypeface()->isLogging()) {
        SkDebugf("GlyphCacheMiss generateFontMetrics: %s\n", this->getRec().dump().c_str());
        SkDEBUGCODE(SkStrikeCache::Dump());
    }

    // Font metrics aren't really used for render, so just zero out the data and return.
    fDiscardableManager->notifyCacheMiss(SkStrikeClient::CacheMissType::kFontMetrics);
    sk_bzero(metrics, sizeof(*metrics));
}

SkTypefaceProxy* SkScalerContextProxy::getProxyTypeface() const {
    return (SkTypefaceProxy*)this->getTypeface();
}
