
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkDevice.h"
#include "SkDeviceProperties.h"
#include "SkDraw.h"
#include "SkImageFilter.h"
#include "SkMetaData.h"
#include "SkRasterClip.h"
#include "SkRect.h"
#include "SkRRect.h"
#include "SkShader.h"

SK_DEFINE_INST_COUNT(SkDevice)

///////////////////////////////////////////////////////////////////////////////

#define CHECK_FOR_NODRAW_ANNOTATION(paint) \
    do { if (paint.isNoDrawAnnotation()) { return; } } while (0)

///////////////////////////////////////////////////////////////////////////////

SkDevice::SkDevice(const SkBitmap& bitmap)
    : fBitmap(bitmap), fLeakyProperties(SkDeviceProperties::MakeDefault())
#ifdef SK_DEBUG
    , fAttachedToCanvas(false)
#endif
{
    fOrigin.setZero();
    fMetaData = NULL;

    SkASSERT(SkBitmap::kARGB_4444_Config != bitmap.config());
}

SkDevice::SkDevice(const SkBitmap& bitmap, const SkDeviceProperties& deviceProperties)
    : fBitmap(bitmap), fLeakyProperties(deviceProperties)
#ifdef SK_DEBUG
    , fAttachedToCanvas(false)
#endif
{
    fOrigin.setZero();
    fMetaData = NULL;
}

SkDevice::SkDevice(SkBitmap::Config config, int width, int height, bool isOpaque)
    : fLeakyProperties(SkDeviceProperties::MakeDefault())
#ifdef SK_DEBUG
    , fAttachedToCanvas(false)
#endif
{
    fOrigin.setZero();
    fMetaData = NULL;

    fBitmap.setConfig(config, width, height);
    fBitmap.allocPixels();
    fBitmap.setIsOpaque(isOpaque);
    if (!isOpaque) {
        fBitmap.eraseColor(SK_ColorTRANSPARENT);
    }
}

SkDevice::SkDevice(SkBitmap::Config config, int width, int height, bool isOpaque,
                   const SkDeviceProperties& deviceProperties)
    : fLeakyProperties(deviceProperties)
#ifdef SK_DEBUG
    , fAttachedToCanvas(false)
#endif
{
    fOrigin.setZero();
    fMetaData = NULL;

    fBitmap.setConfig(config, width, height);
    fBitmap.allocPixels();
    fBitmap.setIsOpaque(isOpaque);
    if (!isOpaque) {
        fBitmap.eraseColor(SK_ColorTRANSPARENT);
    }
}

SkDevice::~SkDevice() {
    delete fMetaData;
}

void SkDevice::replaceBitmapBackendForRasterSurface(const SkBitmap& bm) {
    SkASSERT(bm.width() == fBitmap.width());
    SkASSERT(bm.height() == fBitmap.height());
    fBitmap = bm;   // intent is to use bm's pixelRef (and rowbytes/config)
    fBitmap.lockPixels();
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
    return SkNEW_ARGS(SkDevice,(config, width, height, isOpaque, fLeakyProperties));
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
    const SkBitmap& bitmap = this->onAccessBitmap(&fBitmap);
    if (changePixels) {
        bitmap.notifyPixelsChanged();
    }
    return bitmap;
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

const SkBitmap& SkDevice::onAccessBitmap(SkBitmap* bitmap) {return *bitmap;}

void SkDevice::setMatrixClip(const SkMatrix& matrix, const SkRegion& region,
                             const SkClipStack& clipStack) {
}

bool SkDevice::canHandleImageFilter(SkImageFilter*) {
    return false;
}

bool SkDevice::filterImage(SkImageFilter* filter, const SkBitmap& src,
                           const SkMatrix& ctm, SkBitmap* result,
                           SkIPoint* offset) {
    return false;
}

bool SkDevice::allowImageFilter(SkImageFilter*) {
    return true;
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

#if SK_PMCOLOR_BYTE_ORDER(B,G,R,A)
    const SkCanvas::Config8888 SkDevice::kPMColorAlias =
        SkCanvas::kBGRA_Premul_Config8888;
#elif SK_PMCOLOR_BYTE_ORDER(R,G,B,A)
    const SkCanvas::Config8888 SkDevice::kPMColorAlias =
        SkCanvas::kRGBA_Premul_Config8888;
#else
    const SkCanvas::Config8888 SkDevice::kPMColorAlias =
        (SkCanvas::Config8888) -1;
#endif

#include <SkConfig8888.h>

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
    SkCopyBitmapToConfig8888(bmpPixels, bitmap.rowBytes(), config8888, subset);
    return true;
}

void SkDevice::writePixels(const SkBitmap& bitmap,
                           int x, int y,
                           SkCanvas::Config8888 config8888) {
    if (bitmap.isNull() || bitmap.getTexture()) {
        return;
    }
    const SkBitmap* sprite = &bitmap;
    // check whether we have to handle a config8888 that doesn't match SkPMColor
    if (SkBitmap::kARGB_8888_Config == bitmap.config() &&
        SkCanvas::kNative_Premul_Config8888 != config8888 &&
        kPMColorAlias != config8888) {

        // We're going to have to convert from a config8888 to the native config
        // First we clip to the device bounds.
        SkBitmap dstBmp = this->accessBitmap(true);
        SkIRect spriteRect = SkIRect::MakeXYWH(x, y,
                                               bitmap.width(), bitmap.height());
        SkIRect devRect = SkIRect::MakeWH(dstBmp.width(), dstBmp.height());
        if (!spriteRect.intersect(devRect)) {
            return;
        }

        // write directly to the device if it has pixels and is SkPMColor
        bool drawSprite;
        if (SkBitmap::kARGB_8888_Config == dstBmp.config() && !dstBmp.isNull()) {
            // we can write directly to the dst when doing the conversion
            dstBmp.extractSubset(&dstBmp, spriteRect);
            drawSprite = false;
        } else {
            // we convert to a temporary bitmap and draw that as a sprite
            dstBmp.setConfig(SkBitmap::kARGB_8888_Config,
                             spriteRect.width(),
                             spriteRect.height());
            if (!dstBmp.allocPixels()) {
                return;
            }
            drawSprite = true;
        }

        // copy pixels to dstBmp and convert from config8888 to native config.
        SkAutoLockPixels alp(bitmap);
        uint32_t* srcPixels = bitmap.getAddr32(spriteRect.fLeft - x,
                                               spriteRect.fTop - y);
        SkCopyConfig8888ToBitmap(dstBmp,
                                 srcPixels,
                                 bitmap.rowBytes(),
                                 config8888);

        if (drawSprite) {
            // we've clipped the sprite when we made a copy
            x = spriteRect.fLeft;
            y = spriteRect.fTop;
            sprite = &dstBmp;
        } else {
            return;
        }
    }

    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);
    SkRasterClip clip(SkIRect::MakeWH(fBitmap.width(), fBitmap.height()));
    SkDraw  draw;
    draw.fRC = &clip;
    draw.fClip = &clip.bwRgn();
    draw.fBitmap = &fBitmap; // canvas should have already called accessBitmap
    draw.fMatrix = &SkMatrix::I();
    this->drawSprite(draw, *sprite, x, y, paint);
}

///////////////////////////////////////////////////////////////////////////////

void SkDevice::drawPaint(const SkDraw& draw, const SkPaint& paint) {
    draw.drawPaint(paint);
}

void SkDevice::drawPoints(const SkDraw& draw, SkCanvas::PointMode mode, size_t count,
                          const SkPoint pts[], const SkPaint& paint) {
    CHECK_FOR_NODRAW_ANNOTATION(paint);
    draw.drawPoints(mode, count, pts, paint);
}

void SkDevice::drawRect(const SkDraw& draw, const SkRect& r, const SkPaint& paint) {
    CHECK_FOR_NODRAW_ANNOTATION(paint);
    draw.drawRect(r, paint);
}

void SkDevice::drawOval(const SkDraw& draw, const SkRect& oval, const SkPaint& paint) {
    CHECK_FOR_NODRAW_ANNOTATION(paint);

    SkPath path;
    path.addOval(oval);
    // call the VIRTUAL version, so any subclasses who do handle drawPath aren't
    // required to override drawOval.
    this->drawPath(draw, path, paint, NULL, true);
}

void SkDevice::drawRRect(const SkDraw& draw, const SkRRect& rrect, const SkPaint& paint) {
    CHECK_FOR_NODRAW_ANNOTATION(paint);

    SkPath  path;
    path.addRRect(rrect);
    // call the VIRTUAL version, so any subclasses who do handle drawPath aren't
    // required to override drawRRect.
    this->drawPath(draw, path, paint, NULL, true);
}

void SkDevice::drawPath(const SkDraw& draw, const SkPath& path,
                        const SkPaint& paint, const SkMatrix* prePathMatrix,
                        bool pathIsMutable) {
    CHECK_FOR_NODRAW_ANNOTATION(paint);
    draw.drawPath(path, paint, prePathMatrix, pathIsMutable);
}

void SkDevice::drawBitmap(const SkDraw& draw, const SkBitmap& bitmap,
                          const SkMatrix& matrix, const SkPaint& paint) {
    draw.drawBitmap(bitmap, matrix, paint);
}

void SkDevice::drawBitmapRect(const SkDraw& draw, const SkBitmap& bitmap,
                              const SkRect* src, const SkRect& dst,
                              const SkPaint& paint) {
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
                                               SkShader::kClamp_TileMode);
    if (NULL == s) {
        return;
    }
    s->setLocalMatrix(matrix);

    SkPaint paintWithShader(paint);
    paintWithShader.setStyle(SkPaint::kFill_Style);
    paintWithShader.setShader(s)->unref();

    // Call ourself, in case the subclass wanted to share this setup code
    // but handle the drawRect code themselves.
    this->drawRect(draw, *dstPtr, paintWithShader);
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

#ifdef SK_BUILD_FOR_ANDROID
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
    const SkBitmap& src = device->accessBitmap(false);
    draw.drawSprite(src, x, y, paint);
}

///////////////////////////////////////////////////////////////////////////////

bool SkDevice::filterTextFlags(const SkPaint& paint, TextFlags* flags) {
    if (!paint.isLCDRenderText() || !paint.isAntiAlias()) {
        // we're cool with the paint as is
        return false;
    }

    if (SkBitmap::kARGB_8888_Config != fBitmap.config() ||
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
