/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGGeometryTransform.h"

#include "SkCanvas.h"

namespace sksg {

GeometryTransform::GeometryTransform(sk_sp<GeometryNode> child, sk_sp<Matrix> matrix)
    : fChild(std::move(child))
    , fMatrix(std::move(matrix)) {
    this->observeInval(fChild);
    this->observeInval(fMatrix);
}

GeometryTransform::~GeometryTransform() {
    this->unobserveInval(fChild);
    this->unobserveInval(fMatrix);
}

void GeometryTransform::onClip(SkCanvas* canvas, bool antiAlias) const {
    canvas->clipPath(fTransformed, SkClipOp::kIntersect, antiAlias);
}

void GeometryTransform::onDraw(SkCanvas* canvas, const SkPaint& paint) const {
    canvas->drawPath(fTransformed, paint);
}

SkRect GeometryTransform::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    // We don't care about matrix reval results.
    fMatrix->revalidate(ic, ctm);
    const auto& m = fMatrix->getMatrix();

    auto bounds = fChild->revalidate(ic, ctm);
    fTransformed = fChild->asPath();
    fTransformed.transform(m);

    m.mapRect(&bounds);
    return  bounds;
}

SkPath GeometryTransform::onAsPath() const {
    return fTransformed;
}

} // namespace sksg
