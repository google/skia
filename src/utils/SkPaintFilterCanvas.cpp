/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkPaintFilterCanvas.h"

#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSurface.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkTLazy.h"

class SkPaintFilterCanvas::AutoPaintFilter {
public:
    AutoPaintFilter(const SkPaintFilterCanvas* canvas, const SkPaint* paint)
        : fPaint(paint ? *paint : SkPaint()) {
        fShouldDraw = canvas->onFilter(fPaint);
    }

    AutoPaintFilter(const SkPaintFilterCanvas* canvas, const SkPaint& paint)
        : AutoPaintFilter(canvas, &paint) { }

    const SkPaint& paint() const { return fPaint; }

    bool shouldDraw() const { return fShouldDraw; }

private:
    SkPaint fPaint;
    bool fShouldDraw;
};

SkPaintFilterCanvas::SkPaintFilterCanvas(SkCanvas *canvas)
    : SkCanvasVirtualEnforcer<SkNWayCanvas>(canvas->imageInfo().width(),
                                              canvas->imageInfo().height()) {

    // Transfer matrix & clip state before adding the target canvas.
    this->clipRect(SkRect::Make(canvas->getDeviceClipBounds()));
    this->setMatrix(canvas->getLocalToDevice());

    this->addCanvas(canvas);
}

void SkPaintFilterCanvas::onDrawPaint(const SkPaint& paint) {
    AutoPaintFilter apf(this, paint);
    if (apf.shouldDraw()) {
        this->SkNWayCanvas::onDrawPaint(apf.paint());
    }
}

void SkPaintFilterCanvas::onDrawBehind(const SkPaint& paint) {
    AutoPaintFilter apf(this, paint);
    if (apf.shouldDraw()) {
        this->SkNWayCanvas::onDrawBehind(apf.paint());
    }
}

void SkPaintFilterCanvas::onDrawPoints(PointMode mode, size_t count, const SkPoint pts[],
                                       const SkPaint& paint) {
    AutoPaintFilter apf(this, paint);
    if (apf.shouldDraw()) {
        this->SkNWayCanvas::onDrawPoints(mode, count, pts, apf.paint());
    }
}

void SkPaintFilterCanvas::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    AutoPaintFilter apf(this, paint);
    if (apf.shouldDraw()) {
        this->SkNWayCanvas::onDrawRect(rect, apf.paint());
    }
}

void SkPaintFilterCanvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    AutoPaintFilter apf(this, paint);
    if (apf.shouldDraw()) {
        this->SkNWayCanvas::onDrawRRect(rrect, apf.paint());
    }
}

void SkPaintFilterCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                                       const SkPaint& paint) {
    AutoPaintFilter apf(this, paint);
    if (apf.shouldDraw()) {
        this->SkNWayCanvas::onDrawDRRect(outer, inner, apf.paint());
    }
}

void SkPaintFilterCanvas::onDrawRegion(const SkRegion& region, const SkPaint& paint) {
    AutoPaintFilter apf(this, paint);
    if (apf.shouldDraw()) {
        this->SkNWayCanvas::onDrawRegion(region, apf.paint());
    }
}

void SkPaintFilterCanvas::onDrawOval(const SkRect& rect, const SkPaint& paint) {
    AutoPaintFilter apf(this, paint);
    if (apf.shouldDraw()) {
        this->SkNWayCanvas::onDrawOval(rect, apf.paint());
    }
}

void SkPaintFilterCanvas::onDrawArc(const SkRect& rect, SkScalar startAngle, SkScalar sweepAngle,
                                    bool useCenter, const SkPaint& paint) {
    AutoPaintFilter apf(this, paint);
    if (apf.shouldDraw()) {
        this->SkNWayCanvas::onDrawArc(rect, startAngle, sweepAngle, useCenter, apf.paint());
    }
}

void SkPaintFilterCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    AutoPaintFilter apf(this, paint);
    if (apf.shouldDraw()) {
        this->SkNWayCanvas::onDrawPath(path, apf.paint());
    }
}

void SkPaintFilterCanvas::onDrawImage2(const SkImage* image, SkScalar left, SkScalar top,
                                       const SkSamplingOptions& sampling, const SkPaint* paint) {
    AutoPaintFilter apf(this, paint);
    if (apf.shouldDraw()) {
        this->SkNWayCanvas::onDrawImage2(image, left, top, sampling, &apf.paint());
    }
}

void SkPaintFilterCanvas::onDrawImageRect2(const SkImage* image, const SkRect& src,
                                           const SkRect& dst, const SkSamplingOptions& sampling,
                                           const SkPaint* paint, SrcRectConstraint constraint) {
    AutoPaintFilter apf(this, paint);
    if (apf.shouldDraw()) {
        this->SkNWayCanvas::onDrawImageRect2(image, src, dst, sampling, &apf.paint(), constraint);
    }
}

void SkPaintFilterCanvas::onDrawImageLattice2(const SkImage* image, const Lattice& lattice,
                                              const SkRect& dst, SkFilterMode filter,
                                              const SkPaint* paint) {
    AutoPaintFilter apf(this, paint);
    if (apf.shouldDraw()) {
        this->SkNWayCanvas::onDrawImageLattice2(image, lattice, dst, filter, &apf.paint());
    }
}

void SkPaintFilterCanvas::onDrawVerticesObject(const SkVertices* vertices,
                                               SkBlendMode bmode, const SkPaint& paint) {
    AutoPaintFilter apf(this, paint);
    if (apf.shouldDraw()) {
        this->SkNWayCanvas::onDrawVerticesObject(vertices, bmode, apf.paint());
    }
}

void SkPaintFilterCanvas::onDrawPatch(const SkPoint cubics[], const SkColor colors[],
                                      const SkPoint texCoords[], SkBlendMode bmode,
                                      const SkPaint& paint) {
    AutoPaintFilter apf(this, paint);
    if (apf.shouldDraw()) {
        this->SkNWayCanvas::onDrawPatch(cubics, colors, texCoords, bmode, apf.paint());
    }
}

void SkPaintFilterCanvas::onDrawPicture(const SkPicture* picture, const SkMatrix* m,
                                        const SkPaint* originalPaint) {
    AutoPaintFilter apf(this, originalPaint);
    if (apf.shouldDraw()) {
        const SkPaint* newPaint = &apf.paint();

        // Passing a paint (-vs- passing null) makes drawPicture draw into a layer...
        // much slower, and can produce different blending. Thus we should only do this
        // if the filter's effect actually impacts the picture.
        if (originalPaint == nullptr) {
            if (   newPaint->getAlphaf()      == 1.0f
                && newPaint->getColorFilter() == nullptr
                && newPaint->getImageFilter() == nullptr
                && newPaint->asBlendMode()    == SkBlendMode::kSrcOver) {
                // restore the original nullptr
                newPaint = nullptr;
            }
        }
        this->SkNWayCanvas::onDrawPicture(picture, m, newPaint);
    }
}

void SkPaintFilterCanvas::onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) {
    // There is no paint to filter in this case, but we can still filter on type.
    // Subclasses need to unroll the drawable explicity (by overriding this method) in
    // order to actually filter nested content.
    AutoPaintFilter apf(this, nullptr);
    if (apf.shouldDraw()) {
        this->SkNWayCanvas::onDrawDrawable(drawable, matrix);
    }
}

void SkPaintFilterCanvas::onDrawGlyphRunList(const SkGlyphRunList& list, const SkPaint& paint) {
    AutoPaintFilter apf(this, paint);
    if (apf.shouldDraw()) {
        this->SkNWayCanvas::onDrawGlyphRunList(list, apf.paint());
    }
}

void SkPaintFilterCanvas::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                         const SkPaint& paint) {
    AutoPaintFilter apf(this, paint);
    if (apf.shouldDraw()) {
        this->SkNWayCanvas::onDrawTextBlob(blob, x, y, apf.paint());
    }
}

void SkPaintFilterCanvas::onDrawAtlas2(const SkImage* image, const SkRSXform xform[],
                                       const SkRect tex[], const SkColor colors[], int count,
                                       SkBlendMode bmode, const SkSamplingOptions& sampling,
                                       const SkRect* cull, const SkPaint* paint) {
    AutoPaintFilter apf(this, paint);
    if (apf.shouldDraw()) {
        this->SkNWayCanvas::onDrawAtlas2(image, xform, tex, colors, count, bmode, sampling, cull,
                                         &apf.paint());
    }
}

void SkPaintFilterCanvas::onDrawAnnotation(const SkRect& rect, const char key[], SkData* value) {
    this->SkNWayCanvas::onDrawAnnotation(rect, key, value);
}

void SkPaintFilterCanvas::onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) {
    this->SkNWayCanvas::onDrawShadowRec(path, rec);
}

void SkPaintFilterCanvas::onDrawEdgeAAQuad(const SkRect& rect, const SkPoint clip[4],
                                           QuadAAFlags aa, const SkColor4f& color, SkBlendMode mode) {
    SkPaint paint;
    paint.setColor(color);
    paint.setBlendMode(mode);
    AutoPaintFilter apf(this, paint);
    if (apf.shouldDraw()) {
        this->SkNWayCanvas::onDrawEdgeAAQuad(rect, clip, aa, apf.paint().getColor4f(),
                                             apf.paint().getBlendMode_or(SkBlendMode::kSrcOver));
    }
}

void SkPaintFilterCanvas::onDrawEdgeAAImageSet2(const ImageSetEntry set[], int count,
                                                const SkPoint dstClips[],
                                                const SkMatrix preViewMatrices[],
                                                const SkSamplingOptions& sampling,
                                                const SkPaint* paint,
                                                SrcRectConstraint constraint) {
    AutoPaintFilter apf(this, paint);
    if (apf.shouldDraw()) {
        this->SkNWayCanvas::onDrawEdgeAAImageSet2(
                set, count, dstClips, preViewMatrices, sampling, &apf.paint(), constraint);
    }
}

sk_sp<SkSurface> SkPaintFilterCanvas::onNewSurface(const SkImageInfo& info,
                                                   const SkSurfaceProps& props) {
    return this->proxy()->makeSurface(info, &props);
}

bool SkPaintFilterCanvas::onPeekPixels(SkPixmap* pixmap) {
    return this->proxy()->peekPixels(pixmap);
}

bool SkPaintFilterCanvas::onAccessTopLayerPixels(SkPixmap* pixmap) {
    SkImageInfo info;
    size_t rowBytes;

    void* addr = this->proxy()->accessTopLayerPixels(&info, &rowBytes);
    if (!addr) {
        return false;
    }

    pixmap->reset(info, addr, rowBytes);
    return true;
}

SkImageInfo SkPaintFilterCanvas::onImageInfo() const {
    return this->proxy()->imageInfo();
}

bool SkPaintFilterCanvas::onGetProps(SkSurfaceProps* props) const {
    return this->proxy()->getProps(props);
}
