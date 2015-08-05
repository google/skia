/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapDevice.h"
#include "SkConfig8888.h"
#include "SkDraw.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPixelRef.h"
#include "SkPixmap.h"
#include "SkShader.h"
#include "SkSurface.h"
#include "SkXfermode.h"

class SkColorTable;

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
            *newAlphaType = kUnknown_SkAlphaType;
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

SkBitmapDevice::SkBitmapDevice(const SkBitmap& bitmap)
    : INHERITED(SkSurfaceProps(SkSurfaceProps::kLegacyFontHost_InitType))
    , fBitmap(bitmap) {
    SkASSERT(valid_for_bitmap_device(bitmap.info(), NULL));
}

SkBitmapDevice* SkBitmapDevice::Create(const SkImageInfo& info) {
    return Create(info, SkSurfaceProps(SkSurfaceProps::kLegacyFontHost_InitType));
}

SkBitmapDevice::SkBitmapDevice(const SkBitmap& bitmap, const SkSurfaceProps& surfaceProps)
    : INHERITED(surfaceProps)
    , fBitmap(bitmap) {
    SkASSERT(valid_for_bitmap_device(bitmap.info(), NULL));
}

SkBitmapDevice* SkBitmapDevice::Create(const SkImageInfo& origInfo,
                                       const SkSurfaceProps& surfaceProps) {
    SkAlphaType newAT = origInfo.alphaType();
    if (!valid_for_bitmap_device(origInfo, &newAT)) {
        return NULL;
    }

    const SkImageInfo info = origInfo.makeAlphaType(newAT);
    SkBitmap bitmap;

    if (kUnknown_SkColorType == info.colorType()) {
        if (!bitmap.setInfo(info)) {
            return NULL;
        }
    } else {
        if (!bitmap.tryAllocPixels(info)) {
            return NULL;
        }
        if (!bitmap.info().isOpaque()) {
            bitmap.eraseColor(SK_ColorTRANSPARENT);
        }
    }

    return SkNEW_ARGS(SkBitmapDevice, (bitmap, surfaceProps));
}

SkImageInfo SkBitmapDevice::imageInfo() const {
    return fBitmap.info();
}

void SkBitmapDevice::setNewSize(const SkISize& size) {
    SkASSERT(!fBitmap.pixelRef());
    fBitmap.setInfo(fBitmap.info().makeWH(size.fWidth, size.fHeight));
}

void SkBitmapDevice::replaceBitmapBackendForRasterSurface(const SkBitmap& bm) {
    SkASSERT(bm.width() == fBitmap.width());
    SkASSERT(bm.height() == fBitmap.height());
    fBitmap = bm;   // intent is to use bm's pixelRef (and rowbytes/config)
    fBitmap.lockPixels();
}

SkBaseDevice* SkBitmapDevice::onCreateDevice(const CreateInfo& cinfo, const SkPaint*) {
    const SkSurfaceProps surfaceProps(this->surfaceProps().flags(), cinfo.fPixelGeometry);
    return SkBitmapDevice::Create(cinfo.fInfo, surfaceProps);
}

const SkBitmap& SkBitmapDevice::onAccessBitmap() {
    return fBitmap;
}

bool SkBitmapDevice::onAccessPixels(SkPixmap* pmap) {
    if (fBitmap.lockPixelsAreWritable() && this->onPeekPixels(pmap)) {
        fBitmap.notifyPixelsChanged();
        return true;
    }
    return false;
}

bool SkBitmapDevice::onPeekPixels(SkPixmap* pmap) {
    const SkImageInfo info = fBitmap.info();
    if (fBitmap.getPixels() && (kUnknown_SkColorType != info.colorType())) {
        SkColorTable* ctable = NULL;
        pmap->reset(fBitmap.info(), fBitmap.getPixels(), fBitmap.rowBytes(), ctable);
        return true;
    }
    return false;
}

bool SkBitmapDevice::onWritePixels(const SkImageInfo& srcInfo, const void* srcPixels,
                                   size_t srcRowBytes, int x, int y) {
    // since we don't stop creating un-pixeled devices yet, check for no pixels here
    if (NULL == fBitmap.getPixels()) {
        return false;
    }

    const SkImageInfo dstInfo = fBitmap.info().makeWH(srcInfo.width(), srcInfo.height());

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

void SkBitmapDevice::onAttachToCanvas(SkCanvas* canvas) {
    INHERITED::onAttachToCanvas(canvas);
    if (fBitmap.lockPixelsAreWritable()) {
        fBitmap.lockPixels();
    }
}

void SkBitmapDevice::onDetachFromCanvas() {
    INHERITED::onDetachFromCanvas();
    if (fBitmap.lockPixelsAreWritable()) {
        fBitmap.unlockPixels();
    }
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
    draw.drawBitmap(bitmap, matrix, NULL, paint);
}

void SkBitmapDevice::drawBitmapRect(const SkDraw& draw, const SkBitmap& bitmap,
                                    const SkRect* src, const SkRect& dst,
                                    const SkPaint& paint, SkCanvas::SrcRectConstraint constraint) {
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
        const SkIRect srcIR = tmpSrc.roundOut();
        if(bitmap.pixelRef()->getTexture()) {
            // Accelerated source canvas, don't use extractSubset but readPixels to get the subset.
            // This way, the pixels are copied in CPU memory instead of GPU memory.
            bitmap.pixelRef()->readPixels(&tmpBitmap, &srcIR);
        } else {
            if (!bitmap.extractSubset(&tmpBitmap, srcIR)) {
                return;
            }
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
        draw.drawBitmap(*bitmapPtr, matrix, dstPtr, paint);
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
                                 const SkScalar xpos[], int scalarsPerPos,
                                 const SkPoint& offset, const SkPaint& paint) {
    draw.drawPosText((const char*)text, len, xpos, scalarsPerPos, offset, paint);
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
    draw.drawSprite(static_cast<SkBitmapDevice*>(device)->fBitmap, x, y, paint);
}

SkSurface* SkBitmapDevice::newSurface(const SkImageInfo& info, const SkSurfaceProps& props) {
    return SkSurface::NewRaster(info, &props);
}

SkImageFilter::Cache* SkBitmapDevice::getImageFilterCache() {
    SkImageFilter::Cache* cache = SkImageFilter::Cache::Get();
    cache->ref();
    return cache;
}

///////////////////////////////////////////////////////////////////////////////

bool SkBitmapDevice::onShouldDisableLCD(const SkPaint& paint) const {
    if (kN32_SkColorType != fBitmap.colorType() ||
        paint.getRasterizer() ||
        paint.getPathEffect() ||
        paint.isFakeBoldText() ||
        paint.getStyle() != SkPaint::kFill_Style ||
        !SkXfermode::IsMode(paint.getXfermode(), SkXfermode::kSrcOver_Mode))
    {
        return true;
    }
    return false;
}
