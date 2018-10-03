/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGImage.h"

#include "SkCanvas.h"
#include "SkImage.h"

namespace sksg {

Image::Image(sk_sp<SkImage> image) : fImage(std::move(image)) {}

void Image::onRender(SkCanvas* canvas, const RenderContext* ctx) const {
    if (!fImage) {
        return;
    }

    SkPaint paint;
    paint.setAntiAlias(fAntiAlias);
    paint.setFilterQuality(fQuality);

    if (ctx) {
        ctx->modulatePaint(&paint);
    }

    canvas->drawImage(fImage, 0, 0, &paint);
}

SkRect Image::onRevalidate(InvalidationController*, const SkMatrix& ctm) {
    return fImage ? SkRect::Make(fImage->bounds()) : SkRect::MakeEmpty();
}

} // namespace sksg
