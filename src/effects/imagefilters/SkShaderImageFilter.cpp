/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorSpace.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/imagefilters/SkCropImageFilter.h"

#include <utility>

namespace {

class SkShaderImageFilter final : public SkImageFilter_Base {
public:
    SkShaderImageFilter(sk_sp<SkShader> shader, SkImageFilters::Dither dither)
            : SkImageFilter_Base(nullptr, 0, nullptr)
            , fShader(std::move(shader))
            , fDither(dither) {
        SkASSERT(fShader);
    }

    SkRect computeFastBounds(const SkRect& /*bounds*/) const override {
        return SkRectPriv::MakeLargeS32();
    }

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterShaderImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkShaderImageFilter)

    bool onAffectsTransparentBlack() const override { return true; }

    MatrixCapability onGetCTMCapability() const override { return MatrixCapability::kComplex; }

    skif::FilterResult onFilterImage(const skif::Context&) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping&,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    skif::LayerSpace<SkIRect> onGetOutputLayerBounds(
            const skif::Mapping&,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    sk_sp<SkShader> fShader;
    SkImageFilters::Dither fDither;
};

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::Shader(sk_sp<SkShader> shader,
                                            Dither dither,
                                            const CropRect& cropRect) {
    if (!shader) {
        return SkImageFilters::Empty();
    }

    sk_sp<SkImageFilter> filter{new SkShaderImageFilter(std::move(shader), dither)};
    if (cropRect) {
        filter = SkMakeCropImageFilter(*cropRect, std::move(filter));
    }
    return filter;
}

void SkRegisterShaderImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkShaderImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkPaintImageFilter", SkShaderImageFilter::CreateProc);
    SkFlattenable::Register("SkPaintImageFilterImpl", SkShaderImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkShaderImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 0);
    sk_sp<SkShader> shader;
    bool dither;
    if (buffer.isVersionLT(SkPicturePriv::kShaderImageFilterSerializeShader)) {
        // The old implementation stored an entire SkPaint, but we only need the SkShader and dither
        // boolean. We could fail if the paint stores more effects than that, but this is simpler.
        SkPaint paint = buffer.readPaint();
        shader = paint.getShader() ? paint.refShader()
                                   : SkShaders::Color(paint.getColor4f(), nullptr);
        dither = paint.isDither();
    } else {
        shader = buffer.readShader();
        dither = buffer.readBool();
    }
    return SkImageFilters::Shader(std::move(shader),
                                  SkImageFilters::Dither(dither),
                                  common.cropRect());
}

void SkShaderImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);
    buffer.writeFlattenable(fShader.get());
    buffer.writeBool(fDither == SkImageFilters::Dither::kYes);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

skif::FilterResult SkShaderImageFilter::onFilterImage(const skif::Context& ctx) const {
    const bool dither = fDither == SkImageFilters::Dither::kYes;
    return skif::FilterResult::MakeFromShader(ctx, fShader, dither);
}

skif::LayerSpace<SkIRect> SkShaderImageFilter::onGetInputLayerBounds(
        const skif::Mapping&,
        const skif::LayerSpace<SkIRect>&,
        const skif::LayerSpace<SkIRect>&) const {
    // This is a leaf filter, it requires no input and no further recursion
    return skif::LayerSpace<SkIRect>::Empty();
}

skif::LayerSpace<SkIRect> SkShaderImageFilter::onGetOutputLayerBounds(
        const skif::Mapping&,
        const skif::LayerSpace<SkIRect>&) const {
    // The output of a shader is infinite, unless we were to inspect the shader for a decal
    // tile mode around a gradient or image.
    return skif::LayerSpace<SkIRect>(SkRectPriv::MakeILarge());
}
