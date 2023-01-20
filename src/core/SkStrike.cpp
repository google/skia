/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkStrike.h"

#include "include/core/SkDrawable.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkDistanceFieldGen.h"
#include "src/core/SkEnumerate.h"
#include "src/core/SkGlyphBuffer.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrikeCache.h"
#include "src/text/StrikeForGPU.h"

#if SK_SUPPORT_GPU
    #include "src/text/gpu/StrikeCache.h"
#endif

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

SkStrike::SkStrike(SkStrikeCache* strikeCache,
                   const SkStrikeSpec& strikeSpec,
                   std::unique_ptr<SkScalerContext> scaler,
                   const SkFontMetrics* metrics,
                   std::unique_ptr<SkStrikePinner> pinner)
        : fScalerContext{std::move(scaler)}
        , fFontMetrics{use_or_generate_metrics(metrics, fScalerContext.get())}
        , fRoundingSpec{fScalerContext->isSubpixel(),
                        fScalerContext->computeAxisAlignmentForHText()}
        , fStrikeSpec{strikeSpec}
        , fStrikeCache{strikeCache}
        , fPinner{std::move(pinner)} {
    SkASSERT(fScalerContext != nullptr);
}

SkGlyph* SkStrike::mergeGlyphAndImage(SkPackedGlyphID toID, const SkGlyph& fromGlyph) {
    size_t increase = 0;
    SkGlyph* glyph;
    {
        SkAutoMutexExclusive lock{fMu};
        // TODO(herb): remove finding the glyph when setting the metrics and image are separated
        SkGlyphDigest* digest = fDigestForPackedGlyphID.find(toID);
        if (digest != nullptr) {
            glyph = fGlyphForIndex[digest->index()];
            if (fromGlyph.setImageHasBeenCalled()) {
                if (glyph->setImageHasBeenCalled()) {
                    // Should never set an image on a glyph which already has an image.
                    SkDEBUGFAIL("Re-adding image to existing glyph. This should not happen.");
                }
                // TODO: assert that any metrics on fromGlyph are the same.
                increase = glyph->setMetricsAndImage(&fAlloc, fromGlyph);
            }
        } else {
            glyph = fAlloc.make<SkGlyph>(toID);
            increase = glyph->setMetricsAndImage(&fAlloc, fromGlyph) + sizeof(SkGlyph);
            (void)this->addGlyph(glyph);
        }
    }

    this->updateDelta(increase);

    return glyph;
}

const SkPath* SkStrike::mergePath(SkGlyph* glyph, const SkPath* path, bool hairline) {
    size_t increase = 0;
    {
        SkAutoMutexExclusive lock{fMu};
        if (glyph->setPathHasBeenCalled()) {
            SkDEBUGFAIL("Re-adding path to existing glyph. This should not happen.");
        }
        if (glyph->setPath(&fAlloc, path, hairline)) {
            increase = glyph->path()->approximateBytesUsed();
        }
    }

    this->updateDelta(increase);

    return glyph->path();
}

const SkDrawable* SkStrike::mergeDrawable(SkGlyph* glyph, sk_sp<SkDrawable> drawable) {
    size_t increase = 0;
    {
        SkAutoMutexExclusive lock{fMu};
        if (glyph->setDrawableHasBeenCalled()) {
            SkDEBUGFAIL("Re-adding drawable to existing glyph. This should not happen.");
        }
        if (glyph->setDrawable(&fAlloc, std::move(drawable))) {
            increase = glyph->drawable()->approximateBytesUsed();
            SkASSERT(increase > 0);
        }
    }

    this->updateDelta(increase);
    return glyph->drawable();
}

void SkStrike::findIntercepts(const SkScalar bounds[2], SkScalar scale, SkScalar xPos,
                              SkGlyph* glyph, SkScalar* array, int* count) {
    SkAutoMutexExclusive lock{fMu};
    glyph->ensureIntercepts(bounds, scale, xPos, array, count, &fAlloc);
}

SkSpan<const SkGlyph*> SkStrike::metrics(
        SkSpan<const SkGlyphID> glyphIDs, const SkGlyph* results[]) {
    size_t increase = 0;
    SkSpan<const SkGlyph*> glyphs;
    {
        SkAutoMutexExclusive lock{fMu};
        std::tie(glyphs, increase)= this->internalPrepare(glyphIDs, kMetricsOnly, results);
    }

    this->updateDelta(increase);
    return glyphs;
}

SkSpan<const SkGlyph*> SkStrike::preparePaths(
        SkSpan<const SkGlyphID> glyphIDs, const SkGlyph* results[]) {
    size_t increase = 0;
    SkSpan<const SkGlyph*> glyphs;
    {
        SkAutoMutexExclusive lock{fMu};
        std::tie(glyphs, increase) = this->internalPrepare(glyphIDs, kMetricsAndPath, results);
    }

    this->updateDelta(increase);
    return glyphs;
}

SkSpan<const SkGlyph*> SkStrike::prepareImages(
        SkSpan<const SkPackedGlyphID> glyphIDs, const SkGlyph* results[]) {
    const SkGlyph** cursor = results;
    size_t increase = 0;
    {
        SkAutoMutexExclusive lock{fMu};
        for (auto glyphID : glyphIDs) {
            auto[glyph, glyphSize] = this->glyph(glyphID);
            auto[_, imageSize] = this->prepareImage(glyph);
            increase += glyphSize + imageSize;
            *cursor++ = glyph;
        }
    }

    this->updateDelta(increase);
    return {results, glyphIDs.size()};
}

SkSpan<const SkGlyph*> SkStrike::prepareDrawables(
        SkSpan<const SkGlyphID> glyphIDs, const SkGlyph* results[]) {
    const SkGlyph** cursor = results;
    size_t increase = 0;
    {
        SkAutoMutexExclusive lock{fMu};
        for (auto glyphID : glyphIDs) {
            auto[glyph, glyphSize] = this->glyph(SkPackedGlyphID{glyphID});
            size_t drawableSize = this->prepareDrawable(glyph);
            increase += glyphSize + drawableSize;
            *cursor++ = glyph;
        }
    }

    this->updateDelta(increase);
    return {results, glyphIDs.size()};
}

void SkStrike::prepareForDrawingMasksCPU(SkDrawableGlyphBuffer* accepted) {
    size_t increase = 0;
    {
        SkAutoMutexExclusive lock{fMu};
        increase += this->commonFilterLoop(
                accepted,
                [&](size_t i, SkGlyphDigest digest, SkPoint pos) SK_REQUIRES(fMu) {
                    // If the glyph is too large, then no image is created.
                    SkGlyph* glyph = fGlyphForIndex[digest.index()];
                    auto [image, imageSize] = this->prepareImage(glyph);
                    if (image != nullptr) {
                        accepted->accept(glyph, i);
                        increase += imageSize;
                    }
                });

    }

    this->updateDelta(increase);
}

// Note: this does not actually fill out the image. That happens at atlas building time.
SkRect SkStrike::prepareForMaskDrawing(SkDrawableGlyphBuffer* accepted,
                                       SkSourceGlyphBuffer* rejected) {
    SkGlyphRect boundingRect = skglyph::empty_rect();
    size_t increase = 0;
    {
        SkAutoMutexExclusive lock{fMu};

        for (auto [i, packedID, pos] : SkMakeEnumerate(accepted->input())) {
            if (SkScalarsAreFinite(pos.x(), pos.y())) {
                auto [digest, glyphIncrease] = this->digest(packedID);
                increase += glyphIncrease;
                // N.B. this must have the same behavior as RemoteStrike::prepareForMaskDrawing.
                if (!digest.isEmpty()) {
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
    }

    this->updateDelta(increase);
    return boundingRect.rect();
}

#if !defined(SK_DISABLE_SDF_TEXT)
SkRect SkStrike::prepareForSDFTDrawing(SkDrawableGlyphBuffer* accepted,
                                       SkSourceGlyphBuffer* rejected) {
    SkGlyphRect boundingRect = skglyph::empty_rect();
    size_t increase = 0;
    {
        SkAutoMutexExclusive lock{fMu};
        for (auto [i, packedID, pos] : SkMakeEnumerate(accepted->input())) {
            if (SkScalarsAreFinite(pos.x(), pos.y())) {
                auto [digest, glyphIncrease] = this->digest(packedID);
                increase += glyphIncrease;
                // N.B. this must have the same behavior as RemoteStrike::prepareForSDFTDrawing.
                if (!digest.isEmpty()) {
                    if (digest.canDrawAsSDFT()) {
                        const SkGlyphRect glyphBounds =
                                digest.bounds()
                                    // The SDFT glyphs have 2-pixel wide padding that should
                                    // not be used in calculating the source rectangle.
                                    .inset(SK_DistanceFieldInset, SK_DistanceFieldInset)
                                    .offset(pos);
                        boundingRect = skglyph::rect_union(boundingRect, glyphBounds);
                        accepted->accept(packedID, glyphBounds.leftTop(), digest.maskFormat());
                    } else {
                        // Assume whatever follows SDF doesn't care about the maximum rejected size.
                        rejected->reject(i);
                    }
                }
            }
        }
    }

    this->updateDelta(increase);
    return boundingRect.rect();
}
#endif

void SkStrike::prepareForPathDrawing(SkDrawableGlyphBuffer* accepted,
                                     SkSourceGlyphBuffer* rejected) {
    size_t increase = 0;
    {
        SkAutoMutexExclusive lock{fMu};
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
    }

    this->updateDelta(increase);
}

void SkStrike::prepareForDrawableDrawing(SkDrawableGlyphBuffer* accepted,
                                         SkSourceGlyphBuffer* rejected) {
    size_t increase = 0;
    {
        SkAutoMutexExclusive lock{fMu};
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
    }

    this->updateDelta(increase);
}

SkScalar SkStrike::findMaximumGlyphDimension(SkSpan<const SkGlyphID> glyphs) {
    size_t increase = 0;
    SkScalar maxDimension = 0;
    {
        SkAutoMutexExclusive lock{fMu};
        for (SkGlyphID glyphID : glyphs) {
            auto [digest, glyphIncrease] = this->digest(SkPackedGlyphID{glyphID});
            increase += glyphIncrease;
            maxDimension = std::max(static_cast<SkScalar>(digest.maxDimension()), maxDimension);
        }
    }

    this->updateDelta(increase);
    return maxDimension;
}

void SkStrike::glyphIDsToPaths(SkSpan<sktext::IDOrPath> idsOrPaths) {
    size_t increase = 0;
    {
        SkAutoMutexExclusive lock{fMu};
        for (sktext::IDOrPath& idOrPath : idsOrPaths) {
            auto [glyph, size] = this->glyph(SkPackedGlyphID{idOrPath.fGlyphID});
            increase += size;
            increase += this->preparePath(glyph);
            new (&idOrPath.fPath) SkPath{*glyph->path()};
        }
    }

    this->updateDelta(increase);
}

void SkStrike::glyphIDsToDrawables(SkSpan<sktext::IDOrDrawable> idsOrDrawables) {
    size_t increase = 0;
    {
        SkAutoMutexExclusive lock{fMu};
        for (sktext::IDOrDrawable& idOrDrawable : idsOrDrawables) {
            auto [glyph, size] = this->glyph(SkPackedGlyphID{idOrDrawable.fGlyphID});
            increase += size;
            increase += this->prepareDrawable(glyph);
            SkASSERT(glyph->drawable() != nullptr);
            idOrDrawable.fDrawable = glyph->drawable();
        }
    }

    this->updateDelta(increase);
}

void SkStrike::dump() const {
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

template <typename Fn>
size_t SkStrike::commonFilterLoop(SkDrawableGlyphBuffer* accepted, Fn&& fn) {
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

std::tuple<SkGlyph*, size_t> SkStrike::glyph(SkPackedGlyphID packedGlyphID) {
    auto [digest, size] = this->digest(packedGlyphID);
    return {fGlyphForIndex[digest.index()], size};
}

std::tuple<SkGlyphDigest, size_t> SkStrike::digest(SkPackedGlyphID packedGlyphID) {
    SkGlyphDigest* digest = fDigestForPackedGlyphID.find(packedGlyphID);

    if (digest != nullptr) {
        return {*digest, 0};
    }

    SkGlyph* glyph = fAlloc.make<SkGlyph>(fScalerContext->makeGlyph(packedGlyphID, &fAlloc));
    return {this->addGlyph(glyph), sizeof(SkGlyph)};
}

SkGlyphDigest SkStrike::addGlyph(SkGlyph* glyph) {
    size_t index = fGlyphForIndex.size();
    SkGlyphDigest digest = SkGlyphDigest{index, *glyph};
    fDigestForPackedGlyphID.set(glyph->getPackedID(), digest);
    fGlyphForIndex.push_back(glyph);
    return digest;
}

std::tuple<const void*, size_t> SkStrike::prepareImage(SkGlyph* glyph) {
    size_t delta = 0;
    if (glyph->setImage(&fAlloc, fScalerContext.get())) {
        delta = glyph->imageSize();
    }
    return {glyph->image(), delta};
}

size_t SkStrike::preparePath(SkGlyph* glyph) {
    size_t delta = 0;
    if (glyph->setPath(&fAlloc, fScalerContext.get())) {
        delta = glyph->path()->approximateBytesUsed();
    }
    return delta;
}

size_t SkStrike::prepareDrawable(SkGlyph* glyph) {
    size_t delta = 0;
    if (glyph->setDrawable(&fAlloc, fScalerContext.get())) {
        delta = glyph->drawable()->approximateBytesUsed();
        SkASSERT(delta > 0);
    }
    return delta;
}

int SkStrike::countCachedGlyphs() const {
    SkAutoMutexExclusive lock(fMu);
    return fDigestForPackedGlyphID.count();
}

std::tuple<SkSpan<const SkGlyph*>, size_t> SkStrike::internalPrepare(
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


#if SK_SUPPORT_GPU
sk_sp<sktext::gpu::TextStrike> SkStrike::findOrCreateTextStrike(
        sktext::gpu::StrikeCache* gpuStrikeCache) const {
    return gpuStrikeCache->findOrCreateStrike(fStrikeSpec);
}
#endif

void SkStrike::updateDelta(size_t increase) {
    if (increase != 0) {
        SkAutoMutexExclusive lock{fStrikeCache->fLock};
        fMemoryUsed += increase;
        if (!fRemoved) {
            fStrikeCache->fTotalMemoryUsed += increase;
        }
    }
}
