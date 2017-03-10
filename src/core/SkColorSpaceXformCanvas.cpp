/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXform.h"
#include "SkColorSpaceXformCanvas.h"
#include "SkMakeUnique.h"
#include "SkNoDrawCanvas.h"
#include "SkTLazy.h"

class SkColorSpaceXformCanvas : public SkNoDrawCanvas {
public:
    SkColorSpaceXformCanvas(SkCanvas* target,
                            sk_sp<SkColorSpace> targetCS)
        : SkNoDrawCanvas(SkIRect::MakeSize(target->getBaseLayerSize()))
        , fTarget(target)
        , fTargetCS(std::move(targetCS)) {

        fFromSRGB = SkColorSpaceXform::New(SkColorSpace::MakeSRGB().get(), fTargetCS.get());
    }

    SkColor xform(SkColor srgb) const {
        SkColor xformed;
        SkAssertResult(fFromSRGB->apply(SkColorSpaceXform::kBGRA_8888_ColorFormat, &xformed,
                                        SkColorSpaceXform::kBGRA_8888_ColorFormat, &srgb,
                                        1, kUnpremul_SkAlphaType));
        return xformed;
    }

    const SkPaint& xform(const SkPaint& paint, SkTLazy<SkPaint>* lazy) const {
        const SkPaint* result = &paint;
        auto get_lazy = [&] {
            if (!lazy->isValid()) {
                lazy->init(paint);
                result = lazy->get();
            }
            return lazy->get();
        };

        // All SkColorSpaces have the same black point.
        if (paint.getColor() & 0xffffff) {
            get_lazy()->setColor(xform(paint.getColor()));
        }

        // TODO:
        //    - shaders
        //    - color filters
        //    - image filters?

        return *result;
    }

    const SkImage* xform(const SkImage* img) const {
        // TODO: for real
        return img;
    }

    const SkPaint* xform(const SkPaint* paint, SkTLazy<SkPaint>* lazy) const {
        return paint ? &xform(*paint, lazy) : nullptr;
    }

    void onDrawPaint(const SkPaint& paint) override {
        SkTLazy<SkPaint> lazy;
        fTarget->drawPaint(xform(paint, &lazy));
    }

    void onDrawArc(const SkRect&, SkScalar, SkScalar, bool, const SkPaint&) override {}
    void onDrawAtlas(const SkImage*, const SkRSXform[], const SkRect[], const SkColor[], int, SkBlendMode, const SkRect*, const SkPaint*) override {}
    void onDrawBitmap(const SkBitmap&, SkScalar, SkScalar, const SkPaint*) override {}
    void onDrawBitmapLattice(const SkBitmap&, const Lattice&, const SkRect&, const SkPaint*) override {}
    void onDrawBitmapNine(const SkBitmap&, const SkIRect&, const SkRect&, const SkPaint*) override {}
    void onDrawBitmapRect(const SkBitmap&, const SkRect*, const SkRect&, const SkPaint*, SrcRectConstraint) override {}
    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override {}
    void onDrawDrawable(SkDrawable*, const SkMatrix*) override {}
    void onDrawImage(const SkImage*, SkScalar, SkScalar, const SkPaint*) override {}
    void onDrawImageLattice(const SkImage*, const Lattice&, const SkRect&, const SkPaint*) override {}
    void onDrawImageNine(const SkImage*, const SkIRect&, const SkRect&, const SkPaint*) override {}
    void onDrawImageRect(const SkImage*, const SkRect*, const SkRect&, const SkPaint*, SrcRectConstraint) override {}
    void onDrawOval(const SkRect&, const SkPaint&) override {}
    void onDrawPatch(const SkPoint[12], const SkColor[4], const SkPoint[4], SkBlendMode, const SkPaint&) override {}
    void onDrawPath(const SkPath&, const SkPaint&) override {}
    void onDrawPoints(PointMode, size_t, const SkPoint[], const SkPaint&) override {}
    void onDrawPosText(const void*, size_t, const SkPoint[], const SkPaint&) override {}
    void onDrawPosTextH(const void*, size_t, const SkScalar[], SkScalar, const SkPaint&) override {}
    void onDrawRRect(const SkRRect&, const SkPaint&) override {}
    void onDrawRect(const SkRect&, const SkPaint&) override {}
    void onDrawRegion(const SkRegion&, const SkPaint&) override {}
    void onDrawText(const void*, size_t, SkScalar, SkScalar, const SkPaint&) override {}
    void onDrawTextBlob(const SkTextBlob*, SkScalar, SkScalar, const SkPaint&) override {}
    void onDrawTextOnPath(const void*, size_t, const SkPath&, const SkMatrix*, const SkPaint&) override {}
    void onDrawTextRSXform(const void*, size_t, const SkRSXform[], const SkRect*, const SkPaint&) override {}
    void onDrawVertices(VertexMode, int, const SkPoint[], const SkPoint[], const SkColor[], SkBlendMode, const uint16_t[], int, const SkPaint&) override {}
    void onDrawVerticesObject(sk_sp<SkVertices> vertices, SkBlendMode mode, const SkPaint& paint, uint32_t flags) override {}

private:
    SkCanvas*                          fTarget;
    sk_sp<SkColorSpace>                fTargetCS;
    std::unique_ptr<SkColorSpaceXform> fFromSRGB;
};

std::unique_ptr<SkCanvas> SkCreateColorSpaceXformCanvas(SkCanvas* target,
                                                        sk_sp<SkColorSpace> targetCS) {
    return skstd::make_unique<SkColorSpaceXformCanvas>(target, std::move(targetCS));
}
