/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapDevice.h"
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
#include "SkRasterHandleAllocator.h"
#include "SkShader.h"
#include "SkSpecialImage.h"
#include "SkSurface.h"
#include "SkVertices.h"

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
    , fRCStack(bitmap.width(), bitmap.height())
{
    SkASSERT(valid_for_bitmap_device(bitmap.info(), nullptr));
    fBitmap.lockPixels();
}

SkBitmapDevice* SkBitmapDevice::Create(const SkImageInfo& info) {
    return Create(info, SkSurfaceProps(SkSurfaceProps::kLegacyFontHost_InitType));
}

SkBitmapDevice::SkBitmapDevice(const SkBitmap& bitmap, const SkSurfaceProps& surfaceProps,
                               SkRasterHandleAllocator::Handle hndl)
    : INHERITED(bitmap.info(), surfaceProps)
    , fBitmap(bitmap)
    , fRasterHandle(hndl)
    , fRCStack(bitmap.width(), bitmap.height())
{
    SkASSERT(valid_for_bitmap_device(bitmap.info(), nullptr));
    fBitmap.lockPixels();
}

SkBitmapDevice* SkBitmapDevice::Create(const SkImageInfo& origInfo,
                                       const SkSurfaceProps& surfaceProps,
                                       SkRasterHandleAllocator* allocator) {
    SkAlphaType newAT = origInfo.alphaType();
    if (!valid_for_bitmap_device(origInfo, &newAT)) {
        return nullptr;
    }

    SkRasterHandleAllocator::Handle hndl = nullptr;
    const SkImageInfo info = origInfo.makeAlphaType(newAT);
    SkBitmap bitmap;

    if (kUnknown_SkColorType == info.colorType()) {
        if (!bitmap.setInfo(info)) {
            return nullptr;
        }
    } else if (allocator) {
        hndl = allocator->allocBitmap(info, &bitmap);
        if (!hndl) {
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
        // We use the flag as a faster alloc-then-eraseColor(SK_ColorTRANSPARENT).
        if (!bitmap.tryAllocPixels(info, nullptr/*colortable*/, SkBitmap::kZeroPixels_AllocFlag)) {
            return nullptr;
        }
    }

    return new SkBitmapDevice(bitmap, surfaceProps, hndl);
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
    return SkBitmapDevice::Create(cinfo.fInfo, surfaceProps, cinfo.fAllocator);
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

    if (fBitmap.writePixels(SkPixmap(srcInfo, srcPixels, srcRowBytes), x, y)) {
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

class SkBitmapDevice::BDDraw : public SkDraw {
public:
    BDDraw(SkBitmapDevice* dev) {
        // we need fDst to be set, and if we're actually drawing, to dirty the genID
        if (!dev->accessPixels(&fDst)) {
            // NoDrawDevice uses us (why?) so we have to catch this case w/ no pixels
            fDst.reset(dev->imageInfo(), nullptr, 0);
        }
        fMatrix = &dev->ctm();
        fRC = &dev->fRCStack.rc();
    }
};

void SkBitmapDevice::drawPaint(const SkPaint& paint) {
    BDDraw(this).drawPaint(paint);
}

void SkBitmapDevice::drawPoints(SkCanvas::PointMode mode, size_t count,
                                const SkPoint pts[], const SkPaint& paint) {
    BDDraw(this).drawPoints(mode, count, pts, paint, nullptr);
}

void SkBitmapDevice::drawRect(const SkRect& r, const SkPaint& paint) {
    BDDraw(this).drawRect(r, paint);
}

void SkBitmapDevice::drawOval(const SkRect& oval, const SkPaint& paint) {
    SkPath path;
    path.addOval(oval);
    // call the VIRTUAL version, so any subclasses who do handle drawPath aren't
    // required to override drawOval.
    this->drawPath(path, paint, nullptr, true);
}

void SkBitmapDevice::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
#ifdef SK_IGNORE_BLURRED_RRECT_OPT
    SkPath  path;

    path.addRRect(rrect);
    // call the VIRTUAL version, so any subclasses who do handle drawPath aren't
    // required to override drawRRect.
    this->drawPath(path, paint, nullptr, true);
#else
    BDDraw(this).drawRRect(rrect, paint);
#endif
}

void SkBitmapDevice::drawPath(const SkPath& path,
                              const SkPaint& paint, const SkMatrix* prePathMatrix,
                              bool pathIsMutable) {
    BDDraw(this).drawPath(path, paint, prePathMatrix, pathIsMutable);
}

void SkBitmapDevice::drawBitmap(const SkBitmap& bitmap,
                                const SkMatrix& matrix, const SkPaint& paint) {
    LogDrawScaleFactor(SkMatrix::Concat(this->ctm(), matrix), paint.getFilterQuality());
    BDDraw(this).drawBitmap(bitmap, matrix, nullptr, paint);
}

static inline bool CanApplyDstMatrixAsCTM(const SkMatrix& m, const SkPaint& paint) {
    if (!paint.getMaskFilter()) {
        return true;
    }

    // Some mask filters parameters (sigma) depend on the CTM/scale.
    return m.getType() <= SkMatrix::kTranslate_Mask;
}

void SkBitmapDevice::drawBitmapRect(const SkBitmap& bitmap,
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

    LogDrawScaleFactor(SkMatrix::Concat(this->ctm(), matrix), paint.getFilterQuality());

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

#ifdef SK_DRAWBITMAPRECT_FAST_OFFSET
        SkRect extractedBitmapBounds = SkRect::MakeXYWH(dx, dy,
                                                        SkIntToScalar(bitmapPtr->width()),
                                                        SkIntToScalar(bitmapPtr->height()));
#else
        SkRect extractedBitmapBounds;
        extractedBitmapBounds.isetWH(bitmapPtr->width(), bitmapPtr->height());
#endif
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
            BDDraw(this).drawBitmap(*bitmapPtr, matrix, dstPtr, paint);
            return;
        }
    }

    USE_SHADER:

    // TODO(herb): Move this over to SkArenaAlloc when arena alloc has a facility to return sk_sps.
    // Since the shader need only live for our stack-frame, pass in a custom allocator. This
    // can save malloc calls, and signals to SkMakeBitmapShader to not try to copy the bitmap
    // if its mutable, since that precaution is not needed (give the short lifetime of the shader).

    // construct a shader, so we can call drawRect with the dst
    auto s = SkMakeBitmapShader(*bitmapPtr, SkShader::kClamp_TileMode, SkShader::kClamp_TileMode,
                                &matrix, kNever_SkCopyPixelsMode);
    if (!s) {
        return;
    }

    SkPaint paintWithShader(paint);
    paintWithShader.setStyle(SkPaint::kFill_Style);
    paintWithShader.setShader(s);

    // Call ourself, in case the subclass wanted to share this setup code
    // but handle the drawRect code themselves.
    this->drawRect(*dstPtr, paintWithShader);
}

void SkBitmapDevice::drawSprite(const SkBitmap& bitmap, int x, int y, const SkPaint& paint) {
    BDDraw(this).drawSprite(bitmap, x, y, paint);
}

void SkBitmapDevice::drawText(const void* text, size_t len,
                              SkScalar x, SkScalar y, const SkPaint& paint) {
    BDDraw(this).drawText((const char*)text, len, x, y, paint, &fSurfaceProps);
}

void SkBitmapDevice::drawPosText(const void* text, size_t len, const SkScalar xpos[],
                                 int scalarsPerPos, const SkPoint& offset, const SkPaint& paint) {
    BDDraw(this).drawPosText((const char*)text, len, xpos, scalarsPerPos, offset, paint,
                             &fSurfaceProps);
}

void SkBitmapDevice::drawVertices(const SkVertices* vertices, SkBlendMode bmode,
                                  const SkPaint& paint) {
    BDDraw(this).drawVertices(vertices->mode(), vertices->vertexCount(), vertices->positions(),
                              vertices->texCoords(), vertices->colors(), bmode,
                              vertices->indices(), vertices->indexCount(), paint);
}

void SkBitmapDevice::drawDevice(SkBaseDevice* device, int x, int y, const SkPaint& paint) {
    SkASSERT(!paint.getImageFilter());
    BDDraw(this).drawSprite(static_cast<SkBitmapDevice*>(device)->fBitmap, x, y, paint);
}

///////////////////////////////////////////////////////////////////////////////

void SkBitmapDevice::drawSpecial(SkSpecialImage* srcImg, int x, int y,
                                 const SkPaint& paint) {
    SkASSERT(!srcImg->isTextureBacked());

    SkBitmap resultBM;

    SkImageFilter* filter = paint.getImageFilter();
    if (filter) {
        SkIPoint offset = SkIPoint::Make(0, 0);
        SkMatrix matrix = this->ctm();
        matrix.postTranslate(SkIntToScalar(-x), SkIntToScalar(-y));
        const SkIRect clipBounds = fRCStack.rc().getBounds().makeOffset(-x, -y);
        sk_sp<SkImageFilterCache> cache(this->getImageFilterCache());
        SkImageFilter::OutputProperties outputProperties(fBitmap.colorSpace());
        SkImageFilter::Context ctx(matrix, clipBounds, cache.get(), outputProperties);

        sk_sp<SkSpecialImage> resultImg(filter->filterImage(srcImg, ctx, &offset));
        if (resultImg) {
            SkPaint tmpUnfiltered(paint);
            tmpUnfiltered.setImageFilter(nullptr);
            if (resultImg->getROPixels(&resultBM)) {
                this->drawSprite(resultBM, x + offset.x(), y + offset.y(), tmpUnfiltered);
            }
        }
    } else {
        if (srcImg->getROPixels(&resultBM)) {
            this->drawSprite(resultBM, x, y, paint);
        }
    }
}

sk_sp<SkSpecialImage> SkBitmapDevice::makeSpecial(const SkBitmap& bitmap) {
    return SkSpecialImage::MakeFromRaster(bitmap.bounds(), bitmap);
}

sk_sp<SkSpecialImage> SkBitmapDevice::makeSpecial(const SkImage* image) {
    return SkSpecialImage::MakeFromImage(SkIRect::MakeWH(image->width(), image->height()),
                                         image->makeNonTextureImage(), fBitmap.colorSpace());
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

///////////////////////////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkBitmapDevice::onSave() {
    fRCStack.save();
}

void SkBitmapDevice::onRestore() {
    fRCStack.restore();
}

void SkBitmapDevice::onClipRect(const SkRect& rect, SkClipOp op, bool aa) {
    fRCStack.clipRect(this->ctm(), rect, op, aa);
}

void SkBitmapDevice::onClipRRect(const SkRRect& rrect, SkClipOp op, bool aa) {
    fRCStack.clipRRect(this->ctm(), rrect, op, aa);
}

void SkBitmapDevice::onClipPath(const SkPath& path, SkClipOp op, bool aa) {
    fRCStack.clipPath(this->ctm(), path, op, aa);
}

void SkBitmapDevice::onClipRegion(const SkRegion& rgn, SkClipOp op) {
    SkIPoint origin = this->getOrigin();
    SkRegion tmp;
    const SkRegion* ptr = &rgn;
    if (origin.fX | origin.fY) {
        // translate from "global/canvas" coordinates to relative to this device
        rgn.translate(-origin.fX, -origin.fY, &tmp);
        ptr = &tmp;
    }
    fRCStack.clipRegion(*ptr, op);
}

void SkBitmapDevice::onSetDeviceClipRestriction(SkIRect* mutableClipRestriction) {
    fRCStack.setDeviceClipRestriction(mutableClipRestriction);
    if (!mutableClipRestriction->isEmpty()) {
        SkRegion rgn(*mutableClipRestriction);
        fRCStack.clipRegion(rgn, SkClipOp::kIntersect);
    }
}

bool SkBitmapDevice::onClipIsAA() const {
    const SkRasterClip& rc = fRCStack.rc();
    return !rc.isEmpty() && rc.isAA();
}

void SkBitmapDevice::onAsRgnClip(SkRegion* rgn) const {
    const SkRasterClip& rc = fRCStack.rc();
    if (rc.isAA()) {
        rgn->setRect(rc.getBounds());
    } else {
        *rgn = rc.bwRgn();
    }
}

void SkBitmapDevice::validateDevBounds(const SkIRect& drawClipBounds) {
#ifdef SK_DEBUG
    const SkIRect& stackBounds = fRCStack.rc().getBounds();
    SkASSERT(drawClipBounds == stackBounds);
#endif
}

SkBaseDevice::ClipType SkBitmapDevice::onGetClipType() const {
    const SkRasterClip& rc = fRCStack.rc();
    if (rc.isEmpty()) {
        return kEmpty_ClipType;
    } else if (rc.isRect()) {
        return kRect_ClipType;
    } else {
        return kComplex_ClipType;
    }
}
