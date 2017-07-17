/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkColorSpace.h"
#include "SkCoreBlitters.h"
#include "SkOpts.h"
#include "SkPM4fPriv.h"
#include "SkRasterPipeline.h"
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

class SkRasterPipelineSpriteBlitter : public SkSpriteBlitter {
public:
    SkRasterPipelineSpriteBlitter(const SkPixmap& src, SkArenaAlloc* alloc)
        : INHERITED(src)
        , fAlloc(alloc)
        , fBlitter(nullptr)
        , fSrcPtr(nullptr)
    {}

    void setup(const SkPixmap& dst, int left, int top, const SkPaint& paint) override {
        fDst  = dst;
        fLeft = left;
        fTop  = top;

        fPaintColor = SkColor4f_from_SkColor(paint.getColor(), fDst.colorSpace());

        SkRasterPipeline p(fAlloc);
        switch (fSource.colorType()) {
            case kAlpha_8_SkColorType:   p.append(SkRasterPipeline::load_a8,   &fSrcPtr); break;
            case kGray_8_SkColorType:    p.append(SkRasterPipeline::load_g8,   &fSrcPtr); break;
            case kRGB_565_SkColorType:   p.append(SkRasterPipeline::load_565,  &fSrcPtr); break;
            case kARGB_4444_SkColorType: p.append(SkRasterPipeline::load_4444, &fSrcPtr); break;
            case kBGRA_8888_SkColorType: p.append(SkRasterPipeline::load_bgra, &fSrcPtr); break;
            case kRGBA_8888_SkColorType: p.append(SkRasterPipeline::load_8888, &fSrcPtr); break;
            case kRGBA_F16_SkColorType:  p.append(SkRasterPipeline::load_f16,  &fSrcPtr); break;
            default: SkASSERT(false);
        }
        if (fDst.colorSpace() &&
                (!fSource.colorSpace() || fSource.colorSpace()->gammaCloseToSRGB())) {
            p.append_from_srgb(fSource.alphaType());
        }
        if (fSource.colorType() == kAlpha_8_SkColorType) {
            p.append(SkRasterPipeline::set_rgb, &fPaintColor);
            p.append(SkRasterPipeline::premul);
        }
        append_gamut_transform(&p, fAlloc,
                               fSource.colorSpace(), fDst.colorSpace(), kPremul_SkAlphaType);
        if (fPaintColor.fA != 1.0f) {
            p.append(SkRasterPipeline::scale_1_float, &fPaintColor.fA);
        }

        bool is_opaque = fSource.isOpaque() && fPaintColor.fA == 1.0f;
        fBlitter = SkCreateRasterPipelineBlitter(fDst, paint, p, is_opaque, fAlloc);
    }

    void blitRect(int x, int y, int width, int height) override {
        fSrcPtr = (const char*)fSource.addr(x-fLeft,y-fTop);

        // Our pipeline will load from fSrcPtr+x, fSrcPtr+x+1, etc.,
        // so we back up an extra x pixels to start at 0.
        fSrcPtr -= fSource.info().bytesPerPixel() * x;

        while (height --> 0) {
            fBlitter->blitH(x,y++, width);
            fSrcPtr += fSource.rowBytes();
        }
    }

private:
    SkArenaAlloc* fAlloc;
    SkBlitter*    fBlitter;
    const char*   fSrcPtr;
    SkColor4f     fPaintColor;

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

    if (!blitter && SkSpriteBlitter_Memcpy::Supports(dst, source, paint)) {
        blitter = allocator->make<SkSpriteBlitter_Memcpy>(source);
    }
    if (!blitter && !dst.colorSpace() && dst.colorType() == kN32_SkColorType) {
        blitter = SkSpriteBlitter::ChooseL32(source, paint, allocator);
    }
    if (!blitter) {
        blitter = allocator->make<SkRasterPipelineSpriteBlitter>(source, allocator);
    }

    if (blitter) {
        blitter->setup(dst, left, top, paint);
    }
    return blitter;
}
