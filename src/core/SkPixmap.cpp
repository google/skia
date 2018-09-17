/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPixmap.h"

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorData.h"
#include "SkConvertPixels.h"
#include "SkData.h"
#include "SkHalf.h"
#include "SkImageInfoPriv.h"
#include "SkImageShader.h"
#include "SkMask.h"
#include "SkNx.h"
#include "SkPM4f.h"
#include "SkPixmapPriv.h"
#include "SkReadPixelsRec.h"
#include "SkSurface.h"
#include "SkTemplates.h"
#include "SkTo.h"
#include "SkUnPreMultiply.h"
#include "SkUtils.h"

#include <utility>

/////////////////////////////////////////////////////////////////////////////////////////////////

void SkPixmap::reset() {
    fPixels = nullptr;
    fRowBytes = 0;
    fInfo = SkImageInfo::MakeUnknown();
}

void SkPixmap::reset(const SkImageInfo& info, const void* addr, size_t rowBytes) {
    if (addr) {
        SkASSERT(info.validRowBytes(rowBytes));
    }
    fPixels = addr;
    fRowBytes = rowBytes;
    fInfo = info;
}

bool SkPixmap::reset(const SkMask& src) {
    if (SkMask::kA8_Format == src.fFormat) {
        this->reset(SkImageInfo::MakeA8(src.fBounds.width(), src.fBounds.height()),
                    src.fImage, src.fRowBytes);
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
    result->reset(fInfo.makeWH(r.width(), r.height()), pixels, fRowBytes);
    return true;
}

bool SkPixmap::readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRB,
                          int x, int y) const {
    if (!SkImageInfoValidConversion(dstInfo, fInfo)) {
        return false;
    }

    SkReadPixelsRec rec(dstInfo, dstPixels, dstRB, x, y);
    if (!rec.trim(fInfo.width(), fInfo.height())) {
        return false;
    }

    const void* srcPixels = this->addr(rec.fX, rec.fY);
    const SkImageInfo srcInfo = fInfo.makeWH(rec.fInfo.width(), rec.fInfo.height());
    SkConvertPixels(rec.fInfo, rec.fPixels, rec.fRowBytes, srcInfo, srcPixels, this->rowBytes());
    return true;
}

bool SkPixmap::erase(SkColor color, const SkIRect& area) const {
    return this->erase(SkColor4f::FromColor(color), &area);
}

bool SkPixmap::erase(const SkColor4f& color, const SkIRect* subset) const {
    SkBitmap bm;
    bm.installPixels(*this);

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setColor4f(color, this->colorSpace());

    SkCanvas canvas{bm};
    if (subset) {
        canvas.drawIRect(*subset, paint);
        return SkIRect::Intersects(this->bounds(), *subset);
    } else {
        canvas.drawPaint(paint);
        return true;
    }
}

bool SkPixmap::scalePixels(const SkPixmap& actualDst, SkFilterQuality quality) const {
    // We may need to tweak how we interpret these just a little below, so we make copies.
    SkPixmap src = *this,
             dst = actualDst;

    // Can't do anthing with empty src or dst
    if (src.width() <= 0 || src.height() <= 0 ||
        dst.width() <= 0 || dst.height() <= 0) {
        return false;
    }

    // no scaling involved?
    if (src.width() == dst.width() && src.height() == dst.height()) {
        return src.readPixels(dst);
    }

    // If src and dst are both unpremul, we'll fake the source out to appear as if premul,
    // and mark the destination as opaque.  This odd combination allows us to scale unpremul
    // pixels without ever premultiplying them (perhaps losing information in the color channels).
    // This is an idiosyncratic feature of scalePixels(), and is tested by scalepixels_unpremul GM.
    bool clampAsIfUnpremul = false;
    if (src.alphaType() == kUnpremul_SkAlphaType &&
        dst.alphaType() == kUnpremul_SkAlphaType) {
        src.reset(src.info().makeAlphaType(kPremul_SkAlphaType), src.addr(), src.rowBytes());
        dst.reset(dst.info().makeAlphaType(kOpaque_SkAlphaType), dst.addr(), dst.rowBytes());

        // We'll need to tell the image shader to clamp to [0,1] instead of the
        // usual [0,a] when using a bicubic scaling (kHigh_SkFilterQuality).
        clampAsIfUnpremul = true;
    }

    SkBitmap bitmap;
    if (!bitmap.installPixels(src)) {
        return false;
    }
    bitmap.setImmutable();        // Don't copy when we create an image.
    bitmap.setIsVolatile(true);   // Disable any caching.

    SkMatrix scale = SkMatrix::MakeRectToRect(SkRect::Make(src.bounds()),
                                              SkRect::Make(dst.bounds()),
                                              SkMatrix::kFill_ScaleToFit);

    // We'll create a shader to do this draw so we have control over the bicubic clamp.
    sk_sp<SkShader> shader = SkImageShader::Make(SkImage::MakeFromBitmap(bitmap),
                                                 SkShader::kClamp_TileMode,
                                                 SkShader::kClamp_TileMode,
                                                 &scale,
                                                 clampAsIfUnpremul);

    sk_sp<SkSurface> surface = SkSurface::MakeRasterDirect(dst.info(),
                                                           dst.writable_addr(),
                                                           dst.rowBytes());
    if (!shader || !surface) {
        return false;
    }

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setFilterQuality(quality);
    paint.setShader(std::move(shader));
    surface->getCanvas()->drawPaint(paint);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

SkColor SkPixmap::getColor(int x, int y) const {
    SkASSERT(this->addr());
    SkASSERT((unsigned)x < (unsigned)this->width());
    SkASSERT((unsigned)y < (unsigned)this->height());

    SkColor result;
    auto info = SkImageInfo::Make(1,1,
                                  kBGRA_8888_SkColorType,kUnpremul_SkAlphaType,
                                  sk_ref_sp(this->colorSpace()));
    SkAssertResult(this->readPixels(info, &result, sizeof(result), x,y));
    return result;
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

//////////////////////////////////////////////////////////////////////////////////////////////////

static bool draw_orientation(const SkPixmap& dst, const SkPixmap& src, unsigned flags) {
    auto surf = SkSurface::MakeRasterDirect(dst.info(), dst.writable_addr(), dst.rowBytes());
    if (!surf) {
        return false;
    }

    SkBitmap bm;
    bm.installPixels(src);

    SkMatrix m;
    m.setIdentity();

    SkScalar W = SkIntToScalar(src.width());
    SkScalar H = SkIntToScalar(src.height());
    if (flags & SkPixmapPriv::kSwapXY) {
        SkMatrix s;
        s.setAll(0, 1, 0, 1, 0, 0, 0, 0, 1);
        m.postConcat(s);
        using std::swap;
        swap(W, H);
    }
    if (flags & SkPixmapPriv::kMirrorX) {
        m.postScale(-1, 1);
        m.postTranslate(W, 0);
    }
    if (flags & SkPixmapPriv::kMirrorY) {
        m.postScale(1, -1);
        m.postTranslate(0, H);
    }
    SkPaint p;
    p.setBlendMode(SkBlendMode::kSrc);
    surf->getCanvas()->concat(m);
    surf->getCanvas()->drawBitmap(bm, 0, 0, &p);
    return true;
}

bool SkPixmapPriv::Orient(const SkPixmap& dst, const SkPixmap& src, OrientFlags flags) {
    SkASSERT((flags & ~(kMirrorX | kMirrorY | kSwapXY)) == 0);
    if (src.colorType() != dst.colorType()) {
        return false;
    }
    // note: we just ignore alphaType and colorSpace for this transformation

    int w = src.width();
    int h = src.height();
    if (flags & kSwapXY) {
        using std::swap;
        swap(w, h);
    }
    if (dst.width() != w || dst.height() != h) {
        return false;
    }
    if (w == 0 || h == 0) {
        return true;
    }

    // check for aliasing to self
    if (src.addr() == dst.addr()) {
        return flags == 0;
    }
    return draw_orientation(dst, src, flags);
}

#define kMirrorX    SkPixmapPriv::kMirrorX
#define kMirrorY    SkPixmapPriv::kMirrorY
#define kSwapXY     SkPixmapPriv::kSwapXY

static constexpr uint8_t gOrientationFlags[] = {
    0,                              // kTopLeft_SkEncodedOrigin
    kMirrorX,                       // kTopRight_SkEncodedOrigin
    kMirrorX | kMirrorY,            // kBottomRight_SkEncodedOrigin
               kMirrorY,            // kBottomLeft_SkEncodedOrigin
                          kSwapXY,  // kLeftTop_SkEncodedOrigin
    kMirrorX            | kSwapXY,  // kRightTop_SkEncodedOrigin
    kMirrorX | kMirrorY | kSwapXY,  // kRightBottom_SkEncodedOrigin
               kMirrorY | kSwapXY,  // kLeftBottom_SkEncodedOrigin
};

SkPixmapPriv::OrientFlags SkPixmapPriv::OriginToOrient(SkEncodedOrigin o) {
    unsigned io = static_cast<int>(o) - 1;
    SkASSERT(io < SK_ARRAY_COUNT(gOrientationFlags));
    return static_cast<SkPixmapPriv::OrientFlags>(gOrientationFlags[io]);
}

bool SkPixmapPriv::ShouldSwapWidthHeight(SkEncodedOrigin o) {
    return SkToBool(OriginToOrient(o) & kSwapXY);
}

SkImageInfo SkPixmapPriv::SwapWidthHeight(const SkImageInfo& info) {
    return info.makeWH(info.height(), info.width());
}

