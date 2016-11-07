/*
 * Copyright 2016 Mike Reed
 */

#include "SkData.h"
#include "SkPatchUtils.h"
#include "SkPicture.h"
#include "SkPixelRef.h"
#include "SkRRect.h"
#include "SkString.h"
#include "SkTextBlob.h"
#include <stdarg.h>
#include <stdio.h>

// needed just to know that these are all subclassed from SkFlattenable
#include "SkShader.h"
#include "SkPathEffect.h"
#include "SkColorFilter.h"
#include "SkPathEffect.h"
#include "SkMaskFilter.h"

#include "Sk590Canvas.h"

#define WIDE_OPEN   16384

Sk590Canvas::Sk590Canvas() : INHERITED(WIDE_OPEN, WIDE_OPEN) {
}

void Sk590Canvas::willSave() {
    printf("canvas->save();\n");
    this->INHERITED::willSave();
}

SkCanvas::SaveLayerStrategy Sk590Canvas::getSaveLayerStrategy(const SaveLayerRec& rec) {
    return this->INHERITED::getSaveLayerStrategy(rec);
}

void Sk590Canvas::willRestore() {
    printf("canvas->restore();\n");
    this->INHERITED::willRestore();
}

void Sk590Canvas::didConcat(const SkMatrix& matrix) {
    printf("canvas->concat(GMatrix(%g, %g, %g, %g, %g, %g));\n",
           matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5]);
    this->INHERITED::didConcat(matrix);
}

void Sk590Canvas::didSetMatrix(const SkMatrix& matrix) {
    printf("canvas->concat(GMatrix(%g, %g, %g, %g, %g, %g));\n",
           matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5]);
    this->INHERITED::didSetMatrix(matrix);
}

///////////////////////////////////////////////////////////////////////////////

void Sk590Canvas::onClipRect(const SkRect& rect, ClipOp op, ClipEdgeStyle edgeStyle) {
    this->INHERITED::onClipRect(rect, op, edgeStyle);
}

void Sk590Canvas::onClipRRect(const SkRRect& rrect, ClipOp op, ClipEdgeStyle edgeStyle) {
    this->INHERITED::onClipRRect(rrect, op, edgeStyle);
}

void Sk590Canvas::onClipPath(const SkPath& path, ClipOp op, ClipEdgeStyle edgeStyle) {
    this->INHERITED::onClipPath(path, op, edgeStyle);
}

void Sk590Canvas::onClipRegion(const SkRegion& deviceRgn, ClipOp op) {
    this->INHERITED::onClipRegion(deviceRgn, op);
}

///////////////////////////////////////////////////////////////////////////////

void Sk590Canvas::onDrawPaint(const SkPaint& paint) {
}

void Sk590Canvas::onDrawPoints(PointMode mode, size_t count,
                               const SkPoint pts[], const SkPaint& paint) {
}

void Sk590Canvas::onDrawOval(const SkRect& rect, const SkPaint& paint) {
}

void Sk590Canvas::onDrawArc(const SkRect& rect, SkScalar startAngle, SkScalar sweepAngle,
                             bool useCenter, const SkPaint& paint) {
}

void Sk590Canvas::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    SkPath quad;
    quad.addRect(rect);
    this->onDrawPath(quad, paint);
}

void Sk590Canvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
}

void Sk590Canvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                                const SkPaint& paint) {
}

static float unit(unsigned c) {
    return c / 255.0f;
}

static void dump(int ctr, const SkTDArray<SkPoint>& pts) {
    if (pts.count() > 1) {
        printf("  const GPoint pts%d[] = { ", ctr);
        for (int i = 0; i < pts.count(); ++i) {
            printf("{ %g, %g }, ", pts[i].fX, pts[i].fY);
        }
        printf("};\n");
    }
}
#include "SkGeometry.h"
static void append_cubic(SkTDArray<SkPoint>& poly, const SkPoint pts[4]) {
    const int N = 8;
    for (int i = 1; i < N; ++i) {
        SkEvalCubicAt(pts, i * 1.0f / N, poly.append(), nullptr, nullptr);
    }
    *poly.append() = pts[3];
}

void Sk590Canvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    SkColor c = paint.getColor();
    printf("{ GPaint paint({ %g, %g, %g, %g });\n",
           unit(SkColorGetA(c)), unit(SkColorGetR(c)), unit(SkColorGetG(c)), unit(SkColorGetB(c)));
    if (paint.getStyle() == SkPaint::kStroke_Style) {
        printf("  paint.setStrokeWidth(%g);\n", paint.getStrokeWidth());
    }
    
    SkTDArray<SkPoint> poly;
    SkTDArray<bool> closed;
    int ctr = -1;

    SkPath::Iter iter(path, false);
    for (;;) {
        SkPoint pts[4];
        switch (iter.next(pts)) {
            case SkPath::kMove_Verb:
                if (poly.count() > 1) {
                    SkASSERT(ctr >= 0);
                    dump(ctr, poly);
                    *closed.append() = false;
                }
                poly.reset();
                ctr += 1;
                *poly.append() = pts[0];
                break;
            case SkPath::kLine_Verb:
                *poly.append() = pts[1];
                break;
            case SkPath::kQuad_Verb:
            case SkPath::kConic_Verb:
                *poly.append() = pts[1];
                *poly.append() = pts[2];
                break;
            case SkPath::kCubic_Verb:
                append_cubic(poly, pts);
                break;
            case SkPath::kClose_Verb:
                if (poly.count() > 1) {
                    dump(ctr, poly);
                    *closed.append() = true;
                }
                poly.reset();
                break;
            case SkPath::kDone_Verb:
                if (poly.count() > 1) {
                    dump(ctr, poly);
                    *closed.append() = false;
                }
                goto DONE;
        }
    }
DONE:
    printf("  const GContour ctrs[] = {");
    for (int i = 0; i <= ctr; ++i) {
        printf(" { GARRAY_COUNT(pts%d), pts%d, %s },", i, i, closed[i] ? "true" : "false");
    }
    printf(" };\n");
    printf("  canvas->drawContours(ctrs, %d, paint);\n", ctr + 1);
    printf("}\n");
}

void Sk590Canvas::onDrawBitmap(const SkBitmap& bitmap, SkScalar x, SkScalar y,
                                const SkPaint* paint) {
}

void Sk590Canvas::onDrawBitmapRect(const SkBitmap& bitmap, const SkRect* src, const SkRect& dst,
                                    const SkPaint* paint, SrcRectConstraint) {
}

void Sk590Canvas::onDrawBitmapNine(const SkBitmap& bitmap, const SkIRect& center,
                                    const SkRect& dst, const SkPaint* paint) {
}

void Sk590Canvas::onDrawImage(const SkImage* image, SkScalar x, SkScalar y, const SkPaint* paint) {
}

void Sk590Canvas::onDrawImageRect(const SkImage* image, const SkRect* src, const SkRect& dst,
                                   const SkPaint* paint, SrcRectConstraint) {
}

void Sk590Canvas::onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                              const SkPaint& paint) {
}

void Sk590Canvas::onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                                 const SkPaint& paint) {
}

void Sk590Canvas::onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                                  SkScalar constY, const SkPaint& paint) {
}

void Sk590Canvas::onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                                    const SkMatrix* matrix, const SkPaint& paint) {
}

void Sk590Canvas::onDrawTextRSXform(const void* text, size_t byteLength, const SkRSXform xform[],
                                     const SkRect* cull, const SkPaint& paint) {
}

void Sk590Canvas::onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                  const SkPaint& paint) {
}

void Sk590Canvas::onDrawPicture(const SkPicture* picture, const SkMatrix* matrix,
                                 const SkPaint* paint) {
    picture->playback(this);
}

void Sk590Canvas::onDrawVertices(VertexMode vmode, int vertexCount,
                                  const SkPoint vertices[], const SkPoint texs[],
                                  const SkColor colors[], SkBlendMode,
                                  const uint16_t indices[], int indexCount,
                                  const SkPaint& paint) {
}

void Sk590Canvas::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                               const SkPoint texCoords[4], SkBlendMode,
                               const SkPaint& paint) {
}

void Sk590Canvas::onDrawAnnotation(const SkRect& rect, const char key[], SkData* value) {}
