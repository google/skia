/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/imagefilters/SkRuntimeImageFilter.h"

#ifdef SK_ENABLE_SKSL

namespace {

class SkRuntimeImageFilter final : public SkImageFilter_Base {
public:
    SkRuntimeImageFilter(sk_sp<SkRuntimeEffect> effect,
                         sk_sp<SkData> uniforms,
                         sk_sp<SkImageFilter> input)
            : INHERITED(&input, 1, /*cropRect=*/nullptr)
            , fEffect(std::move(effect))
            , fUniforms(std::move(uniforms)) {}

    bool onAffectsTransparentBlack() const override { return true; }
    MatrixCapability onGetCTMCapability() const override { return MatrixCapability::kTranslate; }

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;

private:
    friend void ::SkRegisterRuntimeImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkRuntimeImageFilter)

    sk_sp<SkRuntimeEffect> fEffect;
    sk_sp<SkData>          fUniforms;

    using INHERITED = SkImageFilter_Base;
};

} // end namespace

sk_sp<SkImageFilter> SkMakeRuntimeImageFilter(sk_sp<SkRuntimeEffect> effect,
                                              sk_sp<SkData> uniforms,
                                              sk_sp<SkImageFilter> input) {
    // Rather than replicate all of the checks from makeShader here, just try to create a shader
    // once, to determine if everything is valid.
    sk_sp<SkShader> child = nullptr;
    auto shader = effect->makeShader(uniforms, &child, 1, nullptr, false);
    if (!shader) {
        // Could be wrong signature, wrong uniform block size, wrong number/type of children, etc...
        return nullptr;
    }

    return sk_sp<SkImageFilter>(
            new SkRuntimeImageFilter(std::move(effect), std::move(uniforms), std::move(input)));
}

void SkRegisterRuntimeImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkRuntimeImageFilter);
}

sk_sp<SkFlattenable> SkRuntimeImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkString sksl;
    buffer.readString(&sksl);
    sk_sp<SkData> uniforms = buffer.readByteArrayAsData();

    auto effect = SkMakeCachedRuntimeEffect(SkRuntimeEffect::MakeForShader, std::move(sksl));
    if (!buffer.validate(effect != nullptr)) {
        return nullptr;
    }
    if (common.cropRect()) {
        return nullptr;
    }

    return SkMakeRuntimeImageFilter(std::move(effect), std::move(uniforms), common.getInput(0));
}

void SkRuntimeImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeString(fEffect->source().c_str());
    buffer.writeDataAsByteArray(fUniforms.get());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkSpecialImage> SkRuntimeImageFilter::onFilterImage(const Context& ctx,
                                                          SkIPoint* offset) const {
    SkIPoint inputOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> input(this->filterInput(0, ctx, &inputOffset));
    if (!input) {
        return nullptr;
    }

    SkIRect outputBounds = SkIRect(ctx.desiredOutput());
    sk_sp<SkSpecialSurface> surf(ctx.makeSurface(outputBounds.size()));
    if (!surf) {
        return nullptr;
    }

    SkMatrix ctm = ctx.ctm();
    SkMatrix inverse;
    SkAssertResult(ctm.invert(&inverse));

    SkMatrix localM = inverse *
                      SkMatrix::Translate(inputOffset) *
                      SkMatrix::Translate(-input->subset().topLeft());
    sk_sp<SkShader> inputShader =
            input->asImage()->makeShader(SkSamplingOptions(SkFilterMode::kLinear), &localM);
    SkASSERT(inputShader);

    auto shader = fEffect->makeShader(fUniforms, &inputShader, 1, nullptr, false);
    SkASSERT(shader);

    SkPaint paint;
    paint.setShader(std::move(shader));
    paint.setBlendMode(SkBlendMode::kSrc);

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);

    // Translate from layer space into surf's image space
    canvas->translate(-outputBounds.fLeft, -outputBounds.fTop);
    // Ensure shader parameters are relative to parameter space, not layer space
    canvas->concat(ctx.ctm());

    canvas->drawPaint(paint);

    *offset = outputBounds.topLeft();
    return surf->makeImageSnapshot();
}

#endif  // SK_ENABLE_SKSL
