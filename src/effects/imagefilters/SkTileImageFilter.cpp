/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkOffsetImageFilter.h"
#include "include/effects/SkTileImageFilter.h"
#include "src/core/SkImageFilterPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkValidationUtils.h"
#include "src/core/SkWriteBuffer.h"

sk_sp<SkImageFilter> SkTileImageFilter::Make(const SkRect& srcRect, const SkRect& dstRect,
                                             sk_sp<SkImageFilter> input) {
    if (!SkIsValidRect(srcRect) || !SkIsValidRect(dstRect)) {
        return nullptr;
    }
    if (srcRect.width() == dstRect.width() && srcRect.height() == dstRect.height()) {
        SkRect ir = dstRect;
        if (!ir.intersect(srcRect)) {
            return input;
        }
        CropRect cropRect(ir);
        return SkOffsetImageFilter::Make(dstRect.x() - srcRect.x(),
                                         dstRect.y() - srcRect.y(),
                                         std::move(input),
                                         &cropRect);
    }
    return sk_sp<SkImageFilter>(new SkTileImageFilter(srcRect, dstRect, std::move(input)));
}

sk_sp<SkSpecialImage> SkTileImageFilter::onFilterImage(SkSpecialImage* source,
                                                       const Context& ctx,
                                                       SkIPoint* offset) const {
    SkIPoint inputOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> input(this->filterInput(0, source, ctx, &inputOffset));
    if (!input) {
        return nullptr;
    }

    SkRect dstRect;
    ctx.ctm().mapRect(&dstRect, fDstRect);
    if (!dstRect.intersect(SkRect::Make(ctx.clipBounds()))) {
        return nullptr;
    }

    const SkIRect dstIRect = dstRect.roundOut();
    if (!fSrcRect.width() || !fSrcRect.height() || !dstIRect.width() || !dstIRect.height()) {
        return nullptr;
    }

    SkRect srcRect;
    ctx.ctm().mapRect(&srcRect, fSrcRect);
    SkIRect srcIRect;
    srcRect.roundOut(&srcIRect);
    srcIRect.offset(-inputOffset);
    const SkIRect inputBounds = SkIRect::MakeWH(input->width(), input->height());

    if (!SkIRect::Intersects(srcIRect, inputBounds)) {
        return nullptr;
    }

    // We create an SkImage here b.c. it needs to be a tight fit for the tiling
    sk_sp<SkImage> subset;
    if (inputBounds.contains(srcIRect)) {
        subset = input->asImage(&srcIRect);
    } else {
        sk_sp<SkSurface> surf(input->makeTightSurface(ctx.outputProperties(), srcIRect.size()));
        if (!surf) {
            return nullptr;
        }

        SkCanvas* canvas = surf->getCanvas();
        SkASSERT(canvas);

        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);

        input->draw(canvas,
                    SkIntToScalar(inputOffset.x()), SkIntToScalar(inputOffset.y()),
                    &paint);

        subset = surf->makeImageSnapshot();
    }
    if (!subset) {
        return nullptr;
    }
    SkASSERT(subset->width() == srcIRect.width());
    SkASSERT(subset->height() == srcIRect.height());

    sk_sp<SkSpecialSurface> surf(source->makeSurface(ctx.outputProperties(), dstIRect.size()));
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setShader(subset->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat));
    canvas->translate(-dstRect.fLeft, -dstRect.fTop);
    canvas->drawRect(dstRect, paint);
    offset->fX = dstIRect.fLeft;
    offset->fY = dstIRect.fTop;
    return surf->makeImageSnapshot();
}

SkIRect SkTileImageFilter::onFilterNodeBounds(const SkIRect& src, const SkMatrix& ctm,
                                              MapDirection dir, const SkIRect* inputRect) const {
    SkRect rect = kReverse_MapDirection == dir ? fSrcRect : fDstRect;
    ctm.mapRect(&rect);
    return rect.roundOut();
}

SkIRect SkTileImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix&,
                                          MapDirection, const SkIRect* inputRect) const {
    // Don't recurse into inputs.
    return src;
}

SkRect SkTileImageFilter::computeFastBounds(const SkRect& src) const {
    return fDstRect;
}

sk_sp<SkFlattenable> SkTileImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkRect src, dst;
    buffer.readRect(&src);
    buffer.readRect(&dst);
    return Make(src, dst, common.getInput(0));
}

void SkTileImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeRect(fSrcRect);
    buffer.writeRect(fDstRect);
}
