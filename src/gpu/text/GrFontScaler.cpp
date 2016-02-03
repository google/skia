
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrFontScaler.h"
#include "SkDescriptor.h"
#include "SkDistanceFieldGen.h"
#include "SkGlyphCache.h"

///////////////////////////////////////////////////////////////////////////////

GrFontScaler::GrFontScaler(SkGlyphCache* strike) {
    fStrike = strike;
    fKey = nullptr;
}

GrFontScaler::~GrFontScaler() {
    SkSafeUnref(fKey);
}

GrMaskFormat GrFontScaler::getMaskFormat() const {
    SkMask::Format format = fStrike->getMaskFormat();
    switch (format) {
        case SkMask::kBW_Format:
            // fall through to kA8 -- we store BW glyphs in our 8-bit cache
        case SkMask::kA8_Format:
            return kA8_GrMaskFormat;
        case SkMask::kLCD16_Format:
            return kA565_GrMaskFormat;
        case SkMask::kARGB32_Format:
            return kARGB_GrMaskFormat;
        default:
            SkDEBUGFAIL("unsupported SkMask::Format");
            return kA8_GrMaskFormat;
    }
}

const GrFontDescKey* GrFontScaler::getKey() {
    if (nullptr == fKey) {
        fKey = new GrFontDescKey(fStrike->getDescriptor());
    }
    return fKey;
}

GrMaskFormat GrFontScaler::getPackedGlyphMaskFormat(const SkGlyph& glyph) const {
    SkMask::Format format = static_cast<SkMask::Format>(glyph.fMaskFormat);
    switch (format) {
        case SkMask::kBW_Format:
            // fall through to kA8 -- we store BW glyphs in our 8-bit cache
        case SkMask::kA8_Format:
            return kA8_GrMaskFormat;
        case SkMask::kLCD16_Format:
            return kA565_GrMaskFormat;
        case SkMask::kARGB32_Format:
            return kARGB_GrMaskFormat;
        default:
            SkDEBUGFAIL("unsupported SkMask::Format");
            return kA8_GrMaskFormat;
    }
}

bool GrFontScaler::getPackedGlyphBounds(const SkGlyph& glyph, SkIRect* bounds) {
#if 1
    // crbug:510931
    // Retrieving the image from the cache can actually change the mask format.
    fStrike->findImage(glyph);
#endif
    bounds->setXYWH(glyph.fLeft, glyph.fTop, glyph.fWidth, glyph.fHeight);

    return true;
}

bool GrFontScaler::getPackedGlyphDFBounds(const SkGlyph& glyph, SkIRect* bounds) {
#if 1
    // crbug:510931
    // Retrieving the image from the cache can actually change the mask format.
    fStrike->findImage(glyph);
#endif
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

bool GrFontScaler::getPackedGlyphImage(const SkGlyph& glyph, int width, int height, int dstRB,
                                       GrMaskFormat expectedMaskFormat, void* dst) {
    SkASSERT(glyph.fWidth == width);
    SkASSERT(glyph.fHeight == height);
    const void* src = fStrike->findImage(glyph);
    if (nullptr == src) {
        return false;
    }

    // crbug:510931
    // Retrieving the image from the cache can actually change the mask format.  This case is very
    // uncommon so for now we just draw a clear box for these glyphs.
    if (getPackedGlyphMaskFormat(glyph) != expectedMaskFormat) {
        const int bpp = GrMaskFormatBytesPerPixel(expectedMaskFormat);
        for (int y = 0; y < height; y++) {
            sk_bzero(dst, width * bpp);
            dst = (char*)dst + dstRB;
        }
        return true;
    }

    int srcRB = glyph.rowBytes();
    // The windows font host sometimes has BW glyphs in a non-BW strike. So it is important here to
    // check the glyph's format, not the strike's format, and to be able to convert to any of the
    // GrMaskFormats.
    if (SkMask::kBW_Format == glyph.fMaskFormat) {
        // expand bits to our mask type
        const uint8_t* bits = reinterpret_cast<const uint8_t*>(src);
        switch (expectedMaskFormat) {
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
            default:
                SkFAIL("Invalid GrMaskFormat");
        }
    } else if (srcRB == dstRB) {
        memcpy(dst, src, dstRB * height);
    } else {
        const int bbp = GrMaskFormatBytesPerPixel(expectedMaskFormat);
        for (int y = 0; y < height; y++) {
            memcpy(dst, src, width * bbp);
            src = (const char*)src + srcRB;
            dst = (char*)dst + dstRB;
        }
    }
    return true;
}

bool GrFontScaler::getPackedGlyphDFImage(const SkGlyph& glyph, int width, int height, void* dst) {
    SkASSERT(glyph.fWidth + 2*SK_DistanceFieldPad == width);
    SkASSERT(glyph.fHeight + 2*SK_DistanceFieldPad == height);
    const void* image = fStrike->findImage(glyph);
    if (nullptr == image) {
        return false;
    }
    // now generate the distance field
    SkASSERT(dst);
    SkMask::Format maskFormat = static_cast<SkMask::Format>(glyph.fMaskFormat);
    if (SkMask::kA8_Format == maskFormat) {
        // make the distance field from the image
        SkGenerateDistanceFieldFromA8Image((unsigned char*)dst,
                                           (unsigned char*)image,
                                           glyph.fWidth, glyph.fHeight,
                                           glyph.rowBytes());
    } else if (SkMask::kBW_Format == maskFormat) {
        // make the distance field from the image
        SkGenerateDistanceFieldFromBWImage((unsigned char*)dst,
                                           (unsigned char*)image,
                                           glyph.fWidth, glyph.fHeight,
                                           glyph.rowBytes());
    } else {
        return false;
    }

    return true;
}

const SkPath* GrFontScaler::getGlyphPath(const SkGlyph& glyph) {
    return fStrike->findPath(glyph);
}

const SkGlyph& GrFontScaler::grToSkGlyph(GrGlyph::PackedID id) {
    return fStrike->getGlyphIDMetrics(GrGlyph::UnpackID(id),
                                      GrGlyph::UnpackFixedX(id),
                                      GrGlyph::UnpackFixedY(id));
}
