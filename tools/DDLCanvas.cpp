/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvasVirtualEnforcer.h"
#include "SkColorFilter.h"
#include "SkColorSpaceXformCanvas.h"
#include "SkColorSpaceXformer.h"
#include "SkDrawShadowInfo.h"
#include "SkGr.h"
#include "SkGradientShader.h"
#include "SkImageFilter.h"
#include "SkImagePriv.h"
#include "SkImage_Base.h"
#include "SkMakeUnique.h"
#include "SkNoDrawCanvas.h"
#include "SkSurface.h"
#include "SkTLazy.h"

namespace sk_tool_utils {

class SkDDLCanvas : public SkCanvasVirtualEnforcer<SkNoDrawCanvas> {
public:
    SkDDLCanvas(SkCanvas* target)
        : SkCanvasVirtualEnforcer<SkNoDrawCanvas>(SkIRect::MakeSize(target->getBaseLayerSize()))
        , fTarget(target) {
        // Set the matrix and clip to match |fTarget|.  Otherwise, we'll answer queries for
        // bounds/matrix differently than |fTarget| would.
        SkCanvas::onClipRect(SkRect::Make(fTarget->getDeviceClipBounds()),
                             SkClipOp::kIntersect, kHard_ClipEdgeStyle);
        SkCanvas::setMatrix(fTarget->getTotalMatrix());
    }

    SkImageInfo onImageInfo() const override {
        return fTarget->imageInfo();
    }

    void onDrawPaint(const SkPaint& paint) override {
        fTarget->drawPaint(paint);
    }
    void onDrawRect(const SkRect& rect, const SkPaint& paint) override {
        fTarget->drawRect(rect, paint);
    }
    void onDrawRRect(const SkRRect& rrect, const SkPaint& paint) override {
        fTarget->drawRRect(rrect, paint);
    }
    void onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) override {
        fTarget->drawDRRect(outer, inner, paint);
    }
    void onDrawOval(const SkRect& oval, const SkPaint& paint) override {
        fTarget->drawOval(oval, paint);
    }
    void onDrawArc(const SkRect& oval, SkScalar start, SkScalar sweep, bool useCenter,
                   const SkPaint& paint) override {
        fTarget->drawArc(oval, start, sweep, useCenter, paint);
    }
    void onDrawPath(const SkPath& path, const SkPaint& paint) override {
        fTarget->drawPath(path, paint);
    }
    void onDrawRegion(const SkRegion& region, const SkPaint& paint) override {
        fTarget->drawRegion(region, paint);
    }
    void onDrawText(const void* ptr, size_t len, SkScalar x, SkScalar y,
                    const SkPaint& paint) override {
        fTarget->drawText(ptr, len, x, y, paint);
    }
    void onDrawPosText(const void* ptr, size_t len,
                       const SkPoint* xys,
                       const SkPaint& paint) override {
        fTarget->drawPosText(ptr, len, xys, paint);
    }
    void onDrawPosTextH(const void* ptr, size_t len,
                        const SkScalar* xs, SkScalar y,
                        const SkPaint& paint) override {
        fTarget->drawPosTextH(ptr, len, xs, y, paint);
    }
    void onDrawTextOnPath(const void* ptr, size_t len,
                          const SkPath& path, const SkMatrix* matrix,
                          const SkPaint& paint) override {
        fTarget->drawTextOnPath(ptr, len, path, matrix, paint);
    }
    void onDrawTextRSXform(const void* ptr, size_t len,
                           const SkRSXform* xforms, const SkRect* cull,
                           const SkPaint& paint) override {
        fTarget->drawTextRSXform(ptr, len, xforms, cull, paint);
    }
    void onDrawTextBlob(const SkTextBlob* blob,
                        SkScalar x, SkScalar y,
                        const SkPaint& paint) override {
        fTarget->drawTextBlob(blob, x, y, paint);
    }
    void onDrawPatch(const SkPoint cubics[12], const SkColor colors[4], const SkPoint texs[4],
                     SkBlendMode mode, const SkPaint& paint) override {
        fTarget->drawPatch(cubics, colors, texs, mode, paint);
    }
    void onDrawPoints(PointMode mode, size_t count, const SkPoint* pts,
                      const SkPaint& paint) override {
        fTarget->drawPoints(mode, count, pts, paint);
    }
    void onDrawVerticesObject(const SkVertices* vertices, SkBlendMode mode,
                              const SkPaint& paint) override {
        fTarget->drawVertices(vertices, mode, paint);
    }
    void onDrawImage(const SkImage* img, SkScalar l, SkScalar t, const SkPaint* paint) override {
        if (!fTarget->quickReject(SkRect::Make(img->bounds()).makeOffset(l,t))) {
            bool isMipped = this->isMipMapped(img, paint);
            fTarget->drawImage(prepareImage(img).get(), l, t, paint);
        }
    }
    void onDrawImageRect(const SkImage* img,
                         const SkRect* src, const SkRect& dst,
                         const SkPaint* paint, SrcRectConstraint constraint) override {
        if (!fTarget->quickReject(dst)) {
            fTarget->drawImageRect(prepareImage(img).get(),
                                   src ? *src : SkRect::MakeIWH(img->width(), img->height()), dst,
                                   paint, constraint);
        }
    }
    void onDrawImageNine(const SkImage* img,
                         const SkIRect& center, const SkRect& dst,
                         const SkPaint* paint) override {
        if (!fTarget->quickReject(dst)) {
            fTarget->drawImageNine(prepareImage(img).get(), center, dst, paint);
        }
    }
    void onDrawImageLattice(const SkImage* img,
                            const Lattice& lattice, const SkRect& dst,
                            const SkPaint* paint) override {
        if (!fTarget->quickReject(dst)) {
            fTarget->drawImageLattice(prepareImage(img).get(), lattice, dst, paint);
        }
    }
    // TODO: quick reject bitmap draw calls before transforming too?
    void onDrawBitmap(const SkBitmap& bitmap,
                      SkScalar l, SkScalar t,
                      const SkPaint* paint) override {
        return fTarget->drawBitmap(bitmap, l, t, paint);
    }
    void onDrawBitmapRect(const SkBitmap& bitmap,
                          const SkRect* src, const SkRect& dst,
                          const SkPaint* paint, SrcRectConstraint constraint) override {
        return fTarget->drawBitmapRect(bitmap,
                 src ? *src : SkRect::MakeIWH(bitmap.width(), bitmap.height()), dst,
                 paint, constraint);
    }
    void onDrawBitmapNine(const SkBitmap& bitmap,
                          const SkIRect& center, const SkRect& dst,
                          const SkPaint* paint) override {
        return fTarget->drawBitmapNine(bitmap, center, dst, paint);
    }
    void onDrawBitmapLattice(const SkBitmap& bitmap,
                             const Lattice& lattice, const SkRect& dst,
                             const SkPaint* paint) override {
        return fTarget->drawBitmapLattice(bitmap, lattice, dst, paint);
    }
    void onDrawAtlas(const SkImage* atlas, const SkRSXform* xforms, const SkRect* tex,
                     const SkColor* colors, int count, SkBlendMode mode,
                     const SkRect* cull, const SkPaint* paint) override {
        fTarget->drawAtlas(prepareImage(atlas).get(), xforms, tex, colors, count, mode, cull,
                           paint);
    }
    void onDrawAnnotation(const SkRect& rect, const char* key, SkData* val) override {
        fTarget->drawAnnotation(rect, key, val);
    }
    void onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) override {
        SkCanvas::onDrawDrawable(drawable, matrix);
    }
    void onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) override {
        fTarget->private_draw_shadow_rec(path, rec);
    }
    void onDrawPicture(const SkPicture* pic, const SkMatrix* matrix, const SkPaint* paint) override {
        SkCanvas::onDrawPicture(pic, matrix, paint);
    }
    void onClipRect(const SkRect& clip, SkClipOp op, ClipEdgeStyle style) override {
        SkCanvas::onClipRect(clip, op, style);
        fTarget->clipRect(clip, op, style);
    }
    void onClipRRect(const SkRRect& clip, SkClipOp op, ClipEdgeStyle style) override {
        SkCanvas::onClipRRect(clip, op, style);
        fTarget->clipRRect(clip, op, style);
    }
    void onClipPath(const SkPath& clip, SkClipOp op, ClipEdgeStyle style) override {
        SkCanvas::onClipPath(clip, op, style);
        fTarget->clipPath(clip, op, style);
    }
    void onClipRegion(const SkRegion& clip, SkClipOp op) override {
        SkCanvas::onClipRegion(clip, op);
        fTarget->clipRegion(clip, op);
    }

    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& rec) override {
#if 0
        sk_sp<SkImageFilter> backdrop = rec.fBackdrop ? fXformer->apply(rec.fBackdrop) : nullptr;
        sk_sp<SkImage> clipMask = rec.fClipMask ? fXformer->apply(rec.fClipMask) : nullptr;
        fTarget->saveLayer({
            rec.fBounds,
            MaybePaint(rec.fPaint, fXformer.get()),
            backdrop.get(),
            clipMask.get(),
            rec.fClipMatrix,
            rec.fSaveLayerFlags,
        });
#else
        return kNoLayer_SaveLayerStrategy;
#endif
    }

#ifdef SK_SUPPORT_LEGACY_DRAWFILTER
    SkDrawFilter* setDrawFilter(SkDrawFilter* filter) override {
        SkCanvas::setDrawFilter(filter);
        return fTarget->setDrawFilter(filter);
    }
#endif

    // Everything from here on should be uninteresting strictly proxied state-change calls.
    void willSave()    override { fTarget->save(); }
    void willRestore() override { fTarget->restore(); }

    void didConcat   (const SkMatrix& m) override { fTarget->concat   (m); }
    void didSetMatrix(const SkMatrix& m) override { fTarget->setMatrix(m); }

    sk_sp<SkSurface> onNewSurface(const SkImageInfo& info, const SkSurfaceProps& props) override {
        return fTarget->makeSurface(info, &props);
    }

    SkISize getBaseLayerSize() const override { return fTarget->getBaseLayerSize(); }
    bool isClipEmpty() const override { return fTarget->isClipEmpty(); }
    bool isClipRect() const override { return fTarget->isClipRect(); }
    bool onPeekPixels(SkPixmap* pixmap) override { return fTarget->peekPixels(pixmap); }
    bool onAccessTopLayerPixels(SkPixmap* pixmap) override {
        SkImageInfo info;
        size_t rowBytes;
        SkIPoint* origin = nullptr;
        void* addr = fTarget->accessTopLayerPixels(&info, &rowBytes, origin);
        if (addr) {
            *pixmap = SkPixmap(info, addr, rowBytes);
            return true;
        }
        return false;
    }

    GrContext* getGrContext() override { return fTarget->getGrContext(); }
    bool onGetProps(SkSurfaceProps* props) const override { return fTarget->getProps(props); }
    void onFlush() override { return fTarget->flush(); }
    GrRenderTargetContext* internal_private_accessTopLayerRenderTargetContext() override {
        return fTarget->internal_private_accessTopLayerRenderTargetContext();
    }

private:
    sk_sp<SkImage> prepareImage(const SkImage* image) {
        return sk_ref_sp(image);
#if 0
        GrContext* gr = fTarget->getGrContext();
        if (gr) {
            // If fTarget is GPU-accelerated, we want to upload to a texture
            // before applying the transform. This way, we can get cache hits
            // in the texture cache and the transform gets applied on the GPU.
            sk_sp<SkImage> textureImage = image->makeTextureImage(gr, nullptr);
            if (textureImage)
                return fXformer->apply(textureImage.get());
        }
        // TODO: Extract a sub image corresponding to the src rect in order
        // to xform only the useful part of the image. Sub image could be reduced
        // even further by taking into account dst_rect+ctm+clip
        return fXformer->apply(image);
#endif
    }

    bool isMipMapped(SkImage* img, const SkPaint& paint) {
        bool doBicubic;
        GrSamplerState::Filter textureFilterMode = GrSkFilterQualityToGrFilterMode(
                paint.getFilterQuality(), this->ctm(), srcToDstMatrix,
                fContext->contextPriv().sharpenMipmappedTextures(), &doBicubic);

        return false;
    }

    SkCanvas*                            fTarget;
};

std::unique_ptr<SkCanvas> CreateDDLCanvas(SkCanvas* target) {
    return skstd::make_unique<SkDDLCanvas>(target);
}

}; // sk_tool_utils
