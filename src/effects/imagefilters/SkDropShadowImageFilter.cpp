/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"

#if defined(SK_USE_LEGACY_DROPSHADOW_IMAGEFILTER)


#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkWriteBuffer.h"

#include <utility>

namespace {

class SkDropShadowImageFilter final : public SkImageFilter_Base {
public:
    SkDropShadowImageFilter(SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY,
                            SkColor color, bool shadowOnly, sk_sp<SkImageFilter> input,
                            const SkRect* cropRect)
            : INHERITED(&input, 1, cropRect)
            , fDx(dx)
            , fDy(dy)
            , fSigmaX(sigmaX)
            , fSigmaY(sigmaY)
            , fColor(color)
            , fShadowOnly(shadowOnly) {}

    static sk_sp<SkImageFilter> Make(SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY,
                                     SkColor color, bool shadowOnly, sk_sp<SkImageFilter> input,
                                     const SkRect* cropRect) {
        return sk_sp<SkImageFilter>(new SkDropShadowImageFilter(
                dx, dy, sigmaX, sigmaY, color, shadowOnly, std::move(input), cropRect));
    }

    SkRect computeFastBounds(const SkRect&) const override;

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;
    SkIRect onFilterNodeBounds(const SkIRect& src, const SkMatrix& ctm,
                               MapDirection, const SkIRect* inputRect) const override;

private:
    friend void ::SkRegisterLegacyDropShadowImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkDropShadowImageFilter)

    SkScalar fDx, fDy, fSigmaX, fSigmaY;
    SkColor  fColor;
    bool     fShadowOnly;

    using INHERITED = SkImageFilter_Base;
};

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::DropShadow(
        SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY, SkColor color,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return SkDropShadowImageFilter::Make(dx, dy, sigmaX, sigmaY, color, /* shadowOnly */ false,
                                         std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::DropShadowOnly(
        SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY, SkColor color,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return SkDropShadowImageFilter::Make(dx, dy, sigmaX, sigmaY, color, /* shadowOnly */ true,
                                         std::move(input), cropRect);
}

void SkRegisterLegacyDropShadowImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkDropShadowImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkDropShadowImageFilterImpl", SkDropShadowImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkDropShadowImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkScalar dx = buffer.readScalar();
    SkScalar dy = buffer.readScalar();
    SkScalar sigmaX = buffer.readScalar();
    SkScalar sigmaY = buffer.readScalar();
    SkColor color = buffer.readColor();

    // For backwards compatibility, the shadow mode had been saved as an enum cast to a 32LE int,
    // where shadow-and-foreground was 0 and shadow-only was 1. Other than the number of bits, this
    // is equivalent to the bool that SkDropShadowImageFilter now uses.
    bool shadowOnly = SkToBool(buffer.read32LE(1));
    return SkDropShadowImageFilter::Make(dx, dy, sigmaX, sigmaY, color, shadowOnly,
                                         common.getInput(0), common.cropRect());
}

void SkDropShadowImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fDx);
    buffer.writeScalar(fDy);
    buffer.writeScalar(fSigmaX);
    buffer.writeScalar(fSigmaY);
    buffer.writeColor(fColor);
    // See CreateProc, but we save the bool as an int to match previous enum serialization.
    buffer.writeInt(fShadowOnly);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkSpecialImage> SkDropShadowImageFilter::onFilterImage(const Context& ctx,
                                                             SkIPoint* offset) const {
    SkIPoint inputOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> input(this->filterInput(0, ctx, &inputOffset));
    if (!input) {
        return nullptr;
    }

    const SkIRect inputBounds = SkIRect::MakeXYWH(inputOffset.x(), inputOffset.y(),
                                                  input->width(), input->height());
    SkIRect bounds;
    if (!this->applyCropRect(ctx, inputBounds, &bounds)) {
        return nullptr;
    }

    sk_sp<SkSpecialSurface> surf(ctx.makeSurface(bounds.size()));
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);

    canvas->clear(0x0);

    SkVector sigma = SkVector::Make(fSigmaX, fSigmaY);
    ctx.ctm().mapVectors(&sigma, 1);
    sigma.fX = SkScalarAbs(sigma.fX);
    sigma.fY = SkScalarAbs(sigma.fY);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setImageFilter(SkImageFilters::Blur(sigma.fX, sigma.fY, nullptr));
    paint.setColorFilter(SkColorFilters::Blend(fColor, SkBlendMode::kSrcIn));

    SkVector offsetVec = SkVector::Make(fDx, fDy);
    ctx.ctm().mapVectors(&offsetVec, 1);

    canvas->translate(SkIntToScalar(inputOffset.fX) - SkIntToScalar(bounds.fLeft),
                      SkIntToScalar(inputOffset.fY) - SkIntToScalar(bounds.fTop));
    input->draw(canvas, offsetVec.fX, offsetVec.fY, SkSamplingOptions(), &paint);

    if (!fShadowOnly) {
        input->draw(canvas, 0, 0);
    }
    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    return surf->makeImageSnapshot();
}

SkRect SkDropShadowImageFilter::computeFastBounds(const SkRect& src) const {
    SkRect bounds = this->getInput(0) ? this->getInput(0)->computeFastBounds(src) : src;
    SkRect shadowBounds = bounds;
    shadowBounds.offset(fDx, fDy);
    shadowBounds.outset(fSigmaX * 3, fSigmaY * 3);
    if (!fShadowOnly) {
        bounds.join(shadowBounds);
    } else {
        bounds = shadowBounds;
    }
    return bounds;
}

SkIRect SkDropShadowImageFilter::onFilterNodeBounds(
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
    if (!fShadowOnly) {
        dst.join(src);
    }
    return dst;
}

#else

#include "include/core/SkBlendMode.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/effects/imagefilters/SkCropImageFilter.h"

#include <optional>
#include <utility>

struct SkRect;

namespace {

static sk_sp<SkImageFilter> make_drop_shadow_graph(SkVector offset,
                                                   SkSize sigma,
                                                   SkColor color,
                                                   bool shadowOnly,
                                                   sk_sp<SkImageFilter> input,
                                                   const SkRect* crop) {
    // A drop shadow blurs the input, filters it to be the solid color + blurred
    // alpha, and then offsets it. If it's not shadow-only, the input is then
    // src-over blended on top. Finally it's cropped to the optional 'crop'.
    sk_sp<SkImageFilter> filter = input;
    filter = SkImageFilters::Blur(sigma.fWidth, sigma.fHeight, std::move(filter));
    filter = SkImageFilters::ColorFilter(
            SkColorFilters::Blend(color, SkBlendMode::kSrcIn),
            std::move(filter));
    filter = SkImageFilters::Offset(offset.fX, offset.fY, std::move(filter));
    if (!shadowOnly) {
        filter = SkImageFilters::Blend(
                SkBlendMode::kSrcOver, std::move(filter), std::move(input));
    }
    if (crop) {
        filter = SkMakeCropImageFilter(*crop, std::move(filter));
    }
    return filter;
}

sk_sp<SkFlattenable> legacy_drop_shadow_create_proc(SkReadBuffer& buffer) {
    if (!buffer.isVersionLT(SkPicturePriv::Version::kDropShadowImageFilterComposition)) {
        // SKPs created with this version or newer just serialize the image filter composition that
        // is equivalent to a drop-shadow, instead of a single dedicated flattenable for the effect.
        return nullptr;
    }

    auto [child, cropRect] = SkImageFilter_Base::Unflatten(buffer);

    SkScalar dx = buffer.readScalar();
    SkScalar dy = buffer.readScalar();
    SkScalar sigmaX = buffer.readScalar();
    SkScalar sigmaY = buffer.readScalar();
    SkColor color = buffer.readColor();

    // For backwards compatibility, the shadow mode had been saved as an enum cast to a 32LE int,
    // where shadow-and-foreground was 0 and shadow-only was 1. Other than the number of bits, this
    // is equivalent to the bool that SkDropShadowImageFilter now uses.
    bool shadowOnly = SkToBool(buffer.read32LE(1));
    return make_drop_shadow_graph({dx, dy}, {sigmaX, sigmaY}, color, shadowOnly,
                                  std::move(child), cropRect ? &*cropRect : nullptr);
}

} // anonymous namespace

sk_sp<SkImageFilter> SkImageFilters::DropShadow(
        SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY, SkColor color,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return make_drop_shadow_graph({dx, dy}, {sigmaX, sigmaY}, color, /*shadowOnly=*/false,
                                  std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::DropShadowOnly(
        SkScalar dx, SkScalar dy, SkScalar sigmaX, SkScalar sigmaY, SkColor color,
        sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return make_drop_shadow_graph({dx, dy}, {sigmaX, sigmaY}, color, /*shadowOnly=*/true,
                                  std::move(input), cropRect);
}

// TODO (michaelludwig) - Remove after grace period for SKPs to stop using old create proc
void SkRegisterLegacyDropShadowImageFilterFlattenable() {
    SkFlattenable::Register("SkDropShadowImageFilter", legacy_drop_shadow_create_proc);
    SkFlattenable::Register("SkDropShadowImageFilterImpl", legacy_drop_shadow_create_proc);
}

#endif // SK_USE_LEGACY_DROPSHADOW_IMAGEFILTER
