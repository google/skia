/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSmallAllocator.h"
#include "SkSpriteBlitter.h"

SkSpriteBlitter::SkSpriteBlitter(const SkPixmap& source) : fSource(source) {}

void SkSpriteBlitter::setup(const SkPixmap& dst, int left, int top, const SkPaint& paint) {
    fDst = dst;
    fLeft = left;
    fTop = top;
    fPaint = &paint;
}

#ifdef SK_DEBUG
void SkSpriteBlitter::blitH(int x, int y, int width) {
    SkDEBUGFAIL("how did we get here?");
}

void SkSpriteBlitter::blitAntiH(int x, int y, const SkAlpha antialias[],
                                const int16_t runs[]) {
    SkDEBUGFAIL("how did we get here?");
}

void SkSpriteBlitter::blitV(int x, int y, int height, SkAlpha alpha) {
    SkDEBUGFAIL("how did we get here?");
}

void SkSpriteBlitter::blitMask(const SkMask&, const SkIRect& clip) {
    SkDEBUGFAIL("how did we get here?");
}
#endif

///////////////////////////////////////////////////////////////////////////////

//  Only valid if...
//      1. src == dst format
//      2. paint has no modifiers (i.e. alpha, colorfilter, etc.)
//      3. xfermode needs no blending: e.g. kSrc_Mode or kSrcOver_Mode + opaque src
//
class SkSpriteBlitter_memcpy : public SkSpriteBlitter {
public:
    static bool Supports(const SkPixmap& dst, const SkPixmap& src, const SkPaint& paint) {
        if (dst.colorType() != src.colorType()) {
            return false;
        }
        if (dst.info().profileType() != src.info().profileType()) {
            return false;
        }
        if (paint.getMaskFilter() || paint.getColorFilter() || paint.getImageFilter()) {
            return false;
        }
        if (0xFF != paint.getAlpha()) {
            return false;
        }
        SkXfermode::Mode mode;
        if (!SkXfermode::AsMode(paint.getXfermode(), &mode)) {
            return false;
        }
        if (SkXfermode::kSrc_Mode == mode) {
            return true;
        }
        if (SkXfermode::kSrcOver_Mode == mode && src.isOpaque()) {
            return true;
        }
        return false;
    }

    SkSpriteBlitter_memcpy(const SkPixmap& src)  : INHERITED(src) {}

    void setup(const SkPixmap& dst, int left, int top, const SkPaint& paint) override {
        SkASSERT(Supports(dst, fSource, paint));
        this->INHERITED::setup(dst, left, top, paint);
    }

    void blitRect(int x, int y, int width, int height) override {
        SkASSERT(fDst.colorType() == fSource.colorType());
        SkASSERT(fDst.info().profileType() == fSource.info().profileType());
        SkASSERT(width > 0 && height > 0);

        char* dst = (char*)fDst.writable_addr(x, y);
        const char* src = (const char*)fSource.addr(x - fLeft, y - fTop);
        const size_t dstRB = fDst.rowBytes();
        const size_t srcRB = fSource.rowBytes();
        const size_t bytesToCopy = width << fSource.shiftPerPixel();

        while (--height >= 0) {
            memcpy(dst, src, bytesToCopy);
            dst += dstRB;
            src += srcRB;
        }
    }

    typedef SkSpriteBlitter INHERITED;
};


// returning null means the caller will call SkBlitter::Choose() and
// have wrapped the source bitmap inside a shader
SkBlitter* SkBlitter::ChooseSprite(const SkPixmap& dst, const SkPaint& paint,
        const SkPixmap& source, int left, int top, SkTBlitterAllocator* allocator) {
    /*  We currently ignore antialiasing and filtertype, meaning we will take our
        special blitters regardless of these settings. Ignoring filtertype seems fine
        since by definition there is no scale in the matrix. Ignoring antialiasing is
        a bit of a hack, since we "could" pass in the fractional left/top for the bitmap,
        and respect that by blending the edges of the bitmap against the device. To support
        this we could either add more special blitters here, or detect antialiasing in the
        paint and return null if it is set, forcing the client to take the slow shader case
        (which does respect soft edges).
    */
    SkASSERT(allocator != nullptr);

    SkSpriteBlitter* blitter;

    if (SkSpriteBlitter_memcpy::Supports(dst, source, paint)) {
        blitter = allocator->createT<SkSpriteBlitter_memcpy>(source);
    } else {
        switch (dst.colorType()) {
            case kRGB_565_SkColorType:
                blitter = SkSpriteBlitter::ChooseD16(source, paint, allocator);
                break;
            case kN32_SkColorType:
                if (dst.info().isSRGB()) {
                    blitter = SkSpriteBlitter::ChooseS32(source, paint, allocator);
                } else {
                    blitter = SkSpriteBlitter::ChooseL32(source, paint, allocator);
                }
                break;
            case kRGBA_F16_SkColorType:
                blitter = SkSpriteBlitter::ChooseF16(source, paint, allocator);
                break;
            default:
                blitter = nullptr;
                break;
        }
    }

    if (blitter) {
        blitter->setup(dst, left, top, paint);
    }
    return blitter;
}
