/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkConvertPixels.h"
#include "SkData.h"
#include "SkImageInfoPriv.h"
#include "SkHalf.h"
#include "SkMask.h"
#include "SkNx.h"
#include "SkPM4f.h"
#include "SkPixmap.h"
#include "SkReadPixelsRec.h"
#include "SkSurface.h"
#include "SkUtils.h"

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

void SkPixmap::setColorSpace(sk_sp<SkColorSpace> cs) {
    fInfo = fInfo.makeColorSpace(std::move(cs));
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

bool SkPixmap::readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB, int x, int y)
const {
    if (!SkImageInfoValidConversion(dstInfo, fInfo)) {
        return false;
    }

    SkReadPixelsRec rec(dstInfo, dstPixels, dstRB, x, y);
    if (!rec.trim(fInfo.width(), fInfo.height())) {
        return false;
    }

    const void* srcPixels = this->addr(rec.fX, rec.fY);
    const SkImageInfo srcInfo = fInfo.makeWH(rec.fInfo.width(), rec.fInfo.height());
    SkConvertPixels(rec.fInfo, rec.fPixels, rec.fRowBytes, srcInfo, srcPixels, this->rowBytes(),
                    this->ctable(), SkTransferFunctionBehavior::kRespect);
    return true;
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
        case kRGBA_F16_SkColorType:
            // The colorspace is unspecified, so assume linear just like getColor().
            this->erase(SkColor4f{(1 / 255.0f) * r,
                                  (1 / 255.0f) * g,
                                  (1 / 255.0f) * b,
                                  (1 / 255.0f) * a}, &area);
            break;
        default:
            return false; // no change, so don't call notifyPixelsChanged()
    }
    return true;
}

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
        return pm.erase(color.toSkColor());
    }

    const uint64_t half4 = color.premul().toF16();
    for (int y = 0; y < pm.height(); ++y) {
        sk_memset64(pm.writable_addr64(0, y), half4, pm.width());
    }
    return true;
}

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
    paint.setBlendMode(SkBlendMode::kSrc);
    surface->getCanvas()->drawBitmapRect(bitmap, SkRect::MakeIWH(dst.width(), dst.height()),
                                         &paint);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

SkColor SkPixmap::getColor(int x, int y) const {
    SkASSERT(this->addr());
    SkASSERT((unsigned)x < (unsigned)this->width());
    SkASSERT((unsigned)y < (unsigned)this->height());
    switch (this->colorType()) {
        case kGray_8_SkColorType: {
            uint8_t value = *this->addr8(x, y);
            return SkColorSetRGB(value, value, value);
        }
        case kAlpha_8_SkColorType: {
            return SkColorSetA(0, *this->addr8(x, y));
        }
        case kIndex_8_SkColorType: {
            SkASSERT(this->ctable());
            SkPMColor pmColor = (*this->ctable())[*this->addr8(x, y)];
            return SkUnPreMultiply::PMColorToColor(pmColor);
        }
        case kRGB_565_SkColorType: {
            return SkPixel16ToColor(*this->addr16(x, y));
        }
        case kARGB_4444_SkColorType: {
            uint16_t value = *this->addr16(x, y);
            SkPMColor c = SkPixel4444ToPixel32(value);
            return SkUnPreMultiply::PMColorToColor(c);
        }
        case kBGRA_8888_SkColorType: {
            uint32_t value = *this->addr32(x, y);
            SkPMColor c = SkSwizzle_BGRA_to_PMColor(value);
            return SkUnPreMultiply::PMColorToColor(c);
        }
        case kRGBA_8888_SkColorType: {
            uint32_t value = *this->addr32(x, y);
            SkPMColor c = SkSwizzle_RGBA_to_PMColor(value);
            return SkUnPreMultiply::PMColorToColor(c);
        }
        case kRGBA_F16_SkColorType: {
             const uint64_t* addr =
                 (const uint64_t*)fPixels + y * (fRowBytes >> 3) + x;
             Sk4f p4 = SkHalfToFloat_finite_ftz(*addr);
             if (p4[3]) {
                 float inva = 1 / p4[3];
                 p4 = p4 * Sk4f(inva, inva, inva, 1);
             }
             SkColor c;
             SkNx_cast<uint8_t>(p4 * Sk4f(255) + Sk4f(0.5f)).store(&c);
             // p4 is RGBA, but we want BGRA, so we need to swap next
             return SkSwizzle_RB(c);
        }
        default:
            SkDEBUGFAIL("");
            return SkColorSetARGB(0, 0, 0, 0);
    }
}

bool SkPixmap::computeIsOpaque() const {
    const int height = this->height();
    const int width = this->width();

    switch (this->colorType()) {
        case kAlpha_8_SkColorType: {
            unsigned a = 0xFF;
            for (int y = 0; y < height; ++y) {
                const uint8_t* row = this->addr8(0, y);
                for (int x = 0; x < width; ++x) {
                    a &= row[x];
                }
                if (0xFF != a) {
                    return false;
                }
            }
            return true;
        } break;
        case kIndex_8_SkColorType: {
            const SkColorTable* ctable = this->ctable();
            if (nullptr == ctable) {
                return false;
            }
            const SkPMColor* table = ctable->readColors();
            SkPMColor c = (SkPMColor)~0;
            for (int i = ctable->count() - 1; i >= 0; --i) {
                c &= table[i];
            }
            return 0xFF == SkGetPackedA32(c);
        } break;
        case kRGB_565_SkColorType:
        case kGray_8_SkColorType:
            return true;
            break;
        case kARGB_4444_SkColorType: {
            unsigned c = 0xFFFF;
            for (int y = 0; y < height; ++y) {
                const SkPMColor16* row = this->addr16(0, y);
                for (int x = 0; x < width; ++x) {
                    c &= row[x];
                }
                if (0xF != SkGetPackedA4444(c)) {
                    return false;
                }
            }
            return true;
        } break;
        case kBGRA_8888_SkColorType:
        case kRGBA_8888_SkColorType: {
            SkPMColor c = (SkPMColor)~0;
            for (int y = 0; y < height; ++y) {
                const SkPMColor* row = this->addr32(0, y);
                for (int x = 0; x < width; ++x) {
                    c &= row[x];
                }
                if (0xFF != SkGetPackedA32(c)) {
                    return false;
                }
            }
            return true;
        }
        case kRGBA_F16_SkColorType: {
            const SkHalf* row = (const SkHalf*)this->addr();
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    if (row[4 * x + 3] < SK_Half1) {
                        return false;
                    }
                }
                row += this->rowBytes() >> 1;
            }
            return true;
        }
        default:
            break;
    }
    return false;
}
