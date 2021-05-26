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

namespace {

class SkRuntimeImageFilter final : public SkImageFilter_Base {
public:
    SkRuntimeImageFilter(sk_sp<SkRuntimeEffect> effect,
                         sk_sp<SkData> uniforms,
                         sk_sp<SkImageFilter> input,
                         const SkRect* cropRect)
            : INHERITED(&input, 1, cropRect)
            , fEffect(std::move(effect))
            , fUniforms(std::move(uniforms)) {}

    bool affectsTransparentBlack() const override { return true; }

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
                                              sk_sp<SkImageFilter> input,
                                              const SkRect* cropRect) {
    // Rather than replicate all of the checks from makeShader here, just try to create a shader
    // once, to determine if everything is valid.
    sk_sp<SkShader> child = nullptr;
    auto shader = effect->makeShader(uniforms, &child, 1, nullptr, false);
    if (!shader) {
        // Could be wrong signature, wrong uniform block size, wrong number/type of children, etc...
        return nullptr;
    }

    return sk_sp<SkImageFilter>(new SkRuntimeImageFilter(
            std::move(effect), std::move(uniforms), std::move(input), cropRect));
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

    return SkMakeRuntimeImageFilter(
            std::move(effect), std::move(uniforms), common.getInput(0), common.cropRect());
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

    // TODO: This isn't properly taking the subset of the image, so we're going to see slop
    // TODO: Once we do take the subset, the GPU backend is going to incur a copy, unless we
    //       specialize the implementation to directly construct an FP.
    sk_sp<SkShader> inputShader =
            input->asImage()->makeShader(SkSamplingOptions(SkFilterMode::kLinear));
    SkASSERT(inputShader);

    auto shader = fEffect->makeShader(fUniforms, &inputShader, 1, nullptr, false);
    SkASSERT(shader);

    SkPaint paint;
    paint.setShader(std::move(shader));

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);

    canvas->clear(0x0);

    SkMatrix matrix(ctx.ctm());
    matrix.postTranslate(SkIntToScalar(-bounds.left()), SkIntToScalar(-bounds.top()));
    SkRect rect = SkRect::MakeIWH(bounds.width(), bounds.height());
    SkMatrix inverse;
    if (matrix.invert(&inverse)) {
        inverse.mapRect(&rect);
    }
    canvas->setMatrix(matrix);
    if (rect.isFinite()) {
        canvas->drawRect(rect, paint);
    }

    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    return surf->makeImageSnapshot();
}
