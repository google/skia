/*
 * Copyright 2018 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/text/GlyphRun.h"

#include "include/core/SkFont.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkScalar.h"
#include "include/private/base/SkTLogic.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTextBlobPriv.h"

#include <cstring>

class SkPaint;

namespace sktext {
// -- GlyphRun -------------------------------------------------------------------------------------
GlyphRun::GlyphRun(const SkFont& font,
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

GlyphRun::GlyphRun(const GlyphRun& that, const SkFont& font)
    : fSource{that.fSource}
    , fText{that.fText}
    , fClusters{that.fClusters}
    , fFont{font} {}

// -- GlyphRunList ---------------------------------------------------------------------------------
GlyphRunList::GlyphRunList(const SkTextBlob* blob,
                           SkRect bounds,
                           SkPoint origin,
                           SkSpan<const GlyphRun> glyphRunList,
                           GlyphRunBuilder* builder)
        : fGlyphRuns{glyphRunList}
        , fOriginalTextBlob{blob}
        , fSourceBounds{bounds}
        , fOrigin{origin}
        , fBuilder{builder} {}

GlyphRunList::GlyphRunList(const GlyphRun& glyphRun,
                           const SkRect& bounds,
                           SkPoint origin,
                           GlyphRunBuilder* builder)
        : fGlyphRuns{SkSpan<const GlyphRun>{&glyphRun, 1}}
        , fOriginalTextBlob{nullptr}
        , fSourceBounds{bounds}
        , fOrigin{origin}
        , fBuilder{builder} {}

uint64_t GlyphRunList::uniqueID() const {
    return fOriginalTextBlob != nullptr ? fOriginalTextBlob->uniqueID()
                                        : SK_InvalidUniqueID;
}

bool GlyphRunList::anyRunsLCD() const {
    for (const auto& r : fGlyphRuns) {
        if (r.font().getEdging() == SkFont::Edging::kSubpixelAntiAlias) {
            return true;
        }
    }
    return false;
}

void GlyphRunList::temporaryShuntBlobNotifyAddedToCache(uint32_t cacheID,
                                                        SkTextBlob::PurgeDelegate pd) const {
    SkASSERT(fOriginalTextBlob != nullptr);
    SkASSERT(pd != nullptr);
    fOriginalTextBlob->notifyAddedToCache(cacheID, pd);
}

sk_sp<SkTextBlob> GlyphRunList::makeBlob() const {
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

// -- GlyphRunBuilder ------------------------------------------------------------------------------
static SkRect glyphrun_source_bounds(
        const SkFont& font,
        const SkPaint& paint,
        SkZip<const SkGlyphID, const SkPoint> source,
        SkSpan<const SkVector> scaledRotations) {
    SkASSERT(!source.empty());
    const SkRect fontBounds = SkFontPriv::GetFontBounds(font);

    SkSpan<const SkGlyphID> glyphIDs = source.get<0>();
    SkSpan<const SkPoint> positions = source.get<1>();

    if (fontBounds.isEmpty()) {
        // Empty font bounds are likely a font bug.  TightBounds has a better chance of
        // producing useful results in this case.
        auto [strikeSpec, strikeToSourceScale] = SkStrikeSpec::MakeCanonicalized(font, &paint);
        SkBulkGlyphMetrics metrics{strikeSpec};
        SkSpan<const SkGlyph*> glyphs = metrics.glyphs(glyphIDs);
        if (scaledRotations.empty()) {
            // No RSXForm data - glyphs x/y aligned.
            auto scaleAndTranslateRect =
                    [scale = strikeToSourceScale](const SkRect& in, const SkPoint& pos) {
                        return SkRect::MakeLTRB(in.left()   * scale + pos.x(),
                                                in.top()    * scale + pos.y(),
                                                in.right()  * scale + pos.x(),
                                                in.bottom() * scale + pos.y());
                    };

            SkRect bounds = SkRect::MakeEmpty();
            for (auto [pos, glyph] : SkMakeZip(positions, glyphs)) {
                if (SkRect r = glyph->rect(); !r.isEmpty()) {
                    bounds.join(scaleAndTranslateRect(r, pos));
                }
            }
            return bounds;
        } else {
            // RSXForm - glyphs can be any scale or rotation.
            SkRect bounds = SkRect::MakeEmpty();
            for (auto [pos, scaleRotate, glyph] : SkMakeZip(positions, scaledRotations, glyphs)) {
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
    if (scaledRotations.empty()) {
        SkRect bounds;
        bounds.setBounds(positions.data(), SkCount(positions));
        bounds.fLeft   += fontBounds.left();
        bounds.fTop    += fontBounds.top();
        bounds.fRight  += fontBounds.right();
        bounds.fBottom += fontBounds.bottom();
        return bounds;
    } else {
        // RSXForm case glyphs can be any scale or rotation.
        SkRect bounds;
        bounds.setEmpty();
        for (auto [pos, scaleRotate] : SkMakeZip(positions, scaledRotations)) {
            const SkRSXform xform{pos.x(), pos.y(), scaleRotate.x(), scaleRotate.y()};
            bounds.join(SkMatrix().setRSXform(xform).mapRect(fontBounds));
        }
        return bounds;
    }
}

GlyphRunList GlyphRunBuilder::makeGlyphRunList(
        const GlyphRun& run, const SkPaint& paint, SkPoint origin) {
    const SkRect bounds =
            glyphrun_source_bounds(run.font(), paint, run.source(), run.scaledRotations());
    return GlyphRunList{run, bounds, origin, this};
}

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

const GlyphRunList& GlyphRunBuilder::textToGlyphRunList(
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
        auto run = fGlyphRunListStorage.front();
        bounds = glyphrun_source_bounds(run.font(), paint, run.source(), run.scaledRotations());
    }

    return this->setGlyphRunList(nullptr, bounds, origin);
}

const GlyphRunList& sktext::GlyphRunBuilder::blobToGlyphRunList(
        const SkTextBlob& blob, SkPoint origin) {
    // Pre-size all the buffers, so they don't move during processing.
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
                positions = SkSpan(positionCursor, runSize);
                for (auto x : SkSpan<const SkScalar>{it.pos(), glyphIDs.size()}) {
                    *positionCursor++ = SkPoint::Make(x, it.offset().y());
                }
                break;
            }
            case SkTextBlobRunIterator::kFull_Positioning: {
                positions = SkSpan(it.points(), runSize);
                break;
            }
            case SkTextBlobRunIterator::kRSXform_Positioning: {
                positions = SkSpan(positionCursor, runSize);
                scaledRotations = SkSpan(scaledRotationsCursor, runSize);
                for (const SkRSXform& xform : SkSpan(it.xforms(), runSize)) {
                    *positionCursor++ = {xform.fTx, xform.fTy};
                    *scaledRotationsCursor++ = {xform.fSCos, xform.fSSin};
                }
                break;
            }
        }

        const uint32_t* clusters = it.clusters();
        this->makeGlyphRun(
                font,
                glyphIDs,
                positions,
                SkSpan<const char>(it.text(), it.textSize()),
                SkSpan<const uint32_t>(clusters, clusters ? runSize : 0),
                scaledRotations);
    }

    return this->setGlyphRunList(&blob, blob.bounds(), origin);
}

std::tuple<SkSpan<const SkPoint>, SkSpan<const SkVector>>
GlyphRunBuilder::convertRSXForm(SkSpan<const SkRSXform> xforms) {
    const int count = SkCount(xforms);
    this->prepareBuffers(count, count);
    auto positions = SkSpan(fPositions.get(), count);
    auto scaledRotations = SkSpan(fScaledRotations.get(), count);
    for (auto [pos, sr, xform] : SkMakeZip(positions, scaledRotations, xforms)) {
        auto [scos, ssin, tx, ty] = xform;
        pos = {tx, ty};
        sr = {scos, ssin};
    }
    return {positions, scaledRotations};
}

void GlyphRunBuilder::initialize(const SkTextBlob& blob) {
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

void GlyphRunBuilder::prepareBuffers(int positionCount, int RSXFormCount) {
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

SkSpan<const SkGlyphID> GlyphRunBuilder::textToGlyphIDs(
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

void GlyphRunBuilder::makeGlyphRun(
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

const GlyphRunList& sktext::GlyphRunBuilder::setGlyphRunList(
        const SkTextBlob* blob, const SkRect& bounds, SkPoint origin) {
    fGlyphRunList.emplace(blob, bounds, origin, SkSpan(fGlyphRunListStorage), this);
    return fGlyphRunList.value();
}
}  // namespace sktext
