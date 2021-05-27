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
    SkGlyphDigest* digest = fDigestForPackedGlyphID.find(packedGlyphID.value());

    if (digest != nullptr) {
        return {*digest, 0};
    }

    SkGlyph* glyph = fAlloc.make<SkGlyph>(fScalerContext->makeGlyph(packedGlyphID, &fAlloc));
    return {this->addGlyph(glyph), sizeof(SkGlyph)};
}

SkGlyphDigest SkScalerCache::addGlyph(SkGlyph* glyph) {
    size_t index = fGlyphForIndex.size();
    SkGlyphDigest digest = SkGlyphDigest{index, *glyph};
    fDigestForPackedGlyphID.set(digest);
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
    if (glyph->setPathHasBeenCalled()) {
        SkDEBUGFAIL("Re-adding path to existing glyph. This should not happen.");
    }
    if (glyph->setPath(&fAlloc, path, hairline)) {
        pathDelta = glyph->path()->approximateBytesUsed();
    }
    return {glyph->path(), pathDelta};
}

std::tuple<SkDrawable*, size_t> SkScalerCache::prepareDrawable(SkGlyph* glyph) {
    size_t delta = 0;
    if (glyph->setDrawable(&fAlloc, fScalerContext.get())) {
        delta = glyph->drawable()->approximateBytesUsed();
    }
    return {glyph->drawable(), delta};
}

std::tuple<SkDrawable*, size_t> SkScalerCache::mergeDrawable(SkGlyph* glyph,
                                                             sk_sp<SkDrawable> drawable) {
    SkAutoMutexExclusive lock{fMu};
    size_t drawableDelta = 0;
    if (glyph->setDrawableHasBeenCalled()) {
        SkDEBUGFAIL("Re-adding drawable to existing glyph. This should not happen.");
    }
    if (glyph->setDrawable(&fAlloc, std::move(drawable))) {
        drawableDelta = glyph->drawable()->approximateBytesUsed();
        SkASSERT(drawableDelta > 0);
    }
    return {glyph->drawable(), drawableDelta};
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
    // TODO(herb): remove finding the glyph when setting the metrics and image are separated
    SkGlyphDigest* digest = fDigestForPackedGlyphID.find(toID.value());
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
size_t SkScalerCache::prepareForMaskDrawing(
        SkDrawableGlyphBuffer* accepted, SkSourceGlyphBuffer* rejected) {
    SkAutoMutexExclusive lock{fMu};
    size_t delta = this->commonFilterLoop(accepted,
        [&](size_t i, SkGlyphDigest digest, SkPoint pos) SK_REQUIRES(fMu) {
            // N.B. this must have the same behavior as RemoteStrike::prepareForMaskDrawing.
            if (digest.canDrawAsMask()) {
                accepted->accept(fGlyphForIndex[digest.index()], i);
            } else {
                rejected->reject(i, digest.maxDimension());
            }
        });

    return delta;
}

size_t SkScalerCache::prepareForSDFTDrawing(
        SkDrawableGlyphBuffer* accepted, SkSourceGlyphBuffer* rejected) {
    SkAutoMutexExclusive lock{fMu};
    size_t delta = this->commonFilterLoop(accepted,
        [&](size_t i, SkGlyphDigest digest, SkPoint pos) SK_REQUIRES(fMu) {
            if (digest.canDrawAsSDFT()) {
                accepted->accept(fGlyphForIndex[digest.index()], i);
            } else {
                // Assume whatever follows SDF doesn't care about the maximum rejected size.
                rejected->reject(i);
            }
        });

    return delta;
}

size_t SkScalerCache::prepareForPathDrawing(
        SkDrawableGlyphBuffer* accepted, SkSourceGlyphBuffer* rejected) {
    SkAutoMutexExclusive lock{fMu};
    size_t pathDelta = 0;
    size_t delta = this->commonFilterLoop(accepted,
        [&](size_t i, SkGlyphDigest digest, SkPoint pos) SK_REQUIRES(fMu) {
            SkGlyph* glyph = fGlyphForIndex[digest.index()];
            auto [path, pathSize] = this->preparePath(glyph);
            pathDelta += pathSize;
            if (path != nullptr) {
                // Save off the path to draw later.
                accepted->accept(path, i);
            } else {
                // Glyph does not have a path.
                rejected->reject(i, digest.maxDimension());
            }
        });

    return delta + pathDelta;
}

size_t SkScalerCache::prepareForDrawableDrawing(
        SkDrawableGlyphBuffer* accepted, SkSourceGlyphBuffer* rejected) {
    SkAutoMutexExclusive lock{fMu};
    size_t drawableDelta = 0;
    size_t delta = this->commonFilterLoop(accepted,
        [&](size_t i, SkGlyphDigest digest, SkPoint pos) SK_REQUIRES(fMu) {
            SkGlyph* glyph = fGlyphForIndex[digest.index()];
            auto [drawable, drawableSize] = this->prepareDrawable(glyph);
            drawableDelta += drawableSize;
            if (drawable != nullptr) {
                // Save off the drawable to draw later.
                accepted->accept(drawable, i);
            } else {
                // Glyph does not have a drawable.
                rejected->reject(i, glyph->maxDimension());
            }
        });

    return delta + drawableDelta;
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

