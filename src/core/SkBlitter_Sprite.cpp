/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
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

//  Only valid if...
//      1. src == dst format
//      2. paint has no modifiers (i.e. alpha, colorfilter, etc.)
//      3. xfermode needs no blending: e.g. kSrc_Mode or kSrcOver_Mode + opaque src
//
class SkSpriteBlitter_Src_SrcOver final : public SkSpriteBlitter {
public:
    static bool Supports(const SkPixmap& dst, const SkPixmap& src, const SkPaint& paint) {
        if (dst.colorType() != src.colorType()) {
            return false;
        }
        if (dst.info().gammaCloseToSRGB() != src.info().gammaCloseToSRGB()) {
            return false;
        }
        if (paint.getMaskFilter() || paint.getColorFilter() || paint.getImageFilter()) {
            return false;
        }
        if (0xFF != paint.getAlpha()) {
            return false;
        }
        SkBlendMode mode = paint.getBlendMode();
        if (SkBlendMode::kSrc == mode) {
            return true;
        }
        if (SkBlendMode::kSrcOver == mode && src.isOpaque()) {
            return true;
        }

        // At this point memcpy can't be used. The following check for using SrcOver.

        if (dst.colorType() != kN32_SkColorType || !dst.info().gammaCloseToSRGB()) {
            return false;
        }

        return SkBlendMode::kSrcOver == mode;
    }

    SkSpriteBlitter_Src_SrcOver(const SkPixmap& src)
        : INHERITED(src) {}

    void setup(const SkPixmap& dst, int left, int top, const SkPaint& paint) override {
        SkASSERT(Supports(dst, fSource, paint));
        this->INHERITED::setup(dst, left, top, paint);
        SkBlendMode mode = paint.getBlendMode();

        SkASSERT(mode == SkBlendMode::kSrcOver || mode == SkBlendMode::kSrc);

        if (mode == SkBlendMode::kSrcOver && !fSource.isOpaque()) {
            fUseMemcpy = false;
        }
    }

    void blitRect(int x, int y, int width, int height) override {
        SkASSERT(fDst.colorType() == fSource.colorType());
        SkASSERT(fDst.info().gammaCloseToSRGB() == fSource.info().gammaCloseToSRGB());
        SkASSERT(width > 0 && height > 0);

        if (fUseMemcpy) {
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
        } else {
            uint32_t* dst       = fDst.writable_addr32(x, y);
            const uint32_t* src = fSource.addr32(x - fLeft, y - fTop);
            const int dstStride = fDst.rowBytesAsPixels();
            const int srcStride = fSource.rowBytesAsPixels();

            while (height --> 0) {
                SkOpts::srcover_srgb_srgb(dst, src, width, width);
                dst += dstStride;
                src += srcStride;
            }
        }
    }

private:
    typedef SkSpriteBlitter INHERITED;

    bool fUseMemcpy {true};
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

    // Defer to the general code if the pixels are unpremultipled. This case is not common,
    // and this simplifies the code.
    if (source.alphaType() == kUnpremul_SkAlphaType) {
        return nullptr;
    }

    SkSpriteBlitter* blitter = nullptr;

    if (SkSpriteBlitter_Src_SrcOver::Supports(dst, source, paint)) {
        blitter = allocator->make<SkSpriteBlitter_Src_SrcOver>(source);
    } else {
        switch (dst.colorType()) {
            case kRGB_565_SkColorType:
                blitter = SkSpriteBlitter::ChooseD16(source, paint, allocator);
                break;
            case kN32_SkColorType:
                if (dst.info().gammaCloseToSRGB()) {
                    blitter = SkSpriteBlitter::ChooseS32(source, paint, allocator);
                } else {
                    blitter = SkSpriteBlitter::ChooseL32(source, paint, allocator);
                }
                break;
            case kRGBA_F16_SkColorType:
                blitter = SkSpriteBlitter::ChooseF16(source, paint, allocator);
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
