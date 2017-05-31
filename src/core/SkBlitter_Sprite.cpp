/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkColorSpace_Base.h"
#include "SkOpts.h"
#include "SkSpriteBlitter.h"

SkSpriteBlitter::SkSpriteBlitter(const SkPixmap& source)
    : fSource(source) {}

void SkSpriteBlitter::setup(const SkPixmap& dst, int left, int top, const SkPaint& paint) {
    fDst = dst;
    fLeft = left;
    fTop = top;
    fPaint = &paint;
}

void SkSpriteBlitter::blitH(int x, int y, int width) {
    SkDEBUGFAIL("how did we get here?");

    // Fallback to blitRect.
    this->blitRect(x, y, width, 1);
}

void SkSpriteBlitter::blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]) {
    SkDEBUGFAIL("how did we get here?");

    // No fallback strategy.
}

void SkSpriteBlitter::blitV(int x, int y, int height, SkAlpha alpha) {
    SkDEBUGFAIL("how did we get here?");

    // Fall back to superclass if the code gets here in release mode.
    INHERITED::blitV(x, y, height, alpha);
}

void SkSpriteBlitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    SkDEBUGFAIL("how did we get here?");

    // Fall back to superclass if the code gets here in release mode.
    INHERITED::blitMask(mask, clip);
}

///////////////////////////////////////////////////////////////////////////////

class SkSpriteBlitter_Memcpy final : public SkSpriteBlitter {
public:
    static bool Supports(const SkPixmap& dst, const SkPixmap& src, const SkPaint& paint) {
        if (dst.colorType() != src.colorType()) {
            return false;
        }
        if (!SkColorSpace::Equals(dst.colorSpace(), src.colorSpace())) {
            return false;
        }
        if (paint.getMaskFilter() || paint.getColorFilter() || paint.getImageFilter()) {
            return false;
        }
        if (0xFF != paint.getAlpha()) {
            return false;
        }
        SkBlendMode mode = paint.getBlendMode();
        return SkBlendMode::kSrc == mode || (SkBlendMode::kSrcOver == mode && src.isOpaque());
    }

    SkSpriteBlitter_Memcpy(const SkPixmap& src)
        : INHERITED(src) {}

    void blitRect(int x, int y, int width, int height) override {
        SkASSERT(fDst.colorType() == fSource.colorType());
        SkASSERT(width > 0 && height > 0);

        char* dst = (char*)fDst.writable_addr(x, y);
        const char* src = (const char*)fSource.addr(x - fLeft, y - fTop);
        const size_t dstRB = fDst.rowBytes();
        const size_t srcRB = fSource.rowBytes();
        const size_t bytesToCopy = width << fSource.shiftPerPixel();

        while (height --> 0) {
            memcpy(dst, src, bytesToCopy);
            dst += dstRB;
            src += srcRB;
        }
    }

private:
    typedef SkSpriteBlitter INHERITED;
};

// returning null means the caller will call SkBlitter::Choose() and
// have wrapped the source bitmap inside a shader
SkBlitter* SkBlitter::ChooseSprite(const SkPixmap& dst, const SkPaint& paint,
        const SkPixmap& source, int left, int top, SkArenaAlloc* allocator) {
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

    if (source.alphaType() == kUnpremul_SkAlphaType) {
        return nullptr;
    }

    SkSpriteBlitter* blitter = nullptr;

    if (SkSpriteBlitter_Memcpy::Supports(dst, source, paint)) {
        blitter = allocator->make<SkSpriteBlitter_Memcpy>(source);
    } else if (!dst.colorSpace()) {
        switch (dst.colorType()) {
            case kN32_SkColorType:
                blitter = SkSpriteBlitter::ChooseL32(source, paint, allocator);
                break;
            default:
                break;
        }
    }

    if (blitter) {
        blitter->setup(dst, left, top, paint);
    }
    return blitter;
}
