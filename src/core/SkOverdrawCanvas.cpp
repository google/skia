/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilter.h"
#include "SkFindAndPlaceGlyph.h"
#include "SkLatticeIter.h"
#include "SkOverdrawCanvas.h"
#include "SkPatchUtils.h"
#include "SkPath.h"
#include "SkRRect.h"
#include "SkRSXform.h"
#include "SkTextBlob.h"
#include "SkTextBlobRunIterator.h"

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
    , fCanvas(canvas)
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

void SkOverdrawCanvas::onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                                  const SkPaint& paint) {
    ProcessOneGlyphBounds processBounds(this);
    SkSurfaceProps props(0, kUnknown_SkPixelGeometry);
    this->getProps(&props);
    SkAutoGlyphCache cache(paint, &props, 0, &this->getTotalMatrix());
    SkFindAndPlaceGlyph::ProcessText(paint.getTextEncoding(), (const char*) text, byteLength,
                                     SkPoint::Make(x, y), SkMatrix(), paint.getTextAlign(),
                                     cache.get(), processBounds);
}

void SkOverdrawCanvas::drawPosTextCommon(const void* text, size_t byteLength, const SkScalar pos[],
                                         int scalarsPerPos, const SkPoint& offset,
                                         const SkPaint& paint) {
    ProcessOneGlyphBounds processBounds(this);
    SkSurfaceProps props(0, kUnknown_SkPixelGeometry);
    this->getProps(&props);
    SkAutoGlyphCache cache(paint, &props, 0, &this->getTotalMatrix());
    SkFindAndPlaceGlyph::ProcessPosText(paint.getTextEncoding(), (const char*) text, byteLength,
                                        SkPoint::Make(0, 0), SkMatrix(), (const SkScalar*) pos, 2,
                                        paint.getTextAlign(), cache.get(), processBounds);
}

void SkOverdrawCanvas::onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                                     const SkPaint& paint) {
    this->drawPosTextCommon(text, byteLength, (SkScalar*) pos, 2, SkPoint::Make(0, 0), paint);
}

void SkOverdrawCanvas::onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xs[],
                                      SkScalar y, const SkPaint& paint) {
    this->drawPosTextCommon(text, byteLength, (SkScalar*) xs, 1, SkPoint::Make(0, y), paint);
}

void SkOverdrawCanvas::onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                                        const SkMatrix* matrix, const SkPaint& paint) {
    SkASSERT(false);
    return;
}

typedef int (*CountTextProc)(const char* text);
static int count_utf16(const char* text) {
    const uint16_t* prev = (uint16_t*)text;
    (void)SkUTF16_NextUnichar(&prev);
    return SkToInt((const char*)prev - text);
}
static int return_4(const char* text) { return 4; }
static int return_2(const char* text) { return 2; }

void SkOverdrawCanvas::onDrawTextRSXform(const void* text, size_t byteLength,
                                         const SkRSXform xform[], const SkRect*,
                                         const SkPaint& paint) {
    CountTextProc proc = nullptr;
    switch (paint.getTextEncoding()) {
        case SkPaint::kUTF8_TextEncoding:
            proc = SkUTF8_CountUTF8Bytes;
            break;
        case SkPaint::kUTF16_TextEncoding:
            proc = count_utf16;
            break;
        case SkPaint::kUTF32_TextEncoding:
            proc = return_4;
            break;
        case SkPaint::kGlyphID_TextEncoding:
            proc = return_2;
            break;
    }
    SkASSERT(proc);

    SkMatrix matrix;
    const void* stopText = (const char*)text + byteLength;
    while ((const char*)text < (const char*)stopText) {
        matrix.setRSXform(*xform++);
        matrix.setConcat(this->getTotalMatrix(), matrix);
        int subLen = proc((const char*)text);

        this->save();
        this->concat(matrix);
        this->drawText(text, subLen, 0, 0, paint);
        this->restore();

        text = (const char*)text + subLen;
    }
}

void SkOverdrawCanvas::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                      const SkPaint& paint) {
    SkPaint runPaint = paint;
    SkTextBlobRunIterator it(blob);
    for (;!it.done(); it.next()) {
        size_t textLen = it.glyphCount() * sizeof(uint16_t);
        const SkPoint& offset = it.offset();
        it.applyFontToPaint(&runPaint);
        switch (it.positioning()) {
            case SkTextBlob::kDefault_Positioning:
                this->onDrawText(it.glyphs(), textLen, x + offset.x(), y + offset.y(), runPaint);
                break;
            case SkTextBlob::kHorizontal_Positioning:
                this->drawPosTextCommon(it.glyphs(), textLen, it.pos(), 1,
                                        SkPoint::Make(x, y + offset.y()), runPaint);
                break;
            case SkTextBlob::kFull_Positioning:
                this->drawPosTextCommon(it.glyphs(), textLen, it.pos(), 2, SkPoint::Make(x, y),
                                        runPaint);
                break;
            default:
                SkASSERT(false);
                break;
        }
    }
}

void SkOverdrawCanvas::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                                   const SkPoint texCoords[4], SkBlendMode blendMode,
                                   const SkPaint&) {
    fCanvas->onDrawPatch(cubics, colors, texCoords, blendMode, fPaint);
}

void SkOverdrawCanvas::onDrawPaint(const SkPaint& paint) {
    if (0 == paint.getColor() && !paint.getColorFilter() && !paint.getShader()) {
        // This is a clear, ignore it.
    } else {
        fCanvas->onDrawPaint(this->overdrawPaint(paint));
    }
}

void SkOverdrawCanvas::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    fCanvas->onDrawRect(rect, this->overdrawPaint(paint));
}

void SkOverdrawCanvas::onDrawRegion(const SkRegion& region, const SkPaint& paint) {
    fCanvas->onDrawRegion(region, this->overdrawPaint(paint));
}

void SkOverdrawCanvas::onDrawOval(const SkRect& oval, const SkPaint& paint) {
    fCanvas->onDrawOval(oval, this->overdrawPaint(paint));
}

void SkOverdrawCanvas::onDrawArc(const SkRect& arc, SkScalar startAngle, SkScalar sweepAngle,
                                 bool useCenter, const SkPaint& paint) {
    fCanvas->onDrawArc(arc, startAngle, sweepAngle, useCenter, this->overdrawPaint(paint));
}

void SkOverdrawCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                                    const SkPaint& paint) {
    fCanvas->onDrawDRRect(outer, inner, this->overdrawPaint(paint));
}

void SkOverdrawCanvas::onDrawRRect(const SkRRect& rect, const SkPaint& paint) {
    fCanvas->onDrawRRect(rect, this->overdrawPaint(paint));
}

void SkOverdrawCanvas::onDrawPoints(PointMode mode, size_t count, const SkPoint points[],
                                    const SkPaint& paint) {
    fCanvas->onDrawPoints(mode, count, points, this->overdrawPaint(paint));
}

void SkOverdrawCanvas::onDrawVertices(VertexMode vertexMode, int vertexCount,
                                      const SkPoint vertices[], const SkPoint texs[],
                                      const SkColor colors[], SkBlendMode blendMode,
                                      const uint16_t indices[], int indexCount,
                                      const SkPaint& paint) {
    fCanvas->onDrawVertices(vertexMode, vertexCount, vertices, texs, colors, blendMode, indices,
                            indexCount, this->overdrawPaint(paint));
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

    fCanvas->onDrawAtlas(image, xform, texs, colors, count, mode, cull, paintPtr);
}

void SkOverdrawCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    fCanvas->onDrawPath(path, fPaint);
}

void SkOverdrawCanvas::onDrawImage(const SkImage* image, SkScalar x, SkScalar y, const SkPaint*) {
    fCanvas->onDrawRect(SkRect::MakeXYWH(x, y, image->width(), image->height()), fPaint);
}

void SkOverdrawCanvas::onDrawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                                       const SkPaint*, SrcRectConstraint) {
    fCanvas->onDrawRect(dst, fPaint);
}

void SkOverdrawCanvas::onDrawImageNine(const SkImage*, const SkIRect&, const SkRect& dst,
                                       const SkPaint*) {
    fCanvas->onDrawRect(dst, fPaint);
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
            fCanvas->onDrawRect(iterDst, fPaint);
        }
    } else {
        fCanvas->onDrawRect(dst, fPaint);
    }
}

void SkOverdrawCanvas::onDrawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y,
                                    const SkPaint*) {
    fCanvas->onDrawRect(SkRect::MakeXYWH(x, y, bitmap.width(), bitmap.height()), fPaint);
}

void SkOverdrawCanvas::onDrawBitmapRect(const SkBitmap&, const SkRect*, const SkRect& dst,
                                        const SkPaint*, SrcRectConstraint) {
    fCanvas->onDrawRect(dst, fPaint);
}

void SkOverdrawCanvas::onDrawBitmapNine(const SkBitmap&, const SkIRect&, const SkRect& dst,
                                        const SkPaint*) {
    fCanvas->onDrawRect(dst, fPaint);
}

void SkOverdrawCanvas::onDrawBitmapLattice(const SkBitmap& bitmap, const Lattice& lattice,
                                           const SkRect& dst, const SkPaint* paint) {
    sk_sp<SkImage> image = SkMakeImageFromRasterBitmap(bitmap, kNever_SkCopyPixelsMode);
    this->onDrawImageLattice(image.get(), lattice, dst, paint);
}

void SkOverdrawCanvas::onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) {
    SkASSERT(false);
    return;
}

inline SkPaint SkOverdrawCanvas::overdrawPaint(const SkPaint& paint) {
    SkPaint newPaint = fPaint;
    newPaint.setStyle(paint.getStyle());
    newPaint.setStrokeWidth(paint.getStrokeWidth());
    return newPaint;
}
