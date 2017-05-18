/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPaintFilterCanvas_DEFINED
#define SkPaintFilterCanvas_DEFINED

#include "SkNWayCanvas.h"
#include "SkTLazy.h"

/** \class SkPaintFilterCanvas

    A utility proxy base class for implementing draw/paint filters.
*/
class SK_API SkPaintFilterCanvas : public SkNWayCanvas {
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
        kText_Type,
        kTextBlob_Type,
        kVertices_Type,
        kPatch_Type,

        kTypeCount
    };

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
    void onDrawPoints(PointMode, size_t count, const SkPoint pts[], const SkPaint&) override;
    void onDrawRect(const SkRect&, const SkPaint&) override;
    void onDrawRRect(const SkRRect&, const SkPaint&) override;
    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override;
    void onDrawOval(const SkRect&, const SkPaint&) override;
    void onDrawArc(const SkRect&, SkScalar, SkScalar, bool, const SkPaint&) override;
    void onDrawPath(const SkPath&, const SkPaint&) override;
    void onDrawBitmap(const SkBitmap&, SkScalar left, SkScalar top, const SkPaint*) override;
    void onDrawBitmapRect(const SkBitmap&, const SkRect* src, const SkRect& dst, const SkPaint*,
                          SrcRectConstraint) override;
    void onDrawBitmapNine(const SkBitmap&, const SkIRect& center, const SkRect& dst,
                          const SkPaint*) override;
    void onDrawImage(const SkImage*, SkScalar left, SkScalar top, const SkPaint*) override;
    void onDrawImageRect(const SkImage*, const SkRect* src, const SkRect& dst,
                         const SkPaint*, SrcRectConstraint) override;
    void onDrawImageNine(const SkImage*, const SkIRect& center, const SkRect& dst,
                         const SkPaint*) override;
    void onDrawVerticesObject(const SkVertices*, SkBlendMode, const SkPaint&) override;
    void onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                             const SkPoint texCoords[4], SkBlendMode,
                             const SkPaint& paint) override;
    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override;

    void onDrawText(const void* text, size_t byteLength, SkScalar x, SkScalar y,
                    const SkPaint&) override;
    void onDrawPosText(const void* text, size_t byteLength, const SkPoint pos[],
                       const SkPaint&) override;
    void onDrawPosTextH(const void* text, size_t byteLength, const SkScalar xpos[],
                        SkScalar constY, const SkPaint&) override;
    void onDrawTextOnPath(const void* text, size_t byteLength, const SkPath& path,
                          const SkMatrix* matrix, const SkPaint&) override;
    void onDrawTextRSXform(const void* text, size_t byteLength, const SkRSXform xform[],
                           const SkRect* cull, const SkPaint& paint) override;
    void onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                        const SkPaint& paint) override;

private:
    class AutoPaintFilter;

    typedef SkNWayCanvas INHERITED;
};

#endif
