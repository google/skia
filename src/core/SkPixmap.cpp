/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorPriv.h"
#include "SkConfig8888.h"
#include "SkData.h"
#include "SkMask.h"
#include "SkPixmap.h"
#include "SkUtils.h"
#include "SkPM4f.h"

void SkAutoPixmapUnlock::reset(const SkPixmap& pm, void (*unlock)(void*), void* ctx) {
    SkASSERT(pm.addr() != nullptr);

    this->unlock();
    fPixmap = pm;
    fUnlockProc = unlock;
    fUnlockContext = ctx;
    fIsLocked = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void SkPixmap::reset() {
    fPixels = nullptr;
    fCTable = nullptr;
    fRowBytes = 0;
    fInfo = SkImageInfo::MakeUnknown();
}

void SkPixmap::reset(const SkImageInfo& info, const void* addr, size_t rowBytes, SkColorTable* ct) {
    if (addr) {
        SkASSERT(info.validRowBytes(rowBytes));
    }
    fPixels = addr;
    fCTable = ct;
    fRowBytes = rowBytes;
    fInfo = info;
}

bool SkPixmap::reset(const SkMask& src) {
    if (SkMask::kA8_Format == src.fFormat) {
        this->reset(SkImageInfo::MakeA8(src.fBounds.width(), src.fBounds.height()),
                    src.fImage, src.fRowBytes, nullptr);
        return true;
    }
    this->reset();
    return false;
}

bool SkPixmap::extractSubset(SkPixmap* result, const SkIRect& subset) const {
    SkIRect srcRect, r;
    srcRect.set(0, 0, this->width(), this->height());
    if (!r.intersect(srcRect, subset)) {
        return false;   // r is empty (i.e. no intersection)
    }

    // If the upper left of the rectangle was outside the bounds of this SkBitmap, we should have
    // exited above.
    SkASSERT(static_cast<unsigned>(r.fLeft) < static_cast<unsigned>(this->width()));
    SkASSERT(static_cast<unsigned>(r.fTop) < static_cast<unsigned>(this->height()));

    const void* pixels = nullptr;
    if (fPixels) {
        const size_t bpp = fInfo.bytesPerPixel();
        pixels = (const uint8_t*)fPixels + r.fTop * fRowBytes + r.fLeft * bpp;
    }
    result->reset(fInfo.makeWH(r.width(), r.height()), pixels, fRowBytes, fCTable);
    return true;
}

bool SkPixmap::readPixels(const SkImageInfo& requestedDstInfo, void* dstPixels, size_t dstRB,
                          int x, int y) const {
    if (kUnknown_SkColorType == requestedDstInfo.colorType()) {
        return false;
    }
    if (nullptr == dstPixels || dstRB < requestedDstInfo.minRowBytes()) {
        return false;
    }
    if (0 == requestedDstInfo.width() || 0 == requestedDstInfo.height()) {
        return false;
    }

    SkIRect srcR = SkIRect::MakeXYWH(x, y, requestedDstInfo.width(), requestedDstInfo.height());
    if (!srcR.intersect(0, 0, this->width(), this->height())) {
        return false;
    }

    // the intersect may have shrunk info's logical size
    const SkImageInfo dstInfo = requestedDstInfo.makeWH(srcR.width(), srcR.height());

    // if x or y are negative, then we have to adjust pixels
    if (x > 0) {
        x = 0;
    }
    if (y > 0) {
        y = 0;
    }
    // here x,y are either 0 or negative
    dstPixels = ((char*)dstPixels - y * dstRB - x * dstInfo.bytesPerPixel());

    const SkImageInfo srcInfo = this->info().makeWH(dstInfo.width(), dstInfo.height());
    const void* srcPixels = this->addr(srcR.x(), srcR.y());
    return SkPixelInfo::CopyPixels(dstInfo, dstPixels, dstRB,
                                   srcInfo, srcPixels, this->rowBytes(), this->ctable());
}

static uint16_t pack_8888_to_4444(unsigned a, unsigned r, unsigned g, unsigned b) {
    unsigned pixel = (SkA32To4444(a) << SK_A4444_SHIFT) |
    (SkR32To4444(r) << SK_R4444_SHIFT) |
    (SkG32To4444(g) << SK_G4444_SHIFT) |
    (SkB32To4444(b) << SK_B4444_SHIFT);
    return SkToU16(pixel);
}

bool SkPixmap::erase(SkColor color, const SkIRect& inArea) const {
    if (nullptr == fPixels) {
        return false;
    }
    SkIRect area;
    if (!area.intersect(this->bounds(), inArea)) {
        return false;
    }

    U8CPU a = SkColorGetA(color);
    U8CPU r = SkColorGetR(color);
    U8CPU g = SkColorGetG(color);
    U8CPU b = SkColorGetB(color);

    int height = area.height();
    const int width = area.width();
    const int rowBytes = this->rowBytes();

    switch (this->colorType()) {
        case kGray_8_SkColorType: {
            if (255 != a) {
                r = SkMulDiv255Round(r, a);
                g = SkMulDiv255Round(g, a);
                b = SkMulDiv255Round(b, a);
            }
            int gray = SkComputeLuminance(r, g, b);
            uint8_t* p = this->writable_addr8(area.fLeft, area.fTop);
            while (--height >= 0) {
                memset(p, gray, width);
                p += rowBytes;
            }
            break;
        }
        case kAlpha_8_SkColorType: {
            uint8_t* p = this->writable_addr8(area.fLeft, area.fTop);
            while (--height >= 0) {
                memset(p, a, width);
                p += rowBytes;
            }
            break;
        }
        case kARGB_4444_SkColorType:
        case kRGB_565_SkColorType: {
            uint16_t* p = this->writable_addr16(area.fLeft, area.fTop);
            uint16_t v;

            // make rgb premultiplied
            if (255 != a) {
                r = SkMulDiv255Round(r, a);
                g = SkMulDiv255Round(g, a);
                b = SkMulDiv255Round(b, a);
            }

            if (kARGB_4444_SkColorType == this->colorType()) {
                v = pack_8888_to_4444(a, r, g, b);
            } else {
                v = SkPackRGB16(r >> (8 - SK_R16_BITS),
                                g >> (8 - SK_G16_BITS),
                                b >> (8 - SK_B16_BITS));
            }
            while (--height >= 0) {
                sk_memset16(p, v, width);
                p = (uint16_t*)((char*)p + rowBytes);
            }
            break;
        }
        case kBGRA_8888_SkColorType:
        case kRGBA_8888_SkColorType: {
            uint32_t* p = this->writable_addr32(area.fLeft, area.fTop);

            if (255 != a && kPremul_SkAlphaType == this->alphaType()) {
                r = SkMulDiv255Round(r, a);
                g = SkMulDiv255Round(g, a);
                b = SkMulDiv255Round(b, a);
            }
            uint32_t v = kRGBA_8888_SkColorType == this->colorType()
                             ? SkPackARGB_as_RGBA(a, r, g, b)
                             : SkPackARGB_as_BGRA(a, r, g, b);

            while (--height >= 0) {
                sk_memset32(p, v, width);
                p = (uint32_t*)((char*)p + rowBytes);
            }
            break;
        }
        default:
            return false; // no change, so don't call notifyPixelsChanged()
    }
    return true;
}

#include "SkNx.h"
#include "SkHalf.h"

bool SkPixmap::erase(const SkColor4f& origColor, const SkIRect* subset) const {
    SkPixmap pm;
    if (subset) {
        if (!this->extractSubset(&pm, *subset)) {
            return false;
        }
    } else {
        pm = *this;
    }

    const SkColor4f color = origColor.pin();

    if (kRGBA_F16_SkColorType != pm.colorType()) {
        Sk4f c4 = Sk4f::Load(color.vec());
        SkColor c;
        (c4 * Sk4f(255) + Sk4f(0.5f)).store(&c);
        return pm.erase(c);
    }

    const uint64_t half4 = color.premul().toF16();
    for (int y = 0; y < pm.height(); ++y) {
        sk_memset64(pm.writable_addr64(0, y), half4, pm.width());
    }
    return true;
}

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkSurface.h"
#include "SkXfermode.h"

bool SkPixmap::scalePixels(const SkPixmap& dst, SkFilterQuality quality) const {
    // Can't do anthing with empty src or dst
    if (this->width() <= 0 || this->height() <= 0 || dst.width() <= 0 || dst.height() <= 0) {
        return false;
    }

    // no scaling involved?
    if (dst.width() == this->width() && dst.height() == this->height()) {
        return this->readPixels(dst);
    }

    SkBitmap bitmap;
    if (!bitmap.installPixels(*this)) {
        return false;
    }
    bitmap.setIsVolatile(true); // so we don't try to cache it

    auto surface(SkSurface::MakeRasterDirect(dst.info(), dst.writable_addr(), dst.rowBytes()));
    if (!surface) {
        return false;
    }

    SkPaint paint;
    paint.setFilterQuality(quality);
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    surface->getCanvas()->drawBitmapRect(bitmap, SkRect::MakeIWH(dst.width(), dst.height()),
                                         &paint);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
