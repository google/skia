/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkScalerCache.h"

#include "include/core/SkDrawable.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkDistanceFieldGen.h"
#include "src/core/SkEnumerate.h"
#include "src/core/SkGlyphBuffer.h"
#include "src/core/SkScalerContext.h"
#include "src/text/StrikeForGPU.h"

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

size_t SkScalerCache::preparePath(SkGlyph* glyph) {
    size_t delta = 0;
    if (glyph->setPath(&fAlloc, fScalerContext.get())) {
        delta = glyph->path()->approximateBytesUsed();
    }
    return delta;
}

std::tuple<const SkPath*, size_t> SkScalerCache::mergePath(
        SkGlyph* glyph, const SkPath* path, bool hairline) {
    SkAutoMutexExclusive lock{fMu};
    size_t pathDelta = 0;
    if (glyph->setPathHasBeenCalled()) {
        SkDEBUGFAIL("Re-adding path to existing glyph. This should not happen.");
    }
    if (glyph->setPath(&fAlloc, path, hairline)) {
        pathDelta = glyph->path()->approximateBytesUsed();
    }
    return {glyph->path(), pathDelta};
}

size_t SkScalerCache::prepareDrawable(SkGlyph* glyph) {
    size_t delta = 0;
    if (glyph->setDrawable(&fAlloc, fScalerContext.get())) {
        delta = glyph->drawable()->approximateBytesUsed();
        SkASSERT(delta > 0);
    }
    return delta;
}

std::tuple<SkDrawable*, size_t> SkScalerCache::mergeDrawable(SkGlyph* glyph,
                                                             sk_sp<SkDrawable> drawable) {
    SkAutoMutexExclusive lock{fMu};
    size_t delta = 0;
    if (glyph->setDrawableHasBeenCalled()) {
        SkDEBUGFAIL("Re-adding drawable to existing glyph. This should not happen.");
    }
    if (glyph->setDrawable(&fAlloc, std::move(drawable))) {
        delta = glyph->drawable()->approximateBytesUsed();
        SkASSERT(delta > 0);
    }
    return {glyph->drawable(), delta};
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
            size_t pathSize = this->preparePath(glyph);
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
    // TODO(herb): remove finding the glyph when setting the metrics and image are separated
    SkGlyphDigest* digest = fDigestForPackedGlyphID.find(toID);
    if (digest != nullptr) {
        SkGlyph* to = fGlyphForIndex[digest->index()];
        size_t delta = 0;
        if (from.setImageHasBeenCalled()) {
            if (to->setImageHasBeenCalled()) {
                // Should never set an image on a glyph which already has an image.
                SkDEBUGFAIL("Re-adding image to existing glyph. This should not happen.");
            }
            // TODO: assert that any metrics on `from` are the same.
            delta = to->setMetricsAndImage(&fAlloc, from);
        }
        return {to, delta};
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

size_t SkScalerCache::glyphIDsToPaths(SkSpan<sktext::IDOrPath> idsOrPaths) {
    size_t increase = 0;
    SkAutoMutexExclusive lock{fMu};
    for (sktext::IDOrPath& idOrPath : idsOrPaths) {
        auto [glyph, size] = this->glyph(SkPackedGlyphID{idOrPath.fGlyphID});
        increase += size;
        increase += this->preparePath(glyph);
        new (&idOrPath.fPath) SkPath{*glyph->path()};
    }
    return increase;
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

std::tuple<SkSpan<const SkGlyph*>, size_t> SkScalerCache::prepareDrawables(
        SkSpan<const SkGlyphID> glyphIDs, const SkGlyph* results[]) {
    const SkGlyph** cursor = results;
    SkAutoMutexExclusive lock{fMu};
    size_t delta = 0;
    for (auto glyphID : glyphIDs) {
        auto[glyph, glyphSize] = this->glyph(SkPackedGlyphID{glyphID});
        size_t drawableSize = this->prepareDrawable(glyph);
        delta += glyphSize + drawableSize;
        *cursor++ = glyph;
    }

    return {{results, glyphIDs.size()}, delta};
}

size_t SkScalerCache::glyphIDsToDrawables(SkSpan<sktext::IDOrDrawable> idsOrDrawables) {
    size_t increase = 0;
    SkAutoMutexExclusive lock{fMu};
    for (sktext::IDOrDrawable& idOrDrawable : idsOrDrawables) {
        auto [glyph, size] = this->glyph(SkPackedGlyphID{idOrDrawable.fGlyphID});
        increase += size;
        increase += this->prepareDrawable(glyph);
        SkASSERT(glyph->drawable() != nullptr);
        idOrDrawable.fDrawable = glyph->drawable();
    }
    return increase;
}

std::tuple<SkScalar, size_t> SkScalerCache::findMaximumGlyphDimension(
        SkSpan<const SkGlyphID> glyphs) {
    size_t totalIncrease = 0;
    SkScalar maxDimension = 0;
    SkAutoMutexExclusive lock{fMu};
    for (SkGlyphID glyphID : glyphs) {
        auto [digest, increase] = this->digest(SkPackedGlyphID{glyphID});
        totalIncrease += increase;
        maxDimension = std::max(static_cast<SkScalar>(digest.maxDimension()), maxDimension);
    }
    return {maxDimension, totalIncrease};
}

template <typename Fn>
size_t SkScalerCache::commonFilterLoop(SkDrawableGlyphBuffer* accepted, Fn&& fn) {
    size_t total = 0;
    for (auto [i, packedID, pos] : SkMakeEnumerate(accepted->input())) {
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

size_t SkScalerCache::prepareForDrawingMasksCPU(SkDrawableGlyphBuffer* accepted) {
    SkAutoMutexExclusive lock{fMu};
    size_t imageDelta = 0;
    size_t delta = this->commonFilterLoop(accepted,
        [&](size_t i, SkGlyphDigest digest, SkPoint pos) SK_REQUIRES(fMu) {
            // If the glyph is too large, then no image is created.
            SkGlyph* glyph = fGlyphForIndex[digest.index()];
            auto [image, imageSize] = this->prepareImage(glyph);
            if (image != nullptr) {
                accepted->accept(glyph, i);
                imageDelta += imageSize;
            }
        });

    return delta + imageDelta;
}

// Note: this does not actually fill out the image. That happens at atlas building time.
std::tuple<SkRect, size_t> SkScalerCache::prepareForMaskDrawing(
        SkDrawableGlyphBuffer* accepted,
        SkSourceGlyphBuffer* rejected) {
    SkAutoMutexExclusive lock{fMu};

    SkGlyphRect boundingRect = skglyph::empty_rect();
    size_t increase = 0;

    for (auto [i, packedID, pos] : SkMakeEnumerate(accepted->input())) {
        if (SkScalarsAreFinite(pos.x(), pos.y())) {
            auto [digest, glyphIncrease] = this->digest(packedID);
            increase += glyphIncrease;
            if (!digest.isEmpty()) {
                // N.B. this must have the same behavior as RemoteStrike::prepareForMaskDrawing.
                if (digest.canDrawAsMask()) {
                    const SkGlyphRect glyphBounds = digest.bounds().offset(pos);
                    boundingRect = skglyph::rect_union(boundingRect, glyphBounds);
                    accepted->accept(packedID, glyphBounds.leftTop(), digest.maskFormat());
                } else {
                    rejected->reject(i);
                }
            }
        }
    }

    return {boundingRect.rect(), increase};
}

std::tuple<SkRect, size_t> SkScalerCache::prepareForSDFTDrawing(
        SkScalar strikeToSourceScale,
        SkDrawableGlyphBuffer* accepted,
        SkSourceGlyphBuffer* rejected) {
    SkAutoMutexExclusive lock{fMu};

    SkGlyphRect boundingRect = skglyph::empty_rect();
    size_t increase = 0;

    for (auto [i, packedID, pos] : SkMakeEnumerate(accepted->input())) {
        if (SkScalarsAreFinite(pos.x(), pos.y())) {
            auto [digest, glyphIncrease] = this->digest(packedID);
            increase += glyphIncrease;
            if (!digest.isEmpty()) {
                if (digest.canDrawAsSDFT()) {
                    // The SDFT glyphs have 2-pixel wide padding that should not be used in
                    // calculating the source rectangle.
                    const SkGlyphRect glyphBounds =
                            digest.bounds()
                                    .inset(SK_DistanceFieldInset, SK_DistanceFieldInset)
                                    .scaleAndOffset(strikeToSourceScale, pos);
                    boundingRect = skglyph::rect_union(boundingRect, glyphBounds);
                    accepted->accept(packedID, glyphBounds.leftTop(), digest.maskFormat());
                } else {
                    // Assume whatever follows SDF doesn't care about the maximum rejected size.
                    rejected->reject(i);
                }
            }
        }
    }

    return {boundingRect.rect(), increase};
}

size_t SkScalerCache::prepareForPathDrawing(
        SkDrawableGlyphBuffer* accepted, SkSourceGlyphBuffer* rejected) {
    SkAutoMutexExclusive lock{fMu};
    size_t increase = 0;
    for (auto [i, packedID, pos] : SkMakeEnumerate(accepted->input())) {
        if (SkScalarsAreFinite(pos.x(), pos.y())) {
            auto [digest, glyphIncrease] = this->digest(packedID);
            increase += glyphIncrease;
            if (!digest.isEmpty()) {
                SkGlyph* glyph = fGlyphForIndex[digest.index()];
                increase += this->preparePath(glyph);
                if (glyph->path() != nullptr) {
                    // Save off the path to draw later.
                    accepted->accept(packedID, pos);
                } else {
                    // Glyph does not have a path.
                    rejected->reject(i);
                }
            }
        }
    }
    return increase;
}

size_t SkScalerCache::prepareForDrawableDrawing(
        SkDrawableGlyphBuffer* accepted, SkSourceGlyphBuffer* rejected) {
    SkAutoMutexExclusive lock{fMu};
    size_t increase = 0;
    for (auto [i, packedID, pos] : SkMakeEnumerate(accepted->input())) {
        if (SkScalarsAreFinite(pos.x(), pos.y())) {
            auto [digest, glyphIncrease] = this->digest(packedID);
            increase += glyphIncrease;
            if (!digest.isEmpty()) {
                SkGlyph* glyph = fGlyphForIndex[digest.index()];
                increase += this->prepareDrawable(glyph);
                if (glyph->drawable() != nullptr) {
                    // Save off the drawable to draw later.
                    accepted->accept(packedID, pos);
                } else {
                    // Glyph does not have a drawable.
                    rejected->reject(i);
                }
            }
        }
    }

    return increase;
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

