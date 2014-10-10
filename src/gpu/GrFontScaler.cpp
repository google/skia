
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrTemplates.h"
#include "GrFontScaler.h"
#include "SkDescriptor.h"
#include "SkDistanceFieldGen.h"
#include "SkGlyphCache.h"

///////////////////////////////////////////////////////////////////////////////

GrFontDescKey::GrFontDescKey(const SkDescriptor& desc) : fHash(desc.getChecksum()) {
    size_t size = desc.getLength();
    if (size <= sizeof(fStorage)) {
        fDesc = GrTCast<SkDescriptor*>(fStorage);
    } else {
        fDesc = SkDescriptor::Alloc(size);
    }
    memcpy(fDesc, &desc, size);
}

GrFontDescKey::~GrFontDescKey() {
    if (fDesc != GrTCast<SkDescriptor*>(fStorage)) {
        SkDescriptor::Free(fDesc);
    }
}

bool GrFontDescKey::lt(const GrFontDescKey& rh) const {
    const SkDescriptor* srcDesc = (&rh)->fDesc;
    size_t lenLH = fDesc->getLength();
    size_t lenRH = srcDesc->getLength();
    int cmp = memcmp(fDesc, srcDesc, SkTMin<size_t>(lenLH, lenRH));
    if (0 == cmp) {
        return lenLH < lenRH;
    } else {
        return cmp < 0;
    }
}

bool GrFontDescKey::eq(const GrFontDescKey& rh) const {
    const SkDescriptor* srcDesc = (&rh)->fDesc;
    return fDesc->equals(*srcDesc);
}

///////////////////////////////////////////////////////////////////////////////

GrFontScaler::GrFontScaler(SkGlyphCache* strike) {
    fStrike = strike;
    fKey = NULL;
}

GrFontScaler::~GrFontScaler() {
    SkSafeUnref(fKey);
}

GrMaskFormat GrFontScaler::getMaskFormat() {
    SkMask::Format format = fStrike->getMaskFormat();
    switch (format) {
        case SkMask::kBW_Format:
            // fall through to kA8 -- we store BW glyphs in our 8-bit cache
        case SkMask::kA8_Format:
            return kA8_GrMaskFormat;
        case SkMask::kLCD16_Format:
            return kA565_GrMaskFormat;
        case SkMask::kLCD32_Format:
            return kA888_GrMaskFormat;
        case SkMask::kARGB32_Format:
            return kARGB_GrMaskFormat;
        default:
            SkDEBUGFAIL("unsupported SkMask::Format");
            return kA8_GrMaskFormat;
    }
}

const GrFontDescKey* GrFontScaler::getKey() {
    if (NULL == fKey) {
        fKey = SkNEW_ARGS(GrFontDescKey, (fStrike->getDescriptor()));
    }
    return fKey;
}

bool GrFontScaler::getPackedGlyphBounds(GrGlyph::PackedID packed, SkIRect* bounds) {
    const SkGlyph& glyph = fStrike->getGlyphIDMetrics(GrGlyph::UnpackID(packed),
                                                      GrGlyph::UnpackFixedX(packed),
                                                      GrGlyph::UnpackFixedY(packed));
    bounds->setXYWH(glyph.fLeft, glyph.fTop, glyph.fWidth, glyph.fHeight);

    return true;
}

bool GrFontScaler::getPackedGlyphDFBounds(GrGlyph::PackedID packed, SkIRect* bounds) {
    const SkGlyph& glyph = fStrike->getGlyphIDMetrics(GrGlyph::UnpackID(packed),
                                                      GrGlyph::UnpackFixedX(packed),
                                                      GrGlyph::UnpackFixedY(packed));
    bounds->setXYWH(glyph.fLeft, glyph.fTop, glyph.fWidth, glyph.fHeight);
    bounds->outset(SK_DistanceFieldPad, SK_DistanceFieldPad);

    return true;
}

namespace {
// expands each bit in a bitmask to 0 or ~0 of type INT_TYPE. Used to expand a BW glyph mask to
// A8, RGB565, or RGBA8888.
template <typename INT_TYPE>
void expand_bits(INT_TYPE* dst,
                 const uint8_t* src,
                 int width,
                 int height,
                 int dstRowBytes,
                 int srcRowBytes) {
    for (int i = 0; i < height; ++i) {
        int rowWritesLeft = width;
        const uint8_t* s = src;
        INT_TYPE* d = dst;
        while (rowWritesLeft > 0) {
            unsigned mask = *s++;
            for (int i = 7; i >= 0 && rowWritesLeft; --i, --rowWritesLeft) {
                *d++ = (mask & (1 << i)) ? (INT_TYPE)(~0UL) : 0;
            }
        }
        dst = reinterpret_cast<INT_TYPE*>(reinterpret_cast<intptr_t>(dst) + dstRowBytes);
        src += srcRowBytes;
    }
}
}

bool GrFontScaler::getPackedGlyphImage(GrGlyph::PackedID packed,
                                         int width, int height,
                                         int dstRB, void* dst) {
    const SkGlyph& glyph = fStrike->getGlyphIDMetrics(GrGlyph::UnpackID(packed),
                                                      GrGlyph::UnpackFixedX(packed),
                                                      GrGlyph::UnpackFixedY(packed));
    SkASSERT(glyph.fWidth == width);
    SkASSERT(glyph.fHeight == height);
    const void* src = fStrike->findImage(glyph);
    if (NULL == src) {
        return false;
    }

    int srcRB = glyph.rowBytes();
    // The windows font host sometimes has BW glyphs in a non-BW strike. So it is important here to
    // check the glyph's format, not the strike's format, and to be able to convert to any of the
    // GrMaskFormats.
    if (SkMask::kBW_Format == glyph.fMaskFormat) {
        // expand bits to our mask type
        const uint8_t* bits = reinterpret_cast<const uint8_t*>(src);
        switch (this->getMaskFormat()) {
            case kA8_GrMaskFormat:{
                uint8_t* bytes = reinterpret_cast<uint8_t*>(dst);
                expand_bits(bytes, bits, width, height, dstRB, srcRB);
                break;
            }
            case kA565_GrMaskFormat: {
                uint16_t* rgb565 = reinterpret_cast<uint16_t*>(dst);
                expand_bits(rgb565, bits, width, height, dstRB, srcRB);
                break;
            }
            case kA888_GrMaskFormat: {
                uint32_t* rgba8888 = reinterpret_cast<uint32_t*>(dst);
                expand_bits(rgba8888, bits, width, height, dstRB, srcRB);
                break;
            }
            default:
                SkFAIL("Invalid GrMaskFormat");
        }
    } else if (srcRB == dstRB) {
        memcpy(dst, src, dstRB * height);
    } else {
        const int bbp = GrMaskFormatBytesPerPixel(this->getMaskFormat());
        for (int y = 0; y < height; y++) {
            memcpy(dst, src, width * bbp);
            src = (const char*)src + srcRB;
            dst = (char*)dst + dstRB;
        }
    }
    return true;
}

bool GrFontScaler::getPackedGlyphDFImage(GrGlyph::PackedID packed,
                                           int width, int height,
                                           void* dst) {
    const SkGlyph& glyph = fStrike->getGlyphIDMetrics(GrGlyph::UnpackID(packed),
                                                      GrGlyph::UnpackFixedX(packed),
                                                      GrGlyph::UnpackFixedY(packed));
    SkASSERT(glyph.fWidth + 2*SK_DistanceFieldPad == width);
    SkASSERT(glyph.fHeight + 2*SK_DistanceFieldPad == height);
    const void* src = fStrike->findDistanceField(glyph);
    if (NULL == src) {
        return false;
    }

    memcpy(dst, src, width * height);

    return true;
}

// we should just return const SkPath* (NULL means false)
bool GrFontScaler::getGlyphPath(uint16_t glyphID, SkPath* path) {

    const SkGlyph& glyph = fStrike->getGlyphIDMetrics(glyphID);
    const SkPath* skPath = fStrike->findPath(glyph);
    if (skPath) {
        *path = *skPath;
        return true;
    }
    return false;
}
