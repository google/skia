/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/private/base/SkAssert.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkBlitter.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkCoreBlitters.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkSpriteBlitter.h"

#include <cstdint>
#include <cstring>
#include <optional>
#include <utility>

struct SkIRect;
struct SkMask;

extern bool gSkForceRasterPipelineBlitter;

SkSpriteBlitter::SkSpriteBlitter(const SkPixmap& source)
    : fSource(source) {}

bool SkSpriteBlitter::setup(const SkPixmap& dst, int left, int top, const SkPaint& paint) {
    fDst = dst;
    fLeft = left;
    fTop = top;
    fPaint = &paint;
    return true;
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
        // the caller has already inspected the colorspace on src and dst
        SkASSERT(0 == SkColorSpaceXformSteps(src,dst).flags.mask());

        if (dst.colorType() != src.colorType()) {
            return false;
        }
        if (paint.getMaskFilter() || paint.getColorFilter() || paint.getImageFilter()) {
            return false;
        }
        if (0xFF != paint.getAlpha()) {
            return false;
        }
        const auto mode = paint.asBlendMode();
        return mode == SkBlendMode::kSrc || (mode == SkBlendMode::kSrcOver && src.isOpaque());
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
    using INHERITED = SkSpriteBlitter;
};

class SkRasterPipelineSpriteBlitter : public SkSpriteBlitter {
public:
    SkRasterPipelineSpriteBlitter(const SkPixmap& src, SkArenaAlloc* alloc,
                                  sk_sp<SkShader> clipShader)
        : INHERITED(src)
        , fAlloc(alloc)
        , fBlitter(nullptr)
        , fSrcPtr{nullptr, 0}
        , fClipShader(std::move(clipShader))
    {}

    bool setup(const SkPixmap& dst, int left, int top, const SkPaint& paint) override {
        fDst  = dst;
        fLeft = left;
        fTop  = top;
        fPaintColor = paint.getColor4f();

        SkRasterPipeline p(fAlloc);
        p.appendLoad(fSource.colorType(), &fSrcPtr);

        if (SkColorTypeIsAlphaOnly(fSource.colorType())) {
            // The color for A8 images comes from the (sRGB) paint color.
            p.appendSetRGB(fAlloc, fPaintColor);
            p.append(SkRasterPipelineOp::premul);
        }
        if (auto dstCS = fDst.colorSpace()) {
            auto srcCS = fSource.colorSpace();
            if (!srcCS || SkColorTypeIsAlphaOnly(fSource.colorType())) {
                // We treat untagged images as sRGB.
                // Alpha-only images get their r,g,b from the paint color, so they're also sRGB.
                srcCS = sk_srgb_singleton();
            }
            auto srcAT = fSource.isOpaque() ? kOpaque_SkAlphaType
                                            : kPremul_SkAlphaType;
            fAlloc->make<SkColorSpaceXformSteps>(srcCS, srcAT,
                                                 dstCS, kPremul_SkAlphaType)
                ->apply(&p);
        }
        if (fPaintColor.fA != 1.0f) {
            p.append(SkRasterPipelineOp::scale_1_float, &fPaintColor.fA);
        }

        bool is_opaque = fSource.isOpaque() && fPaintColor.fA == 1.0f;
        fBlitter = SkCreateRasterPipelineBlitter(fDst, paint, p, is_opaque, fAlloc, fClipShader);
        return fBlitter != nullptr;
    }

    void blitRect(int x, int y, int width, int height) override {
        fSrcPtr.stride = fSource.rowBytesAsPixels();

        // We really want fSrcPtr.pixels = fSource.addr(-fLeft, -fTop) here, but that asserts.
        // Instead we ask for addr(-fLeft+x, -fTop+y), then back up (x,y) manually.
        // Representing bpp as a size_t keeps all this math in size_t instead of int,
        // which could wrap around with large enough fSrcPtr.stride and y.
        size_t bpp = fSource.info().bytesPerPixel();
        fSrcPtr.pixels = (char*)fSource.addr(-fLeft+x, -fTop+y) - bpp * x
                                                                - bpp * y * fSrcPtr.stride;

        fBlitter->blitRect(x,y,width,height);
    }

private:
    SkArenaAlloc*              fAlloc;
    SkBlitter*                 fBlitter;
    SkRasterPipeline_MemoryCtx fSrcPtr;
    SkColor4f                  fPaintColor;
    sk_sp<SkShader>            fClipShader;

    using INHERITED = SkSpriteBlitter;
};

// returning null means the caller will call SkBlitter::Choose() and
// have wrapped the source bitmap inside a shader
SkBlitter* SkBlitter::ChooseSprite(const SkPixmap& dst, const SkPaint& paint,
                                   const SkPixmap& source, int left, int top,
                                   SkArenaAlloc* alloc, sk_sp<SkShader> clipShader) {
    /*  We currently ignore antialiasing and filtertype, meaning we will take our
        special blitters regardless of these settings. Ignoring filtertype seems fine
        since by definition there is no scale in the matrix. Ignoring antialiasing is
        a bit of a hack, since we "could" pass in the fractional left/top for the bitmap,
        and respect that by blending the edges of the bitmap against the device. To support
        this we could either add more special blitters here, or detect antialiasing in the
        paint and return null if it is set, forcing the client to take the slow shader case
        (which does respect soft edges).
    */
    SkASSERT(alloc != nullptr);

    // TODO: in principle SkRasterPipelineSpriteBlitter could be made to handle this.
    if (source.alphaType() == kUnpremul_SkAlphaType) {
        return nullptr;
    }

    SkSpriteBlitter* blitter = nullptr;

    if (gSkForceRasterPipelineBlitter) {
        // Do not use any of these optimized memory blitters
    } else if (0 == SkColorSpaceXformSteps(source,dst).flags.mask() && !clipShader) {
        if (!blitter && SkSpriteBlitter_Memcpy::Supports(dst, source, paint)) {
            blitter = alloc->make<SkSpriteBlitter_Memcpy>(source);
        }
        if (!blitter) {
            switch (dst.colorType()) {
                case kN32_SkColorType:
                    blitter = SkSpriteBlitter::ChooseL32(source, paint, alloc);
                    break;
                default:
                    break;
            }
        }
    }
    if (!blitter && !paint.getMaskFilter()) {
        blitter = alloc->make<SkRasterPipelineSpriteBlitter>(source, alloc, clipShader);
    }

    if (blitter && blitter->setup(dst, left,top, paint)) {
        return blitter;
    }

    return nullptr;
}
