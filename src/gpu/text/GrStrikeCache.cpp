/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkArenaAlloc.h"
#include "src/core/SkDistanceFieldGen.h"
#include "src/core/SkStrikeSpec.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrDistanceFieldGenFromVector.h"
#include "src/gpu/text/GrAtlasManager.h"

GrStrikeCache::~GrStrikeCache() {
    this->freeAll();
}

void GrStrikeCache::freeAll() {
    fCache.reset();
}

///////////////////////////////////////////////////////////////////////////////

/*
    The text strike is specific to a given font/style/matrix setup, which is
    represented by the GrHostFontScaler object we are given in getGlyph().

    We map a 32bit glyphID to a GrGlyph record, which in turn points to a
    atlas and a position within that texture.
 */

GrTextStrike::GrTextStrike(const SkDescriptor& key) : fFontScalerKey(key) {}

GrDrawOpAtlas::ErrorCode GrTextStrike::AddGlyphToAtlas(const SkGlyph& skGlyph,
                                                       GrMaskFormat expectedMaskFormat,
                                                       bool needsPadding,
                                                       GrResourceProvider* resourceProvider,
                                                       GrDeferredUploadTarget* target,
                                                       GrAtlasManager* fullAtlasManager,
                                                       GrGlyph* grGlyph) {
    SkASSERT(grGlyph != nullptr);
//    SkASSERT(fCache.findOrNull(grGlyph->fPackedID));
    SkASSERT(skGlyph.image() != nullptr);

    expectedMaskFormat = fullAtlasManager->resolveMaskFormat(expectedMaskFormat);
    int bytesPerPixel = GrMaskFormatBytesPerPixel(expectedMaskFormat);

    SkDEBUGCODE(bool isSDFGlyph = skGlyph.maskFormat() == SkMask::kSDF_Format;)
    SkASSERT(!needsPadding || !isSDFGlyph);

    // Add 1 pixel padding around grGlyph if needed.
    const int width = needsPadding ? skGlyph.width() + 2 : skGlyph.width();
    const int height = needsPadding ? skGlyph.height() + 2 : skGlyph.height();
    int rowBytes = width * bytesPerPixel;
    size_t size = height * rowBytes;

    // Temporary storage for normalizing grGlyph image.
    SkAutoSMalloc<1024> storage(size);
    void* dataPtr = storage.get();
    if (needsPadding) {
        sk_bzero(dataPtr, size);
        // Advance in one row and one column.
        dataPtr = (char*)(dataPtr) + rowBytes + bytesPerPixel;
    }

    get_packed_glyph_image(skGlyph, rowBytes, expectedMaskFormat, dataPtr);

    return fullAtlasManager->addToAtlas(resourceProvider, target, expectedMaskFormat, width, height,
                                        storage.get(), &grGlyph->fAtlasLocator);
}
>>>>>>> git squash commit for omnibus.

GrGlyph* GrTextStrike::getGlyph(SkPackedGlyphID packedGlyphID) {
    GrGlyph* grGlyph = fCache.findOrNull(packedGlyphID);
    if (grGlyph == nullptr) {
        grGlyph = fAlloc.make<GrGlyph>(packedGlyphID);
        fCache.set(grGlyph);
    }
    return grGlyph;
}

