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
    , fIsSubpixel{fScalerContext->isSubpixel()}
    , fAxisAlignment{fScalerContext->computeAxisAlignmentForHText()}
{
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

SkGlyph* SkStrike::uninitializedGlyph(SkPackedGlyphID id) {
    SkGlyph* glyph = fGlyphMap.findOrNull(id);
    if (glyph == nullptr) {
        glyph = this->makeGlyph(id);
    }
    return glyph;
}

SkGlyph* SkStrike::glyph(SkPackedGlyphID packedGlyphID) {
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

SkGlyph* SkStrike::glyphFromPrototype(const SkGlyphPrototype& p) {
    SkGlyph* glyph = fGlyphMap.findOrNull(p.id);
    if (glyph == nullptr) {
        fMemoryUsed += sizeof(SkGlyph);
        glyph = fAlloc.make<SkGlyph>(p);
        fGlyphMap.set(glyph);
    }
    return glyph;
}

SkGlyph* SkStrike::glyphOrNull(SkPackedGlyphID id) const {
    return fGlyphMap.findOrNull(id);
}

SkGlyph* SkStrike::getRawGlyphByID(SkPackedGlyphID id) {
    return this->uninitializedGlyph(id);
}

const SkGlyph& SkStrike::getGlyphIDMetrics(SkGlyphID glyphID) {
    VALIDATE();
    return *this->glyph(glyphID);
}

const SkGlyph& SkStrike::getGlyphIDMetrics(SkPackedGlyphID id) {
    VALIDATE();
    return *this->glyph(id);
}

const SkGlyph& SkStrike::getGlyphIDMetrics(SkGlyphID glyphID, SkFixed x, SkFixed y) {
    SkPackedGlyphID packedGlyphID(glyphID, x, y);
    return *this->glyph(packedGlyphID);
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

bool SkStrike::isGlyphCached(SkGlyphID glyphID, SkFixed x, SkFixed y) const {
    SkPackedGlyphID packedGlyphID{glyphID, x, y};
    return fGlyphMap.find(packedGlyphID) != nullptr;
}

void SkStrike::getAdvances(SkSpan<const SkGlyphID> glyphIDs, SkPoint advances[]) {
    for (auto glyphID : glyphIDs) {
        auto glyph = this->glyph(glyphID);
        *advances++ = SkPoint::Make(glyph->fAdvanceX, glyph->fAdvanceY);
    }
}

const void* SkStrike::findImage(const SkGlyph& glyph) {
    if (glyph.fWidth > 0 && glyph.fWidth < kMaxGlyphWidth) {
        if (nullptr == glyph.fImage) {
            SkDEBUGCODE(SkMask::Format oldFormat = (SkMask::Format)glyph.fMaskFormat);
            size_t  size = const_cast<SkGlyph&>(glyph).allocImage(&fAlloc);
            // check that alloc() actually succeeded
            if (glyph.fImage) {
                fScalerContext->getImage(glyph);
                // TODO: the scaler may have changed the maskformat during
                // getImage (e.g. from AA or LCD to BW) which means we may have
                // overallocated the buffer. Check if the new computedImageSize
                // is smaller, and if so, strink the alloc size in fImageAlloc.
                fMemoryUsed += size;
            }
            SkASSERT(oldFormat == glyph.fMaskFormat);
        }
    }
    return glyph.fImage;
}

void SkStrike::initializeImage(const volatile void* data, size_t size, SkGlyph* glyph) {
    SkASSERT(!glyph->fImage);

    if (glyph->fWidth > 0 && glyph->fWidth < kMaxGlyphWidth) {
        size_t allocSize = glyph->allocImage(&fAlloc);
        // check that alloc() actually succeeded
        if (glyph->fImage) {
            SkASSERT(size == allocSize);
            memcpy(glyph->fImage, const_cast<const void*>(data), allocSize);
            fMemoryUsed += size;
        }
    }
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

void SkStrike::initializeGlyphFromFallback(SkGlyph* glyph, const SkGlyph& fallback) {
    fMemoryUsed += glyph->copyImageData(fallback, &fAlloc);
}

SkVector SkStrike::rounding() const {
    return SkStrikeCommon::PixelRounding(fIsSubpixel, fAxisAlignment);
}

const SkGlyph& SkStrike::getGlyphMetrics(SkGlyphID glyphID, SkPoint position) {
    if (!fIsSubpixel) {
        return this->getGlyphIDMetrics(glyphID);
    } else {
        SkIPoint lookupPosition = SkStrikeCommon::SubpixelLookup(fAxisAlignment, position);

        return this->getGlyphIDMetrics(glyphID, lookupPosition.x(), lookupPosition.y());
    }
}

// N.B. This glyphMetrics call culls all the glyphs which will not display based on a non-finite
// position or that there are no mask pixels.
SkSpan<const SkGlyphPos> SkStrike::prepareForDrawing(const SkGlyphID glyphIDs[],
                                                     const SkPoint positions[],
                                                     size_t n,
                                                     int maxDimension,
                                                     PreparationDetail detail,
                                                     SkGlyphPos result[]) {
    size_t drawableGlyphCount = 0;
    for (size_t i = 0; i < n; i++) {
        SkPoint position = positions[i];
        if (SkScalarsAreFinite(position.x(), position.y())) {
            // This assumes that the strike has no sub-pixel positioning for glyphs that are
            // transformed from source space to device space.
            const SkGlyph& glyph = this->getGlyphMetrics(glyphIDs[i], position);
            if (!glyph.isEmpty()) {
                result[drawableGlyphCount++] = {i, &glyph, position};
                if (glyph.maxDimension() <= maxDimension) {
                    // Glyph fits in the atlas, good to go.
                    if (detail == SkStrikeInterface::kImageIfNeeded) {
                        this->findImage(glyph);
                    }
                } else if (glyph.fMaskFormat != SkMask::kARGB32_Format) {
                    // The out of atlas glyph is not color so we can draw it using paths.
                    this->preparePath(const_cast<SkGlyph*>(&glyph));
                } else {
                    // This will be handled by the fallback strike.
                    SkASSERT(glyph.maxDimension() > maxDimension
                             && glyph.fMaskFormat == SkMask::kARGB32_Format);
                }
            }
        }
    }

    return SkSpan<const SkGlyphPos>{result, drawableGlyphCount};
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
        if (glyphPtr->fImage) {
            memoryUsed += glyphPtr->computeImageSize();
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


