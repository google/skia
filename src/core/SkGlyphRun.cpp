/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkGlyphRun.h"

#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkTextBlob.h"
#include "include/private/SkTo.h"
#include "src/core/SkDevice.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkScalerCache.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTextBlobPriv.h"
#include "src/core/SkUtils.h"

// -- SkGlyphRun -----------------------------------------------------------------------------------
SkGlyphRun::SkGlyphRun(const SkFont& font,
                       SkSpan<const SkPoint> positions,
                       SkSpan<const SkGlyphID> glyphIDs,
                       SkSpan<const char> text,
                       SkSpan<const uint32_t> clusters)
        : fSource{SkMakeZip(glyphIDs, positions)}
        , fText{text}
        , fClusters{clusters}
        , fFont{font} {}

SkGlyphRun::SkGlyphRun(const SkGlyphRun& that, const SkFont& font)
    : fSource{that.fSource}
    , fText{that.fText}
    , fClusters{that.fClusters}
    , fFont{font} {}

// -- SkGlyphRunList -------------------------------------------------------------------------------
SkGlyphRunList::SkGlyphRunList() = default;
SkGlyphRunList::SkGlyphRunList(
        const SkTextBlob* blob,
        SkPoint origin,
        SkSpan<const SkGlyphRun> glyphRunList)
        : fGlyphRuns{glyphRunList}
        , fOriginalTextBlob{blob}
        , fOrigin{origin} { }

SkGlyphRunList::SkGlyphRunList(const SkGlyphRun& glyphRun)
        : fGlyphRuns{SkSpan<const SkGlyphRun>{&glyphRun, 1}}
        , fOriginalTextBlob{nullptr}
        , fOrigin{SkPoint::Make(0, 0)} {}

uint64_t SkGlyphRunList::uniqueID() const {
    return fOriginalTextBlob != nullptr ? fOriginalTextBlob->uniqueID()
                                        : SK_InvalidUniqueID;
}

bool SkGlyphRunList::anyRunsLCD() const {
    for (const auto& r : fGlyphRuns) {
        if (r.font().getEdging() == SkFont::Edging::kSubpixelAntiAlias) {
            return true;
        }
    }
    return false;
}

void SkGlyphRunList::temporaryShuntBlobNotifyAddedToCache(uint32_t cacheID) const {
    SkASSERT(fOriginalTextBlob != nullptr);
    fOriginalTextBlob->notifyAddedToCache(cacheID);
}

// -- SkGlyphRunBuilder ----------------------------------------------------------------------------
static SkSpan<const SkPoint> draw_text_positions(
        const SkFont& font, SkSpan<const SkGlyphID> glyphIDs, SkPoint origin, SkPoint* buffer) {
    SkStrikeSpec strikeSpec = SkStrikeSpec::MakeWithNoDevice(font);
    SkBulkGlyphMetrics storage{strikeSpec};
    auto glyphs = storage.glyphs(glyphIDs);

    SkPoint* positionCursor = buffer;
    SkPoint endOfLastGlyph = origin;
    for (auto glyph : glyphs) {
        *positionCursor++ = endOfLastGlyph;
        endOfLastGlyph += glyph->advanceVector();
    }
    return SkSpan(buffer, glyphIDs.size());
}

const SkGlyphRunList& SkGlyphRunBuilder::textToGlyphRunList(
        const SkFont& font, const void* bytes, size_t byteLength, SkPoint origin,
        SkTextEncoding encoding) {
    auto glyphIDs = textToGlyphIDs(font, bytes, byteLength, encoding);
    if (!glyphIDs.empty()) {
        this->initialize(glyphIDs.size());
        SkSpan<const SkPoint> positions = draw_text_positions(font, glyphIDs, {0, 0}, fPositions);
        this->makeGlyphRun(font,
                           glyphIDs,
                           positions,
                           SkSpan<const char>{},
                           SkSpan<const uint32_t>{});
    }

    this->makeGlyphRunList(nullptr, origin);
    return fGlyphRunList;
}

void SkGlyphRunBuilder::drawTextBlob(const SkPaint& paint, const SkTextBlob& blob, SkPoint origin,
                                     SkBaseDevice* device) {
    // Figure out all the storage needed to pre-size everything below.
    size_t totalGlyphs = 0;
    for (SkTextBlobRunIterator it(&blob); !it.done(); it.next()) {
        totalGlyphs += it.glyphCount();
    }

    // Pre-size all the buffers so they don't move during processing.
    this->initialize(totalGlyphs);

    SkPoint* positions = fPositions;

    for (SkTextBlobRunIterator it(&blob); !it.done(); it.next()) {
        if (!SkFontPriv::IsFinite(it.font())) {
            // If the font is not finite don't add the run.
            continue;
        }
        if (it.positioning() != SkTextBlobRunIterator::kRSXform_Positioning) {
            simplifyTextBlobIgnoringRSXForm(it, positions);
        } else {
            // Handle kRSXform_Positioning
            if (!this->empty()) {
                // Draw the things we have accumulated so far before drawing the RSX form.
                this->makeGlyphRunList(&blob, origin);
                device->drawGlyphRunList(this->useGlyphRunList(), paint);
                // re-init in case we keep looping and need the builder again
                this->initialize(totalGlyphs);
            }

            device->drawGlyphRunRSXform(it.font(), it.glyphs(), (const SkRSXform*)it.pos(),
                                        it.glyphCount(), origin, paint);

        }
        positions += it.glyphCount();
    }

    if (!this->empty()) {
        this->makeGlyphRunList(&blob, origin);
        device->drawGlyphRunList(this->useGlyphRunList(), paint);
    }
}

void SkGlyphRunBuilder::textBlobToGlyphRunListIgnoringRSXForm(
        const SkTextBlob& blob, SkPoint origin) {
    // Figure out all the storage needed to pre-size everything below.
    size_t positionsNeeded = 0;
    for (SkTextBlobRunIterator it(&blob); !it.done(); it.next()) {
        if (it.positioning() != SkTextBlobRunIterator::kFull_Positioning) {
            positionsNeeded += it.glyphCount();
        }
    }

    // Pre-size all the buffers so they don't move during processing.
    this->initialize(positionsNeeded);

    SkPoint* positionCursor = fPositions;
    for (SkTextBlobRunIterator it(&blob); !it.done(); it.next()) {
        positionCursor = simplifyTextBlobIgnoringRSXForm(it, positionCursor);
    }

    this->makeGlyphRunList(&blob, origin);
}

SkPoint* SkGlyphRunBuilder::simplifyTextBlobIgnoringRSXForm(const SkTextBlobRunIterator& it,
                                                            SkPoint* positionsCursor) {
    size_t runSize = it.glyphCount();
    auto glyphIDs = SkSpan<const SkGlyphID>{it.glyphs(), runSize};

    SkSpan<const SkPoint> positionsSpan;
    const SkFont& font = it.font();
    switch (it.positioning()) {
        case SkTextBlobRunIterator::kDefault_Positioning: {
            positionsSpan = draw_text_positions(font, glyphIDs, it.offset(), positionsCursor);
            positionsCursor += positionsSpan.size();
            break;
        }
        case SkTextBlobRunIterator::kHorizontal_Positioning: {
            positionsSpan = SkSpan(positionsCursor, runSize);
            for (auto x : SkSpan<const SkScalar>{it.pos(), runSize}) {
                *positionsCursor++ = SkPoint::Make(x, it.offset().y());
            }
            break;
        }
        case SkTextBlobRunIterator::kFull_Positioning: {
            positionsSpan = SkSpan(it.points(), runSize);
            break;
        }
        case SkTextBlobRunIterator::kRSXform_Positioning: {
            break;
        }
    }

    if (it.positioning() != SkTextBlobRunIterator::kRSXform_Positioning) {
        this->makeGlyphRun(
                font,
                glyphIDs,
                positionsSpan,
                SkSpan<const char>(it.text(), it.textSize()),
                SkSpan<const uint32_t>(it.clusters(), runSize));
    }

    return positionsCursor;
}

const SkGlyphRunList& SkGlyphRunBuilder::useGlyphRunList() {
    return fGlyphRunList;
}

void SkGlyphRunBuilder::initialize(size_t totalRunSize) {

    if (totalRunSize > fMaxTotalRunSize) {
        fMaxTotalRunSize = totalRunSize;
        fPositions.reset(fMaxTotalRunSize);
    }

    fGlyphRunListStorage.clear();
}

SkSpan<const SkGlyphID> SkGlyphRunBuilder::textToGlyphIDs(
        const SkFont& font, const void* bytes, size_t byteLength, SkTextEncoding encoding) {
    if (encoding != SkTextEncoding::kGlyphID) {
        int count = font.countText(bytes, byteLength, encoding);
        if (count > 0) {
            fScratchGlyphIDs.resize(count);
            font.textToGlyphs(bytes, byteLength, encoding, fScratchGlyphIDs.data(), count);
            return SkSpan(fScratchGlyphIDs);
        } else {
            return SkSpan<const SkGlyphID>();
        }
    } else {
        return SkSpan<const SkGlyphID>((const SkGlyphID*)bytes, byteLength / 2);
    }
}

void SkGlyphRunBuilder::makeGlyphRun(
        const SkFont& font,
        SkSpan<const SkGlyphID> glyphIDs,
        SkSpan<const SkPoint> positions,
        SkSpan<const char> text,
        SkSpan<const uint32_t> clusters) {

    // Ignore empty runs.
    if (!glyphIDs.empty()) {
        fGlyphRunListStorage.emplace_back(
                font,
                positions,
                glyphIDs,
                text,
                clusters);
    }
}

void SkGlyphRunBuilder::makeGlyphRunList(const SkTextBlob* blob, SkPoint origin) {

    fGlyphRunList.~SkGlyphRunList();
    new (&fGlyphRunList) SkGlyphRunList{blob, origin, SkSpan(fGlyphRunListStorage)};
}
