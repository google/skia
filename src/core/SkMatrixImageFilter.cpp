/*
 * Copyright 2014 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMatrixImageFilter.h"

#include "SkCanvas.h"
#include "SkColorSpaceXformer.h"
#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkSpecialSurface.h"
#include "SkWriteBuffer.h"
#include "SkRect.h"

SkMatrixImageFilter::SkMatrixImageFilter(const SkSize& scale, const SkMatrix& transform,
                                         SkFilterQuality filterQuality,
                                         sk_sp<SkImageFilter> input)
    : INHERITED(&input, 1, nullptr)
    , fScale(scale)
    , fTransform1(transform)
    , fFilterQuality(filterQuality) {
}

sk_sp<SkImageFilter> SkMatrixImageFilter::Make(const SkSize& scale,
                                               const SkMatrix& transform,
                                               SkFilterQuality filterQuality,
                                               sk_sp<SkImageFilter> input) {
    return sk_sp<SkImageFilter>(new SkMatrixImageFilter(scale, transform,
                                                        filterQuality,
                                                        std::move(input)));
}

sk_sp<SkFlattenable> SkMatrixImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkMatrix matrix;
    buffer.readMatrix(&matrix);
    SkFilterQuality quality = static_cast<SkFilterQuality>(buffer.readInt());
    return Make(SkSize::Make(1.0f, 1.0f), matrix, quality, common.getInput(0));
}

void SkMatrixImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeMatrix(fTransform1);
    buffer.writeInt(fFilterQuality);
}

sk_sp<SkSpecialImage> SkMatrixImageFilter::onFilterImage(SkSpecialImage* source,
                                                         const Context& ctx,
                                                         SkIPoint* offset) const {

    SkIPoint inputOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> input(this->filterInput(0, source, ctx, &inputOffset));
    if (!input) {
        return nullptr;
    }

#if 0
    SkMatrix matrix;
    if (!ctx.ctm().invert(&matrix)) {
        return nullptr;
    }
    matrix.postConcat(fTransform);
    matrix.postConcat(ctx.ctm());
#else
    SkMatrix matrix;
    if (!ctx.ctm().invert(&matrix)) {
        return nullptr;
    }
    matrix.postScale(fScale.fWidth, fScale.fHeight);
    matrix.postConcat(fTransform1);
    matrix.postScale(1.0f/fScale.fWidth, 1.0f/fScale.fHeight);
    matrix.postConcat(ctx.ctm());
#endif

    const SkIRect srcBounds = SkIRect::MakeXYWH(inputOffset.x(), inputOffset.y(),
                                                input->width(), input->height());
    const SkRect srcRect = SkRect::Make(srcBounds);

    SkRect dstRect;
    matrix.mapRect(&dstRect, srcRect);
    SkIRect dstBounds;
    dstRect.roundOut(&dstBounds);

    sk_sp<SkSpecialSurface> surf(input->makeSurface(ctx.outputProperties(), dstBounds.size()));
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);

    canvas->clear(0x0);

    canvas->translate(-SkIntToScalar(dstBounds.x()), -SkIntToScalar(dstBounds.y()));
    canvas->concat(matrix);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setFilterQuality(fFilterQuality);

    input->draw(canvas, srcRect.x(), srcRect.y(), &paint);

    offset->fX = dstBounds.fLeft;
    offset->fY = dstBounds.fTop;
    return surf->makeImageSnapshot();
}

sk_sp<SkImageFilter> SkMatrixImageFilter::onMakeColorSpace(SkColorSpaceXformer* xformer) const {
    SkASSERT(1 == this->countInputs());
    auto input = xformer->apply(this->getInput(0));
    if (input.get() != this->getInput(0)) {
        return SkMatrixImageFilter::Make(fScale, fTransform1, fFilterQuality, std::move(input));
    }
    return this->refMe();
}

SkRect SkMatrixImageFilter::computeFastBounds(const SkRect& src) const {
    SkRect bounds = this->getInput(0) ? this->getInput(0)->computeFastBounds(src) : src;
    SkRect dst;
    fTransform1.mapRect(&dst, bounds);
    return dst;
}

SkIRect SkMatrixImageFilter::onFilterNodeBounds(const SkIRect& src, const SkMatrix& ctm,
                                                MapDirection direction) const {

#if 0
    SkMatrix matrix;
    if (!ctm.invert(&matrix)) {
        return src;
    }
    if (kForward_MapDirection == direction) {
        matrix.postConcat(fTransform);
        matrix.postConcat(ctm);
    } else {
        SkMatrix transformInverse;
        if (!fTransform.invert(&transformInverse)) {
            return src;
        }
        matrix.postConcat(transformInverse);
        matrix.postConcat(ctm);
    }
#else
    SkMatrix matrix;
    if (!ctm.invert(&matrix)) {
        return src;
    }
    if (kForward_MapDirection == direction) {
        matrix.postScale(fScale.fWidth, fScale.fHeight);
        matrix.postConcat(fTransform1);
        matrix.postScale(1.0f/fScale.fWidth, 1.0f/fScale.fHeight);
        matrix.postConcat(ctm);
    } else {
        SkMatrix transformInverse;
        if (!fTransform1.invert(&transformInverse)) {
            return src;
        }
        matrix.postScale(fScale.fWidth, fScale.fHeight);
        matrix.postConcat(transformInverse);
        matrix.postScale(1.0f/fScale.fWidth, 1.0f/fScale.fHeight);
        matrix.postConcat(ctm);
    }
#endif

    SkRect floatBounds;
    matrix.mapRect(&floatBounds, SkRect::Make(src));
    return floatBounds.roundOut();
}

#ifndef SK_IGNORE_TO_STRING
void SkMatrixImageFilter::toString(SkString* str) const {
    str->appendf("SkMatrixImageFilter: (");

    str->appendf("transform: (%f %f %f %f %f %f %f %f %f)",
                 fTransform1[SkMatrix::kMScaleX],
                 fTransform1[SkMatrix::kMSkewX],
                 fTransform1[SkMatrix::kMTransX],
                 fTransform1[SkMatrix::kMSkewY],
                 fTransform1[SkMatrix::kMScaleY],
                 fTransform1[SkMatrix::kMTransY],
                 fTransform1[SkMatrix::kMPersp0],
                 fTransform1[SkMatrix::kMPersp1],
                 fTransform1[SkMatrix::kMPersp2]);

    str->append("<dt>FilterLevel:</dt><dd>");
    static const char* gFilterLevelStrings[] = { "None", "Low", "Medium", "High" };
    str->append(gFilterLevelStrings[fFilterQuality]);
    str->append("</dd>");

    str->appendf(")");
}
#endif
