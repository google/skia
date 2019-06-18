/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/effects/SkArithmeticImageFilter.h"
#include "include/effects/SkXfermodeImageFilter.h"
#include "include/private/SkNx.h"
#include "src/core/SkImageFilterPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkWriteBuffer.h"
#if SK_SUPPORT_GPU
#include "include/private/GrRecordingContext.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrColorSpaceXform.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/gpu/effects/GrTextureDomain.h"
#include "src/gpu/effects/generated/GrConstColorProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"

GR_FP_SRC_STRING SKSL_ARITHMETIC_SRC = R"(
in uniform float4 k;
layout(key) const in bool enforcePMColor;
in fragmentProcessor child;

void main(inout half4 color) {
    half4 dst = process(child);
    color = saturate(half(k.x) * color * dst + half(k.y) * color + half(k.z) * dst + half(k.w));
    if (enforcePMColor) {
        color.rgb = min(color.rgb, color.a);
    }
}
)";
#endif

class ArithmeticImageFilterImpl : public SkImageFilter {
public:
    ArithmeticImageFilterImpl(float k1, float k2, float k3, float k4, bool enforcePMColor,
                              sk_sp<SkImageFilter> inputs[2], const CropRect* cropRect)
            : INHERITED(inputs, 2, cropRect), fK{k1, k2, k3, k4}, fEnforcePMColor(enforcePMColor) {}

protected:
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;

    SkIRect onFilterBounds(const SkIRect&, const SkMatrix& ctm,
                           MapDirection, const SkIRect* inputRect) const override;

#if SK_SUPPORT_GPU
    sk_sp<SkSpecialImage> filterImageGPU(SkSpecialImage* source,
                                         sk_sp<SkSpecialImage> background,
                                         const SkIPoint& backgroundOffset,
                                         sk_sp<SkSpecialImage> foreground,
                                         const SkIPoint& foregroundOffset,
                                         const SkIRect& bounds,
                                         const OutputProperties& outputProperties) const;
#endif

    void flatten(SkWriteBuffer& buffer) const override {
        this->INHERITED::flatten(buffer);
        for (int i = 0; i < 4; ++i) {
            buffer.writeScalar(fK[i]);
        }
        buffer.writeBool(fEnforcePMColor);
    }

    void drawForeground(SkCanvas* canvas, SkSpecialImage*, const SkIRect&) const;

private:
    SK_FLATTENABLE_HOOKS(ArithmeticImageFilterImpl)

    bool affectsTransparentBlack() const override { return !SkScalarNearlyZero(fK[3]); }

    const float fK[4];
    const bool fEnforcePMColor;

    friend class ::SkArithmeticImageFilter;

    typedef SkImageFilter INHERITED;
};

sk_sp<SkFlattenable> ArithmeticImageFilterImpl::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);
    float k[4];
    for (int i = 0; i < 4; ++i) {
        k[i] = buffer.readScalar();
    }
    const bool enforcePMColor = buffer.readBool();
    if (!buffer.isValid()) {
        return nullptr;
    }
    return SkArithmeticImageFilter::Make(k[0], k[1], k[2], k[3], enforcePMColor, common.getInput(0),
                                         common.getInput(1), &common.cropRect());
}

static Sk4f pin(float min, const Sk4f& val, float max) {
    return Sk4f::Max(min, Sk4f::Min(val, max));
}

template <bool EnforcePMColor>
void arith_span(const float k[], SkPMColor dst[], const SkPMColor src[], int count) {
    const Sk4f k1 = k[0] * (1/255.0f),
               k2 = k[1],
               k3 = k[2],
               k4 = k[3] * 255.0f + 0.5f;

    for (int i = 0; i < count; i++) {
        Sk4f s = SkNx_cast<float>(Sk4b::Load(src+i)),
             d = SkNx_cast<float>(Sk4b::Load(dst+i)),
             r = pin(0, k1*s*d + k2*s + k3*d + k4, 255);
        if (EnforcePMColor) {
            Sk4f a = SkNx_shuffle<3,3,3,3>(r);
            r = Sk4f::Min(a, r);
        }
        SkNx_cast<uint8_t>(r).store(dst+i);
    }
}

// apply mode to src==transparent (0)
template<bool EnforcePMColor> void arith_transparent(const float k[], SkPMColor dst[], int count) {
    const Sk4f k3 = k[2],
               k4 = k[3] * 255.0f + 0.5f;

    for (int i = 0; i < count; i++) {
        Sk4f d = SkNx_cast<float>(Sk4b::Load(dst+i)),
             r = pin(0, k3*d + k4, 255);
        if (EnforcePMColor) {
            Sk4f a = SkNx_shuffle<3,3,3,3>(r);
            r = Sk4f::Min(a, r);
        }
        SkNx_cast<uint8_t>(r).store(dst+i);
    }
}

static bool intersect(SkPixmap* dst, SkPixmap* src, int srcDx, int srcDy) {
    SkIRect dstR = SkIRect::MakeWH(dst->width(), dst->height());
    SkIRect srcR = SkIRect::MakeXYWH(srcDx, srcDy, src->width(), src->height());
    SkIRect sect;
    if (!sect.intersect(dstR, srcR)) {
        return false;
    }
    *dst = SkPixmap(dst->info().makeWH(sect.width(), sect.height()),
                    dst->addr(sect.fLeft, sect.fTop),
                    dst->rowBytes());
    *src = SkPixmap(src->info().makeWH(sect.width(), sect.height()),
                    src->addr(SkTMax(0, -srcDx), SkTMax(0, -srcDy)),
                    src->rowBytes());
    return true;
}

sk_sp<SkSpecialImage> ArithmeticImageFilterImpl::onFilterImage(SkSpecialImage* source,
                                                               const Context& ctx,
                                                               SkIPoint* offset) const {
    SkIPoint backgroundOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> background(this->filterInput(0, source, ctx, &backgroundOffset));

    SkIPoint foregroundOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> foreground(this->filterInput(1, source, ctx, &foregroundOffset));

    SkIRect foregroundBounds = SkIRect::EmptyIRect();
    if (foreground) {
        foregroundBounds = SkIRect::MakeXYWH(foregroundOffset.x(), foregroundOffset.y(),
                                             foreground->width(), foreground->height());
    }

    SkIRect srcBounds = SkIRect::EmptyIRect();
    if (background) {
        srcBounds = SkIRect::MakeXYWH(backgroundOffset.x(), backgroundOffset.y(),
                                      background->width(), background->height());
    }

    srcBounds.join(foregroundBounds);
    if (srcBounds.isEmpty()) {
        return nullptr;
    }

    SkIRect bounds;
    if (!this->applyCropRect(ctx, srcBounds, &bounds)) {
        return nullptr;
    }

    offset->fX = bounds.left();
    offset->fY = bounds.top();

#if SK_SUPPORT_GPU
    if (source->isTextureBacked()) {
        return this->filterImageGPU(source, background, backgroundOffset, foreground,
                                    foregroundOffset, bounds, ctx.outputProperties());
    }
#endif

    sk_sp<SkSpecialSurface> surf(source->makeSurface(ctx.outputProperties(), bounds.size()));
    if (!surf) {
        return nullptr;
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);

    canvas->clear(0x0);  // can't count on background to fully clear the background
    canvas->translate(SkIntToScalar(-bounds.left()), SkIntToScalar(-bounds.top()));

    if (background) {
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);
        background->draw(canvas, SkIntToScalar(backgroundOffset.fX),
                         SkIntToScalar(backgroundOffset.fY), &paint);
    }

    this->drawForeground(canvas, foreground.get(), foregroundBounds);

    return surf->makeImageSnapshot();
}

SkIRect ArithmeticImageFilterImpl::onFilterBounds(const SkIRect& src,
                                                  const SkMatrix& ctm,
                                                  MapDirection dir,
                                                  const SkIRect* inputRect) const {
    if (kReverse_MapDirection == dir) {
        return SkImageFilter::onFilterBounds(src, ctm, dir, inputRect);
    }

    SkASSERT(2 == this->countInputs());

    // result(i1,i2) = k1*i1*i2 + k2*i1 + k3*i2 + k4
    // Note that background (getInput(0)) is i2, and foreground (getInput(1)) is i1.
    auto i2 = this->getInput(0) ? this->getInput(0)->filterBounds(src, ctm, dir, nullptr) : src;
    auto i1 = this->getInput(1) ? this->getInput(1)->filterBounds(src, ctm, dir, nullptr) : src;

    // Arithmetic with non-zero k4 may influence the complete filter primitive
    // region. [k4 > 0 => result(0,0) = k4 => result(i1,i2) >= k4]
    if (!SkScalarNearlyZero(fK[3])) {
        i1.join(i2);
        return i1;
    }

    // If both K2 or K3 are non-zero, both i1 and i2 appear.
    if (!SkScalarNearlyZero(fK[1]) && !SkScalarNearlyZero(fK[2])) {
        i1.join(i2);
        return i1;
    }

    // If k2 is non-zero, output can be produced whenever i1 is non-transparent.
    // [k3 = k4 = 0 => result(i1,i2) = k1*i1*i2 + k2*i1 = (k1*i2 + k2)*i1]
    if (!SkScalarNearlyZero(fK[1])) {
        return i1;
    }

    // If k3 is non-zero, output can be produced whenever i2 is non-transparent.
    // [k2 = k4 = 0 => result(i1,i2) = k1*i1*i2 + k3*i2 = (k1*i1 + k3)*i2]
    if (!SkScalarNearlyZero(fK[2])) {
        return i2;
    }

    // If just k1 is non-zero, output will only be produce where both inputs
    // are non-transparent. Use intersection.
    // [k1 > 0 and k2 = k3 = k4 = 0 => result(i1,i2) = k1*i1*i2]
    if (!SkScalarNearlyZero(fK[0])) {
        if (!i1.intersect(i2)) {
            return SkIRect::MakeEmpty();
        }
        return i1;
    }

    // [k1 = k2 = k3 = k4 = 0 => result(i1,i2) = 0]
    return SkIRect::MakeEmpty();
}

#if SK_SUPPORT_GPU

sk_sp<SkSpecialImage> ArithmeticImageFilterImpl::filterImageGPU(
        SkSpecialImage* source,
        sk_sp<SkSpecialImage> background,
        const SkIPoint& backgroundOffset,
        sk_sp<SkSpecialImage> foreground,
        const SkIPoint& foregroundOffset,
        const SkIRect& bounds,
        const OutputProperties& outputProperties) const {
    SkASSERT(source->isTextureBacked());

    auto context = source->getContext();

    sk_sp<GrTextureProxy> backgroundProxy, foregroundProxy;

    if (background) {
        backgroundProxy = background->asTextureProxyRef(context);
    }

    if (foreground) {
        foregroundProxy = foreground->asTextureProxyRef(context);
    }

    GrPaint paint;
    std::unique_ptr<GrFragmentProcessor> bgFP;

    if (backgroundProxy) {
        SkIRect bgSubset = background->subset();
        SkMatrix backgroundMatrix = SkMatrix::MakeTrans(
                SkIntToScalar(bgSubset.left() - backgroundOffset.fX),
                SkIntToScalar(bgSubset.top()  - backgroundOffset.fY));
        bgFP = GrTextureDomainEffect::Make(
                std::move(backgroundProxy), backgroundMatrix,
                GrTextureDomain::MakeTexelDomain(bgSubset, GrTextureDomain::kDecal_Mode),
                GrTextureDomain::kDecal_Mode, GrSamplerState::Filter::kNearest);
        bgFP = GrColorSpaceXformEffect::Make(std::move(bgFP), background->getColorSpace(),
                                             background->alphaType(),
                                             outputProperties.colorSpace());
    } else {
        bgFP = GrConstColorProcessor::Make(SK_PMColor4fTRANSPARENT,
                                           GrConstColorProcessor::InputMode::kIgnore);
    }

    if (foregroundProxy) {
        SkIRect fgSubset = foreground->subset();
        SkMatrix foregroundMatrix = SkMatrix::MakeTrans(
                SkIntToScalar(fgSubset.left() - foregroundOffset.fX),
                SkIntToScalar(fgSubset.top()  - foregroundOffset.fY));
        auto foregroundFP = GrTextureDomainEffect::Make(
                std::move(foregroundProxy), foregroundMatrix,
                GrTextureDomain::MakeTexelDomain(fgSubset, GrTextureDomain::kDecal_Mode),
                GrTextureDomain::kDecal_Mode, GrSamplerState::Filter::kNearest);
        foregroundFP = GrColorSpaceXformEffect::Make(std::move(foregroundFP),
                                                     foreground->getColorSpace(),
                                                     foreground->alphaType(),
                                                     outputProperties.colorSpace());
        paint.addColorFragmentProcessor(std::move(foregroundFP));

        static int arithmeticIndex = GrSkSLFP::NewIndex();
        ArithmeticFPInputs inputs;
        static_assert(sizeof(inputs.k) == sizeof(fK), "struct size mismatch");
        memcpy(inputs.k, fK, sizeof(inputs.k));
        inputs.enforcePMColor = fEnforcePMColor;
        std::unique_ptr<GrFragmentProcessor> xferFP = GrSkSLFP::Make(context,
                                                                     arithmeticIndex,
                                                                     "Arithmetic",
                                                                     SKSL_ARITHMETIC_SRC,
                                                                     &inputs,
                                                                     sizeof(inputs));
        if (xferFP) {
            ((GrSkSLFP&) *xferFP).addChild(std::move(bgFP));
            paint.addColorFragmentProcessor(std::move(xferFP));
        }
    } else {
        paint.addColorFragmentProcessor(std::move(bgFP));
    }

    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);

    SkColorType colorType = outputProperties.colorType();
    GrBackendFormat format =
            context->priv().caps()->getBackendFormatFromColorType(colorType);

    sk_sp<GrRenderTargetContext> renderTargetContext(
        context->priv().makeDeferredRenderTargetContext(
            format, SkBackingFit::kApprox, bounds.width(), bounds.height(),
            SkColorType2GrPixelConfig(colorType),
            sk_ref_sp(outputProperties.colorSpace())));
    if (!renderTargetContext) {
        return nullptr;
    }

    SkMatrix matrix;
    matrix.setTranslate(SkIntToScalar(-bounds.left()), SkIntToScalar(-bounds.top()));
    renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, matrix,
                                  SkRect::Make(bounds));

    return SkSpecialImage::MakeDeferredFromGpu(
            context,
            SkIRect::MakeWH(bounds.width(), bounds.height()),
            kNeedNewImageUniqueID_SpecialImage,
            renderTargetContext->asTextureProxyRef(),
            renderTargetContext->colorSpaceInfo().refColorSpace());
}
#endif

void ArithmeticImageFilterImpl::drawForeground(SkCanvas* canvas, SkSpecialImage* img,
                                               const SkIRect& fgBounds) const {
    SkPixmap dst;
    if (!canvas->peekPixels(&dst)) {
        return;
    }

    const SkMatrix& ctm = canvas->getTotalMatrix();
    SkASSERT(ctm.getType() <= SkMatrix::kTranslate_Mask);
    const int dx = SkScalarRoundToInt(ctm.getTranslateX());
    const int dy = SkScalarRoundToInt(ctm.getTranslateY());
    // be sure to perform this offset using SkIRect, since it saturates to avoid overflows
    const SkIRect fgoffset = fgBounds.makeOffset(dx, dy);

    if (img) {
        SkBitmap srcBM;
        SkPixmap src;
        if (!img->getROPixels(&srcBM)) {
            return;
        }
        if (!srcBM.peekPixels(&src)) {
            return;
        }

        auto proc = fEnforcePMColor ? arith_span<true> : arith_span<false>;
        SkPixmap tmpDst = dst;
        if (intersect(&tmpDst, &src, fgoffset.fLeft, fgoffset.fTop)) {
            for (int y = 0; y < tmpDst.height(); ++y) {
                proc(fK, tmpDst.writable_addr32(0, y), src.addr32(0, y), tmpDst.width());
            }
        }
    }

    // Now apply the mode with transparent-color to the outside of the fg image
    SkRegion outside(SkIRect::MakeWH(dst.width(), dst.height()));
    outside.op(fgoffset, SkRegion::kDifference_Op);
    auto proc = fEnforcePMColor ? arith_transparent<true> : arith_transparent<false>;
    for (SkRegion::Iterator iter(outside); !iter.done(); iter.next()) {
        const SkIRect r = iter.rect();
        for (int y = r.fTop; y < r.fBottom; ++y) {
            proc(fK, dst.writable_addr32(r.fLeft, y), r.width());
        }
    }
}

sk_sp<SkImageFilter> SkArithmeticImageFilter::Make(float k1, float k2, float k3, float k4,
                                                   bool enforcePMColor,
                                                   sk_sp<SkImageFilter> background,
                                                   sk_sp<SkImageFilter> foreground,
                                                   const SkImageFilter::CropRect* crop) {
    if (!SkScalarIsFinite(k1) || !SkScalarIsFinite(k2) || !SkScalarIsFinite(k3) ||
        !SkScalarIsFinite(k4)) {
        return nullptr;
    }

    // are we nearly some other "std" mode?
    int mode = -1;  // illegal mode
    if (SkScalarNearlyZero(k1) && SkScalarNearlyEqual(k2, SK_Scalar1) && SkScalarNearlyZero(k3) &&
        SkScalarNearlyZero(k4)) {
        mode = (int)SkBlendMode::kSrc;
    } else if (SkScalarNearlyZero(k1) && SkScalarNearlyZero(k2) &&
               SkScalarNearlyEqual(k3, SK_Scalar1) && SkScalarNearlyZero(k4)) {
        mode = (int)SkBlendMode::kDst;
    } else if (SkScalarNearlyZero(k1) && SkScalarNearlyZero(k2) && SkScalarNearlyZero(k3) &&
               SkScalarNearlyZero(k4)) {
        mode = (int)SkBlendMode::kClear;
    }
    if (mode >= 0) {
        return SkXfermodeImageFilter::Make((SkBlendMode)mode, std::move(background),
                                           std::move(foreground), crop);
    }

    sk_sp<SkImageFilter> inputs[2] = {std::move(background), std::move(foreground)};
    return sk_sp<SkImageFilter>(
            new ArithmeticImageFilterImpl(k1, k2, k3, k4, enforcePMColor, inputs, crop));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SkArithmeticImageFilter::RegisterFlattenables() {
    SK_REGISTER_FLATTENABLE(ArithmeticImageFilterImpl);
}
