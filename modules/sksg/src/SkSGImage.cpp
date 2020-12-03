/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/sksg/include/SkSGImage.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"

namespace sksg {

Image::Image(sk_sp<SkImage> image) : fImage(std::move(image)) {}

void Image::onRender(SkCanvas* canvas, const RenderContext* ctx) const {
    if (!fImage) {
        return;
    }

    // Ignoring cubic params and trilerp for now.
    // TODO: convert to drawImage(sampling options) when available.
    auto legacy_quality = [](const SkSamplingOptions& sampling) {
        return
            sampling.useCubic                         ? SkFilterQuality::kHigh_SkFilterQuality :
            sampling.filter == SkFilterMode::kNearest ? SkFilterQuality::kNone_SkFilterQuality :
            sampling.mipmap == SkMipmapMode::kNone    ? SkFilterQuality::kLow_SkFilterQuality  :
                                                        SkFilterQuality::kMedium_SkFilterQuality;
    };

    SkPaint paint;
    paint.setAntiAlias(fAntiAlias);
    paint.setFilterQuality(legacy_quality(fSamplingOptions));

    sksg::RenderNode::ScopedRenderContext local_ctx(canvas, ctx);
    if (ctx) {
        if (ctx->fMaskShader) {
            // Mask shaders cannot be applied via drawImage - we need layer isolation.
            // TODO: remove after clipShader conversion.
            local_ctx.setIsolation(this->bounds(), canvas->getTotalMatrix(), true);
        }
        local_ctx->modulatePaint(canvas->getTotalMatrix(), &paint);
    }

    canvas->drawImage(fImage, 0, 0, &paint);
}

const RenderNode* Image::onNodeAt(const SkPoint& p) const {
    SkASSERT(this->bounds().contains(p.x(), p.y()));
    return this;
}

SkRect Image::onRevalidate(InvalidationController*, const SkMatrix& ctm) {
    return fImage ? SkRect::Make(fImage->bounds()) : SkRect::MakeEmpty();
}

} // namespace sksg
