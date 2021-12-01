/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkScalerCache.h"

#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkEnumerate.h"
#include "src/core/SkScalerContext.h"

static SkFontMetrics use_or_generate_metrics(
        const SkFontMetrics* metrics, SkScalerContext* context) {
    SkFontMetrics answer;
    if (metrics) {
        answer = *metrics;
    } else {
        context->getFontMetrics(&answer);
    }
    return answer;
}

SkScalerCache::SkScalerCache(
    std::unique_ptr<SkScalerContext> scaler,
    const SkFontMetrics* fontMetrics)
        : fScalerContext{std::move(scaler)}
        , fFontMetrics{use_or_generate_metrics(fontMetrics, fScalerContext.get())}
        , fRoundingSpec{fScalerContext->isSubpixel(),
                        fScalerContext->computeAxisAlignmentForHText()} {
    SkASSERT(fScalerContext != nullptr);
}

std::tuple<SkGlyph*, size_t> SkScalerCache::glyph(SkPackedGlyphID packedGlyphID) {
    auto [digest, size] = this->digest(packedGlyphID);
    return {fGlyphForIndex[digest.index()], size};
}

std::tuple<SkGlyphDigest, size_t> SkScalerCache::digest(SkPackedGlyphID packedGlyphID) {
    SkGlyphDigest* digest = fDigestForPackedGlyphID.find(packedGlyphID);

    if (digest != nullptr) {
        return {*digest, 0};
    }

    SkGlyph* glyph = fAlloc.make<SkGlyph>(fScalerContext->makeGlyph(packedGlyphID, &fAlloc));
    return {this->addGlyph(glyph), sizeof(SkGlyph)};
}

SkGlyphDigest SkScalerCache::addGlyph(SkGlyph* glyph) {
    size_t index = fGlyphForIndex.size();
    SkGlyphDigest digest = SkGlyphDigest{index, *glyph};
    fDigestForPackedGlyphID.set(glyph->getPackedID(), digest);
    fGlyphForIndex.push_back(glyph);
    return digest;
}

std::tuple<const SkPath*, size_t> SkScalerCache::preparePath(SkGlyph* glyph) {
    size_t delta = 0;
    if (glyph->setPath(&fAlloc, fScalerContext.get())) {
        delta = glyph->path()->approximateBytesUsed();
    }
    return {glyph->path(), delta};
}

std::tuple<const SkPath*, size_t> SkScalerCache::mergePath(
        SkGlyph* glyph, const SkPath* path, bool hairline) {
    SkAutoMutexExclusive lock{fMu};
    size_t pathDelta = 0;
    if (glyph->setPath(&fAlloc, path, hairline)) {
        pathDelta = glyph->path()->approximateBytesUsed();
    }
    return {glyph->path(), pathDelta};
}

int SkScalerCache::countCachedGlyphs() const {
    SkAutoMutexExclusive lock(fMu);
    return fDigestForPackedGlyphID.count();
}

std::tuple<SkSpan<const SkGlyph*>, size_t> SkScalerCache::internalPrepare(
        SkSpan<const SkGlyphID> glyphIDs, PathDetail pathDetail, const SkGlyph** results) {
    const SkGlyph** cursor = results;
    size_t delta = 0;
    for (auto glyphID : glyphIDs) {
        auto [glyph, size] = this->glyph(SkPackedGlyphID{glyphID});
        delta += size;
        if (pathDetail == kMetricsAndPath) {
            auto [_, pathSize] = this->preparePath(glyph);
            delta += pathSize;
        }
        *cursor++ = glyph;
    }

    return {{results, glyphIDs.size()}, delta};
}

std::tuple<const void*, size_t> SkScalerCache::prepareImage(SkGlyph* glyph) {
    size_t delta = 0;
    if (glyph->setImage(&fAlloc, fScalerContext.get())) {
        delta = glyph->imageSize();
    }
    return {glyph->image(), delta};
}

std::tuple<SkGlyph*, size_t> SkScalerCache::mergeGlyphAndImage(
        SkPackedGlyphID toID, const SkGlyph& from) {
    SkAutoMutexExclusive lock{fMu};
    // TODO(herb): remove finding the glyph when we are sure there are no glyph collisions.
    SkGlyphDigest* digest = fDigestForPackedGlyphID.find(toID);
    if (digest != nullptr) {
        // Since there is no search for replacement glyphs, this glyph should not exist yet.
        SkDEBUGFAIL("This implies adding to an existing glyph. This should not happen.");

        // Just return what we have. The invariants have already been cast in stone.
        return {fGlyphForIndex[digest->index()], 0};
    } else {
        SkGlyph* glyph = fAlloc.make<SkGlyph>(toID);
        size_t delta = glyph->setMetricsAndImage(&fAlloc, from);
        (void)this->addGlyph(glyph);
        return {glyph, sizeof(SkGlyph) + delta};
    }
}

std::tuple<SkSpan<const SkGlyph*>, size_t> SkScalerCache::metrics(
        SkSpan<const SkGlyphID> glyphIDs, const SkGlyph* results[]) {
    SkAutoMutexExclusive lock{fMu};
    auto [glyphs, delta] = this->internalPrepare(glyphIDs, kMetricsOnly, results);
    return {glyphs, delta};
}

std::tuple<SkSpan<const SkGlyph*>, size_t> SkScalerCache::preparePaths(
        SkSpan<const SkGlyphID> glyphIDs, const SkGlyph* results[]) {
    SkAutoMutexExclusive lock{fMu};
    auto [glyphs, delta] = this->internalPrepare(glyphIDs, kMetricsAndPath, results);
    return {glyphs, delta};
}

std::tuple<SkSpan<const SkGlyph*>, size_t> SkScalerCache::prepareImages(
        SkSpan<const SkPackedGlyphID> glyphIDs, const SkGlyph* results[]) {
    const SkGlyph** cursor = results;
    SkAutoMutexExclusive lock{fMu};
    size_t delta = 0;
    for (auto glyphID : glyphIDs) {
        auto[glyph, glyphSize] = this->glyph(glyphID);
        auto[_, imageSize] = this->prepareImage(glyph);
        delta += glyphSize + imageSize;
        *cursor++ = glyph;
    }

    return {{results, glyphIDs.size()}, delta};
}

template <typename Fn>
size_t SkScalerCache::commonFilterLoop(SkDrawableGlyphBuffer* drawables, Fn&& fn) {
    size_t total = 0;
    for (auto [i, packedID, pos] : SkMakeEnumerate(drawables->input())) {
        if (SkScalarsAreFinite(pos.x(), pos.y())) {
            auto [digest, size] = this->digest(packedID);
            total += size;
            if (!digest.isEmpty()) {
                fn(i, digest, pos);
            }
        }
    }
    return total;
}

size_t SkScalerCache::prepareForDrawingMasksCPU(SkDrawableGlyphBuffer* drawables) {
    SkAutoMutexExclusive lock{fMu};
    size_t imageDelta = 0;
    size_t delta = this->commonFilterLoop(drawables,
        [&](size_t i, SkGlyphDigest digest, SkPoint pos) SK_REQUIRES(fMu) {
            // If the glyph is too large, then no image is created.
            SkGlyph* glyph = fGlyphForIndex[digest.index()];
            auto [image, imageSize] = this->prepareImage(glyph);
            if (image != nullptr) {
                drawables->push_back(glyph, i);
                imageDelta += imageSize;
            }
        });

    return delta + imageDelta;
}

// Note: this does not actually fill out the image. That happens at atlas building time.
size_t SkScalerCache::prepareForMaskDrawing(
        SkDrawableGlyphBuffer* drawables, SkSourceGlyphBuffer* rejects) {
    SkAutoMutexExclusive lock{fMu};
    size_t delta = this->commonFilterLoop(drawables,
        [&](size_t i, SkGlyphDigest digest, SkPoint pos) SK_REQUIRES(fMu) {
            if (digest.canDrawAsMask()) {
                drawables->push_back(fGlyphForIndex[digest.index()], i);
            } else {
                rejects->reject(i);
            }
        });

    return delta;
}

size_t SkScalerCache::prepareForSDFTDrawing(
        SkDrawableGlyphBuffer* drawables, SkSourceGlyphBuffer* rejects) {
    SkAutoMutexExclusive lock{fMu};
    size_t delta = this->commonFilterLoop(drawables,
        [&](size_t i, SkGlyphDigest digest, SkPoint pos) SK_REQUIRES(fMu) {
            if (digest.canDrawAsSDFT()) {
                drawables->push_back(fGlyphForIndex[digest.index()], i);
            } else {
                rejects->reject(i);
            }
        });

    return delta;
}

size_t SkScalerCache::prepareForPathDrawing(
        SkDrawableGlyphBuffer* drawables, SkSourceGlyphBuffer* rejects) {
    SkAutoMutexExclusive lock{fMu};
    size_t pathDelta = 0;
    size_t delta = this->commonFilterLoop(drawables,
        [&](size_t i, SkGlyphDigest digest, SkPoint pos) SK_REQUIRES(fMu) {
            SkGlyph* glyph = fGlyphForIndex[digest.index()];
            if (!digest.isColor()) {
                auto [path, pathSize] = this->preparePath(glyph);
                pathDelta += pathSize;
                if (path != nullptr) {
                    // Save off the path to draw later.
                    drawables->push_back(path, i);
                } else {
                    // Glyph does not have a path. It is probably bitmap only.
                    rejects->reject(i, glyph->maxDimension());
                }
            } else {
                // Glyph is color.
                rejects->reject(i, glyph->maxDimension());
            }
        });

    return delta + pathDelta;
}

void SkScalerCache::findIntercepts(const SkScalar bounds[2], SkScalar scale, SkScalar xPos,
        SkGlyph* glyph, SkScalar* array, int* count) {
    SkAutoMutexExclusive lock{fMu};
    glyph->ensureIntercepts(bounds, scale, xPos, array, count, &fAlloc);
}

void SkScalerCache::dump() const {
    SkAutoMutexExclusive lock{fMu};
    const SkTypeface* face = fScalerContext->getTypeface();
    const SkScalerContextRec& rec = fScalerContext->getRec();
    SkMatrix matrix;
    rec.getSingleMatrix(&matrix);
    matrix.preScale(SkScalarInvert(rec.fTextSize), SkScalarInvert(rec.fTextSize));
    SkString name;
    face->getFamilyName(&name);

    SkString msg;
    SkFontStyle style = face->fontStyle();
    msg.printf("cache typeface:%x %25s:(%d,%d,%d)\n %s glyphs:%3d",
               face->uniqueID(), name.c_str(), style.weight(), style.width(), style.slant(),
               rec.dump().c_str(), fDigestForPackedGlyphID.count());
    SkDebugf("%s\n", msg.c_str());
}

