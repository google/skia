/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkStrike.h"

#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkMutex.h"
#include "include/private/SkOnce.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkMakeUnique.h"
#include <cctype>

SkStrike::SkStrike(
    const SkDescriptor& desc,
    std::unique_ptr<SkScalerContext> scaler,
    const SkFontMetrics& fontMetrics)
        : fDesc{desc}
        , fScalerContext{std::move(scaler)}
        , fFontMetrics{fontMetrics}
        , fRoundingSpec{fScalerContext->isSubpixel(),
                        fScalerContext->computeAxisAlignmentForHText()} {
    SkASSERT(fScalerContext != nullptr);
    fMemoryUsed = sizeof(*this);
}

#ifdef SK_DEBUG
#define VALIDATE()  AutoValidate av(this)
#else
#define VALIDATE()
#endif

// -- glyph creation -------------------------------------------------------------------------------
SkGlyph* SkStrike::makeGlyph(SkPackedGlyphID packedGlyphID) {
    fMemoryUsed += sizeof(SkGlyph);
    SkGlyph* glyph = fAlloc.make<SkGlyph>(packedGlyphID);
    fGlyphMap.set(glyph);
    return glyph;
}

SkGlyph* SkStrike::glyph(SkPackedGlyphID packedGlyphID) {
    VALIDATE();
    SkGlyph* glyph = fGlyphMap.findOrNull(packedGlyphID);
    if (glyph == nullptr) {
        glyph = this->makeGlyph(packedGlyphID);
        fScalerContext->getMetrics(glyph);
    }
    return glyph;
}

SkGlyph* SkStrike::glyph(SkGlyphID glyphID) {
    return this->glyph(SkPackedGlyphID{glyphID});
}

SkGlyph* SkStrike::glyph(SkGlyphID glyphID, SkPoint position) {
    SkIPoint mask = fRoundingSpec.ignorePositionMask;
    SkFixed subX = SkScalarToFixed(position.x()) & mask.x(),
            subY = SkScalarToFixed(position.y()) & mask.y();
    return this->glyph(SkPackedGlyphID{glyphID, subX, subY});
}

SkGlyph* SkStrike::glyphFromPrototype(const SkGlyphPrototype& p, void* image) {
    SkGlyph* glyph = fGlyphMap.findOrNull(p.id);
    if (glyph == nullptr) {
        fMemoryUsed += sizeof(SkGlyph);
        glyph = fAlloc.make<SkGlyph>(p);
        fGlyphMap.set(glyph);
    }
    if (glyph->setImage(&fAlloc, image)) {
        fMemoryUsed += glyph->imageSize();
    }
    return glyph;
}

SkGlyph* SkStrike::glyphOrNull(SkPackedGlyphID id) const {
    return fGlyphMap.findOrNull(id);
}

const SkPath* SkStrike::preparePath(SkGlyph* glyph) {
    if (glyph->setPath(&fAlloc, fScalerContext.get())) {
        fMemoryUsed += glyph->path()->approximateBytesUsed();
    }
    return glyph->path();
}

const SkPath* SkStrike::preparePath(SkGlyph* glyph, const SkPath* path) {
    if (glyph->setPath(&fAlloc, path)) {
        fMemoryUsed += glyph->path()->approximateBytesUsed();
    }
    return glyph->path();
}

const SkDescriptor& SkStrike::getDescriptor() const {
    return *fDesc.getDesc();
}

unsigned SkStrike::getGlyphCount() const {
    return fScalerContext->getGlyphCount();
}

int SkStrike::countCachedGlyphs() const {
    return fGlyphMap.count();
}

SkSpan<const SkGlyph*> SkStrike::internalPrepare(
        SkSpan<const SkGlyphID> glyphIDs, PathDetail pathDetail, const SkGlyph** results) {
    const SkGlyph** cursor = results;
    for (auto glyphID : glyphIDs) {
        SkGlyph* glyphPtr = this->glyph(glyphID);
        if (pathDetail == kMetricsAndPath) {
            this->preparePath(glyphPtr);
        }
        *cursor++ = glyphPtr;
    }

    return {results, glyphIDs.size()};
}

const void* SkStrike::prepareImage(SkGlyph* glyph) {
    if (glyph->setImage(&fAlloc, fScalerContext.get())) {
        fMemoryUsed += glyph->imageSize();
    }
    return glyph->image();
}

SkGlyph* SkStrike::mergeGlyphAndImage(SkPackedGlyphID toID, const SkGlyph& from) {
    SkGlyph* glyph = fGlyphMap.findOrNull(toID);
    if (glyph == nullptr) {
        glyph = this->makeGlyph(toID);
    }
    if (glyph->setMetricsAndImage(&fAlloc, from)) {
        fMemoryUsed += glyph->imageSize();
    }
    return glyph;
}

bool SkStrike::belongsToCache(const SkGlyph* glyph) const {
    return glyph && fGlyphMap.findOrNull(glyph->getPackedID()) == glyph;
}

const SkGlyph* SkStrike::getCachedGlyphAnySubPix(SkGlyphID glyphID,
                                                     SkPackedGlyphID vetoID) const {
    for (SkFixed subY = 0; subY < SK_Fixed1; subY += SK_FixedQuarter) {
        for (SkFixed subX = 0; subX < SK_Fixed1; subX += SK_FixedQuarter) {
            SkPackedGlyphID packedGlyphID{glyphID, subX, subY};
            if (packedGlyphID == vetoID) continue;
            if (SkGlyph* glyphPtr = fGlyphMap.findOrNull(packedGlyphID)) {
                return glyphPtr;
            }
        }
    }

    return nullptr;
}

SkSpan<const SkGlyph*> SkStrike::metrics(SkSpan<const SkGlyphID> glyphIDs,
                                         const SkGlyph* results[]) {
    return this->internalPrepare(glyphIDs, kMetricsOnly, results);
}

SkSpan<const SkGlyph*> SkStrike::preparePaths(SkSpan<const SkGlyphID> glyphIDs,
                                              const SkGlyph* results[]) {
    return this->internalPrepare(glyphIDs, kMetricsAndPath, results);
}

SkSpan<const SkGlyph*>
SkStrike::prepareImages(SkSpan<const SkPackedGlyphID> glyphIDs, const SkGlyph* results[]) {
    const SkGlyph** cursor = results;
    for (auto glyphID : glyphIDs) {
        SkGlyph* glyphPtr = this->glyph(glyphID);
        (void)this->prepareImage(glyphPtr);
        *cursor++ = glyphPtr;
    }

    return {results, glyphIDs.size()};
}

// N.B. This glyphMetrics call culls all the glyphs which will not display based on a non-finite
// position or that there are no mask pixels.
SkSpan<const SkGlyphPos>
SkStrike::prepareForDrawingRemoveEmpty(const SkPackedGlyphID packedGlyphIDs[],
                                       const SkPoint positions[],
                                       size_t n,
                                       int maxDimension,
                                       SkGlyphPos results[]) {
    size_t drawableGlyphCount = 0;
    for (size_t i = 0; i < n; i++) {
        SkPoint pos = positions[i];
        if (SkScalarsAreFinite(pos.x(), pos.y())) {
            SkGlyph* glyphPtr = this->glyph(packedGlyphIDs[i]);
            if (!glyphPtr->isEmpty()) {
                results[drawableGlyphCount++] = {i, glyphPtr, pos};
                if (glyphPtr->maxDimension() <= maxDimension) {
                    // The glyph fits. Prepare image later.
                } else if (!glyphPtr->isColor()) {
                    // The out of atlas glyph is not color so we can draw it using paths.
                    this->preparePath(glyphPtr);
                } else {
                    // This will be handled by the fallback strike.
                    SkASSERT(glyphPtr->maxDimension() > maxDimension && glyphPtr->isColor());
                }
            }
        }
    }

    return SkSpan<const SkGlyphPos>{results, drawableGlyphCount};
}

void SkStrike::findIntercepts(const SkScalar bounds[2], SkScalar scale, SkScalar xPos,
        SkGlyph* glyph, SkScalar* array, int* count) {
    glyph->ensureIntercepts(bounds, scale, xPos, array, count, &fAlloc);
}

void SkStrike::dump() const {
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
               rec.dump().c_str(), fGlyphMap.count());
    SkDebugf("%s\n", msg.c_str());
}

void SkStrike::onAboutToExitScope() { }

#ifdef SK_DEBUG
void SkStrike::forceValidate() const {
    size_t memoryUsed = sizeof(*this);
    fGlyphMap.foreach ([&memoryUsed](const SkGlyph* glyphPtr) {
        memoryUsed += sizeof(SkGlyph);
        if (glyphPtr->setImageHasBeenCalled()) {
            memoryUsed += glyphPtr->imageSize();
        }
        if (glyphPtr->setPathHasBeenCalled() && glyphPtr->path() != nullptr) {
            memoryUsed += glyphPtr->path()->approximateBytesUsed();
        }
    });
    SkASSERT(fMemoryUsed == memoryUsed);
}

void SkStrike::validate() const {
#ifdef SK_DEBUG_GLYPH_CACHE
    forceValidate();
#endif
}
#endif  // SK_DEBUG


