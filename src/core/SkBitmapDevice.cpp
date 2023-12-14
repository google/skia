/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkBitmapDevice.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBlender.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkRasterHandleAllocator.h"
#include "include/core/SkRegion.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTileMode.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkDraw.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkSpecialImage.h"
#include "src/image/SkImage_Base.h"
#include "src/text/GlyphRun.h"

#include <utility>

class SkVertices;

struct Bounder {
    SkRect  fBounds;
    bool    fHasBounds;

    Bounder(const SkRect& r, const SkPaint& paint) {
        if ((fHasBounds = paint.canComputeFastBounds())) {
            fBounds = paint.computeFastBounds(r, &fBounds);
        }
    }

    bool hasBounds() const { return fHasBounds; }
    const SkRect* bounds() const { return fHasBounds ? &fBounds : nullptr; }
    operator const SkRect* () const { return this->bounds(); }
};

class SkDrawTiler {
    enum {
        // 8K is 1 too big, since 8K << supersample == 32768 which is too big for SkFixed
        kMaxDim = 8192 - 1
    };

    SkBitmapDevice* fDevice;
    SkPixmap        fRootPixmap;
    SkIRect         fSrcBounds;

    // Used for tiling and non-tiling
    SkDraw          fDraw;

    // fTileMatrix... are only used if fNeedTiling
    SkTLazy<SkMatrix> fTileMatrix;
    SkRasterClip      fTileRC;
    SkIPoint          fOrigin;

    bool            fDone, fNeedsTiling;

public:
    static bool NeedsTiling(SkBitmapDevice* dev) {
        return dev->width() > kMaxDim || dev->height() > kMaxDim;
    }

    SkDrawTiler(SkBitmapDevice* dev, const SkRect* bounds) : fDevice(dev) {
        fDone = false;

        // we need fDst to be set, and if we're actually drawing, to dirty the genID
        if (!dev->accessPixels(&fRootPixmap)) {
            // NoDrawDevice uses us (why?) so we have to catch this case w/ no pixels
            fRootPixmap.reset(dev->imageInfo(), nullptr, 0);
        }

        // do a quick check, so we don't even have to process "bounds" if there is no need
        const SkIRect clipR = dev->fRCStack.rc().getBounds();
        fNeedsTiling = clipR.right() > kMaxDim || clipR.bottom() > kMaxDim;
        if (fNeedsTiling) {
            if (bounds) {
                // Make sure we round first, and then intersect. We can't rely on promoting the
                // clipR to floats (and then intersecting with devBounds) since promoting
                // int --> float can make the float larger than the int.
                // rounding(out) first runs the risk of clamping if the float is larger an intmax
                // but our roundOut() is saturating, which is fine for this use case
                //
                // e.g. the older version of this code did this:
                //    devBounds = mapRect(bounds);
                //    if (devBounds.intersect(SkRect::Make(clipR))) {
                //        fSrcBounds = devBounds.roundOut();
                // The problem being that the promotion of clipR to SkRect was unreliable
                //
                fSrcBounds = dev->localToDevice().mapRect(*bounds).roundOut();
                if (fSrcBounds.intersect(clipR)) {
                    // Check again, now that we have computed srcbounds.
                    fNeedsTiling = fSrcBounds.right() > kMaxDim || fSrcBounds.bottom() > kMaxDim;
                } else {
                    fNeedsTiling = false;
                    fDone = true;
                }
            } else {
                fSrcBounds = clipR;
            }
        }

        if (fNeedsTiling) {
            // fDraw.fDst and fCTM are reset each time in setupTileDraw()
            fDraw.fRC = &fTileRC;
            // we'll step/increase it before using it
            fOrigin.set(fSrcBounds.fLeft - kMaxDim, fSrcBounds.fTop);
        } else {
            // don't reference fSrcBounds, as it may not have been set
            fDraw.fDst = fRootPixmap;
            fDraw.fCTM = &dev->localToDevice();
            fDraw.fRC = &dev->fRCStack.rc();
            fOrigin.set(0, 0);
        }

        fDraw.fProps = &fDevice->surfaceProps();
    }

    bool needsTiling() const { return fNeedsTiling; }

    const SkDraw* next() {
        if (fDone) {
            return nullptr;
        }
        if (fNeedsTiling) {
            do {
                this->stepAndSetupTileDraw();  // might set the clip to empty and fDone to true
            } while (!fDone && fTileRC.isEmpty());
            // if we exit the loop and we're still empty, we're (past) done
            if (fTileRC.isEmpty()) {
                SkASSERT(fDone);
                return nullptr;
            }
            SkASSERT(!fTileRC.isEmpty());
        } else {
            fDone = true;   // only draw untiled once
        }
        return &fDraw;
    }

private:
    void stepAndSetupTileDraw() {
        SkASSERT(!fDone);
        SkASSERT(fNeedsTiling);

        // We do fRootPixmap.width() - kMaxDim instead of fOrigin.fX + kMaxDim to avoid overflow.
        if (fOrigin.fX >= fSrcBounds.fRight - kMaxDim) {    // too far
            fOrigin.fX = fSrcBounds.fLeft;
            fOrigin.fY += kMaxDim;
        } else {
            fOrigin.fX += kMaxDim;
        }
        // fDone = next origin will be invalid.
        fDone = fOrigin.fX >= fSrcBounds.fRight - kMaxDim &&
                fOrigin.fY >= fSrcBounds.fBottom - kMaxDim;

        SkIRect bounds = SkIRect::MakeXYWH(fOrigin.x(), fOrigin.y(), kMaxDim, kMaxDim);
        SkASSERT(!bounds.isEmpty());
        bool success = fRootPixmap.extractSubset(&fDraw.fDst, bounds);
        SkASSERT_RELEASE(success);
        // now don't use bounds, since fDst has the clipped dimensions.

        fTileMatrix.init(fDevice->localToDevice());
        fTileMatrix->postTranslate(-fOrigin.x(), -fOrigin.y());
        fDraw.fCTM = fTileMatrix.get();
        fDevice->fRCStack.rc().translate(-fOrigin.x(), -fOrigin.y(), &fTileRC);
        fTileRC.op(SkIRect::MakeSize(fDraw.fDst.dimensions()), SkClipOp::kIntersect);
    }
};

// Passing a bounds allows the tiler to only visit the dst-tiles that might intersect the
// drawing. If null is passed, the tiler has to visit everywhere. The bounds is expected to be
// in local coordinates, as the tiler itself will transform that into device coordinates.
//
#define LOOP_TILER(code, boundsPtr)                         \
    SkDrawTiler priv_tiler(this, boundsPtr);                \
    while (const SkDraw* priv_draw = priv_tiler.next()) {   \
        priv_draw->code;                                    \
    }

// Helper to create an SkDraw from a device
class SkBitmapDevice::BDDraw : public SkDraw {
public:
    BDDraw(SkBitmapDevice* dev) {
        // we need fDst to be set, and if we're actually drawing, to dirty the genID
        if (!dev->accessPixels(&fDst)) {
            // NoDrawDevice uses us (why?) so we have to catch this case w/ no pixels
            fDst.reset(dev->imageInfo(), nullptr, 0);
        }
        fCTM = &dev->localToDevice();
        fRC = &dev->fRCStack.rc();
    }
};

static bool valid_for_bitmap_device(const SkImageInfo& info,
                                    SkAlphaType* newAlphaType) {
    if (info.width() < 0 || info.height() < 0 || kUnknown_SkColorType == info.colorType()) {
        return false;
    }

    if (newAlphaType) {
        *newAlphaType = SkColorTypeIsAlwaysOpaque(info.colorType()) ? kOpaque_SkAlphaType
                                                                    : info.alphaType();
    }

    return true;
}

SkBitmapDevice::SkBitmapDevice(const SkBitmap& bitmap)
        : SkDevice(bitmap.info(), SkSurfaceProps())
        , fBitmap(bitmap)
        , fRCStack(bitmap.width(), bitmap.height())
        , fGlyphPainter(this->surfaceProps(), bitmap.colorType(), bitmap.colorSpace()) {
    SkASSERT(valid_for_bitmap_device(bitmap.info(), nullptr));
}

SkBitmapDevice::SkBitmapDevice(const SkBitmap& bitmap, const SkSurfaceProps& surfaceProps,
                               SkRasterHandleAllocator::Handle hndl)
        : SkDevice(bitmap.info(), surfaceProps)
        , fBitmap(bitmap)
        , fRasterHandle(hndl)
        , fRCStack(bitmap.width(), bitmap.height())
        , fGlyphPainter(this->surfaceProps(), bitmap.colorType(), bitmap.colorSpace()) {
    SkASSERT(valid_for_bitmap_device(bitmap.info(), nullptr));
}

sk_sp<SkBitmapDevice> SkBitmapDevice::Create(const SkImageInfo& origInfo,
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
        if (!bitmap.tryAllocPixelsFlags(info, SkBitmap::kZeroPixels_AllocFlag)) {
            return nullptr;
        }
    }

    return sk_make_sp<SkBitmapDevice>(bitmap, surfaceProps, hndl);
}

void SkBitmapDevice::replaceBitmapBackendForRasterSurface(const SkBitmap& bm) {
    SkASSERT(bm.width() == fBitmap.width());
    SkASSERT(bm.height() == fBitmap.height());
    fBitmap = bm;   // intent is to use bm's pixelRef (and rowbytes/config)
}

sk_sp<SkDevice> SkBitmapDevice::createDevice(const CreateInfo& cinfo, const SkPaint* layerPaint) {
    const SkSurfaceProps surfaceProps(this->surfaceProps().flags(), cinfo.fPixelGeometry);

    // Need to force L32 for now if we have an image filter.
    // If filters ever support other colortypes, e.g. F16, we can modify this check.
    SkImageInfo info = cinfo.fInfo;
    if (layerPaint && layerPaint->getImageFilter()) {
        // TODO: can we query the imagefilter, to see if it can handle floats (so we don't always
        //       use N32 when the layer itself was float)?
        info = info.makeColorType(kN32_SkColorType);
    }

    return SkBitmapDevice::Create(info, surfaceProps, cinfo.fAllocator);
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
        pmap->reset(fBitmap.info(), fBitmap.getPixels(), fBitmap.rowBytes());
        return true;
    }
    return false;
}

bool SkBitmapDevice::onWritePixels(const SkPixmap& pm, int x, int y) {
    // since we don't stop creating un-pixeled devices yet, check for no pixels here
    if (nullptr == fBitmap.getPixels()) {
        return false;
    }

    if (fBitmap.writePixels(pm, x, y)) {
        fBitmap.notifyPixelsChanged();
        return true;
    }
    return false;
}

bool SkBitmapDevice::onReadPixels(const SkPixmap& pm, int x, int y) {
    return fBitmap.readPixels(pm, x, y);
}

///////////////////////////////////////////////////////////////////////////////

void SkBitmapDevice::drawPaint(const SkPaint& paint) {
    BDDraw(this).drawPaint(paint);
}

void SkBitmapDevice::drawPoints(SkCanvas::PointMode mode, size_t count,
                                const SkPoint pts[], const SkPaint& paint) {
    LOOP_TILER( drawPoints(mode, count, pts, paint, nullptr), nullptr)
}

void SkBitmapDevice::drawRect(const SkRect& r, const SkPaint& paint) {
    LOOP_TILER( drawRect(r, paint), Bounder(r, paint))
}

void SkBitmapDevice::drawOval(const SkRect& oval, const SkPaint& paint) {
    // call the VIRTUAL version, so any subclasses who do handle drawPath aren't
    // required to override drawOval.
    this->drawPath(SkPath::Oval(oval), paint, true);
}

void SkBitmapDevice::drawRRect(const SkRRect& rrect, const SkPaint& paint) {
#ifdef SK_IGNORE_BLURRED_RRECT_OPT
    // call the VIRTUAL version, so any subclasses who do handle drawPath aren't
    // required to override drawRRect.
    this->drawPath(SkPath::RRect(rrect), paint, true);
#else
    LOOP_TILER( drawRRect(rrect, paint), Bounder(rrect.getBounds(), paint))
#endif
}

void SkBitmapDevice::drawPath(const SkPath& path,
                              const SkPaint& paint,
                              bool pathIsMutable) {
    const SkRect* bounds = nullptr;
    if (SkDrawTiler::NeedsTiling(this) && !path.isInverseFillType()) {
        bounds = &path.getBounds();
    }
    SkDrawTiler tiler(this, bounds ? Bounder(*bounds, paint).bounds() : nullptr);
    if (tiler.needsTiling()) {
        pathIsMutable = false;
    }
    while (const SkDraw* draw = tiler.next()) {
        draw->drawPath(path, paint, nullptr, pathIsMutable);
    }
}

void SkBitmapDevice::drawBitmap(const SkBitmap& bitmap, const SkMatrix& matrix,
                                const SkRect* dstOrNull, const SkSamplingOptions& sampling,
                                const SkPaint& paint) {
    const SkRect* bounds = dstOrNull;
    SkRect storage;
    if (!bounds && SkDrawTiler::NeedsTiling(this)) {
        matrix.mapRect(&storage, SkRect::MakeIWH(bitmap.width(), bitmap.height()));
        Bounder b(storage, paint);
        if (b.hasBounds()) {
            storage = *b.bounds();
            bounds = &storage;
        }
    }
    LOOP_TILER(drawBitmap(bitmap, matrix, dstOrNull, sampling, paint), bounds)
}

static inline bool CanApplyDstMatrixAsCTM(const SkMatrix& m, const SkPaint& paint) {
    if (!paint.getMaskFilter()) {
        return true;
    }

    // Some mask filters parameters (sigma) depend on the CTM/scale.
    return m.getType() <= SkMatrix::kTranslate_Mask;
}

void SkBitmapDevice::drawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                                   const SkSamplingOptions& sampling, const SkPaint& paint,
                                   SkCanvas::SrcRectConstraint constraint) {
    SkASSERT(dst.isFinite());
    SkASSERT(dst.isSorted());

    SkBitmap bitmap;
    // TODO: Elevate direct context requirement to public API and remove cheat.
    auto dContext = as_IB(image)->directContext();
    if (!as_IB(image)->getROPixels(dContext, &bitmap)) {
        return;
    }

    SkRect      bitmapBounds, tmpSrc, tmpDst;
    SkBitmap    tmpBitmap;

    bitmapBounds.setIWH(bitmap.width(), bitmap.height());

    // Compute matrix from the two rectangles
    if (src) {
        tmpSrc = *src;
    } else {
        tmpSrc = bitmapBounds;
    }
    SkMatrix matrix = SkMatrix::RectToRect(tmpSrc, dst);

    const SkRect* dstPtr = &dst;
    const SkBitmap* bitmapPtr = &bitmap;

    // clip the tmpSrc to the bounds of the bitmap, and recompute dstRect if
    // needed (if the src was clipped). No check needed if src==null.
    bool srcIsSubset = false;
    if (src) {
        if (!bitmapBounds.contains(*src)) {
            if (!tmpSrc.intersect(bitmapBounds)) {
                return; // nothing to draw
            }
            // recompute dst, based on the smaller tmpSrc
            matrix.mapRect(&tmpDst, tmpSrc);
            if (!tmpDst.isFinite()) {
                return;
            }
            dstPtr = &tmpDst;
        }
        srcIsSubset = !tmpSrc.contains(bitmapBounds);
    }

    if (srcIsSubset &&
        SkCanvas::kFast_SrcRectConstraint == constraint &&
        sampling != SkSamplingOptions()) {
        // src is smaller than the bounds of the bitmap, and we are filtering, so we don't know
        // how much more of the bitmap we need, so we can't use extractSubset or drawBitmap,
        // but we must use a shader w/ dst bounds (which can access all of the bitmap needed).
        goto USE_SHADER;
    }

    if (srcIsSubset) {
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
        extractedBitmapBounds.setIWH(bitmapPtr->width(), bitmapPtr->height());
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
            this->drawBitmap(*bitmapPtr, matrix, dstPtr, sampling, paint);
            return;
        }
    }

    USE_SHADER:

    // construct a shader, so we can call drawRect with the dst
    auto s = SkMakeBitmapShaderForPaint(paint, *bitmapPtr, SkTileMode::kClamp, SkTileMode::kClamp,
                                        sampling, &matrix, kNever_SkCopyPixelsMode);
    if (!s) {
        return;
    }

    SkPaint paintWithShader(paint);
    paintWithShader.setStyle(SkPaint::kFill_Style);
    paintWithShader.setShader(std::move(s));

    // Call ourself, in case the subclass wanted to share this setup code
    // but handle the drawRect code themselves.
    this->drawRect(*dstPtr, paintWithShader);
}

void SkBitmapDevice::onDrawGlyphRunList(SkCanvas* canvas,
                                        const sktext::GlyphRunList& glyphRunList,
                                        const SkPaint& initialPaint,
                                        const SkPaint& drawingPaint) {
    SkASSERT(!glyphRunList.hasRSXForm());
    LOOP_TILER( drawGlyphRunList(canvas, &fGlyphPainter, glyphRunList, drawingPaint), nullptr )
}

void SkBitmapDevice::drawVertices(const SkVertices* vertices,
                                  sk_sp<SkBlender> blender,
                                  const SkPaint& paint,
                                  bool skipColorXform) {
#ifdef SK_LEGACY_IGNORE_DRAW_VERTICES_BLEND_WITH_NO_SHADER
    if (!paint.getShader()) {
        blender = SkBlender::Mode(SkBlendMode::kDst);
    }
#endif
    BDDraw(this).drawVertices(vertices, std::move(blender), paint, skipColorXform);
}

void SkBitmapDevice::drawMesh(const SkMesh&, sk_sp<SkBlender>, const SkPaint&) {
    // TODO(brianosman): Implement, maybe with a subclass of BitmapDevice that has SkSL support.
}

void SkBitmapDevice::drawAtlas(const SkRSXform xform[],
                               const SkRect tex[],
                               const SkColor colors[],
                               int count,
                               sk_sp<SkBlender> blender,
                               const SkPaint& paint) {
    // set this to true for performance comparisons with the old drawVertices way
    if ((false)) {
        this->SkDevice::drawAtlas(xform, tex, colors, count, std::move(blender), paint);
        return;
    }
    BDDraw(this).drawAtlas(xform, tex, colors, count, std::move(blender), paint);
}

///////////////////////////////////////////////////////////////////////////////

void SkBitmapDevice::drawSpecial(SkSpecialImage* src,
                                 const SkMatrix& localToDevice,
                                 const SkSamplingOptions& sampling,
                                 const SkPaint& paint,
                                 SkCanvas::SrcRectConstraint) {
    SkASSERT(!paint.getImageFilter());
    SkASSERT(!paint.getMaskFilter());
    SkASSERT(!src->isGaneshBacked());
    SkASSERT(!src->isGraphiteBacked());

    SkBitmap resultBM;
    if (SkSpecialImages::AsBitmap(src, &resultBM)) {
        SkDraw draw;
        if (!this->accessPixels(&draw.fDst)) {
          return; // no pixels to draw to so skip it
        }
        draw.fCTM = &localToDevice;
        draw.fRC = &fRCStack.rc();
        draw.drawBitmap(resultBM, SkMatrix::I(), nullptr, sampling, paint);
    }
}
sk_sp<SkSpecialImage> SkBitmapDevice::makeSpecial(const SkBitmap& bitmap) {
    return SkSpecialImages::MakeFromRaster(bitmap.bounds(), bitmap, this->surfaceProps());
}

sk_sp<SkSpecialImage> SkBitmapDevice::makeSpecial(const SkImage* image) {
    return SkSpecialImages::MakeFromRaster(SkIRect::MakeWH(image->width(), image->height()),
                                           image->makeNonTextureImage(),
                                           this->surfaceProps());
}

sk_sp<SkSpecialImage> SkBitmapDevice::snapSpecial(const SkIRect& bounds, bool forceCopy) {
    if (forceCopy) {
        return SkSpecialImages::CopyFromRaster(bounds, fBitmap, this->surfaceProps());
    } else {
        return SkSpecialImages::MakeFromRaster(bounds, fBitmap, this->surfaceProps());
    }
}

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkSurface> SkBitmapDevice::makeSurface(const SkImageInfo& info, const SkSurfaceProps& props) {
    return SkSurfaces::Raster(info, &props);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkBitmapDevice::pushClipStack() {
    fRCStack.save();
}

void SkBitmapDevice::popClipStack() {
    fRCStack.restore();
}

void SkBitmapDevice::clipRect(const SkRect& rect, SkClipOp op, bool aa) {
    fRCStack.clipRect(this->localToDevice(), rect, op, aa);
}

void SkBitmapDevice::clipRRect(const SkRRect& rrect, SkClipOp op, bool aa) {
    fRCStack.clipRRect(this->localToDevice(), rrect, op, aa);
}

void SkBitmapDevice::clipPath(const SkPath& path, SkClipOp op, bool aa) {
    fRCStack.clipPath(this->localToDevice(), path, op, aa);
}

void SkBitmapDevice::onClipShader(sk_sp<SkShader> sh) {
    fRCStack.clipShader(std::move(sh));
}

void SkBitmapDevice::clipRegion(const SkRegion& rgn, SkClipOp op) {
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

void SkBitmapDevice::replaceClip(const SkIRect& rect) {
    // Transform from "global/canvas" coordinates to relative to this device
    SkRect deviceRect = SkMatrixPriv::MapRect(this->globalToDevice(), SkRect::Make(rect));
    fRCStack.replaceClip(deviceRect.round());
}

bool SkBitmapDevice::isClipWideOpen() const {
    const SkRasterClip& rc = fRCStack.rc();
    // If we're AA, we can't be wide-open (we would represent that as BW)
    return rc.isBW() && rc.bwRgn().isRect() &&
           rc.bwRgn().getBounds() == SkIRect{0, 0, this->width(), this->height()};
}

bool SkBitmapDevice::isClipEmpty() const {
    return fRCStack.rc().isEmpty();
}

bool SkBitmapDevice::isClipRect() const {
    const SkRasterClip& rc = fRCStack.rc();
    return !rc.isEmpty() && rc.isRect() && !SkToBool(rc.clipShader());
}

bool SkBitmapDevice::isClipAntiAliased() const {
    const SkRasterClip& rc = fRCStack.rc();
    return !rc.isEmpty() && rc.isAA();
}

void SkBitmapDevice::android_utils_clipAsRgn(SkRegion* rgn) const {
    const SkRasterClip& rc = fRCStack.rc();
    if (rc.isAA()) {
        rgn->setRect(   rc.getBounds());
    } else {
        *rgn = rc.bwRgn();
    }
}

SkIRect SkBitmapDevice::devClipBounds() const {
    return fRCStack.rc().getBounds();
}
