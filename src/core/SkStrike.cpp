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
#include "src/core/SkGlyph.h"
#include "src/core/SkGlyphBuffer.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrikeCache.h"
#include "src/text/StrikeForGPU.h"

#if SK_SUPPORT_GPU
    #include "src/text/gpu/StrikeCache.h"
#endif

using namespace skglyph;

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
        fStrike->lock();
    }

    ~Monitor() SK_RELEASE_CAPABILITY() {
        fStrike->unlock();
    }

private:
    SkStrike* const fStrike;
};

void SkStrike::lock() {
    fStrikeLock.acquire();
    fMemoryIncrease = 0;
}

void SkStrike::unlock() {
    const size_t memoryIncrease = fMemoryIncrease;
    fStrikeLock.release();
    this->updateMemoryUsage(memoryIncrease);
}

SkGlyph* SkStrike::mergeGlyphAndImage(SkPackedGlyphID toID, const SkGlyph& fromGlyph) {
    Monitor m{this};
    // TODO(herb): remove finding the glyph when setting the metrics and image are separated
    SkGlyphDigest* digest = fDigestForPackedGlyphID.find(toID);
    if (digest != nullptr) {
        SkGlyph* glyph = fGlyphForIndex[digest->index()];
        if (fromGlyph.setImageHasBeenCalled()) {
            if (glyph->setImageHasBeenCalled()) {
                // Should never set an image on a glyph which already has an image.
                SkDEBUGFAIL("Re-adding image to existing glyph. This should not happen.");
            }
            // TODO: assert that any metrics on fromGlyph are the same.
            fMemoryIncrease += glyph->setMetricsAndImage(&fAlloc, fromGlyph);
        }
        return glyph;
    } else {
        SkGlyph* glyph = fAlloc.make<SkGlyph>(toID);
        fMemoryIncrease += glyph->setMetricsAndImage(&fAlloc, fromGlyph) + sizeof(SkGlyph);
        (void)this->addGlyph(glyph);
        return glyph;
    }
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
    Monitor m{this};
    return this->internalPrepare(glyphIDs, kMetricsOnly, results);
}

SkSpan<const SkGlyph*> SkStrike::preparePaths(
        SkSpan<const SkGlyphID> glyphIDs, const SkGlyph* results[]) {
    Monitor m{this};
    return this->internalPrepare(glyphIDs, kMetricsAndPath, results);
}

SkSpan<const SkGlyph*> SkStrike::prepareImages(
        SkSpan<const SkPackedGlyphID> glyphIDs, const SkGlyph* results[]) {
    const SkGlyph** cursor = results;
    Monitor m{this};
    for (auto glyphID : glyphIDs) {
        SkGlyph* glyph = this->glyph(glyphID);
        (void)this->prepareImage(glyph);
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
            SkGlyph* glyph = this->glyph(SkPackedGlyphID{glyphID});
            this->prepareDrawable(glyph);
            *cursor++ = glyph;
        }
    }

    return {results, glyphIDs.size()};
}

void SkStrike::prepareForDrawingMasksCPU(SkDrawableGlyphBuffer* accepted) {
    Monitor m{this};
    for (auto [i, packedID, pos] : SkMakeEnumerate(accepted->input())) {
        if (SkScalarsAreFinite(pos.x(), pos.y())) {
            SkGlyphDigest digest = this->digest(packedID);
            if (!digest.isEmpty()) {
                // If the glyph is too large, then no image is created.
                SkGlyph* glyph = fGlyphForIndex[digest.index()];
                const void* image = this->prepareImage(glyph);
                if (image != nullptr) {
                    accepted->accept(glyph, i);
                }
            }
        }
    }
}

void SkStrike::glyphIDsToPaths(SkSpan<sktext::IDOrPath> idsOrPaths) {
    Monitor m{this};
    for (sktext::IDOrPath& idOrPath : idsOrPaths) {
        SkGlyph* glyph = this->glyph(SkPackedGlyphID{idOrPath.fGlyphID});
        this->preparePath(glyph);
        new (&idOrPath.fPath) SkPath{*glyph->path()};
    }
}

void SkStrike::glyphIDsToDrawables(SkSpan<sktext::IDOrDrawable> idsOrDrawables) {
    Monitor m{this};
    for (sktext::IDOrDrawable& idOrDrawable : idsOrDrawables) {
        SkGlyph* glyph = this->glyph(SkPackedGlyphID{idOrDrawable.fGlyphID});
        this->prepareDrawable(glyph);
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

SkGlyph* SkStrike::glyph(SkPackedGlyphID packedGlyphID) {
    SkGlyphDigest digest = this->digest(packedGlyphID);
    return fGlyphForIndex[digest.index()];
}

SkGlyphDigest SkStrike::digest(SkPackedGlyphID packedGlyphID) {
    return *this->digestPtr(packedGlyphID);
}

SkGlyphDigest* SkStrike::digestPtr(SkPackedGlyphID packedGlyphID) {
    SkGlyphDigest* digest = fDigestForPackedGlyphID.find(packedGlyphID);

    if (digest != nullptr) {
        return digest;
    }

    SkGlyph* glyph = fAlloc.make<SkGlyph>(fScalerContext->makeGlyph(packedGlyphID, &fAlloc));
    fMemoryIncrease += sizeof(SkGlyph);
    return this->addGlyph(glyph);
}

GlyphAction SkStrike::pathAction(SkGlyphID glyphID) {
    SkGlyphDigest* const digestPtr = this->digestPtr(SkPackedGlyphID{glyphID});
    if (const GlyphAction action = digestPtr->pathAction(); action != GlyphAction::kUnset) {
        return action;
    }

    GlyphAction action;
    if (digestPtr->isEmpty()) {
        action = GlyphAction::kDrop;
    } else {
        SkGlyph* glyph = fGlyphForIndex[digestPtr->index()];
        this->preparePath(glyph);
        if (glyph->path() != nullptr) {
            action = GlyphAction::kAccept;
        } else {
            action = GlyphAction::kReject;
        }
    }

    digestPtr->setPathAction(action);
    return digestPtr->pathAction();
}

GlyphAction SkStrike::drawableAction(SkGlyphID glyphID) {
    SkGlyphDigest* const digestPtr = this->digestPtr(SkPackedGlyphID{glyphID});
    if (const GlyphAction action = digestPtr->drawableAction(); action != GlyphAction::kUnset) {
        return action;
    }

    GlyphAction action;
    if (digestPtr->isEmpty()) {
        action = GlyphAction::kDrop;
    } else {
        SkGlyph* glyph = fGlyphForIndex[digestPtr->index()];
        this->prepareDrawable(glyph);
        if (glyph->drawable()  != nullptr) {
            action = GlyphAction::kAccept;
        } else {
            action = GlyphAction::kReject;
        }
    }

    digestPtr->setDrawableAction(action);
    return digestPtr->drawableAction();
}

SkGlyphDigest SkStrike::directMaskDigest(SkPackedGlyphID packedGlyphID) {
    SkGlyphDigest* const digestPtr = this->digestPtr(packedGlyphID);
    if (digestPtr->directMaskAction() != GlyphAction::kUnset) {
        return *digestPtr;
    }

    GlyphAction action;
    if (digestPtr->isEmpty()) {
        action = GlyphAction::kDrop;
    } else {
        if (digestPtr->fitsInAtlasDirect()) {
            action = GlyphAction::kAccept;
        } else {
            action = GlyphAction::kReject;
        }
    }

    digestPtr->setDirectMaskAction(action);
    return *digestPtr;
}

SkGlyphDigest SkStrike::sdftDigest(SkGlyphID glyphID) {
    SkGlyphDigest* const digestPtr = this->digestPtr(SkPackedGlyphID{glyphID});
    if (digestPtr->SDFTAction() != GlyphAction::kUnset) {
        return *digestPtr;
    }

    GlyphAction action;
    if (digestPtr->isEmpty()) {
        action = GlyphAction::kDrop;
    } else {
        if (digestPtr->fitsInAtlasDirect() &&
            digestPtr->maskFormat() == SkMask::Format::kSDF_Format) {
            action = GlyphAction::kAccept;
        } else {
            action = GlyphAction::kReject;
        }
    }

    digestPtr->setSDFTAction(action);
    return *digestPtr;
}

SkGlyphDigest SkStrike::maskDigest(SkGlyphID glyphID) {
    SkGlyphDigest* const digestPtr = this->digestPtr(SkPackedGlyphID{glyphID});
    if (digestPtr->maskAction() != GlyphAction::kUnset) {
        return *digestPtr;
    }

    GlyphAction action;
    if (digestPtr->isEmpty()) {
        action = GlyphAction::kDrop;
    } else {
        if (digestPtr->fitsInAtlasDirect()) {
            action = GlyphAction::kAccept;
        } else {
            action = GlyphAction::kReject;
        }
    }

    digestPtr->setMaskAction(action);
    return *digestPtr;
}

SkGlyphDigest* SkStrike::addGlyph(SkGlyph* glyph) {
    size_t index = fGlyphForIndex.size();
    SkGlyphDigest digest = SkGlyphDigest{index, *glyph};
    SkGlyphDigest* newDigest = fDigestForPackedGlyphID.set(glyph->getPackedID(), digest);
    fGlyphForIndex.push_back(glyph);
    return newDigest;
}

const void* SkStrike::prepareImage(SkGlyph* glyph) {
    if (glyph->setImage(&fAlloc, fScalerContext.get())) {
        fMemoryIncrease += glyph->imageSize();
    }
    return glyph->image();
}

void SkStrike::preparePath(SkGlyph* glyph) {
    if (glyph->setPath(&fAlloc, fScalerContext.get())) {
        fMemoryIncrease += glyph->path()->approximateBytesUsed();
    }
}

void SkStrike::prepareDrawable(SkGlyph* glyph) {
    if (glyph->setDrawable(&fAlloc, fScalerContext.get())) {
        size_t increase = glyph->drawable()->approximateBytesUsed();
        SkASSERT(increase > 0);
        fMemoryIncrease += increase;
    }
}

SkSpan<const SkGlyph*> SkStrike::internalPrepare(
        SkSpan<const SkGlyphID> glyphIDs, PathDetail pathDetail, const SkGlyph** results) {
    const SkGlyph** cursor = results;
    for (auto glyphID : glyphIDs) {
        SkGlyph* glyph = this->glyph(SkPackedGlyphID{glyphID});
        if (pathDetail == kMetricsAndPath) {
            this->preparePath(glyph);
        }
        *cursor++ = glyph;
    }

    return {results, glyphIDs.size()};
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
