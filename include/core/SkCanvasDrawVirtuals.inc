X(onDrawText, const void* text, size_t byteLength, SkScalar x, SkScalar y, const SkPaint& paint)
X(onDrawPosText, const void* text, size_t byteLength, const SkPoint pos[], const SkPaint& paint)
X(onDrawPosTextH, const void* text, size_t byteLength, const SkScalar xpos[], SkScalar constY,
                  const SkPaint& paint)
X(onDrawTextOnPath, const void* text, size_t byteLength, const SkPath& path, const SkMatrix* matrix,
                    const SkPaint& paint)
X(onDrawTextRSXform, const void* text, size_t byteLength, const SkRSXform xform[],
                     const SkRect* cullRect, const SkPaint& paint)
X(onDrawTextBlob, const SkTextBlob* blob, SkScalar x, SkScalar y, const SkPaint& paint)

X(onDrawPaint, const SkPaint& paint)
X(onDrawRect, const SkRect& rect, const SkPaint& paint)
X(onDrawRRect, const SkRRect& rrect, const SkPaint& paint)
X(onDrawDRRect, const SkRRect& outer, const SkRRect& inner, const SkPaint& paint)
X(onDrawOval, const SkRect& rect, const SkPaint& paint)
X(onDrawArc, const SkRect& rect, SkScalar startAngle, SkScalar sweepAngle, bool useCenter,
             const SkPaint& paint)
X(onDrawPath, const SkPath& path, const SkPaint& paint)
X(onDrawRegion, const SkRegion& region, const SkPaint& paint)

X(onDrawPatch, const SkPoint cubics[12], const SkColor colors[4], const SkPoint texCoords[4],
               SkBlendMode mode, const SkPaint& paint)
X(onDrawPoints, PointMode mode, size_t count, const SkPoint pts[], const SkPaint& paint)
X(onDrawVerticesObject, const SkVertices* vertices, SkBlendMode mode, const SkPaint& paint)

X(onDrawImage, const SkImage* image, SkScalar dx, SkScalar dy, const SkPaint* paint)
X(onDrawImageRect, const SkImage* image, const SkRect* src, const SkRect& dst, const SkPaint* paint,
                   SrcRectConstraint constraint)
X(onDrawImageNine, const SkImage* image, const SkIRect& center, const SkRect& dst,
                   const SkPaint* paint)
X(onDrawImageLattice, const SkImage* image, const Lattice& lattice, const SkRect& dst,
                      const SkPaint* paint)
X(onDrawBitmap, const SkBitmap& bitmap, SkScalar dx, SkScalar dy, const SkPaint* paint)
X(onDrawBitmapRect, const SkBitmap& bitmap, const SkRect* src, const SkRect& dst,
                    const SkPaint* paint, SrcRectConstraint constraint)
X(onDrawBitmapNine, const SkBitmap& bitmap, const SkIRect& center, const SkRect& dst,
                    const SkPaint* paint)
X(onDrawBitmapLattice, const SkBitmap& bitmap, const Lattice& lattice, const SkRect& dst,
                       const SkPaint* paint)
X(onDrawAtlas, const SkImage* atlas, const SkRSXform xform[], const SkRect rect[],
               const SkColor colors[], int count, SkBlendMode mode, const SkRect* cull,
               const SkPaint* paint)

X(onDrawAnnotation, const SkRect& rect, const char key[], SkData* value)
X(onDrawDrawable, SkDrawable* drawable, const SkMatrix* matrix)
X(onDrawShadowRec, const SkPath&, const SkDrawShadowRec&)

#undef X
