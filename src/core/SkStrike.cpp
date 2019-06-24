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

SkGlyph* SkStrike::glyph(SkGlyphID glyphID, SkPoint position) {
    const SkFixed maskX = (!fIsSubpixel || fAxisAlignment == kY_SkAxisAlignment) ? 0 : ~0;
    const SkFixed maskY = (!fIsSubpixel || fAxisAlignment == kX_SkAxisAlignment) ? 0 : ~0;
    SkFixed subX = SkScalarToFixed(position.x()) & maskX,
            subY = SkScalarToFixed(position.y()) & maskY;
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

SkSpan<const SkGlyph*> SkStrike::metrics(
        SkSpan<const SkGlyphID>glyphIDs, const SkGlyph* results[]) {

    size_t glyphCount = 0;
    for (auto glyphID : glyphIDs) {
        SkGlyph* glyphPtr = this->glyph(glyphID);
        results[glyphCount++] = glyphPtr;
    }

    return {results, glyphCount};
}

bool SkStrike::isGlyphCached(SkGlyphID glyphID, SkFixed x, SkFixed y) const {
    SkPackedGlyphID packedGlyphID{glyphID, x, y};
    return fGlyphMap.find(packedGlyphID) != nullptr;
}

SkSpan<SkPoint> SkStrike::getAdvances(SkSpan<const SkGlyphID> glyphIDs, SkPoint advances[]) {
    auto cursor = advances;
    SkAutoSTArray<50, const SkGlyph*> glyphStorage{SkTo<int>(glyphIDs.size())};
    auto glyphs = this->metrics(glyphIDs, glyphStorage.get());
    for (const SkGlyph* glyph : glyphs) {
        *cursor++ = glyph->advanceVector();
    }
    return {advances, glyphIDs.size()};
}

const void* SkStrike::findImage(const SkGlyph& glyphRef) {
    SkGlyph* glyph = const_cast<SkGlyph*>(&glyphRef);
    if (glyph->setImage(&fAlloc, fScalerContext.get())) {
        fMemoryUsed += glyph->imageSize();
    }
    return glyph->image();
}

void SkStrike::initializeImage(const void* data, size_t size, SkGlyph* glyph) {
    SkASSERT(size == glyph->imageSize());
    if (glyph->setImage(&fAlloc, data)) {
        fMemoryUsed += glyph->imageSize();
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
    return *this->glyph(glyphID, position);
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


