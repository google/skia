/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapDevice.h"
#include "SkConfig8888.h"
#include "SkDraw.h"
#include "SkRasterClip.h"
#include "SkShader.h"
#include "SkSurface.h"

#define CHECK_FOR_ANNOTATION(paint) \
    do { if (paint.getAnnotation()) { return; } } while (0)

static bool valid_for_bitmap_device(const SkImageInfo& info,
                                    SkAlphaType* newAlphaType) {
    if (info.width() < 0 || info.height() < 0) {
        return false;
    }

    // TODO: can we stop supporting kUnknown in SkBitmkapDevice?
    if (kUnknown_SkColorType == info.colorType()) {
        if (newAlphaType) {
            *newAlphaType = kIgnore_SkAlphaType;
        }
        return true;
    }

    switch (info.alphaType()) {
        case kPremul_SkAlphaType:
        case kOpaque_SkAlphaType:
            break;
        default:
            return false;
    }

    SkAlphaType canonicalAlphaType = info.alphaType();

    switch (info.colorType()) {
        case kAlpha_8_SkColorType:
            break;
        case kRGB_565_SkColorType:
            canonicalAlphaType = kOpaque_SkAlphaType;
            break;
        case kN32_SkColorType:
            break;
        default:
            return false;
    }

    if (newAlphaType) {
        *newAlphaType = canonicalAlphaType;
    }
    return true;
}

SkBitmapDevice::SkBitmapDevice(const SkBitmap& bitmap) : fBitmap(bitmap) {
    SkASSERT(valid_for_bitmap_device(bitmap.info(), NULL));
}

SkBitmapDevice::SkBitmapDevice(const SkBitmap& bitmap, const SkDeviceProperties& deviceProperties)
    : SkBaseDevice(deviceProperties)
    , fBitmap(bitmap)
{
    SkASSERT(valid_for_bitmap_device(bitmap.info(), NULL));
}

SkBitmapDevice* SkBitmapDevice::Create(const SkImageInfo& origInfo,
                                       const SkDeviceProperties* props) {
    SkImageInfo info = origInfo;
    if (!valid_for_bitmap_device(info, &info.fAlphaType)) {
        return NULL;
    }

    SkBitmap bitmap;

    if (kUnknown_SkColorType == info.colorType()) {
        if (!bitmap.setInfo(info)) {
            return NULL;
        }
    } else {
        if (!bitmap.allocPixels(info)) {
            return NULL;
        }
        if (!bitmap.info().isOpaque()) {
            bitmap.eraseColor(SK_ColorTRANSPARENT);
        }
    }

    if (props) {
        return SkNEW_ARGS(SkBitmapDevice, (bitmap, *props));
    } else {
        return SkNEW_ARGS(SkBitmapDevice, (bitmap));
    }
}

SkImageInfo SkBitmapDevice::imageInfo() const {
    return fBitmap.info();
}

void SkBitmapDevice::replaceBitmapBackendForRasterSurface(const SkBitmap& bm) {
    SkASSERT(bm.width() == fBitmap.width());
    SkASSERT(bm.height() == fBitmap.height());
    fBitmap = bm;   // intent is to use bm's pixelRef (and rowbytes/config)
    fBitmap.lockPixels();
}

SkBaseDevice* SkBitmapDevice::onCreateDevice(const SkImageInfo& info, Usage usage) {
    return SkBitmapDevice::Create(info, &this->getDeviceProperties());
}

void SkBitmapDevice::lockPixels() {
    if (fBitmap.lockPixelsAreWritable()) {
        fBitmap.lockPixels();
    }
}

void SkBitmapDevice::unlockPixels() {
    if (fBitmap.lockPixelsAreWritable()) {
        fBitmap.unlockPixels();
    }
}

void SkBitmapDevice::clear(SkColor color) {
    fBitmap.eraseColor(color);
}

const SkBitmap& SkBitmapDevice::onAccessBitmap() {
    return fBitmap;
}

void* SkBitmapDevice::onAccessPixels(SkImageInfo* info, size_t* rowBytes) {
    if (fBitmap.getPixels()) {
        *info = fBitmap.info();
        *rowBytes = fBitmap.rowBytes();
        return fBitmap.getPixels();
    }
    return NULL;
}

#include "SkConfig8888.h"

bool SkBitmapDevice::onWritePixels(const SkImageInfo& srcInfo, const void* srcPixels,
                                   size_t srcRowBytes, int x, int y) {
    // since we don't stop creating un-pixeled devices yet, check for no pixels here
    if (NULL == fBitmap.getPixels()) {
        return false;
    }

    SkImageInfo dstInfo = fBitmap.info();
    dstInfo.fWidth = srcInfo.width();
    dstInfo.fHeight = srcInfo.height();

    void* dstPixels = fBitmap.getAddr(x, y);
    size_t dstRowBytes = fBitmap.rowBytes();

    if (SkPixelInfo::CopyPixels(dstInfo, dstPixels, dstRowBytes, srcInfo, srcPixels, srcRowBytes)) {
        fBitmap.notifyPixelsChanged();
        return true;
    }
    return false;
}

bool SkBitmapDevice::onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                                  int x, int y) {
    return fBitmap.readPixels(dstInfo, dstPixels, dstRowBytes, x, y);
}

///////////////////////////////////////////////////////////////////////////////

void SkBitmapDevice::drawPaint(const SkDraw& draw, const SkPaint& paint) {
    draw.drawPaint(paint);
}

void SkBitmapDevice::drawPoints(const SkDraw& draw, SkCanvas::PointMode mode, size_t count,
                                const SkPoint pts[], const SkPaint& paint) {
    CHECK_FOR_ANNOTATION(paint);
    draw.drawPoints(mode, count, pts, paint);
}

void SkBitmapDevice::drawRect(const SkDraw& draw, const SkRect& r, const SkPaint& paint) {
    CHECK_FOR_ANNOTATION(paint);
    draw.drawRect(r, paint);
}

void SkBitmapDevice::drawOval(const SkDraw& draw, const SkRect& oval, const SkPaint& paint) {
    CHECK_FOR_ANNOTATION(paint);

    SkPath path;
    path.addOval(oval);
    // call the VIRTUAL version, so any subclasses who do handle drawPath aren't
    // required to override drawOval.
    this->drawPath(draw, path, paint, NULL, true);
}

void SkBitmapDevice::drawRRect(const SkDraw& draw, const SkRRect& rrect, const SkPaint& paint) {
    CHECK_FOR_ANNOTATION(paint);

#ifdef SK_IGNORE_BLURRED_RRECT_OPT
    SkPath  path;

    path.addRRect(rrect);
    // call the VIRTUAL version, so any subclasses who do handle drawPath aren't
    // required to override drawRRect.
    this->drawPath(draw, path, paint, NULL, true);
#else
    draw.drawRRect(rrect, paint);
#endif
}

void SkBitmapDevice::drawPath(const SkDraw& draw, const SkPath& path,
                              const SkPaint& paint, const SkMatrix* prePathMatrix,
                              bool pathIsMutable) {
    CHECK_FOR_ANNOTATION(paint);
    draw.drawPath(path, paint, prePathMatrix, pathIsMutable);
}

void SkBitmapDevice::drawBitmap(const SkDraw& draw, const SkBitmap& bitmap,
                                const SkMatrix& matrix, const SkPaint& paint) {
    draw.drawBitmap(bitmap, matrix, paint);
}

void SkBitmapDevice::drawBitmapRect(const SkDraw& draw, const SkBitmap& bitmap,
                                    const SkRect* src, const SkRect& dst,
                                    const SkPaint& paint,
                                    SkCanvas::DrawBitmapRectFlags flags) {
    SkMatrix    matrix;
    SkRect      bitmapBounds, tmpSrc, tmpDst;
    SkBitmap    tmpBitmap;

    bitmapBounds.isetWH(bitmap.width(), bitmap.height());

    // Compute matrix from the two rectangles
    if (src) {
        tmpSrc = *src;
    } else {
        tmpSrc = bitmapBounds;
    }
    matrix.setRectToRect(tmpSrc, dst, SkMatrix::kFill_ScaleToFit);

    const SkRect* dstPtr = &dst;
    const SkBitmap* bitmapPtr = &bitmap;

    // clip the tmpSrc to the bounds of the bitmap, and recompute dstRect if
    // needed (if the src was clipped). No check needed if src==null.
    if (src) {
        if (!bitmapBounds.contains(*src)) {
            if (!tmpSrc.intersect(bitmapBounds)) {
                return; // nothing to draw
            }
            // recompute dst, based on the smaller tmpSrc
            matrix.mapRect(&tmpDst, tmpSrc);
            dstPtr = &tmpDst;
        }

        // since we may need to clamp to the borders of the src rect within
        // the bitmap, we extract a subset.
        SkIRect srcIR;
        tmpSrc.roundOut(&srcIR);
        if (!bitmap.extractSubset(&tmpBitmap, srcIR)) {
            return;
        }
        bitmapPtr = &tmpBitmap;

        // Since we did an extract, we need to adjust the matrix accordingly
        SkScalar dx = 0, dy = 0;
        if (srcIR.fLeft > 0) {
            dx = SkIntToScalar(srcIR.fLeft);
        }
        if (srcIR.fTop > 0) {
            dy = SkIntToScalar(srcIR.fTop);
        }
        if (dx || dy) {
            matrix.preTranslate(dx, dy);
        }

        SkRect extractedBitmapBounds;
        extractedBitmapBounds.isetWH(bitmapPtr->width(), bitmapPtr->height());
        if (extractedBitmapBounds == tmpSrc) {
            // no fractional part in src, we can just call drawBitmap
            goto USE_DRAWBITMAP;
        }
    } else {
        USE_DRAWBITMAP:
        // We can go faster by just calling drawBitmap, which will concat the
        // matrix with the CTM, and try to call drawSprite if it can. If not,
        // it will make a shader and call drawRect, as we do below.
        this->drawBitmap(draw, *bitmapPtr, matrix, paint);
        return;
    }

    // construct a shader, so we can call drawRect with the dst
    SkShader* s = SkShader::CreateBitmapShader(*bitmapPtr,
                                               SkShader::kClamp_TileMode,
                                               SkShader::kClamp_TileMode,
                                               &matrix);
    if (NULL == s) {
        return;
    }

    SkPaint paintWithShader(paint);
    paintWithShader.setStyle(SkPaint::kFill_Style);
    paintWithShader.setShader(s)->unref();

    // Call ourself, in case the subclass wanted to share this setup code
    // but handle the drawRect code themselves.
    this->drawRect(draw, *dstPtr, paintWithShader);
}

void SkBitmapDevice::drawSprite(const SkDraw& draw, const SkBitmap& bitmap,
                                int x, int y, const SkPaint& paint) {
    draw.drawSprite(bitmap, x, y, paint);
}

void SkBitmapDevice::drawText(const SkDraw& draw, const void* text, size_t len,
                              SkScalar x, SkScalar y, const SkPaint& paint) {
    draw.drawText((const char*)text, len, x, y, paint);
}

void SkBitmapDevice::drawPosText(const SkDraw& draw, const void* text, size_t len,
                                 const SkScalar xpos[], SkScalar y,
                                 int scalarsPerPos, const SkPaint& paint) {
    draw.drawPosText((const char*)text, len, xpos, y, scalarsPerPos, paint);
}

void SkBitmapDevice::drawTextOnPath(const SkDraw& draw, const void* text,
                                    size_t len, const SkPath& path,
                                    const SkMatrix* matrix,
                                    const SkPaint& paint) {
    draw.drawTextOnPath((const char*)text, len, path, matrix, paint);
}

void SkBitmapDevice::drawVertices(const SkDraw& draw, SkCanvas::VertexMode vmode,
                                  int vertexCount,
                                  const SkPoint verts[], const SkPoint textures[],
                                  const SkColor colors[], SkXfermode* xmode,
                                  const uint16_t indices[], int indexCount,
                                  const SkPaint& paint) {
    draw.drawVertices(vmode, vertexCount, verts, textures, colors, xmode,
                      indices, indexCount, paint);
}

void SkBitmapDevice::drawDevice(const SkDraw& draw, SkBaseDevice* device,
                                int x, int y, const SkPaint& paint) {
    const SkBitmap& src = device->accessBitmap(false);
    draw.drawSprite(src, x, y, paint);
}

SkSurface* SkBitmapDevice::newSurface(const SkImageInfo& info) {
    return SkSurface::NewRaster(info);
}

const void* SkBitmapDevice::peekPixels(SkImageInfo* info, size_t* rowBytes) {
    const SkImageInfo bmInfo = fBitmap.info();
    if (fBitmap.getPixels() && (kUnknown_SkColorType != bmInfo.colorType())) {
        if (info) {
            *info = bmInfo;
        }
        if (rowBytes) {
            *rowBytes = fBitmap.rowBytes();
        }
        return fBitmap.getPixels();
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

bool SkBitmapDevice::filterTextFlags(const SkPaint& paint, TextFlags* flags) {
    if (!paint.isLCDRenderText() || !paint.isAntiAlias()) {
        // we're cool with the paint as is
        return false;
    }

    if (kN32_SkColorType != fBitmap.colorType() ||
        paint.getRasterizer() ||
        paint.getPathEffect() ||
        paint.isFakeBoldText() ||
        paint.getStyle() != SkPaint::kFill_Style ||
        !SkXfermode::IsMode(paint.getXfermode(), SkXfermode::kSrcOver_Mode)) {
        // turn off lcd
        flags->fFlags = paint.getFlags() & ~SkPaint::kLCDRenderText_Flag;
        flags->fHinting = paint.getHinting();
        return true;
    }
    // we're cool with the paint as is
    return false;
}
