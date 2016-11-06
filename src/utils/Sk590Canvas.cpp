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

void Sk590Canvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    SkColor c = paint.getColor();
    printf("{ GPaint paint({ %g, %g, %g, %g });\n",
           unit(SkColorGetA(c)), unit(SkColorGetR(c)), unit(SkColorGetG(c)), unit(SkColorGetB(c)));
    
    bool isClosed = false;
    int count = 0;
    SkPath::Iter iter(path, false);
    for (;;) {
        SkPoint pts[4];
        switch (iter.next(pts)) {
            case SkPath::kMove_Verb:
                isClosed = false;
                if (++count > 1) {
                    printf(" };\n");
                }
                printf("  GPoint pts%d[] = { { %g, %g },", count - 1, pts[0].fX, pts[0].fY);
                break;
            case SkPath::kLine_Verb:
                printf(" { %g, %g },", pts[1].fX, pts[1].fY);
                break;
            case SkPath::kQuad_Verb:
            case SkPath::kConic_Verb:
                printf(" { %g, %g }, { %g, %g },", pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY);
                break;
            case SkPath::kCubic_Verb:
                printf(" { %g, %g }, { %g %g }, { %g %g },",
                       pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY, pts[3].fX, pts[3].fY);
                break;
            case SkPath::kClose_Verb:
                isClosed = true;
                break;
            case SkPath::kDone_Verb:
                goto DONE;
        }
    }
DONE:
    printf(" };\n");
    printf("  const GContour ctrs[] = {");
    for (int i = 0; i < count; ++i) {
        printf(" { GARRAY_COUNT(pts%d), pts%d, %s },", i, i, isClosed ? "true" : "false");
    }
    printf(" };\n");
    printf("  canvas->drawContours(ctrs, %d, paint);\n", count);
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
