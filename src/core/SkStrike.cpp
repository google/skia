/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkStrike.h"

#include "include/core/SkDrawable.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkString.h"
#include "include/core/SkTraceMemoryDump.h"
#include "include/core/SkTypeface.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTFitsIn.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMask.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkWriteBuffer.h"
#include "src/text/StrikeForGPU.h"

#include <cctype>
#include <new>
#include <optional>
#include <utility>

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

void
SkStrike::FlattenGlyphsByType(SkWriteBuffer& buffer,
                              SkSpan<SkGlyph> images,
                              SkSpan<SkGlyph> paths,
                              SkSpan<SkGlyph> drawables) {
    SkASSERT_RELEASE(SkTFitsIn<int>(images.size()) &&
                     SkTFitsIn<int>(paths.size()) &&
                     SkTFitsIn<int>(drawables.size()));

    buffer.writeInt(images.size());
    for (SkGlyph& glyph : images) {
        SkASSERT(SkMask::IsValidFormat(glyph.maskFormat()));
        glyph.flattenMetrics(buffer);
        glyph.flattenImage(buffer);
    }

    buffer.writeInt(paths.size());
    for (SkGlyph& glyph : paths) {
        SkASSERT(SkMask::IsValidFormat(glyph.maskFormat()));
        glyph.flattenMetrics(buffer);
        glyph.flattenPath(buffer);
    }

    buffer.writeInt(drawables.size());
    for (SkGlyph& glyph : drawables) {
        SkASSERT(SkMask::IsValidFormat(glyph.maskFormat()));
        glyph.flattenMetrics(buffer);
        glyph.flattenDrawable(buffer);
    }
}

bool SkStrike::mergeFromBuffer(SkReadBuffer& buffer) {
    // Read glyphs with images for the current strike.
    const int imagesCount = buffer.readInt();
    if (imagesCount == 0 && !buffer.isValid()) {
        return false;
    }

    {
        Monitor m{this};
        for (int curImage = 0; curImage < imagesCount; ++curImage) {
            if (!this->mergeGlyphAndImageFromBuffer(buffer)) {
                return false;
            }
        }
    }

    // Read glyphs with paths for the current strike.
    const int pathsCount = buffer.readInt();
    if (pathsCount == 0 && !buffer.isValid()) {
        return false;
    }
    {
        Monitor m{this};
        for (int curPath = 0; curPath < pathsCount; ++curPath) {
            if (!this->mergeGlyphAndPathFromBuffer(buffer)) {
                return false;
            }
        }
    }

    // Read glyphs with drawables for the current strike.
    const int drawablesCount = buffer.readInt();
    if (drawablesCount == 0 && !buffer.isValid()) {
        return false;
    }
    {
        Monitor m{this};
        for (int curDrawable = 0; curDrawable < drawablesCount; ++curDrawable) {
            if (!this->mergeGlyphAndDrawableFromBuffer(buffer)) {
                return false;
            }
        }
    }

    return true;
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
        (void)this->addGlyphAndDigest(glyph);
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
        this->prepareForImage(glyph);
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
            this->prepareForDrawable(glyph);
            *cursor++ = glyph;
        }
    }

    return {results, glyphIDs.size()};
}

void SkStrike::glyphIDsToPaths(SkSpan<sktext::IDOrPath> idsOrPaths) {
    Monitor m{this};
    for (sktext::IDOrPath& idOrPath : idsOrPaths) {
        SkGlyph* glyph = this->glyph(SkPackedGlyphID{idOrPath.fGlyphID});
        this->prepareForPath(glyph);
        new (&idOrPath.fPath) SkPath{*glyph->path()};
    }
}

void SkStrike::glyphIDsToDrawables(SkSpan<sktext::IDOrDrawable> idsOrDrawables) {
    Monitor m{this};
    for (sktext::IDOrDrawable& idOrDrawable : idsOrDrawables) {
        SkGlyph* glyph = this->glyph(SkPackedGlyphID{idOrDrawable.fGlyphID});
        this->prepareForDrawable(glyph);
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

    SkString dumpName = SkStringPrintf("%s/%s_%u/%p",
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

SkGlyph* SkStrike::glyph(SkGlyphDigest digest) {
    return fGlyphForIndex[digest.index()];
}

SkGlyph* SkStrike::glyph(SkPackedGlyphID packedGlyphID) {
    SkGlyphDigest digest = this->digestFor(kDirectMask, packedGlyphID);
    return this->glyph(digest);
}

SkGlyphDigest SkStrike::digestFor(ActionType actionType, SkPackedGlyphID packedGlyphID) {
    SkGlyphDigest* digestPtr = fDigestForPackedGlyphID.find(packedGlyphID);
    if (digestPtr != nullptr && digestPtr->actionFor(actionType) != GlyphAction::kUnset) {
        return *digestPtr;
    }

    SkGlyph* glyph;
    if (digestPtr != nullptr) {
        glyph = fGlyphForIndex[digestPtr->index()];
    } else {
        glyph = fAlloc.make<SkGlyph>(fScalerContext->makeGlyph(packedGlyphID, &fAlloc));
        fMemoryIncrease += sizeof(SkGlyph);
        digestPtr = this->addGlyphAndDigest(glyph);
    }

    digestPtr->setActionFor(actionType, glyph, this);

    return *digestPtr;
}

SkGlyphDigest* SkStrike::addGlyphAndDigest(SkGlyph* glyph) {
    size_t index = fGlyphForIndex.size();
    SkGlyphDigest digest = SkGlyphDigest{index, *glyph};
    SkGlyphDigest* newDigest = fDigestForPackedGlyphID.set(digest);
    fGlyphForIndex.push_back(glyph);
    return newDigest;
}

bool SkStrike::prepareForImage(SkGlyph* glyph) {
    if (glyph->setImage(&fAlloc, fScalerContext.get())) {
        fMemoryIncrease += glyph->imageSize();
    }
    return glyph->image() != nullptr;
}

bool SkStrike::prepareForPath(SkGlyph* glyph) {
    if (glyph->setPath(&fAlloc, fScalerContext.get())) {
        fMemoryIncrease += glyph->path()->approximateBytesUsed();
    }
    return glyph->path() !=nullptr;
}

bool SkStrike::prepareForDrawable(SkGlyph* glyph) {
    if (glyph->setDrawable(&fAlloc, fScalerContext.get())) {
        size_t increase = glyph->drawable()->approximateBytesUsed();
        SkASSERT(increase > 0);
        fMemoryIncrease += increase;
    }
    return glyph->drawable() != nullptr;
}

SkGlyph* SkStrike::mergeGlyphFromBuffer(SkReadBuffer& buffer) {
    SkASSERT(buffer.isValid());
    std::optional<SkGlyph> prototypeGlyph = SkGlyph::MakeFromBuffer(buffer);
    if (!buffer.validate(prototypeGlyph.has_value())) {
        return nullptr;
    }

    // Check if this glyph has already been seen.
    SkGlyphDigest* digestPtr = fDigestForPackedGlyphID.find(prototypeGlyph->getPackedID());
    if (digestPtr != nullptr) {
        return fGlyphForIndex[digestPtr->index()];
    }

    // This is the first time. Allocate a new glyph.
    SkGlyph* glyph = fAlloc.make<SkGlyph>(prototypeGlyph.value());
    fMemoryIncrease += sizeof(SkGlyph);
    this->addGlyphAndDigest(glyph);
    return glyph;
}

bool SkStrike::mergeGlyphAndImageFromBuffer(SkReadBuffer& buffer) {
    SkASSERT(buffer.isValid());
    SkGlyph* glyph = this->mergeGlyphFromBuffer(buffer);
    if (!buffer.validate(glyph != nullptr)) {
        return false;
    }
    fMemoryIncrease += glyph->addImageFromBuffer(buffer, &fAlloc);
    return buffer.isValid();
}

bool SkStrike::mergeGlyphAndPathFromBuffer(SkReadBuffer& buffer) {
    SkASSERT(buffer.isValid());
    SkGlyph* glyph = this->mergeGlyphFromBuffer(buffer);
    if (!buffer.validate(glyph != nullptr)) {
        return false;
    }
    fMemoryIncrease += glyph->addPathFromBuffer(buffer, &fAlloc);
    return buffer.isValid();
}

bool SkStrike::mergeGlyphAndDrawableFromBuffer(SkReadBuffer& buffer) {
    SkASSERT(buffer.isValid());
    SkGlyph* glyph = this->mergeGlyphFromBuffer(buffer);
    if (!buffer.validate(glyph != nullptr)) {
        return false;
    }
    fMemoryIncrease += glyph->addDrawableFromBuffer(buffer, &fAlloc);
    return buffer.isValid();
}

SkSpan<const SkGlyph*> SkStrike::internalPrepare(
        SkSpan<const SkGlyphID> glyphIDs, PathDetail pathDetail, const SkGlyph** results) {
    const SkGlyph** cursor = results;
    for (auto glyphID : glyphIDs) {
        SkGlyph* glyph = this->glyph(SkPackedGlyphID{glyphID});
        if (pathDetail == kMetricsAndPath) {
            this->prepareForPath(glyph);
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
