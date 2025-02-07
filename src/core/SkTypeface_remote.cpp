/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkTypeface_remote.h"

#include "include/core/SkDrawable.h"
#include "include/core/SkFontMetrics.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/chromium/SkChromeRemoteGlyphCache.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkTraceEvent.h"
#include "src/core/SkWriteBuffer.h"

#include <optional>
#include <utility>

class SkArenaAlloc;
class SkDescriptor;
class SkPath;

SkScalerContextProxy::SkScalerContextProxy(SkTypeface& tf,
                                           const SkScalerContextEffects& effects,
                                           const SkDescriptor* desc,
                                           sk_sp<SkStrikeClient::DiscardableHandleManager> manager)
        : SkScalerContext{tf, effects, desc}
        , fDiscardableManager{std::move(manager)} {}

SkScalerContext::GlyphMetrics SkScalerContextProxy::generateMetrics(const SkGlyph& glyph,
                                                                    SkArenaAlloc*) {
    TRACE_EVENT1("skia", "generateMetrics", "rec", TRACE_STR_COPY(this->getRec().dump().c_str()));
    if (this->getProxyTypeface()->isLogging()) {
        SkDebugf("GlyphCacheMiss generateMetrics looking for glyph: %x\n  generateMetrics: %s\n",
                 glyph.getPackedID().value(), this->getRec().dump().c_str());
    }

    fDiscardableManager->notifyCacheMiss(
                                         SkStrikeClient::CacheMissType::kGlyphMetrics, fRec.fTextSize);

    return {glyph.maskFormat()};
}

void SkScalerContextProxy::generateImage(const SkGlyph& glyph, void*) {
    TRACE_EVENT1("skia", "generateImage", "rec", TRACE_STR_COPY(this->getRec().dump().c_str()));
    if (this->getProxyTypeface()->isLogging()) {
        SkDebugf("GlyphCacheMiss generateImage: %s\n", this->getRec().dump().c_str());
    }

    // There is no desperation search here, because if there was an image to be found it was
    // copied over with the metrics search.
    fDiscardableManager->notifyCacheMiss(
            SkStrikeClient::CacheMissType::kGlyphImage, fRec.fTextSize);
}

bool SkScalerContextProxy::generatePath(const SkGlyph& glyph, SkPath* path, bool* modified) {
    TRACE_EVENT1("skia", "generatePath", "rec", TRACE_STR_COPY(this->getRec().dump().c_str()));
    if (this->getProxyTypeface()->isLogging()) {
        SkDebugf("GlyphCacheMiss generatePath: %s\n", this->getRec().dump().c_str());
    }

    fDiscardableManager->notifyCacheMiss(
            SkStrikeClient::CacheMissType::kGlyphPath, fRec.fTextSize);
    return false;
}

sk_sp<SkDrawable> SkScalerContextProxy::generateDrawable(const SkGlyph&) {
    TRACE_EVENT1("skia", "generateDrawable", "rec", TRACE_STR_COPY(this->getRec().dump().c_str()));
    if (this->getProxyTypeface()->isLogging()) {
        SkDebugf("GlyphCacheMiss generateDrawable: %s\n", this->getRec().dump().c_str());
    }

    fDiscardableManager->notifyCacheMiss(
            SkStrikeClient::CacheMissType::kGlyphDrawable, fRec.fTextSize);
    return nullptr;
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

std::optional<SkTypefaceProxyPrototype>
SkTypefaceProxyPrototype::MakeFromBuffer(SkReadBuffer& buffer) {
    SkASSERT(buffer.isValid());
    const SkTypefaceID typefaceID = buffer.readUInt();
    const int glyphCount = buffer.readInt();
    const int32_t styleValue = buffer.read32();
    const bool isFixedPitch = buffer.readBool();
    const bool glyphMaskNeedsCurrentColor = buffer.readBool();

    if (buffer.isValid()) {
        return SkTypefaceProxyPrototype{
            typefaceID, glyphCount, styleValue, isFixedPitch, glyphMaskNeedsCurrentColor};
    }

    return std::nullopt;
}

SkTypefaceProxyPrototype::SkTypefaceProxyPrototype(const SkTypeface& typeface)
        : fServerTypefaceID{typeface.uniqueID()}
        , fGlyphCount{typeface.countGlyphs()}
        , fStyleValue{typeface.fontStyle().fValue}
        , fIsFixedPitch{typeface.isFixedPitch()}
        , fGlyphMaskNeedsCurrentColor{typeface.glyphMaskNeedsCurrentColor()} {}

SkTypefaceProxyPrototype::SkTypefaceProxyPrototype(SkTypefaceID typefaceID, int glyphCount,
                                                   int32_t styleValue, bool isFixedPitch,
                                                   bool glyphMaskNeedsCurrentColor)
        : fServerTypefaceID {typefaceID}
        , fGlyphCount{glyphCount}
        , fStyleValue{styleValue}
        , fIsFixedPitch{isFixedPitch}
        , fGlyphMaskNeedsCurrentColor{glyphMaskNeedsCurrentColor} {}

void SkTypefaceProxyPrototype::flatten(SkWriteBuffer& buffer) const {
    buffer.writeUInt(fServerTypefaceID);
    buffer.writeInt(fGlyphCount);
    buffer.write32(fStyleValue);
    buffer.writeBool(fIsFixedPitch);
    buffer.writeBool(fGlyphMaskNeedsCurrentColor);
}


SkTypefaceProxy::SkTypefaceProxy(const SkTypefaceProxyPrototype& prototype,
                                 sk_sp<SkStrikeClient::DiscardableHandleManager> manager,
                                 bool isLogging)
        : SkTypeface{prototype.style(), prototype.fIsFixedPitch}
        , fTypefaceID{prototype.fServerTypefaceID}
        , fGlyphCount{prototype.fGlyphCount}
        , fIsLogging{isLogging}
        , fGlyphMaskNeedsCurrentColor{prototype.fGlyphMaskNeedsCurrentColor}
        , fDiscardableManager{std::move(manager)} {}

SkTypefaceProxy::SkTypefaceProxy(SkTypefaceID typefaceID,
                                 int glyphCount,
                                 const SkFontStyle& style,
                                 bool isFixedPitch,
                                 bool glyphMaskNeedsCurrentColor,
                                 sk_sp<SkStrikeClient::DiscardableHandleManager> manager,
                                 bool isLogging)
        : SkTypeface{style, isFixedPitch}
        , fTypefaceID{typefaceID}
        , fGlyphCount{glyphCount}
        , fIsLogging{isLogging}
        , fGlyphMaskNeedsCurrentColor(glyphMaskNeedsCurrentColor)
        , fDiscardableManager{std::move(manager)} {}

SkTypefaceProxy* SkScalerContextProxy::getProxyTypeface() const {
    return (SkTypefaceProxy*)this->getTypeface();
}
