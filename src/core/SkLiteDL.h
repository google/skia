/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLiteDL_DEFINED
#define SkLiteDL_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/private/SkTDArray.h"
#include "include/private/SkTemplates.h"

class SkLiteDL final {
public:
    ~SkLiteDL();

    void draw(SkCanvas* canvas) const;

    void reset();
    bool empty() const { return fUsed == 0; }

    void flush();

    void save();
    void saveLayer(const SkRect*, const SkPaint*, const SkImageFilter*, const SkImage*,
                   const SkMatrix*, SkCanvas::SaveLayerFlags);
    void saveBehind(const SkRect*);
    void restore();

    void    concat (const SkMatrix&);
    void setMatrix (const SkMatrix&);
    void translate(SkScalar, SkScalar);
    void translateZ(SkScalar);

    void clipPath  (const   SkPath&, SkClipOp, bool aa);
    void clipRect  (const   SkRect&, SkClipOp, bool aa);
    void clipRRect (const  SkRRect&, SkClipOp, bool aa);
    void clipRegion(const SkRegion&, SkClipOp);

    void drawPaint (const SkPaint&);
    void drawBehind(const SkPaint&);
    void drawPath  (const SkPath&, const SkPaint&);
    void drawRect  (const SkRect&, const SkPaint&);
    void drawRegion(const SkRegion&, const SkPaint&);
    void drawOval  (const SkRect&, const SkPaint&);
    void drawArc   (const SkRect&, SkScalar, SkScalar, bool, const SkPaint&);
    void drawRRect (const SkRRect&, const SkPaint&);
    void drawDRRect(const SkRRect&, const SkRRect&, const SkPaint&);

    void drawAnnotation     (const SkRect&, const char*, SkData*);
    void drawDrawable       (SkDrawable*, const SkMatrix*);
    void drawPicture        (const SkPicture*, const SkMatrix*, const SkPaint*);

    void drawTextBlob   (const SkTextBlob*, SkScalar,SkScalar, const SkPaint&);

    void drawImage    (sk_sp<const SkImage>, SkScalar,SkScalar,             const SkPaint*);
    void drawImageNine(sk_sp<const SkImage>, const SkIRect&, const SkRect&, const SkPaint*);
    void drawImageRect(sk_sp<const SkImage>, const SkRect*, const SkRect&,  const SkPaint*,
                       SkCanvas::SrcRectConstraint);
    void drawImageLattice(sk_sp<const SkImage>, const SkCanvas::Lattice&,
                          const SkRect&, const SkPaint*);

    void drawPatch(const SkPoint[12], const SkColor[4], const SkPoint[4],
                   SkBlendMode, const SkPaint&);
    void drawPoints(SkCanvas::PointMode, size_t, const SkPoint[], const SkPaint&);
    void drawVertices(const SkVertices*, const SkVertices::Bone bones[], int boneCount, SkBlendMode,
                      const SkPaint&);
    void drawAtlas(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[], int,
                   SkBlendMode, const SkRect*, const SkPaint*);
    void drawShadowRec(const SkPath&, const SkDrawShadowRec&);

    void drawEdgeAAQuad(const SkRect& rect, const SkPoint clip[4], SkCanvas::QuadAAFlags aaFlags,
                        SkColor color, SkBlendMode mode);
    void drawEdgeAAImageSet(const SkCanvas::ImageSetEntry[], int count, const SkPoint dstClips[],
                            const SkMatrix preViewMatrices[], const SkPaint* paint,
                            SkCanvas::SrcRectConstraint constraint);

private:
    template <typename T, typename... Args>
    void* push(size_t, Args&&...);

    template <typename Fn, typename... Args>
    void map(const Fn[], Args...) const;

    SkAutoTMalloc<uint8_t> fBytes;
    size_t                 fUsed = 0;
    size_t                 fReserved = 0;
};

#endif//SkLiteDL_DEFINED
