/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvasVirtualEnforcer.h"
#include "SkColorFilter.h"
#include "SkColorSpaceXformCanvas.h"
#include "SkColorSpaceXformer.h"
#include "SkDrawShadowInfo.h"
#include "SkGradientShader.h"
#include "SkImageFilter.h"
#include "SkImagePriv.h"
#include "SkImage_Base.h"
#include "SkMakeUnique.h"
#include "SkNoDrawCanvas.h"
#include "SkSurface.h"
#include "SkTLazy.h"

namespace {
    struct MaybePaint {
       SkTLazy<SkPaint> fStorage;
       const SkPaint* fPaint = nullptr;
       MaybePaint(const SkPaint* p, SkColorSpaceXformer* xformer) {
           if (p) { fPaint = fStorage.set(xformer->apply(*p)); }
       }
       operator const SkPaint*() const { return fPaint; }
    };
};

class SkColorSpaceXformCanvas : public SkCanvasVirtualEnforcer<SkNoDrawCanvas> {
public:
    SkColorSpaceXformCanvas(SkCanvas* target, sk_sp<SkColorSpace> targetCS,
                            std::unique_ptr<SkColorSpaceXformer> xformer)
        : SkCanvasVirtualEnforcer<SkNoDrawCanvas>(SkIRect::MakeSize(target->getBaseLayerSize()))
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
        return fTarget->imageInfo().makeColorSpace(fTargetCS);
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

    void onDrawVerticesObject(const SkVertices* vertices, const SkVertices::Bone bones[], int boneCount,
                              SkBlendMode mode, const SkPaint& paint) override {
        sk_sp<SkVertices> copy;
        if (vertices->hasColors()) {
            int count = vertices->vertexCount();
            SkSTArray<8, SkColor> xformed(count);
            fXformer->apply(xformed.begin(), vertices->colors(), count);
            copy = SkVertices::MakeCopy(vertices->mode(), count, vertices->positions(),
                                        vertices->texCoords(), xformed.begin(),
                                        vertices->boneIndices(), vertices->boneWeights(),
                                        vertices->indexCount(), vertices->indices());
            vertices = copy.get();
        }

        fTarget->drawVertices(vertices, bones, boneCount, mode, fXformer->apply(paint));
    }

    void onDrawTextBlob(const SkTextBlob* blob,
                        SkScalar x, SkScalar y,
                        const SkPaint& paint) override {
        fTarget->drawTextBlob(blob, x, y, fXformer->apply(paint));
    }

    void onDrawImage(const SkImage* img,
                     SkScalar l, SkScalar t,
                     const SkPaint* paint) override {
        if (!fTarget->quickReject(SkRect::Make(img->bounds()).makeOffset(l,t))) {
            fTarget->drawImage(prepareImage(img).get(), l, t, MaybePaint(paint, fXformer.get()));
        }
    }
    void onDrawImageRect(const SkImage* img,
                         const SkRect* src, const SkRect& dst,
                         const SkPaint* paint, SrcRectConstraint constraint) override {
        if (!fTarget->quickReject(dst)) {
            fTarget->drawImageRect(prepareImage(img).get(),
                                   src ? *src : SkRect::MakeIWH(img->width(), img->height()), dst,
                                   MaybePaint(paint, fXformer.get()), constraint);
        }
    }
    void onDrawImageNine(const SkImage* img,
                         const SkIRect& center, const SkRect& dst,
                         const SkPaint* paint) override {
        if (!fTarget->quickReject(dst)) {
            fTarget->drawImageNine(prepareImage(img).get(), center, dst,
                                   MaybePaint(paint, fXformer.get()));
        }
    }
    void onDrawImageLattice(const SkImage* img,
                            const Lattice& lattice, const SkRect& dst,
                            const SkPaint* paint) override {
        if (!fTarget->quickReject(dst)) {
            SkSTArray<16, SkColor> colorBuffer;
            int count = lattice.fRectTypes && lattice.fColors ?
                        (lattice.fXCount + 1) * (lattice.fYCount + 1) : 0;
            colorBuffer.reset(count);
            fTarget->drawImageLattice(prepareImage(img).get(),
                                      fXformer->apply(lattice, colorBuffer.begin(), count),
                                      dst, MaybePaint(paint, fXformer.get()));
        }
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
        fTarget->drawAtlas(prepareImage(atlas).get(), xforms, tex, colors, count, mode, cull,
                           MaybePaint(paint, fXformer.get()));
    }

    // TODO: quick reject bitmap draw calls before transforming too?
    void onDrawBitmap(const SkBitmap& bitmap,
                      SkScalar l, SkScalar t,
                      const SkPaint* paint) override {
        if (this->skipXform(bitmap)) {
            return fTarget->drawBitmap(bitmap, l, t, MaybePaint(paint, fXformer.get()));
        }

        fTarget->drawImage(fXformer->apply(bitmap).get(), l, t, MaybePaint(paint, fXformer.get()));
    }
    void onDrawBitmapRect(const SkBitmap& bitmap,
                          const SkRect* src, const SkRect& dst,
                          const SkPaint* paint, SrcRectConstraint constraint) override {
        if (this->skipXform(bitmap)) {
            return fTarget->drawBitmapRect(bitmap,
                    src ? *src : SkRect::MakeIWH(bitmap.width(), bitmap.height()), dst,
                    MaybePaint(paint, fXformer.get()), constraint);
        }

        fTarget->drawImageRect(fXformer->apply(bitmap).get(),
                               src ? *src : SkRect::MakeIWH(bitmap.width(), bitmap.height()), dst,
                               MaybePaint(paint, fXformer.get()), constraint);
    }
    void onDrawBitmapNine(const SkBitmap& bitmap,
                          const SkIRect& center, const SkRect& dst,
                          const SkPaint* paint) override {
        if (this->skipXform(bitmap)) {
            return fTarget->drawBitmapNine(bitmap, center, dst, MaybePaint(paint, fXformer.get()));
        }

        fTarget->drawImageNine(fXformer->apply(bitmap).get(), center, dst,
                               MaybePaint(paint, fXformer.get()));

    }
    void onDrawBitmapLattice(const SkBitmap& bitmap,
                             const Lattice& lattice, const SkRect& dst,
                             const SkPaint* paint) override {
        if (this->skipXform(bitmap)) {
            return fTarget->drawBitmapLattice(bitmap, lattice, dst,
                                              MaybePaint(paint, fXformer.get()));
        }

        SkSTArray<16, SkColor> colorBuffer;
        int count = lattice.fRectTypes && lattice.fColors?
                    (lattice.fXCount + 1) * (lattice.fYCount + 1) : 0;
        colorBuffer.reset(count);
        fTarget->drawImageLattice(fXformer->apply(bitmap).get(),
                                  fXformer->apply(lattice, colorBuffer.begin(), count), dst,
                                  MaybePaint(paint, fXformer.get()));
    }
    void onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) override {
        SkDrawShadowRec newRec(rec);
        newRec.fAmbientColor = fXformer->apply(rec.fAmbientColor);
        newRec.fSpotColor = fXformer->apply(rec.fSpotColor);
        fTarget->private_draw_shadow_rec(path, newRec);
    }
    void onDrawPicture(const SkPicture* pic,
                       const SkMatrix* matrix,
                       const SkPaint* paint) override {
        SkCanvas::onDrawPicture(pic, matrix, MaybePaint(paint, fXformer.get()));
    }
    void onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) override {
        SkCanvas::onDrawDrawable(drawable, matrix);
    }

    void onDrawEdgeAAQuad(const SkRect& rect, const SkPoint clip[4],
                          QuadAAFlags aa, SkColor color, SkBlendMode mode) override {
        fTarget->experimental_DrawEdgeAAQuad(
                rect, clip, aa, fXformer->apply(color), mode);
    }
    void onDrawEdgeAAImageSet(const ImageSetEntry set[], int count,
                              const SkPoint dstClips[], const SkMatrix preViewMatrices[],
                              const SkPaint* paint, SrcRectConstraint constraint) override {
        SkAutoTArray<ImageSetEntry> xformedSet(count);
        for (int i = 0; i < count; ++i) {
            xformedSet[i].fImage = this->prepareImage(set[i].fImage.get());
            xformedSet[i].fSrcRect = set[i].fSrcRect;
            xformedSet[i].fDstRect = set[i].fDstRect;
            xformedSet[i].fMatrixIndex = set[i].fMatrixIndex;
            xformedSet[i].fAlpha = set[i].fAlpha;
            xformedSet[i].fAAFlags = set[i].fAAFlags;
            xformedSet[i].fHasClip = set[i].fHasClip;
        }
        fTarget->experimental_DrawEdgeAAImageSet(xformedSet.get(), count, dstClips, preViewMatrices,
                                                 MaybePaint(paint, fXformer.get()), constraint);
    }

    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& rec) override {
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
        return kNoLayer_SaveLayerStrategy;
    }

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
        GrContext* gr = fTarget->getGrContext();
        // If fTarget is GPU-accelerated, we want to upload to a texture before applying the
        // transform. This way, we can get cache hits in the texture cache and the transform gets
        // applied on the GPU.
        if (gr) {
            sk_sp<SkImage> textureImage = image->makeTextureImage(gr, nullptr);
            if (textureImage) {
                return fXformer->apply(textureImage.get());
            }
        }
        // TODO: Extract a sub image corresponding to the src rect in order
        // to xform only the useful part of the image. Sub image could be reduced
        // even further by taking into account dst_rect+ctm+clip
        return fXformer->apply(image);
    }

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
