/*
 * Copyright 2014 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFlattenable.h"
#include "src/core/SkImageFilter_Base.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkRect.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSamplingPriv.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkWriteBuffer.h"

namespace {

class SkMatrixTransformImageFilter final : public SkImageFilter_Base {
public:
    // TODO(michaelludwig): Update this to use SkM44.
    SkMatrixTransformImageFilter(const SkMatrix& transform,
                                 const SkSamplingOptions& sampling,
                                 sk_sp<SkImageFilter> input)
            : SkImageFilter_Base(&input, 1, nullptr)
            , fTransform(transform)
            , fSampling(sampling) {
        // Pre-cache so future calls to fTransform.getType() are threadsafe.
        (void)fTransform.getType();
    }

    SkRect computeFastBounds(const SkRect&) const override;

protected:

    void flatten(SkWriteBuffer&) const override;

    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;
    SkIRect onFilterNodeBounds(const SkIRect& src, const SkMatrix& ctm,
                               MapDirection, const SkIRect* inputRect) const override;

private:
    friend void ::SkRegisterMatrixTransformImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkMatrixTransformImageFilter)

    SkMatrix            fTransform;
    SkSamplingOptions   fSampling;
};

} // namespace

sk_sp<SkImageFilter> SkImageFilters::MatrixTransform(const SkMatrix& transform,
                                                     const SkSamplingOptions& sampling,
                                                     sk_sp<SkImageFilter> input) {
    return sk_sp<SkImageFilter>(new SkMatrixTransformImageFilter(transform,
                                                                 sampling,
                                                                 std::move(input)));
}

void SkRegisterMatrixTransformImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkMatrixTransformImageFilter);
    // TODO(michaelludwig): Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkMatrixImageFilter", SkMatrixTransformImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkMatrixTransformImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkMatrix matrix;
    buffer.readMatrix(&matrix);

    auto sampling = [&]() {
        if (buffer.isVersionLT(SkPicturePriv::kMatrixImageFilterSampling_Version)) {
            return SkSamplingPriv::FromFQ(buffer.read32LE(kLast_SkLegacyFQ), kLinear_SkMediumAs);
        } else {
            return buffer.readSampling();
        }
    }();
    return SkImageFilters::MatrixTransform(matrix, sampling, common.getInput(0));
}

void SkMatrixTransformImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);
    buffer.writeMatrix(fTransform);
    buffer.writeSampling(fSampling);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkSpecialImage> SkMatrixTransformImageFilter::onFilterImage(const Context& ctx,
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

SkRect SkMatrixTransformImageFilter::computeFastBounds(const SkRect& src) const {
    SkRect bounds = this->getInput(0) ? this->getInput(0)->computeFastBounds(src) : src;
    SkRect dst;
    fTransform.mapRect(&dst, bounds);
    return dst;
}

SkIRect SkMatrixTransformImageFilter::onFilterNodeBounds(const SkIRect& src,
                                                         const SkMatrix& ctm,
                                                         MapDirection dir,
                                                         const SkIRect* inputRect) const {
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
