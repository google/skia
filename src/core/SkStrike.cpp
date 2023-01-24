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
#include "include/core/SkTraceMemoryDump.h"
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
        : fFontMetrics{use_or_generate_metrics(metrics, scaler.get())}
        , fRoundingSpec{scaler->isSubpixel(),
                        scaler->computeAxisAlignmentForHText()}
        , fStrikeSpec{strikeSpec}
        , fStrikeCache{strikeCache}
        , fScalerContext{std::move(scaler)}
        , fPinner{std::move(pinner)} {
    SkASSERT(fScalerContext != nullptr);
}

class SK_SCOPED_CAPABILITY SkStrike::Monitor {
public:
    Monitor(SkStrike* strike) SK_ACQUIRE(strike->fStrikeLock)
            : fStrike{strike} {
        fStrike->fStrikeLock.acquire();
        fStrike->fMemoryIncrease = 0;
    }

    ~Monitor() SK_RELEASE_CAPABILITY() {
        const size_t memoryIncrease = fStrike->fMemoryIncrease;
        fStrike->fStrikeLock.release();
        fStrike->updateMemoryUsage(memoryIncrease);
    }

private:
    SkStrike* const fStrike;
};

SkGlyph* SkStrike::mergeGlyphAndImage(SkPackedGlyphID toID, const SkGlyph& fromGlyph) {
    SkGlyph* glyph;
    Monitor m{this};
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
            fMemoryIncrease += glyph->setMetricsAndImage(&fAlloc, fromGlyph);
        }
    } else {
        glyph = fAlloc.make<SkGlyph>(toID);
        fMemoryIncrease += glyph->setMetricsAndImage(&fAlloc, fromGlyph) + sizeof(SkGlyph);
        (void)this->addGlyph(glyph);
    }

    return glyph;
}

const SkPath* SkStrike::mergePath(SkGlyph* glyph, const SkPath* path, bool hairline) {
    Monitor m{this};
    if (glyph->setPathHasBeenCalled()) {
        SkDEBUGFAIL("Re-adding path to existing glyph. This should not happen.");
    }
    if (glyph->setPath(&fAlloc, path, hairline)) {
        fMemoryIncrease += glyph->path()->approximateBytesUsed();
    }

    return glyph->path();
}

const SkDrawable* SkStrike::mergeDrawable(SkGlyph* glyph, sk_sp<SkDrawable> drawable) {
    Monitor m{this};
    if (glyph->setDrawableHasBeenCalled()) {
        SkDEBUGFAIL("Re-adding drawable to existing glyph. This should not happen.");
    }
    if (glyph->setDrawable(&fAlloc, std::move(drawable))) {
        fMemoryIncrease += glyph->drawable()->approximateBytesUsed();
        SkASSERT(fMemoryIncrease > 0);
    }

    return glyph->drawable();
}

void SkStrike::findIntercepts(const SkScalar bounds[2], SkScalar scale, SkScalar xPos,
                              SkGlyph* glyph, SkScalar* array, int* count) {
    SkAutoMutexExclusive lock{fStrikeLock};
    glyph->ensureIntercepts(bounds, scale, xPos, array, count, &fAlloc);
}

SkSpan<const SkGlyph*> SkStrike::metrics(
        SkSpan<const SkGlyphID> glyphIDs, const SkGlyph* results[]) {
    SkSpan<const SkGlyph*> glyphs;
    Monitor m{this};
    size_t increase;
    std::tie(glyphs, increase)= this->internalPrepare(glyphIDs, kMetricsOnly, results);
    fMemoryIncrease += increase;

    return glyphs;
}

SkSpan<const SkGlyph*> SkStrike::preparePaths(
        SkSpan<const SkGlyphID> glyphIDs, const SkGlyph* results[]) {
    SkSpan<const SkGlyph*> glyphs;
    Monitor m{this};
    size_t increase;
    std::tie(glyphs, increase) = this->internalPrepare(glyphIDs, kMetricsAndPath, results);
    fMemoryIncrease += increase;

    return glyphs;
}

SkSpan<const SkGlyph*> SkStrike::prepareImages(
        SkSpan<const SkPackedGlyphID> glyphIDs, const SkGlyph* results[]) {
    const SkGlyph** cursor = results;
    Monitor m{this};
    for (auto glyphID : glyphIDs) {
        auto[glyph, glyphSize] = this->glyph(glyphID);
        auto[_, imageSize] = this->prepareImage(glyph);
        fMemoryIncrease += glyphSize + imageSize;
        *cursor++ = glyph;
    }

    return {results, glyphIDs.size()};
}

SkSpan<const SkGlyph*> SkStrike::prepareDrawables(
        SkSpan<const SkGlyphID> glyphIDs, const SkGlyph* results[]) {
    const SkGlyph** cursor = results;
    {
        Monitor m{this};
        for (auto glyphID : glyphIDs) {
            auto[glyph, glyphSize] = this->glyph(SkPackedGlyphID{glyphID});
            size_t drawableSize = this->prepareDrawable(glyph);
            fMemoryIncrease += glyphSize + drawableSize;
            *cursor++ = glyph;
        }
    }

    return {results, glyphIDs.size()};
}

void SkStrike::prepareForDrawingMasksCPU(SkDrawableGlyphBuffer* accepted) {
    Monitor m{this};
    fMemoryIncrease += this->commonFilterLoop(
        accepted,
        [&](size_t i, SkGlyphDigest digest, SkPoint pos) SK_REQUIRES(fStrikeLock) {
            // If the glyph is too large, then no image is created.
            SkGlyph* glyph = fGlyphForIndex[digest.index()];
            auto [image, imageSize] = this->prepareImage(glyph);
            if (image != nullptr) {
                accepted->accept(glyph, i);
                fMemoryIncrease += imageSize;
            }
        });

}

// Note: this does not actually fill out the image. That happens at atlas building time.
SkRect SkStrike::prepareForMaskDrawing(SkDrawableGlyphBuffer* accepted,
                                       SkSourceGlyphBuffer* rejected) {
    SkGlyphRect boundingRect = skglyph::empty_rect();
    Monitor m{this};
    for (auto [i, packedID, pos] : SkMakeEnumerate(accepted->input())) {
        if (SkScalarsAreFinite(pos.x(), pos.y())) {
            auto [digest, glyphIncrease] = this->digest(packedID);
            fMemoryIncrease += glyphIncrease;
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

    return boundingRect.rect();
}

#if !defined(SK_DISABLE_SDF_TEXT)
SkRect SkStrike::prepareForSDFTDrawing(SkDrawableGlyphBuffer* accepted,
                                       SkSourceGlyphBuffer* rejected) {
    SkGlyphRect boundingRect = skglyph::empty_rect();
    Monitor m{this};
    for (auto [i, packedID, pos] : SkMakeEnumerate(accepted->input())) {
        if (SkScalarsAreFinite(pos.x(), pos.y())) {
            auto [digest, glyphIncrease] = this->digest(packedID);
            fMemoryIncrease += glyphIncrease;
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

    return boundingRect.rect();
}
#endif

void SkStrike::prepareForPathDrawing(SkDrawableGlyphBuffer* accepted,
                                     SkSourceGlyphBuffer* rejected) {
    Monitor m{this};
    for (auto [i, packedID, pos] : SkMakeEnumerate(accepted->input())) {
        if (SkScalarsAreFinite(pos.x(), pos.y())) {
            auto [digest, glyphIncrease] = this->digest(packedID);
            fMemoryIncrease += glyphIncrease;
            if (!digest.isEmpty()) {
                SkGlyph* glyph = fGlyphForIndex[digest.index()];
                fMemoryIncrease += this->preparePath(glyph);
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

void SkStrike::prepareForDrawableDrawing(SkDrawableGlyphBuffer* accepted,
                                         SkSourceGlyphBuffer* rejected) {
    Monitor m{this};
    for (auto [i, packedID, pos] : SkMakeEnumerate(accepted->input())) {
        if (SkScalarsAreFinite(pos.x(), pos.y())) {
            auto [digest, glyphIncrease] = this->digest(packedID);
            fMemoryIncrease += glyphIncrease;
            if (!digest.isEmpty()) {
                SkGlyph* glyph = fGlyphForIndex[digest.index()];
                fMemoryIncrease += this->prepareDrawable(glyph);
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

SkScalar SkStrike::findMaximumGlyphDimension(SkSpan<const SkGlyphID> glyphs) {
    SkScalar maxDimension = 0;
    Monitor m{this};
    for (SkGlyphID glyphID : glyphs) {
        auto [digest, glyphIncrease] = this->digest(SkPackedGlyphID{glyphID});
        fMemoryIncrease += glyphIncrease;
        maxDimension = std::max(static_cast<SkScalar>(digest.maxDimension()), maxDimension);
    }

    return maxDimension;
}

void SkStrike::glyphIDsToPaths(SkSpan<sktext::IDOrPath> idsOrPaths) {
    Monitor m{this};
    for (sktext::IDOrPath& idOrPath : idsOrPaths) {
        auto [glyph, size] = this->glyph(SkPackedGlyphID{idOrPath.fGlyphID});
        fMemoryIncrease += size;
        fMemoryIncrease += this->preparePath(glyph);
        new (&idOrPath.fPath) SkPath{*glyph->path()};
    }
}

void SkStrike::glyphIDsToDrawables(SkSpan<sktext::IDOrDrawable> idsOrDrawables) {
    Monitor m{this};
    for (sktext::IDOrDrawable& idOrDrawable : idsOrDrawables) {
        auto [glyph, size] = this->glyph(SkPackedGlyphID{idOrDrawable.fGlyphID});
        fMemoryIncrease += size;
        fMemoryIncrease += this->prepareDrawable(glyph);
        SkASSERT(glyph->drawable() != nullptr);
        idOrDrawable.fDrawable = glyph->drawable();
    }
}

void SkStrike::dump() const {
    SkAutoMutexExclusive lock{fStrikeLock};
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

void SkStrike::dumpMemoryStatistics(SkTraceMemoryDump* dump) const {
    SkAutoMutexExclusive lock{fStrikeLock};
    const SkTypeface* face = fScalerContext->getTypeface();
    const SkScalerContextRec& rec = fScalerContext->getRec();

    SkString fontName;
    face->getFamilyName(&fontName);
    // Replace all special characters with '_'.
    for (size_t index = 0; index < fontName.size(); ++index) {
        if (!std::isalnum(fontName[index])) {
            fontName[index] = '_';
        }
    }

    SkString dumpName = SkStringPrintf("%s/%s_%d/%p",
                                       SkStrikeCache::kGlyphCacheDumpName,
                                       fontName.c_str(),
                                       rec.fTypefaceID,
                                       this);

    dump->dumpNumericValue(dumpName.c_str(), "size", "bytes", fMemoryUsed);
    dump->dumpNumericValue(dumpName.c_str(),
                           "glyph_count", "objects",
                           fDigestForPackedGlyphID.count());
    dump->setMemoryBacking(dumpName.c_str(), "malloc", nullptr);
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

void SkStrike::updateMemoryUsage(size_t increase) {
    if (increase > 0) {
        // fRemoved and the cache's total memory are managed under the cache's lock. This allows
        // them to be accessed under LRU operation.
        SkAutoMutexExclusive lock{fStrikeCache->fLock};
        fMemoryUsed += increase;
        if (!fRemoved) {
            fStrikeCache->fTotalMemoryUsed += increase;
        }
    }
}
