/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGMaskEffect.h"

#include "SkCanvas.h"
#include "SkPictureRecorder.h"
#include "SkPictureShader.h"
#include "SkShaderMaskFilter.h"

namespace sksg {

MaskEffect::MaskEffect(sk_sp<RenderNode> child, sk_sp<RenderNode> mask, Mode mode)
    : INHERITED(std::move(child))
    , fMaskNode(std::move(mask))
    , fMaskMode(mode) {
    this->observeInval(fMaskNode);
}

MaskEffect::~MaskEffect() {
    this->unobserveInval(fMaskNode);
}

void MaskEffect::onRender(SkCanvas* canvas) const {
    if (this->bounds().isEmpty())
        return;

    SkAutoCanvasRestore acr(canvas, false);

    SkPaint p;
    p.setMaskFilter(fMaskFilter);
    canvas->saveLayer(this->bounds(), &p);

    this->INHERITED::onRender(canvas);
}


SkRect MaskEffect::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    // TODO: it would be cool to use kDecal instead of devspace padding, but
    //   1) it isn't currently supported on Ganesh
    //   2) produces aliased edges

    SkSize scale;
    if (!ctm.decomposeScale(&scale, nullptr)) {
        scale = SkSize::Make(SkVector::Length(ctm.getScaleX(), ctm.getSkewY()),
                             SkVector::Length(ctm.getScaleY(), ctm.getSkewX()));
    }

    static constexpr SkScalar kDevPadding = 1.0f;
    const auto minScale = SkTMin(scale.width(), scale.height()),
                // when the scale collapses to 0, we're not going to draw anyway.
                padding = SkScalarNearlyZero(minScale) ? 0 : kDevPadding / minScale;

    auto childBounds = this->INHERITED::onRevalidate(ic, ctm);
    const auto maskBounds = fMaskNode->revalidate(ic, ctm).makeOutset(padding, padding),
                   bounds = (fMaskMode == Mode::kInvert || childBounds.intersect(maskBounds))
                                ? childBounds
                                : SkRect::MakeEmpty();

    const auto localMatrix = SkMatrix::MakeTrans(maskBounds.x(), maskBounds.y());

    // TODO: only rebuild the mask filter when the mask is invalidate.
    SkPictureRecorder recorder;
    fMaskNode->render(recorder.beginRecording(maskBounds));
    fMaskFilter = SkShaderMaskFilter::Make(
        SkPictureShader::Make(recorder.finishRecordingAsPicture(),
                              SkShader::kClamp_TileMode,
                              SkShader::kClamp_TileMode,
                              &localMatrix, nullptr));

    return bounds;
}

} // namespace sksg
