/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFCanvas_DEFINED
#define SkPDFCanvas_DEFINED

#include "SkCanvas.h"

class SkPDFDevice;

class SkPDFCanvas : public SkCanvas {
public:
    SkPDFCanvas(const sk_sp<SkPDFDevice>&);
    ~SkPDFCanvas() override;

protected:
    void onClipRect(const SkRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipRRect(const SkRRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipPath(const SkPath&, SkClipOp, ClipEdgeStyle) override;

    void onDrawBitmapNine(const SkBitmap&, const SkIRect&, const SkRect&,
                          const SkPaint*) override;

    void onDrawImageNine(const SkImage*, const SkIRect&, const SkRect&,
                         const SkPaint*) override;

    void onDrawImageRect(const SkImage*,
                         const SkRect*,
                         const SkRect&,
                         const SkPaint*,
                         SkCanvas::SrcRectConstraint) override;

    void onDrawBitmapRect(const SkBitmap&,
                          const SkRect*,
                          const SkRect&,
                          const SkPaint*,
                          SkCanvas::SrcRectConstraint) override;

    void onDrawImageLattice(const SkImage*,
                            const Lattice&,
                            const SkRect&,
                            const SkPaint*) override;

    void onDrawBitmapLattice(const SkBitmap&,
                             const Lattice&,
                             const SkRect&,
                             const SkPaint*) override;

private:
    typedef SkCanvas INHERITED;
};

#endif  // SkPDFCanvas_DEFINED
