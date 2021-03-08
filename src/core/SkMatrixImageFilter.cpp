/*
 * Copyright 2014 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkMatrixImageFilter.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkRect.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSamplingPriv.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkWriteBuffer.h"

SkMatrixImageFilter::SkMatrixImageFilter(const SkMatrix& transform,
                                         const SkSamplingOptions& sampling,
                                         sk_sp<SkImageFilter> input)
    : INHERITED(&input, 1, nullptr)
    , fTransform(transform)
    , fSampling(sampling) {
    // Pre-cache so future calls to fTransform.getType() are threadsafe.
    (void)fTransform.getType();
}

sk_sp<SkImageFilter> SkMatrixImageFilter::Make(const SkMatrix& transform,
                                               const SkSamplingOptions& sampling,
                                               sk_sp<SkImageFilter> input) {
    return sk_sp<SkImageFilter>(new SkMatrixImageFilter(transform,
                                                        sampling,
                                                        std::move(input)));
}

sk_sp<SkImageFilter> SkImageFilters::MatrixTransform(
        const SkMatrix& transform, const SkSamplingOptions& sampling, sk_sp<SkImageFilter> input) {
    return SkMatrixImageFilter::Make(transform, sampling, std::move(input));
}

#ifdef SK_SUPPORT_LEGACY_MATRIX_IMAGEFILTER
sk_sp<SkImageFilter> SkImageFilters::MatrixTransform(
        const SkMatrix& transform, SkFilterQuality filterQuality, sk_sp<SkImageFilter> input) {
    auto sampling = SkSamplingOptions(filterQuality,
                                      SkSamplingOptions::kMedium_asMipmapLinear);
    return SkMatrixImageFilter::Make(transform, sampling, std::move(input));
}
#endif

sk_sp<SkFlattenable> SkMatrixImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkMatrix matrix;
    buffer.readMatrix(&matrix);

    auto sampling = [&]() {
        if (buffer.isVersionLT(SkPicturePriv::kMatrixImageFilterSampling_Version)) {
            return SkSamplingOptions(buffer.read32LE(kLast_SkFilterQuality),
                                     SkSamplingOptions::kMedium_asMipmapLinear);
        } else {
            return SkSamplingPriv::Read(buffer);
        }
    }();
    return Make(matrix, sampling, common.getInput(0));
}

void SkMatrixImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeMatrix(fTransform);
    SkSamplingPriv::Write(buffer, fSampling);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkSpecialImage> SkMatrixImageFilter::onFilterImage(const Context& ctx,
                                                         SkIPoint* offset) const {

    SkIPoint inputOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> input(this->filterInput(0, ctx, &inputOffset));
    if (!input) {
        return nullptr;
    }

    SkMatrix matrix;
    if (!ctx.ctm().invert(&matrix)) {
        return nullptr;
    }
    matrix.postConcat(fTransform);
    matrix.postConcat(ctx.ctm());

    const SkIRect srcBounds = SkIRect::MakeXYWH(inputOffset.x(), inputOffset.y(),
                                                input->width(), input->height());
    const SkRect srcRect = SkRect::Make(srcBounds);

    SkRect dstRect;
    matrix.mapRect(&dstRect, srcRect);
    SkIRect dstBounds;
    dstRect.roundOut(&dstBounds);

    sk_sp<SkSpecialSurface> surf(ctx.makeSurface(dstBounds.size()));
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

    input->draw(canvas, srcRect.x(), srcRect.y(), fSampling, &paint);

    offset->fX = dstBounds.fLeft;
    offset->fY = dstBounds.fTop;
    return surf->makeImageSnapshot();
}

SkRect SkMatrixImageFilter::computeFastBounds(const SkRect& src) const {
    SkRect bounds = this->getInput(0) ? this->getInput(0)->computeFastBounds(src) : src;
    SkRect dst;
    fTransform.mapRect(&dst, bounds);
    return dst;
}

SkIRect SkMatrixImageFilter::onFilterNodeBounds(const SkIRect& src, const SkMatrix& ctm,
                                                MapDirection dir, const SkIRect* inputRect) const {
    SkMatrix matrix;
    if (!ctm.invert(&matrix)) {
        return src;
    }
    if (kForward_MapDirection == dir) {
        matrix.postConcat(fTransform);
    } else {
        SkMatrix transformInverse;
        if (!fTransform.invert(&transformInverse)) {
            return src;
        }
        matrix.postConcat(transformInverse);
    }
    matrix.postConcat(ctm);
    SkRect floatBounds;
    matrix.mapRect(&floatBounds, SkRect::Make(src));
    SkIRect result = floatBounds.roundOut();

    if (kReverse_MapDirection == dir && SkSamplingOptions() != fSampling) {
        // When filtering we might need some pixels in the source that might be otherwise
        // clipped off.
        result.outset(1, 1);
    }

    return result;
}
