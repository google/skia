/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilter.h"
#include "SkColorSpaceXformCanvas.h"
#include "SkColorSpaceXformer.h"
#include "SkGradientShader.h"
#include "SkImage_Base.h"
#include "SkImagePriv.h"
#include "SkMakeUnique.h"
#include "SkNoDrawCanvas.h"
#include "SkSurface.h"

class SkColorSpaceXformCanvas : public SkNoDrawCanvas {
public:
    SkColorSpaceXformCanvas(SkCanvas* target, sk_sp<SkColorSpace> targetCS,
                            std::unique_ptr<SkColorSpaceXformer> xformer)
        : SkNoDrawCanvas(SkIRect::MakeSize(target->getBaseLayerSize()))
        , fTarget(target)
        , fTargetCS(targetCS)
        , fXformer(std::move(xformer))
    {
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
        fTarget->drawPaint(fXformer->apply(paint));
    }

    void onDrawRect(const SkRect& rect, const SkPaint& paint) override {
        fTarget->drawRect(rect, fXformer->apply(paint));
    }
    void onDrawOval(const SkRect& oval, const SkPaint& paint) override {
        fTarget->drawOval(oval, fXformer->apply(paint));
    }
    void onDrawRRect(const SkRRect& rrect, const SkPaint& paint) override {
        fTarget->drawRRect(rrect, fXformer->apply(paint));
    }
    void onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) override {
        fTarget->drawDRRect(outer, inner, fXformer->apply(paint));
    }
    void onDrawPath(const SkPath& path, const SkPaint& paint) override {
        fTarget->drawPath(path, fXformer->apply(paint));
    }
    void onDrawArc(const SkRect& oval, SkScalar start, SkScalar sweep, bool useCenter,
                   const SkPaint& paint) override {
        fTarget->drawArc(oval, start, sweep, useCenter, fXformer->apply(paint));
    }
    void onDrawRegion(const SkRegion& region, const SkPaint& paint) override {
        fTarget->drawRegion(region, fXformer->apply(paint));
    }
    void onDrawPatch(const SkPoint cubics[12], const SkColor colors[4], const SkPoint texs[4],
                     SkBlendMode mode, const SkPaint& paint) override {
        SkColor xformed[4];
        if (colors) {
            fXformer->apply(xformed, colors, 4);
            colors = xformed;
        }

        fTarget->drawPatch(cubics, colors, texs, mode, fXformer->apply(paint));
    }
    void onDrawPoints(PointMode mode, size_t count, const SkPoint* pts,
                      const SkPaint& paint) override {
        fTarget->drawPoints(mode, count, pts, fXformer->apply(paint));
    }
    void onDrawVerticesObject(const SkVertices* vertices, SkBlendMode mode,
                              const SkPaint& paint) override {
        sk_sp<SkVertices> copy;
        if (vertices->hasColors()) {
            int count = vertices->vertexCount();
            SkSTArray<8, SkColor> xformed(count);
            fXformer->apply(xformed.begin(), vertices->colors(), count);
            copy = SkVertices::MakeCopy(vertices->mode(), count, vertices->positions(),
                                        vertices->texCoords(), xformed.begin(),
                                        vertices->indexCount(), vertices->indices());
            vertices = copy.get();
        }

        fTarget->drawVertices(vertices, mode, fXformer->apply(paint));
    }

    void onDrawText(const void* ptr, size_t len,
                    SkScalar x, SkScalar y,
                    const SkPaint& paint) override {
        fTarget->drawText(ptr, len, x, y, fXformer->apply(paint));
    }
    void onDrawPosText(const void* ptr, size_t len,
                       const SkPoint* xys,
                       const SkPaint& paint) override {
        fTarget->drawPosText(ptr, len, xys, fXformer->apply(paint));
    }
    void onDrawPosTextH(const void* ptr, size_t len,
                        const SkScalar* xs, SkScalar y,
                        const SkPaint& paint) override {
        fTarget->drawPosTextH(ptr, len, xs, y, fXformer->apply(paint));
    }
    void onDrawTextOnPath(const void* ptr, size_t len,
                          const SkPath& path, const SkMatrix* matrix,
                          const SkPaint& paint) override {
        fTarget->drawTextOnPath(ptr, len, path, matrix, fXformer->apply(paint));
    }
    void onDrawTextRSXform(const void* ptr, size_t len,
                           const SkRSXform* xforms, const SkRect* cull,
                           const SkPaint& paint) override {
        fTarget->drawTextRSXform(ptr, len, xforms, cull, fXformer->apply(paint));
    }
    void onDrawTextBlob(const SkTextBlob* blob,
                        SkScalar x, SkScalar y,
                        const SkPaint& paint) override {
        fTarget->drawTextBlob(blob, x, y, fXformer->apply(paint));
    }

    void onDrawImage(const SkImage* img,
                     SkScalar l, SkScalar t,
                     const SkPaint* paint) override {
        fTarget->drawImage(fXformer->apply(img).get(),
                           l, t,
                           fXformer->apply(paint));
    }
    void onDrawImageRect(const SkImage* img,
                         const SkRect* src, const SkRect& dst,
                         const SkPaint* paint, SrcRectConstraint constraint) override {
        fTarget->drawImageRect(fXformer->apply(img).get(),
                               src ? *src : SkRect::MakeIWH(img->width(), img->height()), dst,
                               fXformer->apply(paint), constraint);
    }
    void onDrawImageNine(const SkImage* img,
                         const SkIRect& center, const SkRect& dst,
                         const SkPaint* paint) override {
        fTarget->drawImageNine(fXformer->apply(img).get(),
                               center, dst,
                               fXformer->apply(paint));
    }
    void onDrawImageLattice(const SkImage* img,
                            const Lattice& lattice, const SkRect& dst,
                            const SkPaint* paint) override {
        fTarget->drawImageLattice(fXformer->apply(img).get(),
                                  lattice, dst,
                                  fXformer->apply(paint));
    }
    void onDrawAtlas(const SkImage* atlas, const SkRSXform* xforms, const SkRect* tex,
                     const SkColor* colors, int count, SkBlendMode mode,
                     const SkRect* cull, const SkPaint* paint) override {
        SkSTArray<8, SkColor> xformed;
        if (colors) {
            xformed.reset(count);
            fXformer->apply(xformed.begin(), colors, count);
            colors = xformed.begin();
        }

        fTarget->drawAtlas(fXformer->apply(atlas).get(), xforms, tex, colors, count, mode, cull,
                           fXformer->apply(paint));
    }

    void onDrawBitmap(const SkBitmap& bitmap,
                      SkScalar l, SkScalar t,
                      const SkPaint* paint) override {
        if (this->skipXform(bitmap)) {
            return fTarget->drawBitmap(bitmap, l, t, fXformer->apply(paint));
        }

        fTarget->drawImage(fXformer->apply(bitmap).get(), l, t, fXformer->apply(paint));
    }
    void onDrawBitmapRect(const SkBitmap& bitmap,
                          const SkRect* src, const SkRect& dst,
                          const SkPaint* paint, SrcRectConstraint constraint) override {
        if (this->skipXform(bitmap)) {
            return fTarget->drawBitmapRect(bitmap,
                    src ? *src : SkRect::MakeIWH(bitmap.width(), bitmap.height()), dst,
                    fXformer->apply(paint), constraint);
        }

        fTarget->drawImageRect(fXformer->apply(bitmap).get(),
                               src ? *src : SkRect::MakeIWH(bitmap.width(), bitmap.height()), dst,
                               fXformer->apply(paint), constraint);
    }
    void onDrawBitmapNine(const SkBitmap& bitmap,
                          const SkIRect& center, const SkRect& dst,
                          const SkPaint* paint) override {
        if (this->skipXform(bitmap)) {
            return fTarget->drawBitmapNine(bitmap, center, dst, fXformer->apply(paint));
        }

        fTarget->drawImageNine(fXformer->apply(bitmap).get(), center, dst, fXformer->apply(paint));

    }
    void onDrawBitmapLattice(const SkBitmap& bitmap,
                             const Lattice& lattice, const SkRect& dst,
                             const SkPaint* paint) override {
        if (this->skipXform(bitmap)) {
            return fTarget->drawBitmapLattice(bitmap, lattice, dst, fXformer->apply(paint));
        }


        fTarget->drawImageLattice(fXformer->apply(bitmap).get(), lattice, dst,
                                  fXformer->apply(paint));
    }

    // TODO: May not be ideal to unfurl pictures.
    void onDrawPicture(const SkPicture* pic,
                       const SkMatrix* matrix,
                       const SkPaint* paint) override {
        SkCanvas::onDrawPicture(pic, matrix, fXformer->apply(paint));
    }
    void onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) override {
        SkCanvas::onDrawDrawable(drawable, matrix);
    }

    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& rec) override {
        fTarget->saveLayer({
            rec.fBounds,
            fXformer->apply(rec.fPaint),
            rec.fBackdrop,  // TODO: this is an image filter
            rec.fSaveLayerFlags,
        });
        return kNoLayer_SaveLayerStrategy;
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

    void onDrawAnnotation(const SkRect& rect, const char* key, SkData* val) override {
        fTarget->drawAnnotation(rect, key, val);
    }

    sk_sp<SkSurface> onNewSurface(const SkImageInfo& info, const SkSurfaceProps& props) override {
        return fTarget->makeSurface(info, &props);
    }

    SkISize getBaseLayerSize() const override { return fTarget->getBaseLayerSize(); }
    SkRect onGetLocalClipBounds() const override { return fTarget->getLocalClipBounds(); }
    SkIRect onGetDeviceClipBounds() const override { return fTarget->getDeviceClipBounds(); }
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

    bool onGetProps(SkSurfaceProps* props) const override { return fTarget->getProps(props); }
    void onFlush() override { return fTarget->flush(); }

private:
    bool skipXform(const SkBitmap& bitmap) {
        return (!bitmap.colorSpace() && fTargetCS->isSRGB()) ||
               (SkColorSpace::Equals(bitmap.colorSpace(), fTargetCS.get())) ||
               (kAlpha_8_SkColorType == bitmap.colorType());
    }

    SkCanvas*                            fTarget;
    sk_sp<SkColorSpace>                  fTargetCS;
    std::unique_ptr<SkColorSpaceXformer> fXformer;
};

std::unique_ptr<SkCanvas> SkCreateColorSpaceXformCanvas(SkCanvas* target,
                                                        sk_sp<SkColorSpace> targetCS) {
    std::unique_ptr<SkColorSpaceXformer> xformer = SkColorSpaceXformer::Make(targetCS);
    if (!xformer) {
        return nullptr;
    }

    return skstd::make_unique<SkColorSpaceXformCanvas>(target, std::move(targetCS),
                                                       std::move(xformer));
}
