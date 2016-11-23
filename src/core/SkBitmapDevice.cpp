/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapDevice.h"
#include "SkConfig8888.h"
#include "SkDraw.h"
#include "SkImageFilter.h"
#include "SkImageFilterCache.h"
#include "SkMallocPixelRef.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPixelRef.h"
#include "SkPixmap.h"
#include "SkRasterClip.h"
#include "SkShader.h"
#include "SkSpecialImage.h"
#include "SkSurface.h"

class SkColorTable;

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
        case kRGBA_F16_SkColorType:
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
    : INHERITED(bitmap.info(), SkSurfaceProps(SkSurfaceProps::kLegacyFontHost_InitType))
    , fBitmap(bitmap)
{
    SkASSERT(valid_for_bitmap_device(bitmap.info(), nullptr));
    fBitmap.lockPixels();
}

SkBitmapDevice* SkBitmapDevice::Create(const SkImageInfo& info) {
    return Create(info, SkSurfaceProps(SkSurfaceProps::kLegacyFontHost_InitType));
}

SkBitmapDevice::SkBitmapDevice(const SkBitmap& bitmap, const SkSurfaceProps& surfaceProps)
    : INHERITED(bitmap.info(), surfaceProps)
    , fBitmap(bitmap)
{
    SkASSERT(valid_for_bitmap_device(bitmap.info(), nullptr));
    fBitmap.lockPixels();
}

SkBitmapDevice* SkBitmapDevice::Create(const SkImageInfo& origInfo,
                                       const SkSurfaceProps& surfaceProps) {
    SkAlphaType newAT = origInfo.alphaType();
    if (!valid_for_bitmap_device(origInfo, &newAT)) {
        return nullptr;
    }

    const SkImageInfo info = origInfo.makeAlphaType(newAT);
    SkBitmap bitmap;

    if (kUnknown_SkColorType == info.colorType()) {
        if (!bitmap.setInfo(info)) {
            return nullptr;
        }
    } else if (info.isOpaque()) {
        // If this bitmap is opaque, we don't have any sensible default color,
        // so we just return uninitialized pixels.
        if (!bitmap.tryAllocPixels(info)) {
            return nullptr;
        }
    } else {
        // This bitmap has transparency, so we'll zero the pixels (to transparent).
        // We use a ZeroedPRFactory as a faster alloc-then-eraseColor(SK_ColorTRANSPARENT).
        SkMallocPixelRef::ZeroedPRFactory factory;
        if (!bitmap.tryAllocPixels(info, &factory, nullptr/*color table*/)) {
            return nullptr;
        }
    }

    return new SkBitmapDevice(bitmap, surfaceProps);
}

void SkBitmapDevice::setNewSize(const SkISize& size) {
    SkASSERT(!fBitmap.pixelRef());
    fBitmap.setInfo(fBitmap.info().makeWH(size.fWidth, size.fHeight));
    this->privateResize(fBitmap.info().width(), fBitmap.info().height());
}

void SkBitmapDevice::replaceBitmapBackendForRasterSurface(const SkBitmap& bm) {
    SkASSERT(bm.width() == fBitmap.width());
    SkASSERT(bm.height() == fBitmap.height());
    fBitmap = bm;   // intent is to use bm's pixelRef (and rowbytes/config)
    fBitmap.lockPixels();
    this->privateResize(fBitmap.info().width(), fBitmap.info().height());
}

SkBaseDevice* SkBitmapDevice::onCreateDevice(const CreateInfo& cinfo, const SkPaint*) {
    const SkSurfaceProps surfaceProps(this->surfaceProps().flags(), cinfo.fPixelGeometry);
    return SkBitmapDevice::Create(cinfo.fInfo, surfaceProps);
}

const SkBitmap& SkBitmapDevice::onAccessBitmap() {
    return fBitmap;
}

bool SkBitmapDevice::onAccessPixels(SkPixmap* pmap) {
    if (this->onPeekPixels(pmap)) {
        fBitmap.notifyPixelsChanged();
        return true;
    }
    return false;
}

bool SkBitmapDevice::onPeekPixels(SkPixmap* pmap) {
    const SkImageInfo info = fBitmap.info();
    if (fBitmap.getPixels() && (kUnknown_SkColorType != info.colorType())) {
        SkColorTable* ctable = nullptr;
        pmap->reset(fBitmap.info(), fBitmap.getPixels(), fBitmap.rowBytes(), ctable);
        return true;
    }
    return false;
}

bool SkBitmapDevice::onWritePixels(const SkImageInfo& srcInfo, const void* srcPixels,
                                   size_t srcRowBytes, int x, int y) {
    // since we don't stop creating un-pixeled devices yet, check for no pixels here
    if (nullptr == fBitmap.getPixels()) {
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

///////////////////////////////////////////////////////////////////////////////

void SkBitmapDevice::drawPaint(const SkDraw& draw, const SkPaint& paint) {
    draw.drawPaint(paint);
}

void SkBitmapDevice::drawPoints(const SkDraw& draw, SkCanvas::PointMode mode, size_t count,
                                const SkPoint pts[], const SkPaint& paint) {
    draw.drawPoints(mode, count, pts, paint);
}

void SkBitmapDevice::drawRect(const SkDraw& draw, const SkRect& r, const SkPaint& paint) {
    draw.drawRect(r, paint);
}

void SkBitmapDevice::drawOval(const SkDraw& draw, const SkRect& oval, const SkPaint& paint) {
    SkPath path;
    path.addOval(oval);
    // call the VIRTUAL version, so any subclasses who do handle drawPath aren't
    // required to override drawOval.
    this->drawPath(draw, path, paint, nullptr, true);
}

void SkBitmapDevice::drawRRect(const SkDraw& draw, const SkRRect& rrect, const SkPaint& paint) {
#ifdef SK_IGNORE_BLURRED_RRECT_OPT
    SkPath  path;

    path.addRRect(rrect);
    // call the VIRTUAL version, so any subclasses who do handle drawPath aren't
    // required to override drawRRect.
    this->drawPath(draw, path, paint, nullptr, true);
#else
    draw.drawRRect(rrect, paint);
#endif
}

void SkBitmapDevice::drawPath(const SkDraw& draw, const SkPath& path,
                              const SkPaint& paint, const SkMatrix* prePathMatrix,
                              bool pathIsMutable) {
    draw.drawPath(path, paint, prePathMatrix, pathIsMutable);
}

void SkBitmapDevice::drawBitmap(const SkDraw& draw, const SkBitmap& bitmap,
                                const SkMatrix& matrix, const SkPaint& paint) {
    LogDrawScaleFactor(SkMatrix::Concat(*draw.fMatrix, matrix), paint.getFilterQuality());
    draw.drawBitmap(bitmap, matrix, nullptr, paint);
}

static inline bool CanApplyDstMatrixAsCTM(const SkMatrix& m, const SkPaint& paint) {
    if (!paint.getMaskFilter()) {
        return true;
    }

    // Some mask filters parameters (sigma) depend on the CTM/scale.
    return m.getType() <= SkMatrix::kTranslate_Mask;
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

    LogDrawScaleFactor(SkMatrix::Concat(*draw.fMatrix, matrix), paint.getFilterQuality());

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
    }

    if (src && !src->contains(bitmapBounds) &&
        SkCanvas::kFast_SrcRectConstraint == constraint &&
        paint.getFilterQuality() != kNone_SkFilterQuality) {
        // src is smaller than the bounds of the bitmap, and we are filtering, so we don't know
        // how much more of the bitmap we need, so we can't use extractSubset or drawBitmap,
        // but we must use a shader w/ dst bounds (which can access all of the bitmap needed).
        goto USE_SHADER;
    }

    if (src) {
        // since we may need to clamp to the borders of the src rect within
        // the bitmap, we extract a subset.
        const SkIRect srcIR = tmpSrc.roundOut();
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
        if (CanApplyDstMatrixAsCTM(matrix, paint)) {
            draw.drawBitmap(*bitmapPtr, matrix, dstPtr, paint);
            return;
        }
    }

    USE_SHADER:

    // Since the shader need only live for our stack-frame, pass in a custom allocator. This
    // can save malloc calls, and signals to SkMakeBitmapShader to not try to copy the bitmap
    // if its mutable, since that precaution is not needed (give the short lifetime of the shader).
    SkTBlitterAllocator allocator;
    // construct a shader, so we can call drawRect with the dst
    auto s = SkMakeBitmapShader(*bitmapPtr, SkShader::kClamp_TileMode, SkShader::kClamp_TileMode,
                                &matrix, kNever_SkCopyPixelsMode, &allocator);
    if (!s) {
        return;
    }
    // we deliberately add a ref, since the allocator wants to be the last owner
    s.get()->ref();

    SkPaint paintWithShader(paint);
    paintWithShader.setStyle(SkPaint::kFill_Style);
    paintWithShader.setShader(s);

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
                                  const SkColor colors[], SkBlendMode bmode,
                                  const uint16_t indices[], int indexCount,
                                  const SkPaint& paint) {
    draw.drawVertices(vmode, vertexCount, verts, textures, colors, bmode,
                      indices, indexCount, paint);
}

void SkBitmapDevice::drawDevice(const SkDraw& draw, SkBaseDevice* device,
                                int x, int y, const SkPaint& paint) {
    SkASSERT(!paint.getImageFilter());
    draw.drawSprite(static_cast<SkBitmapDevice*>(device)->fBitmap, x, y, paint);
}

///////////////////////////////////////////////////////////////////////////////

void SkBitmapDevice::drawSpecial(const SkDraw& draw, SkSpecialImage* srcImg, int x, int y,
                                 const SkPaint& paint) {
    SkASSERT(!srcImg->isTextureBacked());

    SkBitmap resultBM;

    SkImageFilter* filter = paint.getImageFilter();
    if (filter) {
        SkIPoint offset = SkIPoint::Make(0, 0);
        SkMatrix matrix = *draw.fMatrix;
        matrix.postTranslate(SkIntToScalar(-x), SkIntToScalar(-y));
        const SkIRect clipBounds = draw.fRC->getBounds().makeOffset(-x, -y);
        sk_sp<SkImageFilterCache> cache(this->getImageFilterCache());
        SkImageFilter::OutputProperties outputProperties(fBitmap.colorSpace());
        SkImageFilter::Context ctx(matrix, clipBounds, cache.get(), outputProperties);

        sk_sp<SkSpecialImage> resultImg(filter->filterImage(srcImg, ctx, &offset));
        if (resultImg) {
            SkPaint tmpUnfiltered(paint);
            tmpUnfiltered.setImageFilter(nullptr);
            if (resultImg->getROPixels(&resultBM)) {
                this->drawSprite(draw, resultBM, x + offset.x(), y + offset.y(), tmpUnfiltered);
            }
        }
    } else {
        if (srcImg->getROPixels(&resultBM)) {
            this->drawSprite(draw, resultBM, x, y, paint);
        }
    }
}

sk_sp<SkSpecialImage> SkBitmapDevice::makeSpecial(const SkBitmap& bitmap) {
    return SkSpecialImage::MakeFromRaster(bitmap.bounds(), bitmap);
}

sk_sp<SkSpecialImage> SkBitmapDevice::makeSpecial(const SkImage* image) {
    // This is called when we're about to draw the special-ized version of image to *this* device,
    // so we can use our own presense/absence of a color space to decide how to decode the image.
    SkDestinationSurfaceColorMode decodeColorMode = fBitmap.colorSpace()
        ? SkDestinationSurfaceColorMode::kGammaAndColorSpaceAware
        : SkDestinationSurfaceColorMode::kLegacy;
    return SkSpecialImage::MakeFromImage(SkIRect::MakeWH(image->width(), image->height()),
                                         image->makeNonTextureImage(), decodeColorMode);
}

sk_sp<SkSpecialImage> SkBitmapDevice::snapSpecial() {
    return this->makeSpecial(fBitmap);
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkSurface> SkBitmapDevice::makeSurface(const SkImageInfo& info, const SkSurfaceProps& props) {
    return SkSurface::MakeRaster(info, &props);
}

SkImageFilterCache* SkBitmapDevice::getImageFilterCache() {
    SkImageFilterCache* cache = SkImageFilterCache::Get();
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
        !paint.isSrcOver())
    {
        return true;
    }
    return false;
}
