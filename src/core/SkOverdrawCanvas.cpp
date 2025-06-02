/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkOverdrawCanvas.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorType.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurfaceProps.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkPoint_impl.h"
#include "include/private/base/SkTDArray.h"
#include "src/base/SkZip.h"
#include "src/core/SkDevice.h"
#include "src/core/SkDrawShadowInfo.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkGlyphRunPainter.h"
#include "src/core/SkLatticeIter.h"
#include "src/core/SkMask.h"
#include "src/text/GlyphRun.h"

class SkBitmap;
class SkData;
class SkPicture;
class SkRRect;
class SkRegion;
class SkTextBlob;
class SkVertices;

SkOverdrawCanvas::SkOverdrawCanvas(SkCanvas* canvas)
    : INHERITED(canvas->onImageInfo().width(), canvas->onImageInfo().height())
{
    // Non-drawing calls that SkOverdrawCanvas does not override (translate, save, etc.)
    // will pass through to the input canvas.
    this->addCanvas(canvas);

    static constexpr float kIncrementAlpha[] = {
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f/255,
    };

    fPaint.setAntiAlias(false);
    fPaint.setBlendMode(SkBlendMode::kPlus);
    fPaint.setColorFilter(SkColorFilters::Matrix(kIncrementAlpha));
}

namespace {
class TextDevice : public SkNoPixelsDevice, public SkGlyphRunListPainterCPU::BitmapDevicePainter {
public:
    TextDevice(SkCanvas* overdrawCanvas, const SkSurfaceProps& props)
            : SkNoPixelsDevice{SkIRect::MakeWH(32767, 32767), props},
              fOverdrawCanvas{overdrawCanvas},
              fPainter{props, kN32_SkColorType, nullptr} {}

    void paintMasks(SkZip<const SkGlyph*, SkPoint> accepted, const SkPaint& paint) const override {
        for (auto [glyph, pos] : accepted) {
            SkMask mask = glyph->mask(pos);
            // We need to ignore any matrix on the overdraw canvas (it's already been baked into
            // our glyph positions). Otherwise, the CTM is double-applied. (skbug.com/40044818)
            fOverdrawCanvas->save();
            fOverdrawCanvas->resetMatrix();
            fOverdrawCanvas->drawRect(SkRect::Make(mask.fBounds), SkPaint());
            fOverdrawCanvas->restore();
        }
    }

    void drawBitmap(const SkBitmap&, const SkMatrix&, const SkRect* dstOrNull,
                    const SkSamplingOptions&, const SkPaint&) const override {}

    void onDrawGlyphRunList(SkCanvas* canvas,
                            const sktext::GlyphRunList& glyphRunList,
                            const SkPaint& paint) override {
        SkASSERT(!glyphRunList.hasRSXForm());
        fPainter.drawForBitmapDevice(
                canvas, this, glyphRunList, paint, fOverdrawCanvas->getTotalMatrix());
    }

private:
    SkCanvas* const fOverdrawCanvas;
    SkGlyphRunListPainterCPU fPainter;
};
}  // namespace

void SkOverdrawCanvas::onDrawTextBlob(
        const SkTextBlob* blob, SkScalar x, SkScalar y, const SkPaint& paint) {
    sktext::GlyphRunBuilder b;
    auto glyphRunList = b.blobToGlyphRunList(*blob, {x, y});
    this->onDrawGlyphRunList(glyphRunList, paint);
}

void SkOverdrawCanvas::onDrawGlyphRunList(const sktext::GlyphRunList& glyphRunList,
                                          const SkPaint& paint) {
    SkSurfaceProps props;
    this->getProps(&props);
    TextDevice device{this, props};

    device.drawGlyphRunList(this, glyphRunList, paint);
}

void SkOverdrawCanvas::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                                   const SkPoint texCoords[4], SkBlendMode blendMode,
                                   const SkPaint&) {
    fList[0]->onDrawPatch(cubics, colors, texCoords, blendMode, fPaint);
}

void SkOverdrawCanvas::onDrawPaint(const SkPaint& paint) {
    if (0 == paint.getColor() && !paint.getColorFilter() && !paint.getShader()) {
        // This is a clear, ignore it.
    } else {
        fList[0]->onDrawPaint(this->overdrawPaint(paint));
    }
}

void SkOverdrawCanvas::onDrawBehind(const SkPaint& paint) {
    fList[0]->onDrawBehind(this->overdrawPaint(paint));
}

void SkOverdrawCanvas::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    fList[0]->onDrawRect(rect, this->overdrawPaint(paint));
}

void SkOverdrawCanvas::onDrawRegion(const SkRegion& region, const SkPaint& paint) {
    fList[0]->onDrawRegion(region, this->overdrawPaint(paint));
}

void SkOverdrawCanvas::onDrawOval(const SkRect& oval, const SkPaint& paint) {
    fList[0]->onDrawOval(oval, this->overdrawPaint(paint));
}

void SkOverdrawCanvas::onDrawArc(const SkRect& arc, SkScalar startAngle, SkScalar sweepAngle,
                                 bool useCenter, const SkPaint& paint) {
    fList[0]->onDrawArc(arc, startAngle, sweepAngle, useCenter, this->overdrawPaint(paint));
}

void SkOverdrawCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                                    const SkPaint& paint) {
    fList[0]->onDrawDRRect(outer, inner, this->overdrawPaint(paint));
}

void SkOverdrawCanvas::onDrawRRect(const SkRRect& rect, const SkPaint& paint) {
    fList[0]->onDrawRRect(rect, this->overdrawPaint(paint));
}

void SkOverdrawCanvas::onDrawPoints(PointMode mode, size_t count, const SkPoint points[],
                                    const SkPaint& paint) {
    fList[0]->onDrawPoints(mode, count, points, this->overdrawPaint(paint));
}

void SkOverdrawCanvas::onDrawVerticesObject(const SkVertices* vertices,
                                            SkBlendMode blendMode, const SkPaint& paint) {
    fList[0]->onDrawVerticesObject(vertices, blendMode, this->overdrawPaint(paint));
}

void SkOverdrawCanvas::onDrawAtlas2(const SkImage* image, const SkRSXform xform[],
                                    const SkRect texs[], const SkColor colors[], int count,
                                    SkBlendMode mode, const SkSamplingOptions& sampling,
                                    const SkRect* cull, const SkPaint* paint) {
    SkPaint* paintPtr = &fPaint;
    SkPaint storage;
    if (paint) {
        storage = this->overdrawPaint(*paint);
        paintPtr = &storage;
    }

    fList[0]->onDrawAtlas2(image, xform, texs, colors, count, mode, sampling, cull, paintPtr);
}

void SkOverdrawCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    fList[0]->onDrawPath(path, fPaint);
}

void SkOverdrawCanvas::onDrawImage2(const SkImage* image, SkScalar x, SkScalar y,
                                    const SkSamplingOptions&, const SkPaint*) {
    fList[0]->onDrawRect(SkRect::MakeXYWH(x, y, image->width(), image->height()), fPaint);
}

void SkOverdrawCanvas::onDrawImageRect2(const SkImage* image, const SkRect& src, const SkRect& dst,
                                        const SkSamplingOptions&, const SkPaint*, SrcRectConstraint) {
    fList[0]->onDrawRect(dst, fPaint);
}

void SkOverdrawCanvas::onDrawImageLattice2(const SkImage* image, const Lattice& lattice,
                                           const SkRect& dst, SkFilterMode, const SkPaint*) {
    SkIRect bounds;
    Lattice latticePlusBounds = lattice;
    if (!latticePlusBounds.fBounds) {
        bounds = SkIRect::MakeWH(image->width(), image->height());
        latticePlusBounds.fBounds = &bounds;
    }

    if (SkLatticeIter::Valid(image->width(), image->height(), latticePlusBounds)) {
        SkLatticeIter iter(latticePlusBounds, dst);

        SkRect ignored, iterDst;
        while (iter.next(&ignored, &iterDst)) {
            fList[0]->onDrawRect(iterDst, fPaint);
        }
    } else {
        fList[0]->onDrawRect(dst, fPaint);
    }
}

void SkOverdrawCanvas::onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) {
    drawable->draw(this, matrix);
}

void SkOverdrawCanvas::onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) {
    SkASSERT(false);
}

void SkOverdrawCanvas::onDrawAnnotation(const SkRect&, const char[], SkData*) {}

void SkOverdrawCanvas::onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) {
    SkRect bounds;
    SkDrawShadowMetrics::GetLocalBounds(path, rec, this->getTotalMatrix(), &bounds);
    fList[0]->onDrawRect(bounds, fPaint);
}

void SkOverdrawCanvas::onDrawEdgeAAQuad(const SkRect& rect, const SkPoint clip[4],
                                        QuadAAFlags aa, const SkColor4f& color, SkBlendMode mode) {
    if (clip) {
        fList[0]->onDrawPath(SkPath::Polygon({clip, 4}, true), fPaint);
    } else {
        fList[0]->onDrawRect(rect, fPaint);
    }
}

void SkOverdrawCanvas::onDrawEdgeAAImageSet2(const ImageSetEntry set[], int count,
                                             const SkPoint dstClips[],
                                             const SkMatrix preViewMatrices[],
                                             const SkSamplingOptions& sampling,
                                             const SkPaint* paint,
                                             SrcRectConstraint constraint) {
    int clipIndex = 0;
    for (int i = 0; i < count; ++i) {
        if (set[i].fMatrixIndex >= 0) {
            fList[0]->save();
            fList[0]->concat(preViewMatrices[set[i].fMatrixIndex]);
        }
        if (set[i].fHasClip) {
            fList[0]->onDrawPath(SkPath::Polygon({dstClips + clipIndex, 4}, true), fPaint);
            clipIndex += 4;
        } else {
            fList[0]->onDrawRect(set[i].fDstRect, fPaint);
        }
        if (set[i].fMatrixIndex >= 0) {
            fList[0]->restore();
        }
    }
}

inline SkPaint SkOverdrawCanvas::overdrawPaint(const SkPaint& paint) {
    SkPaint newPaint = fPaint;
    newPaint.setStyle(paint.getStyle());
    newPaint.setStrokeWidth(paint.getStrokeWidth());
    return newPaint;
}
