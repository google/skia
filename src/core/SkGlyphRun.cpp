/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkGlyphRun.h"

#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRSXform.h"
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
                       SkSpan<const uint32_t> clusters,
                       SkSpan<const SkVector> scaledRotations)
        : fSource{SkMakeZip(glyphIDs, positions)}
        , fText{text}
        , fClusters{clusters}
        , fScaledRotations{scaledRotations}
        , fFont{font} {}

SkGlyphRun::SkGlyphRun(const SkGlyphRun& that, const SkFont& font)
    : fSource{that.fSource}
    , fText{that.fText}
    , fClusters{that.fClusters}
    , fFont{font} {}

SkRect SkGlyphRun::sourceBounds(const SkPaint& paint) const {
    SkASSERT(this->runSize() > 0);
    const SkRect fontBounds = SkFontPriv::GetFontBounds(fFont);

    if (fontBounds.isEmpty()) {
        // Empty font bounds are likely a font bug.  TightBounds has a better chance of
        // producing useful results in this case.
        auto [strikeSpec, strikeToSourceScale] = SkStrikeSpec::MakeCanonicalized(fFont, &paint);
        SkBulkGlyphMetrics metrics{strikeSpec};
        SkSpan<const SkGlyph*> glyphs = metrics.glyphs(this->glyphsIDs());
        if (fScaledRotations.empty()) {
            // No RSXForm data - glyphs x/y aligned.
            auto scaleAndTranslateRect =
                [scale = strikeToSourceScale](const SkRect& in, const SkPoint& pos) {
                    return SkRect::MakeLTRB(in.left()   * scale + pos.x(),
                                            in.top()    * scale + pos.y(),
                                            in.right()  * scale + pos.x(),
                                            in.bottom() * scale + pos.y());
                };

            SkRect bounds = SkRect::MakeEmpty();
            for (auto [pos, glyph] : SkMakeZip(this->positions(), glyphs)) {
                if (SkRect r = glyph->rect(); !r.isEmpty()) {
                    bounds.join(scaleAndTranslateRect(r, pos));
                }
            }
            return bounds;
        } else {
            // RSXForm - glyphs can be any scale or rotation.
            SkRect bounds = SkRect::MakeEmpty();
            for (auto [pos, scaleRotate, glyph] :
                    SkMakeZip(this->positions(), fScaledRotations, glyphs)) {
                if (!glyph->rect().isEmpty()) {
                    SkMatrix xform = SkMatrix().setRSXform(
                            SkRSXform{pos.x(), pos.y(), scaleRotate.x(), scaleRotate.y()});
                    xform.preScale(strikeToSourceScale, strikeToSourceScale);
                    bounds.join(xform.mapRect(glyph->rect()));
                }
            }
            return bounds;
        }
    }

    // Use conservative bounds. All glyph have a box of fontBounds size.
    if (fScaledRotations.empty()) {
        SkRect bounds;
        bounds.setBounds(this->positions().data(), SkCount(this->positions()));
        bounds.fLeft   += fontBounds.left();
        bounds.fTop    += fontBounds.top();
        bounds.fRight  += fontBounds.right();
        bounds.fBottom += fontBounds.bottom();
        return bounds;
    } else {
        // RSXForm case glyphs can be any scale or rotation.
        SkRect bounds;
        bounds.setEmpty();
        for (auto [pos, scaleRotate] : SkMakeZip(this->positions(), fScaledRotations)) {
            const SkRSXform xform{pos.x(), pos.y(), scaleRotate.x(), scaleRotate.y()};
            bounds.join(SkMatrix().setRSXform(xform).mapRect(fontBounds));
        }
        return bounds;
    }
}

// -- SkGlyphRunList -------------------------------------------------------------------------------
SkGlyphRunList::SkGlyphRunList() = default;
SkGlyphRunList::SkGlyphRunList(
        const SkTextBlob* blob,
        SkRect bounds,
        SkPoint origin,
        SkSpan<const SkGlyphRun> glyphRunList)
        : fGlyphRuns{glyphRunList}
        , fOriginalTextBlob{blob}
        , fSourceBounds{bounds}
        , fOrigin{origin} { }

SkGlyphRunList::SkGlyphRunList(const SkGlyphRun& glyphRun, const SkRect& bounds, SkPoint origin)
        : fGlyphRuns{SkSpan<const SkGlyphRun>{&glyphRun, 1}}
        , fOriginalTextBlob{nullptr}
        , fSourceBounds{bounds}
        , fOrigin{origin} {}

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

sk_sp<SkTextBlob> SkGlyphRunList::makeBlob() const {
    SkTextBlobBuilder builder;
    for (auto& run : *this) {
        SkTextBlobBuilder::RunBuffer buffer;
        if (run.scaledRotations().empty()) {
            if (run.text().empty()) {
                buffer = builder.allocRunPos(run.font(), run.runSize(), nullptr);
            } else {
                buffer = builder.allocRunTextPos(run.font(), run.runSize(), run.text().size(), nullptr);
                auto text = run.text();
                memcpy(buffer.utf8text, text.data(), text.size_bytes());
                auto clusters = run.clusters();
                memcpy(buffer.clusters, clusters.data(), clusters.size_bytes());
            }
            auto positions = run.positions();
            memcpy(buffer.points(), positions.data(), positions.size_bytes());
        } else {
            buffer = builder.allocRunRSXform(run.font(), run.runSize());
            for (auto [xform, pos, sr] : SkMakeZip(buffer.xforms(),
                                                   run.positions(),
                                                   run.scaledRotations())) {
                xform = SkRSXform::Make(sr.x(), sr.y(), pos.x(), pos.y());
            }
        }
        auto glyphIDs = run.glyphsIDs();
        memcpy(buffer.glyphs, glyphIDs.data(), glyphIDs.size_bytes());
    }
    return builder.make();
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
    return SkMakeSpan(buffer, glyphIDs.size());
}

const SkGlyphRunList& SkGlyphRunBuilder::textToGlyphRunList(
        const SkFont& font, const SkPaint& paint,
        const void* bytes, size_t byteLength, SkPoint origin,
        SkTextEncoding encoding) {
    auto glyphIDs = textToGlyphIDs(font, bytes, byteLength, encoding);
    SkRect bounds = SkRect::MakeEmpty();
    this->prepareBuffers(glyphIDs.size(), 0);
    if (!glyphIDs.empty()) {
        SkSpan<const SkPoint> positions = draw_text_positions(font, glyphIDs, {0, 0}, fPositions);
        this->makeGlyphRun(font,
                           glyphIDs,
                           positions,
                           SkSpan<const char>{},
                           SkSpan<const uint32_t>{},
                           SkSpan<const SkVector>{});
        bounds = fGlyphRunListStorage.front().sourceBounds(paint);
    }

    return this->makeGlyphRunList(nullptr, bounds.makeOffset(origin), origin);
}

const SkGlyphRunList& SkGlyphRunBuilder::blobToGlyphRunList(
        const SkTextBlob& blob, SkPoint origin) {
    // Pre-size all the buffers so they don't move during processing.
    this->initialize(blob);

    SkPoint* positionCursor = fPositions;
    SkVector* scaledRotationsCursor = fScaledRotations;
    for (SkTextBlobRunIterator it(&blob); !it.done(); it.next()) {
        size_t runSize = it.glyphCount();
        if (runSize == 0 || !SkFontPriv::IsFinite(it.font())) {
            // If no glyphs or the font is not finite, don't add the run.
            continue;
        }

        const SkFont& font = it.font();
        auto glyphIDs = SkSpan<const SkGlyphID>{it.glyphs(), runSize};

        SkSpan<const SkPoint> positions;
        SkSpan<const SkVector> scaledRotations;
        switch (it.positioning()) {
            case SkTextBlobRunIterator::kDefault_Positioning: {
                positions = draw_text_positions(font, glyphIDs, it.offset(), positionCursor);
                positionCursor += positions.size();
                break;
            }
            case SkTextBlobRunIterator::kHorizontal_Positioning: {
                positions = SkMakeSpan(positionCursor, runSize);
                for (auto x : SkSpan<const SkScalar>{it.pos(), glyphIDs.size()}) {
                    *positionCursor++ = SkPoint::Make(x, it.offset().y());
                }
                break;
            }
            case SkTextBlobRunIterator::kFull_Positioning: {
                positions = SkMakeSpan(it.points(), runSize);
                break;
            }
            case SkTextBlobRunIterator::kRSXform_Positioning: {
                positions = SkMakeSpan(positionCursor, runSize);
                scaledRotations = SkMakeSpan(scaledRotationsCursor, runSize);
                for (const SkRSXform& xform : SkMakeSpan(it.xforms(), runSize)) {
                    *positionCursor++ = {xform.fTx, xform.fTy};
                    *scaledRotationsCursor++ = {xform.fSCos, xform.fSSin};
                }
                break;
            }
        }

        this->makeGlyphRun(
                font,
                glyphIDs,
                positions,
                SkSpan<const char>(it.text(), it.textSize()),
                SkSpan<const uint32_t>(it.clusters(), runSize),
                scaledRotations);
    }

    return this->makeGlyphRunList(&blob, blob.bounds().makeOffset(origin), origin);
}

std::tuple<SkSpan<const SkPoint>, SkSpan<const SkVector>>
SkGlyphRunBuilder::convertRSXForm(SkSpan<const SkRSXform> xforms) {
    const int count = SkCount(xforms);
    this->prepareBuffers(count, count);
    auto positions = SkMakeSpan(fPositions.get(), count);
    auto scaledRotations = SkMakeSpan(fScaledRotations.get(), count);
    for (auto [pos, sr, xform] : SkMakeZip(positions, scaledRotations, xforms)) {
        auto [scos, ssin, tx, ty] = xform;
        pos = {tx, ty};
        sr = {scos, ssin};
    }
    return {positions, scaledRotations};
}

void SkGlyphRunBuilder::initialize(const SkTextBlob& blob) {
    int positionCount = 0;
    int rsxFormCount = 0;
    for (SkTextBlobRunIterator it(&blob); !it.done(); it.next()) {
        if (it.positioning() != SkTextBlobRunIterator::kFull_Positioning) {
            positionCount += it.glyphCount();
        }
        if (it.positioning() == SkTextBlobRunIterator::kRSXform_Positioning) {
            rsxFormCount += it.glyphCount();
        }
    }

    prepareBuffers(positionCount, rsxFormCount);
}

void SkGlyphRunBuilder::prepareBuffers(int positionCount, int RSXFormCount) {
    if (positionCount > fMaxTotalRunSize) {
        fMaxTotalRunSize = positionCount;
        fPositions.reset(fMaxTotalRunSize);
    }

    if (RSXFormCount > fMaxScaledRotations) {
        fMaxScaledRotations = RSXFormCount;
        fScaledRotations.reset(RSXFormCount);
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
            return SkMakeSpan(fScratchGlyphIDs);
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
        SkSpan<const uint32_t> clusters,
        SkSpan<const SkVector> scaledRotations) {

    // Ignore empty runs.
    if (!glyphIDs.empty()) {
        fGlyphRunListStorage.emplace_back(
                font,
                positions,
                glyphIDs,
                text,
                clusters,
                scaledRotations);
    }
}

const SkGlyphRunList& SkGlyphRunBuilder::makeGlyphRunList(
        const SkTextBlob* blob, const SkRect& bounds, SkPoint origin) {
    fGlyphRunList.~SkGlyphRunList();
    return *new (&fGlyphRunList)
            SkGlyphRunList{blob, bounds, origin, SkMakeSpan(fGlyphRunListStorage)};
}
