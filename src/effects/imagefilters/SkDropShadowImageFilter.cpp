/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkDropShadowImageFilter.h"

#include "include/core/SkCanvas.h"
#include "include/effects/SkBlurImageFilter.h"
#include "src/core/SkImageFilterPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkWriteBuffer.h"

namespace {

class SkDropShadowImageFilterImpl final : public SkImageFilter {
public:
    static sk_sp<SkImageFilter> Make(SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY,
                                     SkColor color, SkDropShadowImageFilter::ShadowMode shadowMode,
                                     sk_sp<SkImageFilter> input,
                                     const CropRect* cropRect = nullptr) {
        return sk_sp<SkImageFilter>(new SkDropShadowImageFilterImpl(
                dx, dy, sigmaX, sigmaY, color, shadowMode, std::move(input), cropRect));
    }

    SkRect computeFastBounds(const SkRect&) const override;

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;
    SkIRect onFilterNodeBounds(const SkIRect& src, const SkMatrix& ctm,
                               MapDirection, const SkIRect* inputRect) const override;

private:
    friend void SkDropShadowImageFilter::RegisterFlattenables();
    SK_FLATTENABLE_HOOKS(SkDropShadowImageFilterImpl)

    SkDropShadowImageFilterImpl(SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY,
                                SkColor color, SkDropShadowImageFilter::ShadowMode shadowMode,
                                sk_sp<SkImageFilter> input, const CropRect* cropRect)
            : INHERITED(&input, 1, cropRect)
            , fDx(dx)
            , fDy(dy)
            , fSigmaX(sigmaX)
            , fSigmaY(sigmaY)
            , fColor(color)
            , fShadowMode(shadowMode) {}

    SkScalar fDx, fDy, fSigmaX, fSigmaY;
    SkColor fColor;
    SkDropShadowImageFilter::ShadowMode fShadowMode;

    typedef SkImageFilter INHERITED;
};

} // end namespace

sk_sp<SkImageFilter> SkDropShadowImageFilter::Make(SkScalar dx, SkScalar dy,
                                                   SkScalar sigmaX, SkScalar sigmaY,
                                                   SkColor color, ShadowMode shadowMode,
                                                   sk_sp<SkImageFilter> input,
                                                   const SkImageFilter::CropRect* cropRect) {
    return SkDropShadowImageFilterImpl::Make(
            dx, dy, sigmaX, sigmaY, color, shadowMode, std::move(input), cropRect);
}

void SkDropShadowImageFilter::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkDropShadowImageFilterImpl);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkDropShadowImageFilter", SkDropShadowImageFilterImpl::CreateProc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkDropShadowImageFilterImpl::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkScalar dx = buffer.readScalar();
    SkScalar dy = buffer.readScalar();
    SkScalar sigmaX = buffer.readScalar();
    SkScalar sigmaY = buffer.readScalar();
    SkColor color = buffer.readColor();

    SkDropShadowImageFilter::ShadowMode shadowMode = buffer.read32LE(
            SkDropShadowImageFilter::kLast_ShadowMode);

    return Make(dx, dy, sigmaX, sigmaY, color, shadowMode, common.getInput(0), &common.cropRect());
}

void SkDropShadowImageFilterImpl::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fDx);
    buffer.writeScalar(fDy);
    buffer.writeScalar(fSigmaX);
    buffer.writeScalar(fSigmaY);
    buffer.writeColor(fColor);
    buffer.writeInt(static_cast<int>(fShadowMode));
}

sk_sp<SkSpecialImage> SkDropShadowImageFilterImpl::onFilterImage(SkSpecialImage* source,
                                                                 const Context& ctx,
                                                                 SkIPoint* offset) const {
    SkIPoint inputOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> input(this->filterInput(0, source, ctx, &inputOffset));
    if (!input) {
        return nullptr;
    }

    const SkIRect inputBounds = SkIRect::MakeXYWH(inputOffset.x(), inputOffset.y(),
                                                  input->width(), input->height());
    SkIRect bounds;
    if (!this->applyCropRect(ctx, inputBounds, &bounds)) {
        return nullptr;
    }

    sk_sp<SkSpecialSurface> surf(source->makeSurface(ctx.outputProperties(), bounds.size()));
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);

    canvas->clear(0x0);

    SkVector sigma = SkVector::Make(fSigmaX, fSigmaY);
    ctx.ctm().mapVectors(&sigma, 1);
    sigma.fX = SkMaxScalar(0, sigma.fX);
    sigma.fY = SkMaxScalar(0, sigma.fY);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setImageFilter(SkBlurImageFilter::Make(sigma.fX, sigma.fY, nullptr));
    paint.setColorFilter(SkColorFilters::Blend(fColor, SkBlendMode::kSrcIn));

    SkVector offsetVec = SkVector::Make(fDx, fDy);
    ctx.ctm().mapVectors(&offsetVec, 1);

    canvas->translate(SkIntToScalar(inputOffset.fX - bounds.fLeft),
                      SkIntToScalar(inputOffset.fY - bounds.fTop));
    input->draw(canvas, offsetVec.fX, offsetVec.fY, &paint);

    if (fShadowMode == SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode) {
        input->draw(canvas, 0, 0, nullptr);
    }
    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    return surf->makeImageSnapshot();
}

SkRect SkDropShadowImageFilterImpl::computeFastBounds(const SkRect& src) const {
    SkRect bounds = this->getInput(0) ? this->getInput(0)->computeFastBounds(src) : src;
    SkRect shadowBounds = bounds;
    shadowBounds.offset(fDx, fDy);
    shadowBounds.outset(fSigmaX * 3, fSigmaY * 3);
    if (fShadowMode == SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode) {
        bounds.join(shadowBounds);
    } else {
        bounds = shadowBounds;
    }
    return bounds;
}

SkIRect SkDropShadowImageFilterImpl::onFilterNodeBounds(
        const SkIRect& src, const SkMatrix& ctm, MapDirection dir, const SkIRect* inputRect) const {
    SkVector offsetVec = SkVector::Make(fDx, fDy);
    if (kReverse_MapDirection == dir) {
        offsetVec.negate();
    }
    ctm.mapVectors(&offsetVec, 1);
    SkIRect dst = src.makeOffset(SkScalarCeilToInt(offsetVec.x()),
                                 SkScalarCeilToInt(offsetVec.y()));
    SkVector sigma = SkVector::Make(fSigmaX, fSigmaY);
    ctm.mapVectors(&sigma, 1);
    dst.outset(
        SkScalarCeilToInt(SkScalarAbs(sigma.x() * 3)),
        SkScalarCeilToInt(SkScalarAbs(sigma.y() * 3)));
    if (fShadowMode == SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode) {
        dst.join(src);
    }
    return dst;
}

