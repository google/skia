/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOverdrawCanvas.h"

#include "SkColorFilter.h"
#include "SkDrawShadowInfo.h"
#include "SkDrawable.h"
#include "SkFindAndPlaceGlyph.h"
#include "SkImagePriv.h"
#include "SkLatticeIter.h"
#include "SkPatchUtils.h"
#include "SkPath.h"
#include "SkRRect.h"
#include "SkRSXform.h"
#include "SkStrikeCache.h"
#include "SkTextBlob.h"
#include "SkTextBlobPriv.h"
#include "SkTo.h"

namespace {
class ProcessOneGlyphBounds {
public:
    ProcessOneGlyphBounds(SkOverdrawCanvas* canvas)
        : fCanvas(canvas)
    {}

    void operator()(const SkGlyph& glyph, SkPoint position, SkPoint rounding) {
        int left = SkScalarFloorToInt(position.fX) + glyph.fLeft;
        int top = SkScalarFloorToInt(position.fY) + glyph.fTop;
        int right = left + glyph.fWidth;
        int bottom = top + glyph.fHeight;
        fCanvas->onDrawRect(SkRect::MakeLTRB(left, top, right, bottom), SkPaint());
    }

private:
    SkOverdrawCanvas* fCanvas;
};
};

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
            0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    };

    fPaint.setAntiAlias(false);
    fPaint.setBlendMode(SkBlendMode::kPlus);
    fPaint.setColorFilter(SkColorFilter::MakeMatrixFilterRowMajor255(kIncrementAlpha));
}

void SkOverdrawCanvas::drawPosTextCommon(const SkGlyphID glyphs[], int count, const SkScalar pos[],
                                         int scalarsPerPos, const SkPoint& offset,
                                         const SkFont& font, const SkPaint& paint) {
    ProcessOneGlyphBounds processBounds(this);
    SkSurfaceProps props(0, kUnknown_SkPixelGeometry);
    this->getProps(&props);
    auto cache = SkStrikeCache::FindOrCreateStrikeExclusive(
                                font, paint, props,
                                SkScalerContextFlags::kNone, this->getTotalMatrix());
    SkFindAndPlaceGlyph::ProcessPosText(glyphs, count,
                                        SkPoint::Make(0, 0), SkMatrix(), (const SkScalar*) pos, 2,
                                        cache.get(), processBounds);
}

void SkOverdrawCanvas::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                      const SkPaint& paint) {
    SkTextBlobRunIterator it(blob);
    for (;!it.done(); it.next()) {
        const SkPoint& offset = it.offset();
        switch (it.positioning()) {
            case SkTextBlobRunIterator::kDefault_Positioning:
                SK_ABORT("This canvas does not support draw text.");
                break;
            case SkTextBlobRunIterator::kHorizontal_Positioning:
                this->drawPosTextCommon(it.glyphs(), it.glyphCount(), it.pos(), 1,
                                        SkPoint::Make(x, y + offset.y()), it.font(), paint);
                break;
            case SkTextBlobRunIterator::kFull_Positioning:
                this->drawPosTextCommon(it.glyphs(), it.glyphCount(), it.pos(), 2, {x, y},
                                        it.font(), paint);
                break;
            case SkTextBlobRunIterator::kRSXform_Positioning:
                // unimplemented ...
                break;
        }
    }
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

void SkOverdrawCanvas::onDrawEdgeAARect(const SkRect& rect, SkCanvas::QuadAAFlags aa, SkColor color,
                                        SkBlendMode mode) {
    fList[0]->onDrawRect(rect, fPaint);
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
                                            const SkVertices::Bone bones[], int boneCount,
                                            SkBlendMode blendMode, const SkPaint& paint) {
    fList[0]->onDrawVerticesObject(vertices,
                                   bones,
                                   boneCount,
                                   blendMode,
                                   this->overdrawPaint(paint));
}

void SkOverdrawCanvas::onDrawAtlas(const SkImage* image, const SkRSXform xform[],
                                   const SkRect texs[], const SkColor colors[], int count,
                                   SkBlendMode mode, const SkRect* cull, const SkPaint* paint) {
    SkPaint* paintPtr = &fPaint;
    SkPaint storage;
    if (paint) {
        storage = this->overdrawPaint(*paint);
        paintPtr = &storage;
    }

    fList[0]->onDrawAtlas(image, xform, texs, colors, count, mode, cull, paintPtr);
}

void SkOverdrawCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    fList[0]->onDrawPath(path, fPaint);
}

void SkOverdrawCanvas::onDrawImage(const SkImage* image, SkScalar x, SkScalar y, const SkPaint*) {
    fList[0]->onDrawRect(SkRect::MakeXYWH(x, y, image->width(), image->height()), fPaint);
}

void SkOverdrawCanvas::onDrawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                                       const SkPaint*, SrcRectConstraint) {
    fList[0]->onDrawRect(dst, fPaint);
}

void SkOverdrawCanvas::onDrawImageNine(const SkImage*, const SkIRect&, const SkRect& dst,
                                       const SkPaint*) {
    fList[0]->onDrawRect(dst, fPaint);
}

void SkOverdrawCanvas::onDrawImageLattice(const SkImage* image, const Lattice& lattice,
                                          const SkRect& dst, const SkPaint*) {
    SkIRect bounds;
    Lattice latticePlusBounds = lattice;
    if (!latticePlusBounds.fBounds) {
        bounds = SkIRect::MakeWH(image->width(), image->height());
        latticePlusBounds.fBounds = &bounds;
    }

    if (SkLatticeIter::Valid(image->width(), image->height(), latticePlusBounds)) {
        SkLatticeIter iter(latticePlusBounds, dst);

        SkRect dummy, iterDst;
        while (iter.next(&dummy, &iterDst)) {
            fList[0]->onDrawRect(iterDst, fPaint);
        }
    } else {
        fList[0]->onDrawRect(dst, fPaint);
    }
}

void SkOverdrawCanvas::onDrawImageSet(const ImageSetEntry set[], int count, SkFilterQuality,
                                      SkBlendMode) {
    for (int i = 0; i < count; ++i) {
        fList[0]->onDrawRect(set[i].fDstRect, fPaint);
    }
}

void SkOverdrawCanvas::onDrawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y,
                                    const SkPaint*) {
    fList[0]->onDrawRect(SkRect::MakeXYWH(x, y, bitmap.width(), bitmap.height()), fPaint);
}

void SkOverdrawCanvas::onDrawBitmapRect(const SkBitmap&, const SkRect*, const SkRect& dst,
                                        const SkPaint*, SrcRectConstraint) {
    fList[0]->onDrawRect(dst, fPaint);
}

void SkOverdrawCanvas::onDrawBitmapNine(const SkBitmap&, const SkIRect&, const SkRect& dst,
                                        const SkPaint*) {
    fList[0]->onDrawRect(dst, fPaint);
}

void SkOverdrawCanvas::onDrawBitmapLattice(const SkBitmap& bitmap, const Lattice& lattice,
                                           const SkRect& dst, const SkPaint* paint) {
    sk_sp<SkImage> image = SkMakeImageFromRasterBitmap(bitmap, kNever_SkCopyPixelsMode);
    this->onDrawImageLattice(image.get(), lattice, dst, paint);
}

void SkOverdrawCanvas::onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) {
    drawable->draw(this, matrix);
}

void SkOverdrawCanvas::onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) {
    SkASSERT(false);
    return;
}

void SkOverdrawCanvas::onDrawAnnotation(const SkRect&, const char[], SkData*) {}

void SkOverdrawCanvas::onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& rec) {
    SkRect bounds;
    SkDrawShadowMetrics::GetLocalBounds(path, rec, this->getTotalMatrix(), &bounds);
    fList[0]->onDrawRect(bounds, fPaint);
}

inline SkPaint SkOverdrawCanvas::overdrawPaint(const SkPaint& paint) {
    SkPaint newPaint = fPaint;
    newPaint.setStyle(paint.getStyle());
    newPaint.setStrokeWidth(paint.getStrokeWidth());
    return newPaint;
}
