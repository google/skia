/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilter.h"
#include "SkColorSpaceXform.h"
#include "SkColorSpaceXformCanvas.h"
#include "SkGradientShader.h"
#include "SkImage_Base.h"
#include "SkMakeUnique.h"
#include "SkNoDrawCanvas.h"
#include "SkSurface.h"
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

    sk_sp<const SkImage> xform(const SkImage* img) const {
        return as_IB(img)->makeColorSpace(fTargetCS);
    }

    void xform(SkColor* xformed, const SkColor* srgb, int n) const {
        SkAssertResult(fFromSRGB->apply(SkColorSpaceXform::kBGRA_8888_ColorFormat, xformed,
                                        SkColorSpaceXform::kBGRA_8888_ColorFormat, srgb,
                                        n, kUnpremul_SkAlphaType));
    }

    SkColor xform(SkColor srgb) const {
        SkColor xformed;
        this->xform(&xformed, &srgb, 1);
        return xformed;
    }

    // TODO: Is this introspection going to be enough, or do we need a new SkShader method?
    sk_sp<SkShader> xform(const SkShader* shader) const {
        SkColor color;
        if (shader->isConstant() && shader->asLuminanceColor(&color)) {
            return SkShader::MakeColorShader(this->xform(color));
        }

        SkShader::TileMode xy[2];
        SkMatrix local;
        if (auto img = shader->isAImage(&local, xy)) {
            return this->xform(img)->makeShader(xy[0], xy[1], &local);
        }

        SkShader::ComposeRec compose;
        if (shader->asACompose(&compose)) {
            auto A = this->xform(compose.fShaderA),
                 B = this->xform(compose.fShaderB);
            if (A && B) {
                return SkShader::MakeComposeShader(std::move(A), std::move(B), compose.fBlendMode);
            }
        }

        SkShader::GradientInfo gradient;
        sk_bzero(&gradient, sizeof(gradient));
        if (auto type = shader->asAGradient(&gradient)) {
            SkSTArray<8, SkColor>  colors(gradient.fColorCount);
            SkSTArray<8, SkScalar>    pos(gradient.fColorCount);

            gradient.fColors       = colors.begin();
            gradient.fColorOffsets =    pos.begin();
            shader->asAGradient(&gradient);

            SkSTArray<8, SkColor> xformed(gradient.fColorCount);
            this->xform(xformed.begin(), gradient.fColors, gradient.fColorCount);

            switch (type) {
                case SkShader::kNone_GradientType:
                case SkShader::kColor_GradientType:
                    SkASSERT(false);  // Should be unreachable.
                    break;

                case SkShader::kLinear_GradientType:
                    return SkGradientShader::MakeLinear(gradient.fPoint,
                                                        xformed.begin(),
                                                        gradient.fColorOffsets,
                                                        gradient.fColorCount,
                                                        gradient.fTileMode,
                                                        gradient.fGradientFlags,
                                                        &shader->getLocalMatrix());
                case SkShader::kRadial_GradientType:
                    return SkGradientShader::MakeRadial(gradient.fPoint[0],
                                                        gradient.fRadius[0],
                                                        xformed.begin(),
                                                        gradient.fColorOffsets,
                                                        gradient.fColorCount,
                                                        gradient.fTileMode,
                                                        gradient.fGradientFlags,
                                                        &shader->getLocalMatrix());
                case SkShader::kSweep_GradientType:
                    return SkGradientShader::MakeSweep(gradient.fPoint[0].fX,
                                                       gradient.fPoint[0].fY,
                                                       xformed.begin(),
                                                       gradient.fColorOffsets,
                                                       gradient.fColorCount,
                                                       gradient.fGradientFlags,
                                                       &shader->getLocalMatrix());
                case SkShader::kConical_GradientType:
                    return SkGradientShader::MakeTwoPointConical(gradient.fPoint[0],
                                                                 gradient.fRadius[0],
                                                                 gradient.fPoint[1],
                                                                 gradient.fRadius[1],
                                                                 xformed.begin(),
                                                                 gradient.fColorOffsets,
                                                                 gradient.fColorCount,
                                                                 gradient.fTileMode,
                                                                 gradient.fGradientFlags,
                                                                 &shader->getLocalMatrix());
            }
        }

        return nullptr;
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

        if (auto shader = paint.getShader()) {
            if (auto replacement = this->xform(shader)) {
                get_lazy()->setShader(std::move(replacement));
            }
        }

        // As far as I know, SkModeColorFilter is the only color filter that holds a color.
        if (auto cf = paint.getColorFilter()) {
            SkColor color;
            SkBlendMode mode;
            if (cf->asColorMode(&color, &mode)) {
                get_lazy()->setColorFilter(SkColorFilter::MakeModeFilter(this->xform(color), mode));
            }
        }

        // TODO:
        //    - image filters?

        return *result;
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
    void onDrawRegion(const SkRegion& region, const SkPaint& paint) override {
        SkTLazy<SkPaint> lazy;
        fTarget->drawRegion(region, this->xform(paint, &lazy));
    }
    void onDrawPatch(const SkPoint cubics[12], const SkColor colors[4], const SkPoint texs[4],
                     SkBlendMode mode, const SkPaint& paint) override {
        SkColor xformed[4];
        if (colors) {
            this->xform(xformed, colors, 4);
            colors = xformed;
        }

        SkTLazy<SkPaint> lazy;
        fTarget->drawPatch(cubics, colors, texs, mode, this->xform(paint, &lazy));
    }
    void onDrawPoints(PointMode mode, size_t count, const SkPoint* pts,
                      const SkPaint& paint) override {
        SkTLazy<SkPaint> lazy;
        fTarget->drawPoints(mode, count, pts, this->xform(paint, &lazy));
    }
    void onDrawVertices(VertexMode vmode, int count,
                        const SkPoint* verts, const SkPoint* texs, const SkColor* colors,
                        SkBlendMode mode,
                        const uint16_t* indices, int indexCount, const SkPaint& paint) override {
        SkSTArray<8, SkColor> xformed;
        if (colors) {
            xformed.reset(count);
            this->xform(xformed.begin(), colors, count);
            colors = xformed.begin();
        }

        SkTLazy<SkPaint> lazy;
        fTarget->drawVertices(vmode, count, verts, texs, colors, mode, indices, indexCount,
                              this->xform(paint, &lazy));
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

    void onDrawImage(const SkImage* img,
                     SkScalar l, SkScalar t,
                     const SkPaint* paint) override {
        SkTLazy<SkPaint> lazy;
        fTarget->drawImage(this->xform(img).get(),
                           l, t,
                           this->xform(paint, &lazy));
    }
    void onDrawImageRect(const SkImage* img,
                         const SkRect* src, const SkRect& dst,
                         const SkPaint* paint, SrcRectConstraint constraint) override {
        SkTLazy<SkPaint> lazy;
        fTarget->drawImageRect(this->xform(img).get(),
                               src ? *src : dst, dst,
                               this->xform(paint, &lazy), constraint);
    }
    void onDrawImageNine(const SkImage* img,
                         const SkIRect& center, const SkRect& dst,
                         const SkPaint* paint) override {
        SkTLazy<SkPaint> lazy;
        fTarget->drawImageNine(this->xform(img).get(),
                               center, dst,
                               this->xform(paint, &lazy));
    }
    void onDrawImageLattice(const SkImage* img,
                            const Lattice& lattice, const SkRect& dst,
                            const SkPaint* paint) override {
        SkTLazy<SkPaint> lazy;
        fTarget->drawImageLattice(this->xform(img).get(),
                                  lattice, dst,
                                  this->xform(paint, &lazy));
    }
    void onDrawAtlas(const SkImage* atlas, const SkRSXform* xforms, const SkRect* tex,
                     const SkColor* colors, int count, SkBlendMode mode,
                     const SkRect* cull, const SkPaint* paint) override {
        SkSTArray<8, SkColor> xformed;
        if (colors) {
            xformed.reset(count);
            this->xform(xformed.begin(), colors, count);
            colors = xformed.begin();
        }

        SkTLazy<SkPaint> lazy;
        fTarget->drawAtlas(this->xform(atlas).get(), xforms, tex, colors, count, mode, cull,
                           this->xform(paint, &lazy));
    }

    void onDrawBitmap(const SkBitmap& bitmap,
                      SkScalar l, SkScalar t,
                      const SkPaint* paint) override {
        if (auto image = SkImage::MakeFromBitmap(bitmap)) {
            this->onDrawImage(image.get(), l, t, paint);
        }
    }
    void onDrawBitmapRect(const SkBitmap& bitmap,
                          const SkRect* src, const SkRect& dst,
                          const SkPaint* paint, SrcRectConstraint constraint) override {
        if (auto image = SkImage::MakeFromBitmap(bitmap)) {
            this->onDrawImageRect(image.get(), src, dst, paint, constraint);
        }
    }
    void onDrawBitmapNine(const SkBitmap& bitmap,
                          const SkIRect& center, const SkRect& dst,
                          const SkPaint* paint) override {
        if (auto image = SkImage::MakeFromBitmap(bitmap)) {
            this->onDrawImageNine(image.get(), center, dst, paint);
        }
    }
    void onDrawBitmapLattice(const SkBitmap& bitmap,
                             const Lattice& lattice, const SkRect& dst,
                             const SkPaint* paint) override {
        if (auto image = SkImage::MakeFromBitmap(bitmap)) {
            this->onDrawImageLattice(image.get(), lattice, dst, paint);
        }
    }

    // TODO: May not be ideal to unfurl pictures.
    void onDrawPicture(const SkPicture* pic,
                       const SkMatrix* matrix,
                       const SkPaint* paint) override {
        SkTLazy<SkPaint> lazy;
        SkCanvas::onDrawPicture(pic, matrix, this->xform(paint, &lazy));
    }
    void onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) override {
        SkTLazy<SkPaint> lazy;
        SkCanvas::onDrawDrawable(drawable, matrix);
    }

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

    sk_sp<SkSurface> onNewSurface(const SkImageInfo& info, const SkSurfaceProps& props) override {
        return fTarget->makeSurface(info, &props);
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
