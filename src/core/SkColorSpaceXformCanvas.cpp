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
            get_lazy()->setColor(this->xform(paint.getColor()));
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
        return paint ? &this->xform(*paint, lazy) : nullptr;
    }

    void onDrawPaint(const SkPaint& paint) override {
        SkTLazy<SkPaint> lazy;
        fTarget->drawPaint(this->xform(paint, &lazy));
    }

    void onDrawRect(const SkRect& rect, const SkPaint& paint) override {
        SkTLazy<SkPaint> lazy;
        fTarget->drawRect(rect, this->xform(paint, &lazy));
    }
    void onDrawOval(const SkRect& oval, const SkPaint& paint) override {
        SkTLazy<SkPaint> lazy;
        fTarget->drawOval(oval, this->xform(paint, &lazy));
    }
    void onDrawRRect(const SkRRect& rrect, const SkPaint& paint) override {
        SkTLazy<SkPaint> lazy;
        fTarget->drawRRect(rrect, this->xform(paint, &lazy));
    }
    void onDrawDRRect(const SkRRect& outer, const SkRRect& inner, const SkPaint& paint) override {
        SkTLazy<SkPaint> lazy;
        fTarget->drawDRRect(outer, inner, this->xform(paint, &lazy));
    }
    void onDrawPath(const SkPath& path, const SkPaint& paint) override {
        SkTLazy<SkPaint> lazy;
        fTarget->drawPath(path, this->xform(paint, &lazy));
    }
    void onDrawArc(const SkRect& oval, SkScalar start, SkScalar sweep, bool useCenter,
                   const SkPaint& paint) override {
        SkTLazy<SkPaint> lazy;
        fTarget->drawArc(oval, start, sweep, useCenter, this->xform(paint, &lazy));
    }

    void onDrawText(const void* ptr, size_t len,
                    SkScalar x, SkScalar y,
                    const SkPaint& paint) override {
        SkTLazy<SkPaint> lazy;
        fTarget->drawText(ptr, len, x, y, this->xform(paint, &lazy));
    }
    void onDrawPosText(const void* ptr, size_t len,
                       const SkPoint* xys,
                       const SkPaint& paint) override {
        SkTLazy<SkPaint> lazy;
        fTarget->drawPosText(ptr, len, xys, this->xform(paint, &lazy));
    }
    void onDrawPosTextH(const void* ptr, size_t len,
                        const SkScalar* xs, SkScalar y,
                        const SkPaint& paint) override {
        SkTLazy<SkPaint> lazy;
        fTarget->drawPosTextH(ptr, len, xs, y, this->xform(paint, &lazy));
    }
    void onDrawTextOnPath(const void* ptr, size_t len,
                          const SkPath& path, const SkMatrix* matrix,
                          const SkPaint& paint) override {
        SkTLazy<SkPaint> lazy;
        fTarget->drawTextOnPath(ptr, len, path, matrix, this->xform(paint, &lazy));
    }
    void onDrawTextRSXform(const void* ptr, size_t len,
                           const SkRSXform* xforms, const SkRect* cull,
                           const SkPaint& paint) override {
        SkTLazy<SkPaint> lazy;
        fTarget->drawTextRSXform(ptr, len, xforms, cull, this->xform(paint, &lazy));
    }
    void onDrawTextBlob(const SkTextBlob* blob,
                        SkScalar x, SkScalar y,
                        const SkPaint& paint) override {
        SkTLazy<SkPaint> lazy;
        fTarget->drawTextBlob(blob, x, y, this->xform(paint, &lazy));
    }

    void onDrawBitmap(const SkBitmap&, SkScalar left, SkScalar top, const SkPaint*) override {}
    void onDrawBitmapNine(const SkBitmap&, const SkIRect& center, const SkRect& dst, const SkPaint*) override {}
    void onDrawBitmapRect(const SkBitmap&, const SkRect* src, const SkRect& dst, const SkPaint*, SrcRectConstraint) override {}

    void onDrawImage(const SkImage*, SkScalar left, SkScalar top, const SkPaint*) override {}
    void onDrawImageRect(const SkImage*, const SkRect* src, const SkRect& dst, const SkPaint*, SrcRectConstraint) override {}

    void onDrawPatch(const SkPoint cubics[12], const SkColor colors[4], const SkPoint texCoords[4], SkBlendMode, const SkPaint& paint) override {}
    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override {}
    void onDrawPoints(PointMode, size_t count, const SkPoint pts[], const SkPaint&) override {}
    void onDrawVertices(VertexMode vmode, int vertexCount, const SkPoint vertices[], const SkPoint texs[], const SkColor colors[], SkBlendMode, const uint16_t indices[], int indexCount, const SkPaint&) override {}

    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec& rec) override {
        SkTLazy<SkPaint> lazy;
        fTarget->saveLayer({
            rec.fBounds,
            this->xform(rec.fPaint, &lazy),
            rec.fBackdrop,  // TODO: this is an image filter
            rec.fSaveLayerFlags,
        });
        return kNoLayer_SaveLayerStrategy;
    }

    // Everything from here on should be uninteresting strictly proxied state-change calls.
    void willSave()    override { fTarget->save(); }
    void willRestore() override { fTarget->restore(); }

    void didConcat   (const SkMatrix& m) override { fTarget->concat   (m); }
    void didSetMatrix(const SkMatrix& m) override { fTarget->setMatrix(m); }

    void onClipRect(const SkRect& clip, SkClipOp op, ClipEdgeStyle style) override {
        fTarget->clipRect(clip, op, style);
    }
    void onClipRRect(const SkRRect& clip, SkClipOp op, ClipEdgeStyle style) override {
        fTarget->clipRRect(clip, op, style);
    }
    void onClipPath(const SkPath& clip, SkClipOp op, ClipEdgeStyle style) override {
        fTarget->clipPath(clip, op, style);
    }
    void onClipRegion(const SkRegion& clip, SkClipOp op) override {
        fTarget->clipRegion(clip, op);
    }

    void onDrawAnnotation(const SkRect& rect, const char* key, SkData* val) override {
        fTarget->drawAnnotation(rect, key, val);
    }

private:
    SkCanvas*                          fTarget;
    sk_sp<SkColorSpace>                fTargetCS;
    std::unique_ptr<SkColorSpaceXform> fFromSRGB;
};

std::unique_ptr<SkCanvas> SkCreateColorSpaceXformCanvas(SkCanvas* target,
                                                        sk_sp<SkColorSpace> targetCS) {
    return skstd::make_unique<SkColorSpaceXformCanvas>(target, std::move(targetCS));
}
