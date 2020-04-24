/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/sksg/include/SkSGPaint.h"

#include "modules/sksg/include/SkSGRenderEffect.h"

namespace sksg {

// Paint nodes don't generate damage on their own, but via their aggregation ancestor Draw nodes.
PaintNode::PaintNode() : INHERITED(kBubbleDamage_Trait) {}

SkPaint PaintNode::makePaint() const {
    SkASSERT(!this->hasInval());

    SkPaint paint;

    paint.setAntiAlias(fAntiAlias);
    paint.setBlendMode(fBlendMode);
    paint.setStyle(fStyle);
    paint.setStrokeWidth(fStrokeWidth);
    paint.setStrokeMiter(fStrokeMiter);
    paint.setStrokeJoin(fStrokeJoin);
    paint.setStrokeCap(fStrokeCap);

    this->onApplyToPaint(&paint);

    // Compose opacity on top of the subclass value.
    paint.setAlpha(SkScalarRoundToInt(paint.getAlpha() * SkTPin<SkScalar>(fOpacity, 0, 1)));

    return paint;
}

sk_sp<Color> Color::Make(SkColor c) {
    return sk_sp<Color>(new Color(c));
}

Color::Color(SkColor c) : fColor(c) {}

SkRect Color::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    return SkRect::MakeEmpty();
}

void Color::onApplyToPaint(SkPaint* paint) const {
    paint->setColor(fColor);
}

sk_sp<ShaderPaint> ShaderPaint::Make(sk_sp<Shader> sh) {
    return sh ? sk_sp<ShaderPaint>(new ShaderPaint(std::move(sh)))
              : nullptr;
}

ShaderPaint::ShaderPaint(sk_sp<Shader> sh)
    : fShader(std::move(sh)) {
    this->observeInval(fShader);
}

ShaderPaint::~ShaderPaint() {
    this->unobserveInval(fShader);
}

SkRect ShaderPaint::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    return fShader->revalidate(ic, ctm);
}

void ShaderPaint::onApplyToPaint(SkPaint* paint) const {
    paint->setShader(fShader->getShader());
}

} // namespace sksg
