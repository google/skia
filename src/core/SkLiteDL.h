/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLiteDL_DEFINED
#define SkLiteDL_DEFINED

#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkDrawable.h"
#include "SkRect.h"
#include "SkTDArray.h"

class SkLiteDL final : public SkDrawable {
public:
    static sk_sp<SkLiteDL> New(SkRect);
    void reset(SkRect);

    void makeThreadsafe();
    bool empty() const { return fUsed == 0; }

#ifdef SK_SUPPORT_LEGACY_DRAWFILTER
    void setDrawFilter(SkDrawFilter*);
#endif

    // Draws as if...
    //   SkRect bounds = this->getBounds();
    //   canvas->saveLayer(&bounds, paint);
    //       this->draw(canvas, matrix);
    //   canvas->restore();
    void drawAsLayer(SkCanvas*, const SkMatrix*, const SkPaint*);

    void save();
    void saveLayer(const SkRect*, const SkPaint*, const SkImageFilter*, SkCanvas::SaveLayerFlags);
    void restore();

    void    concat (const SkMatrix&);
    void setMatrix (const SkMatrix&);
    void translate(SkScalar, SkScalar);
    void translateZ(SkScalar);

    void clipPath  (const   SkPath&, SkCanvas::ClipOp, bool aa);
    void clipRect  (const   SkRect&, SkCanvas::ClipOp, bool aa);
    void clipRRect (const  SkRRect&, SkCanvas::ClipOp, bool aa);
    void clipRegion(const SkRegion&, SkCanvas::ClipOp);

    void drawPaint (const SkPaint&);
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
    void drawShadowedPicture(const SkPicture*, const SkMatrix*,
                             const SkPaint*, const SkShadowParams& params);

    void drawText       (const void*, size_t, SkScalar, SkScalar, const SkPaint&);
    void drawPosText    (const void*, size_t, const SkPoint[], const SkPaint&);
    void drawPosTextH   (const void*, size_t, const SkScalar[], SkScalar, const SkPaint&);
    void drawTextOnPath (const void*, size_t, const SkPath&, const SkMatrix*, const SkPaint&);
    void drawTextRSXform(const void*, size_t, const SkRSXform[], const SkRect*, const SkPaint&);
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
    void drawVertices(SkCanvas::VertexMode, int, const SkPoint[], const SkPoint[], const SkColor[],
                      SkBlendMode, const uint16_t[], int, const SkPaint&);
    void drawAtlas(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[], int,
                   SkBlendMode, const SkRect*, const SkPaint*);

    void setBounds(const SkRect& bounds);

private:
    SkLiteDL(SkRect);
    ~SkLiteDL();

    SkRect   onGetBounds() override;
    void onDraw(SkCanvas*) override;

    template <typename T, typename... Args>
    void* push(size_t, Args&&...);

    template <typename Fn, typename... Args>
    void map(const Fn[], Args...);

    SkAutoTMalloc<uint8_t> fBytes;
    size_t                 fUsed;
    size_t                 fReserved;
    SkRect                 fBounds;
};

#endif//SkLiteDL_DEFINED
