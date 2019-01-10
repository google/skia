/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGGeometryTransform.h"

#include "SkCanvas.h"
#include "SkSGTransform.h"
#include "SkSGTransformPriv.h"

namespace sksg {

GeometryTransform::GeometryTransform(sk_sp<GeometryNode> child, sk_sp<Transform> transform)
    : fChild(std::move(child))
    , fTransform(std::move(transform)) {
    this->observeInval(fChild);
    this->observeInval(fTransform);
}

GeometryTransform::~GeometryTransform() {
    this->unobserveInval(fChild);
    this->unobserveInval(fTransform);
}

void GeometryTransform::onClip(SkCanvas* canvas, bool antiAlias) const {
    canvas->clipPath(fTransformedPath, SkClipOp::kIntersect, antiAlias);
}

void GeometryTransform::onDraw(SkCanvas* canvas, const SkPaint& paint) const {
    canvas->drawPath(fTransformedPath, paint);
}

SkRect GeometryTransform::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    // We don't care about matrix reval results.
    fTransform->revalidate(ic, ctm);
    const auto m = TransformPriv::As<SkMatrix>(fTransform);

    auto bounds = fChild->revalidate(ic, ctm);
    fTransformedPath = fChild->asPath();
    fTransformedPath.transform(m);
    fTransformedPath.shrinkToFit();

    m.mapRect(&bounds);
    return  bounds;
}

SkPath GeometryTransform::onAsPath() const {
    return fTransformedPath;
}

} // namespace sksg
