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

    void save();
    void saveLayer(const SkRect*, const SkPaint*, const SkImageFilter*, uint32_t) {/*TODO*/}
    void restore();

    void    concat (const SkMatrix&);
    void setMatrix (const SkMatrix&);
    void translateZ(SkScalar) {/*TODO*/}

    void clipPath  (const   SkPath&, SkRegion::Op, bool aa) {/*TODO*/}
    void clipRRect (const  SkRRect&, SkRegion::Op, bool aa) {/*TODO*/}
    void clipRect  (const   SkRect&, SkRegion::Op, bool aa);
    void clipRegion(const SkRegion&, SkRegion::Op) {/*TODO*/}


    void drawPaint (const SkPaint&) {/*TODO*/}
    void drawPath  (const SkPath&, const SkPaint&);
    void drawRect  (const SkRect&, const SkPaint&);
    void drawOval  (const SkRect&, const SkPaint&) {/*TODO*/}
    void drawRRect (const SkRRect&, const SkPaint&) {/*TODO*/}
    void drawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) {/*TODO*/}

    void drawAnnotation     (const SkRect&, const char*, SkData*) {/*TODO*/}
    void drawDrawable       (SkDrawable*, const SkMatrix*) {/*TODO*/}
    void drawPicture        (const SkPicture*, const SkMatrix*, const SkPaint*) {/*TODO*/}
    void drawShadowedPicture(const SkPicture*, const SkMatrix*, const SkPaint*) {/*TODO*/}

    void drawText       (const void*, size_t, SkScalar, SkScalar, const SkPaint&) {/*TODO*/}
    void drawPosText    (const void*, size_t, const SkPoint[], const SkPaint&) {/*TODO*/}
    void drawPosTextH   (const void*, size_t, const SkScalar[], SkScalar, const SkPaint&) {/*TODO*/}
    void drawTextOnPath (const void*, size_t, const SkPath&, const SkMatrix*, const SkPaint&) {/*TODO*/}
    void drawTextRSXForm(const void*, size_t, const SkRSXform[], const SkRect*, const SkPaint&) {/*TODO*/}
    void drawTextBlob   (const SkTextBlob*, SkScalar,SkScalar, const SkPaint&) {/*TODO*/}

    void drawBitmap    (const SkBitmap&, SkScalar,SkScalar,     const SkPaint*) {/*TODO*/}
    void drawBitmapNine(const SkBitmap&, SkIRect, const SkRect&,       const SkPaint*) {/*TODO*/}
    void drawBitmapRect(const SkBitmap&, const SkRect*, const SkRect&, const SkPaint*, bool) {/*TODO*/}

    void drawImage       (const SkImage*, SkScalar,SkScalar,     const SkPaint*) {/*TODO*/}
    void drawImageNine   (const SkImage*, SkIRect, const SkRect&,       const SkPaint*) {/*TODO*/}
    void drawImageRect   (const SkImage*, const SkRect*, const SkRect&, const SkPaint*, bool) {/*TODO*/}
    void drawImageLattice(const SkImage*, SkCanvas::Lattice, const SkRect&, const SkPaint*) {/*TODO*/}

    void drawPatch(const SkPoint[12], const SkColor[4], const SkPoint[4],
                   SkXfermode*, const SkPaint&) {/*TODO*/}
    void drawPoints(SkCanvas::PointMode, size_t, const SkPoint[], const SkPaint&) {/*TODO*/}
    void drawVertices(SkCanvas::VertexMode, int, const SkPoint[], const SkPoint[], const SkColor[],
                      SkXfermode*, const uint16_t[], int, const SkPaint&) {/*TODO*/}
    void drawAtlas(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[], int,
                   SkXfermode::Mode, const SkRect*, const SkPaint*) {/*TODO*/}

private:
    SkLiteDL();
    ~SkLiteDL();

    void internal_dispose() const override;

    SkRect   onGetBounds() override;
    void onDraw(SkCanvas*) override;

    SkLiteDL*          fNext;
    SkRect             fBounds;
    SkTDArray<uint8_t> fBytes;
};

#endif//SkLiteDL_DEFINED
