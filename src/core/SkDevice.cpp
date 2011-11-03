
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkDevice.h"
#include "SkDraw.h"
#include "SkMetaData.h"
#include "SkRect.h"

///////////////////////////////////////////////////////////////////////////////

SkDevice::SkDevice(const SkBitmap& bitmap) : fBitmap(bitmap) {
    fOrigin.setZero();
    fMetaData = NULL;
}

SkDevice::SkDevice(SkBitmap::Config config, int width, int height, bool isOpaque) {
    fOrigin.setZero();
    fMetaData = NULL;

    fBitmap.setConfig(config, width, height);
    fBitmap.allocPixels();
    fBitmap.setIsOpaque(isOpaque);
    if (!isOpaque) {
        fBitmap.eraseColor(0);
    }
}

SkDevice::~SkDevice() {
    delete fMetaData;
}

SkDevice* SkDevice::createCompatibleDevice(SkBitmap::Config config, 
                                           int width, int height,
                                           bool isOpaque) {
    return this->onCreateCompatibleDevice(config, width, height,
                                          isOpaque, kGeneral_Usage);
}

SkDevice* SkDevice::createCompatibleDeviceForSaveLayer(SkBitmap::Config config,
                                                       int width, int height,
                                                       bool isOpaque) {
    return this->onCreateCompatibleDevice(config, width, height,
                                          isOpaque, kSaveLayer_Usage);
}

SkDevice* SkDevice::onCreateCompatibleDevice(SkBitmap::Config config, 
                                             int width, int height, 
                                             bool isOpaque,
                                             Usage usage) {
    return SkNEW_ARGS(SkDevice,(config, width, height, isOpaque));
}

SkMetaData& SkDevice::getMetaData() {
    // metadata users are rare, so we lazily allocate it. If that changes we
    // can decide to just make it a field in the device (rather than a ptr)
    if (NULL == fMetaData) {
        fMetaData = new SkMetaData;
    }
    return *fMetaData;
}

void SkDevice::lockPixels() {
    if (fBitmap.lockPixelsAreWritable()) {
        fBitmap.lockPixels();
    }
}

void SkDevice::unlockPixels() {
    if (fBitmap.lockPixelsAreWritable()) {
        fBitmap.unlockPixels();
    }
}

const SkBitmap& SkDevice::accessBitmap(bool changePixels) {
    this->onAccessBitmap(&fBitmap);
    if (changePixels) {
        fBitmap.notifyPixelsChanged();
    }
    return fBitmap;
}

void SkDevice::getGlobalBounds(SkIRect* bounds) const {
    if (bounds) {
        bounds->setXYWH(fOrigin.x(), fOrigin.y(),
                        fBitmap.width(), fBitmap.height());
    }
}

void SkDevice::clear(SkColor color) {
    fBitmap.eraseColor(color);
}

void SkDevice::onAccessBitmap(SkBitmap* bitmap) {}

void SkDevice::setMatrixClip(const SkMatrix& matrix, const SkRegion& region,
                             const SkClipStack& clipStack) {
}

///////////////////////////////////////////////////////////////////////////////

bool SkDevice::readPixels(SkBitmap* bitmap, int x, int y,
                          SkCanvas::Config8888 config8888) {
    if (SkBitmap::kARGB_8888_Config != bitmap->config() ||
        NULL != bitmap->getTexture()) {
        return false;
    }

    const SkBitmap& src = this->accessBitmap(false);

    SkIRect srcRect = SkIRect::MakeXYWH(x, y, bitmap->width(),
                                              bitmap->height());
    SkIRect devbounds = SkIRect::MakeWH(src.width(), src.height());
    if (!srcRect.intersect(devbounds)) {
        return false;
    }

    SkBitmap tmp;
    SkBitmap* bmp;
    if (bitmap->isNull()) {
        tmp.setConfig(SkBitmap::kARGB_8888_Config, bitmap->width(),
                                                   bitmap->height());
        if (!tmp.allocPixels()) {
            return false;
        }
        bmp = &tmp;
    } else {
        bmp = bitmap;
    }

    SkIRect subrect = srcRect;
    subrect.offset(-x, -y);
    SkBitmap bmpSubset;
    bmp->extractSubset(&bmpSubset, subrect);

    bool result = this->onReadPixels(bmpSubset,
                                     srcRect.fLeft,
                                     srcRect.fTop,
                                     config8888);
    if (result && bmp == &tmp) {
        tmp.swap(*bitmap);
    }
    return result;
}

#ifdef SK_CPU_LENDIAN
    #if   24 == SK_A32_SHIFT && 16 == SK_R32_SHIFT && \
           8 == SK_G32_SHIFT &&  0 == SK_B32_SHIFT
        const SkCanvas::Config8888 SkDevice::kPMColorAlias =
            SkCanvas::kBGRA_Premul_Config8888;
    #elif 24 == SK_A32_SHIFT &&  0 == SK_R32_SHIFT && \
           8 == SK_G32_SHIFT && 16 == SK_B32_SHIFT
        const SkCanvas::Config8888 SkDevice::kPMColorAlias =
            SkCanvas::kRGBA_Premul_Config8888;
    #else
        const SkCanvas::Config8888 SkDevice::kPMColorAlias =
            (SkCanvas::Config8888) -1;
    #endif
    static const int NATIVE_A_IDX = SK_A32_SHIFT / 8;
    static const int NATIVE_R_IDX = SK_R32_SHIFT / 8;
    static const int NATIVE_G_IDX = SK_G32_SHIFT / 8;
    static const int NATIVE_B_IDX = SK_B32_SHIFT / 8;
#else
    #if    0 == SK_A32_SHIFT &&   8 == SK_R32_SHIFT && \
          16 == SK_G32_SHIFT &&  24 == SK_B32_SHIFT
        const SkCanvas::Config8888 SkDevice::kPMColorAlias =
            SkCanvas::kBGRA_Premul_Config8888;
    #elif  0 == SK_A32_SHIFT &&  24 == SK_R32_SHIFT && \
          16 == SK_G32_SHIFT &&   8 == SK_B32_SHIFT
        const SkCanvas::Config8888 SkDevice::kPMColorAlias =
            SkCanvas::kRGBA_Premul_Config8888;
    #else
        const SkCanvas::Config8888 SkDevice::kPMColorAlias =
            (SkCanvas::Config8888) -1;
    #endif
    static const int NATIVE_A_IDX = 3 - (SK_A32_SHIFT / 8);
    static const int NATIVE_R_IDX = 3 - (SK_R32_SHIFT / 8);
    static const int NATIVE_G_IDX = 3 - (SK_G32_SHIFT / 8);
    static const int NATIVE_B_IDX = 3 - (SK_B32_SHIFT / 8);
#endif

#include <SkColorPriv.h>

namespace {

template <int A_IDX, int R_IDX, int G_IDX, int B_IDX>
inline uint32_t pack_config8888(uint32_t a, uint32_t r,
                                uint32_t g, uint32_t b) {
#ifdef SK_CPU_LENDIAN
    return (a << (A_IDX * 8)) | (r << (R_IDX * 8)) |
           (g << (G_IDX * 8)) | (b << (B_IDX * 8));
#else
    return (a << ((3-A_IDX) * 8)) | (r << ((3-R_IDX) * 8)) |
           (g << ((3-G_IDX) * 8)) | (b << ((3-B_IDX) * 8));
#endif
}

template <bool UNPM, int A_IDX, int R_IDX, int G_IDX, int B_IDX>
inline void bitmap_copy_to_config8888(const SkBitmap& srcBmp,
                                        uint32_t* dstPixels,
                                        size_t dstRowBytes) {
    SkASSERT(SkBitmap::kARGB_8888_Config == srcBmp.config());
    SkAutoLockPixels alp(srcBmp);
    int w = srcBmp.width();
    int h = srcBmp.height();
    size_t srcRowBytes = srcBmp.rowBytes();

    intptr_t src = reinterpret_cast<intptr_t>(srcBmp.getPixels());
    intptr_t dst = reinterpret_cast<intptr_t>(dstPixels);

    for (int y = 0; y < h; ++y) {
        const SkPMColor* srcRow = reinterpret_cast<SkPMColor*>(src);
        uint32_t* dstRow  = reinterpret_cast<uint32_t*>(dst);
        for (int x = 0; x < w; ++x) {
            SkPMColor pmcolor = srcRow[x];
            if (UNPM) {
                U8CPU a, r, g, b;
                a = SkGetPackedA32(pmcolor);
                if (a) {
                    // We're doing the explicit divide to match WebKit layout
                    // test expectations. We can modify and rebaseline if there
                    // it can be shown that there is a more performant way to
                    // unpremul.
                    r = SkGetPackedR32(pmcolor) * 0xff / a;
                    g = SkGetPackedG32(pmcolor) * 0xff / a;
                    b = SkGetPackedB32(pmcolor) * 0xff / a;
                    dstRow[x] = pack_config8888<A_IDX, R_IDX,
                                                G_IDX, B_IDX>(a, r, g, b);
                } else {
                    dstRow[x] = 0;
                }
            } else {
                dstRow[x] = pack_config8888<A_IDX, R_IDX,
                                            G_IDX, B_IDX>(
                                                   SkGetPackedA32(pmcolor),
                                                   SkGetPackedR32(pmcolor),
                                                   SkGetPackedG32(pmcolor),
                                                   SkGetPackedB32(pmcolor));
            }
        }
        dst += dstRowBytes;
        src += srcRowBytes;
    }
}

inline void bitmap_copy_to_native(const SkBitmap& srcBmp,
                                  uint32_t* dstPixels,
                                  size_t dstRowBytes) {
    SkASSERT(SkBitmap::kARGB_8888_Config == srcBmp.config());

    SkAutoLockPixels alp(srcBmp);

    int w = srcBmp.width();
    int h = srcBmp.height();
    size_t srcRowBytes = srcBmp.rowBytes();

    size_t tightRowBytes = w * 4;

    char* src = reinterpret_cast<char*>(srcBmp.getPixels());
    char* dst = reinterpret_cast<char*>(dstPixels);

    if (tightRowBytes == srcRowBytes &&
        tightRowBytes == dstRowBytes) {
        memcpy(dst, src, tightRowBytes * h);
    } else {
        for (int y = 0; y < h; ++y) {
            memcpy(dst, src, tightRowBytes);
            dst += dstRowBytes;
            src += srcRowBytes;
        }
    }
}

}

bool SkDevice::onReadPixels(const SkBitmap& bitmap,
                            int x, int y,
                            SkCanvas::Config8888 config8888) {
    SkASSERT(SkBitmap::kARGB_8888_Config == bitmap.config());
    SkASSERT(!bitmap.isNull());
    SkASSERT(SkIRect::MakeWH(this->width(), this->height()).contains(SkIRect::MakeXYWH(x, y, bitmap.width(), bitmap.height())));

    SkIRect srcRect = SkIRect::MakeXYWH(x, y, bitmap.width(),
                                              bitmap.height());
    const SkBitmap& src = this->accessBitmap(false);

    SkBitmap subset;
    if (!src.extractSubset(&subset, srcRect)) {
        return false;
    }
    if (SkBitmap::kARGB_8888_Config != subset.config()) {
        // It'd be preferable to do this directly to bitmap.
        subset.copyTo(&subset, SkBitmap::kARGB_8888_Config); 
    }
    SkAutoLockPixels alp(bitmap);
    uint32_t* bmpPixels = reinterpret_cast<uint32_t*>(bitmap.getPixels());
    if ((SkCanvas::kNative_Premul_Config8888 == config8888 ||
         kPMColorAlias == config8888)) {
        bitmap_copy_to_native(subset, bmpPixels, bitmap.rowBytes());
    } else {
        switch (config8888) {
            case SkCanvas::kNative_Premul_Config8888:
                bitmap_copy_to_config8888<false,
                                          NATIVE_A_IDX, NATIVE_R_IDX,
                                          NATIVE_G_IDX, NATIVE_B_IDX>(
                                                subset,
                                                bmpPixels,
                                                bitmap.rowBytes());
                break;
            case SkCanvas::kNative_Unpremul_Config8888:
                bitmap_copy_to_config8888<true,
                                          NATIVE_A_IDX, NATIVE_R_IDX,
                                          NATIVE_G_IDX, NATIVE_B_IDX>(
                                                subset,
                                                bmpPixels,
                                                bitmap.rowBytes());
                break;
            case SkCanvas::kBGRA_Premul_Config8888:
                bitmap_copy_to_config8888<false, 3, 2, 1, 0> (
                                        subset, bmpPixels, bitmap.rowBytes());
                break;
            case SkCanvas::kBGRA_Unpremul_Config8888:
                bitmap_copy_to_config8888<true, 3, 2, 1, 0> (
                                        subset, bmpPixels, bitmap.rowBytes());
                break;
            case SkCanvas::kRGBA_Premul_Config8888:
                bitmap_copy_to_config8888<false, 3, 0, 1, 2> (
                                        subset, bmpPixels, bitmap.rowBytes());
                break;
            case SkCanvas::kRGBA_Unpremul_Config8888:
                bitmap_copy_to_config8888<true, 3, 0, 1, 2> (
                                        subset, bmpPixels, bitmap.rowBytes());
                break;
            default:
                SkASSERT(false && "unexpected Config8888");
                break;
        }
    }
    return true;
}

void SkDevice::writePixels(const SkBitmap& bitmap, int x, int y) {
    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);

    SkCanvas canvas(this);
    canvas.drawSprite(bitmap, x, y, &paint);
}

///////////////////////////////////////////////////////////////////////////////

void SkDevice::drawPaint(const SkDraw& draw, const SkPaint& paint) {
    draw.drawPaint(paint);
}

void SkDevice::drawPoints(const SkDraw& draw, SkCanvas::PointMode mode, size_t count,
                              const SkPoint pts[], const SkPaint& paint) {
    draw.drawPoints(mode, count, pts, paint);
}

void SkDevice::drawRect(const SkDraw& draw, const SkRect& r,
                            const SkPaint& paint) {
    draw.drawRect(r, paint);
}

void SkDevice::drawPath(const SkDraw& draw, const SkPath& path,
                        const SkPaint& paint, const SkMatrix* prePathMatrix,
                        bool pathIsMutable) {
    draw.drawPath(path, paint, prePathMatrix, pathIsMutable);
}

void SkDevice::drawBitmap(const SkDraw& draw, const SkBitmap& bitmap,
                          const SkIRect* srcRect,
                          const SkMatrix& matrix, const SkPaint& paint) {
    SkBitmap        tmp;    // storage if we need a subset of bitmap
    const SkBitmap* bitmapPtr = &bitmap;

    if (srcRect) {
        if (!bitmap.extractSubset(&tmp, *srcRect)) {
            return;     // extraction failed
        }
        bitmapPtr = &tmp;
    }
    draw.drawBitmap(*bitmapPtr, matrix, paint);
}

void SkDevice::drawSprite(const SkDraw& draw, const SkBitmap& bitmap,
                              int x, int y, const SkPaint& paint) {
    draw.drawSprite(bitmap, x, y, paint);
}

void SkDevice::drawText(const SkDraw& draw, const void* text, size_t len,
                            SkScalar x, SkScalar y, const SkPaint& paint) {
    draw.drawText((const char*)text, len, x, y, paint);
}

void SkDevice::drawPosText(const SkDraw& draw, const void* text, size_t len,
                               const SkScalar xpos[], SkScalar y,
                               int scalarsPerPos, const SkPaint& paint) {
    draw.drawPosText((const char*)text, len, xpos, y, scalarsPerPos, paint);
}

void SkDevice::drawTextOnPath(const SkDraw& draw, const void* text,
                                  size_t len, const SkPath& path,
                                  const SkMatrix* matrix,
                                  const SkPaint& paint) {
    draw.drawTextOnPath((const char*)text, len, path, matrix, paint);
}

#ifdef ANDROID
void SkDevice::drawPosTextOnPath(const SkDraw& draw, const void* text, size_t len,
                                     const SkPoint pos[], const SkPaint& paint,
                                     const SkPath& path, const SkMatrix* matrix) {
    draw.drawPosTextOnPath((const char*)text, len, pos, paint, path, matrix);
}
#endif

void SkDevice::drawVertices(const SkDraw& draw, SkCanvas::VertexMode vmode,
                                int vertexCount,
                                const SkPoint verts[], const SkPoint textures[],
                                const SkColor colors[], SkXfermode* xmode,
                                const uint16_t indices[], int indexCount,
                                const SkPaint& paint) {
    draw.drawVertices(vmode, vertexCount, verts, textures, colors, xmode,
                      indices, indexCount, paint);
}

void SkDevice::drawDevice(const SkDraw& draw, SkDevice* device,
                              int x, int y, const SkPaint& paint) {
    draw.drawSprite(device->accessBitmap(false), x, y, paint);
}

///////////////////////////////////////////////////////////////////////////////

bool SkDevice::filterTextFlags(const SkPaint& paint, TextFlags* flags) {
    if (!paint.isLCDRenderText() || !paint.isAntiAlias()) {
        // we're cool with the paint as is
        return false;
    }

    if (SkBitmap::kARGB_8888_Config != fBitmap.config() ||
        paint.getShader() ||
        paint.getXfermode() || // unless its srcover
        paint.getMaskFilter() ||
        paint.getRasterizer() ||
        paint.getColorFilter() ||
        paint.getPathEffect() ||
        paint.isFakeBoldText() ||
        paint.getStyle() != SkPaint::kFill_Style) {
        // turn off lcd
        flags->fFlags = paint.getFlags() & ~SkPaint::kLCDRenderText_Flag;
        flags->fHinting = paint.getHinting();
        return true;
    }
    // we're cool with the paint as is
    return false;
}

