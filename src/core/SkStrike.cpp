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
    VALIDATE();
    SkGlyph* glyph = fGlyphMap.findOrNull(id);
    if (glyph == nullptr) {
        glyph = this->makeGlyph(id);
    }
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
    const SkFixed maskX = !fIsSubpixel || fAxisAlignment == kY_SkAxisAlignment ? 0 : ~0;
    const SkFixed maskY = !fIsSubpixel || fAxisAlignment == kX_SkAxisAlignment ? 0 : ~0;
    SkFixed subX = SkScalarToFixed(position.x()) & maskX,
            subY = SkScalarToFixed(position.y()) & maskY;
    return this->glyph(SkPackedGlyphID{glyphID, subX, subY});
}

SkGlyph* SkStrike::mergeMetrics(const SkGlyph& from) {
    SkGlyph* glyph = fGlyphMap.findOrNull(from.getPackedID());
    if (glyph == nullptr) {
        glyph = this->makeGlyph(from.getPackedID());
    }

    return glyph;
}

void SkStrike::mergeImage(SkGlyph* glyph, const void* image, size_t size) {
    if (glyph->mergeImage(image, size, &fAlloc)) {
        fMemoryUsed += glyph->imageSize();
    }
}

void SkStrike::mergePath(SkGlyph* glyph, const SkPath* path) {
    if (glyph->mergePath(path, &fAlloc)) {
        fMemoryUsed += glyph->pathSize();
    }
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

SkSpan<SkGlyph*> SkStrike::metrics(
        SkSpan<const SkGlyphID>glyphIDs, SkGlyph* results[]) {

    size_t glyphCount = 0;
    for (auto glyphID : glyphIDs) {
        SkGlyph* glyphPtr = this->glyph(glyphID);
        results[glyphCount++] = glyphPtr;
    }

    return {results, glyphCount};
}

SkSpan<SkGlyphPos> SkStrike::metricsWithoutEmpty(
        SkSpan<const SkGlyphID>glyphIDs, const SkPoint positions[], SkGlyphPos results[]) {
    const SkFixed maskX = !fIsSubpixel || fAxisAlignment == kY_SkAxisAlignment ? 0 : ~0;
    const SkFixed maskY = !fIsSubpixel || fAxisAlignment == kX_SkAxisAlignment ? 0 : ~0;
    size_t glyphCount = 0;
    for (size_t i = 0; i < glyphIDs.size(); i++) {
        SkGlyphID glyphID = glyphIDs[i];
        SkPoint pos = positions[i];
        if (SkScalarsAreFinite(pos.x(), pos.y())) {
            SkFixed subX = SkScalarToFixed(pos.x()) & maskX,
                    subY = SkScalarToFixed(pos.y()) & maskY;
            SkPackedGlyphID packedID{glyphID, subX, subY};
            SkGlyph* glyphPtr = this->glyph(packedID);
            if (!glyphPtr->isEmpty()) {
                results[glyphCount++] = {i, glyphPtr, pos};
            }
        }
    }

    return {results, glyphCount};
}

bool SkStrike::isGlyphCached(SkGlyphID glyphID, SkFixed x, SkFixed y) const {
    SkPackedGlyphID packedGlyphID{glyphID, x, y};
    return fGlyphMap.find(packedGlyphID) != nullptr;
}

SkSpan<SkPoint> SkStrike::getAdvances(SkSpan<const SkGlyphID> glyphIDs, SkPoint advances[]) {
    auto cursor = advances;
    SkAutoSTArray<50, SkGlyph*> glyphStorage{SkTo<int>(glyphIDs.size())};
    auto glyphs = this->metrics(glyphIDs, glyphStorage.get());
    for (const SkGlyph* glyph : glyphs) {
        *cursor++ = glyph->advanceVector();
    }
    return {advances, glyphIDs.size()};
}

SkSpan<const SkGlyph*> SkStrike::prepareImages(
        SkSpan<const SkGlyphID> glyphIDs, SkGlyph* results[]) {
    auto glyphs = this->metrics(glyphIDs, results);
    for (auto glyph : glyphs) {
        this->ensureImage(glyph);
    }

    return {(const SkGlyph**)results, glyphIDs.size()};
}

const void* SkStrike::ensureImage(SkGlyph* glyph) {
    if (!glyph->imageIsInitialized()) {
        glyph->ensureImage(fScalerContext.get(), &fAlloc);
        fMemoryUsed += glyph->imageSize();
    }
    return glyph->image();
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

const SkPath* SkStrike::ensurePath(SkGlyph* glyph) {
    if (!glyph->pathIsInitialized()) {
        if (glyph->ensurePath(fScalerContext.get(), &fAlloc) != nullptr) {
            fMemoryUsed += glyph->pathSize();
        }
    }
    return glyph->path();
}

bool SkStrike::initializePath(SkGlyph* glyph, const volatile void* data, size_t size) {
    SkPath path;
    SkPath* pathPtr = nullptr;
    bool answer = true;
    if (size > 0 && (answer = path.readFromMemory(const_cast<const void*>(data), size))) {
        pathPtr = &path;
    }

    glyph->installPath(pathPtr, &fAlloc);
    fMemoryUsed += glyph->pathSize();

    return answer;
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

    auto g = SkSpan<const SkGlyphID>{glyphIDs, n};
    auto drawables = this->metricsWithoutEmpty(g, positions, result);

    size_t drawableGlyphCount = 0;
    for (auto glyphPos : drawables) {
        result[drawableGlyphCount++] = glyphPos;
        SkGlyph* glyph = glyphPos.glyph;
        if (glyph->maxDimension() <= maxDimension) {
            // The glyph fits; ensure the image if needed.
            if (detail == SkStrikeInterface::kImageIfNeeded) {
                this->ensureImage(glyph);
            }
        } else if (!glyph->isColor()) {
            // The out of atlas glyph is not color so we can draw it using paths.
            this->ensurePath(glyph);
        } else {
            // This will be handled by the fallback strike.
            SkASSERT(glyph->maxDimension() > maxDimension && glyph->isColor());
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
        if (glyphPtr->imageIsInitialized()) {
            memoryUsed += glyphPtr->imageSize();
        }
        if (glyphPtr->pathIsInitialized()) {
            memoryUsed += glyphPtr->pathSize();
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


