/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPaintFilterCanvas_DEFINED
#define SkPaintFilterCanvas_DEFINED

#include "SkCanvasVirtualEnforcer.h"
#include "SkNWayCanvas.h"
#include "SkTLazy.h"

class SkAndroidFrameworkUtils;

/** \class SkPaintFilterCanvas

    A utility proxy base class for implementing draw/paint filters.
*/
class SK_API SkPaintFilterCanvas : public SkCanvasVirtualEnforcer<SkNWayCanvas> {
public:
    /**
     * The new SkPaintFilterCanvas is configured for forwarding to the
     * specified canvas.  Also copies the target canvas matrix and clip bounds.
     */
    SkPaintFilterCanvas(SkCanvas* canvas);

    enum Type {
        kPaint_Type,
        kPoint_Type,
        kArc_Type,
        kBitmap_Type,
        kRect_Type,
        kRRect_Type,
        kDRRect_Type,
        kOval_Type,
        kPath_Type,
        kPicture_Type,
        kDrawable_Type,
        kText_Type,
        kTextBlob_Type,
        kVertices_Type,
        kPatch_Type,

        kTypeCount
    };

    // Forwarded to the wrapped canvas.
    SkISize getBaseLayerSize() const override { return proxy()->getBaseLayerSize(); }
    GrContext*  getGrContext() override { return proxy()->getGrContext();     }
    GrRenderTargetContext* internal_private_accessTopLayerRenderTargetContext() override {
        return proxy()->internal_private_accessTopLayerRenderTargetContext();
    }

protected:
    /**
     *  Called with the paint that will be used to draw the specified type.
     *  The implementation may modify the paint as they wish (using SkTCopyOnFirstWrite::writable).
     *
     *  The result bool is used to determine whether the draw op is to be
     *  executed (true) or skipped (false).
     *
     *  Note: The base implementation calls onFilter() for top-level/explicit paints only.
     *        To also filter encapsulated paints (e.g. SkPicture, SkTextBlob), clients may need to
     *        override the relevant methods (i.e. drawPicture, drawTextBlob).
     */
    virtual bool onFilter(SkTCopyOnFirstWrite<SkPaint>* paint, Type type) const = 0;

    void onDrawPaint(const SkPaint&) override;
    void onDrawBehind(const SkPaint&) override;
    void onDrawPoints(PointMode, size_t count, const SkPoint pts[], const SkPaint&) override;
    void onDrawRect(const SkRect&, const SkPaint&) override;
    void onDrawEdgeAARect(const SkRect&, SkCanvas::QuadAAFlags, SkColor, SkBlendMode) override;
    void onDrawRRect(const SkRRect&, const SkPaint&) override;
    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override;
    void onDrawRegion(const SkRegion&, const SkPaint&) override;
    void onDrawOval(const SkRect&, const SkPaint&) override;
    void onDrawArc(const SkRect&, SkScalar, SkScalar, bool, const SkPaint&) override;
    void onDrawPath(const SkPath&, const SkPaint&) override;
    void onDrawBitmap(const SkBitmap&, SkScalar left, SkScalar top, const SkPaint*) override;
    void onDrawBitmapRect(const SkBitmap&, const SkRect* src, const SkRect& dst, const SkPaint*,
                          SrcRectConstraint) override;
    void onDrawBitmapNine(const SkBitmap&, const SkIRect& center, const SkRect& dst,
                          const SkPaint*) override;
    void onDrawBitmapLattice(const SkBitmap&, const Lattice&, const SkRect&,
                             const SkPaint*) override;
    void onDrawImage(const SkImage*, SkScalar left, SkScalar top, const SkPaint*) override;
    void onDrawImageRect(const SkImage*, const SkRect* src, const SkRect& dst,
                         const SkPaint*, SrcRectConstraint) override;
    void onDrawImageNine(const SkImage*, const SkIRect& center, const SkRect& dst,
                         const SkPaint*) override;
    void onDrawImageLattice(const SkImage*, const Lattice&, const SkRect&,
                            const SkPaint*) override;
    void onDrawImageSet(const SkCanvas::ImageSetEntry[], int count, SkFilterQuality,
                        SkBlendMode) override;
    void onDrawVerticesObject(const SkVertices*, const SkVertices::Bone bones[], int boneCount,
                              SkBlendMode, const SkPaint&) override;
    void onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                             const SkPoint texCoords[4], SkBlendMode,
                             const SkPaint& paint) override;
    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override;
    void onDrawDrawable(SkDrawable*, const SkMatrix*) override;

    void onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                        const SkPaint& paint) override;
    void onDrawAtlas(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[],
                     int, SkBlendMode, const SkRect*, const SkPaint*) override;
    void onDrawAnnotation(const SkRect& rect, const char key[], SkData* value) override;
    void onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) override;

    // Forwarded to the wrapped canvas.
    sk_sp<SkSurface> onNewSurface(const SkImageInfo&, const SkSurfaceProps&) override;
    bool onPeekPixels(SkPixmap* pixmap) override;
    bool onAccessTopLayerPixels(SkPixmap* pixmap) override;
    SkImageInfo onImageInfo() const override;
    bool onGetProps(SkSurfaceProps* props) const override;

private:
    class AutoPaintFilter;

    SkCanvas* proxy() const { SkASSERT(fList.count() == 1); return fList[0]; }

    SkPaintFilterCanvas* internal_private_asPaintFilterCanvas() const override {
        return const_cast<SkPaintFilterCanvas*>(this);
    }

    friend class SkAndroidFrameworkUtils;
};

#endif
