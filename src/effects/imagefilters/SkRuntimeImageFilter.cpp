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
#include "include/private/SkSpinlock.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/imagefilters/SkRuntimeImageFilter.h"

#ifdef SK_ENABLE_SKSL

class SkRuntimeImageFilter final : public SkImageFilter_Base {
public:
    SkRuntimeImageFilter(sk_sp<SkRuntimeEffect> effect,
                         sk_sp<SkData> uniforms,
                         sk_sp<SkImageFilter> input)
            : INHERITED(&input, 1, /*cropRect=*/nullptr)
            , fShaderBuilder(std::move(effect), std::move(uniforms))
            , fChildShaderName(fShaderBuilder.effect()->children().front().name) {}
    SkRuntimeImageFilter(const SkRuntimeShaderBuilder& builder,
                         const char* childShaderName,
                         sk_sp<SkImageFilter> input)
            : INHERITED(&input, 1, /*cropRect=*/nullptr)
            , fShaderBuilder(builder)
            , fChildShaderName(childShaderName) {}

    bool onAffectsTransparentBlack() const override { return true; }
    MatrixCapability onGetCTMCapability() const override { return MatrixCapability::kTranslate; }

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;

private:
    friend void ::SkRegisterRuntimeImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkRuntimeImageFilter)

    mutable SkSpinlock fShaderBuilderLock;
    mutable SkRuntimeShaderBuilder fShaderBuilder;
    SkString fChildShaderName;

    using INHERITED = SkImageFilter_Base;
};

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
    if (common.cropRect()) {
        return nullptr;
    }

    // Read the SkSL string and convert it into a runtime effect
    SkString sksl;
    buffer.readString(&sksl);
    auto effect = SkMakeCachedRuntimeEffect(SkRuntimeEffect::MakeForShader, std::move(sksl));
    if (!buffer.validate(effect != nullptr)) {
        return nullptr;
    }

    // Read the uniform data and make sure it matches the size from the runtime effect
    sk_sp<SkData> uniforms = buffer.readByteArrayAsData();
    if (!buffer.validate(uniforms->size() == effect->uniformSize())) {
        return nullptr;
    }

    // Read the child shader name and make sure it matches one declared in the effect
    SkString childShaderName;
    buffer.readString(&childShaderName);
    if (!buffer.validate(effect->findChild(childShaderName.c_str()) != nullptr)) {
        return nullptr;
    }

    SkRuntimeShaderBuilder builder(std::move(effect), std::move(uniforms));

    // Populate the builder with the corresponding children
    for (auto& child : builder.effect()->children()) {
        const char* name = child.name.c_str();
        switch (child.type) {
            case SkRuntimeEffect::ChildType::kBlender: {
                builder.child(name) = buffer.readBlender();
                break;
            }
            case SkRuntimeEffect::ChildType::kColorFilter: {
                builder.child(name) = buffer.readColorFilter();
                break;
            }
            case SkRuntimeEffect::ChildType::kShader: {
                builder.child(name) = buffer.readShader();
                break;
            }
        }
    }

    if (!buffer.isValid()) {
        return nullptr;
    }

    return SkImageFilters::RuntimeShader(builder, childShaderName.c_str(), common.getInput(0));
}

void SkRuntimeImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    fShaderBuilderLock.acquire();
    buffer.writeString(fShaderBuilder.effect()->source().c_str());
    buffer.writeDataAsByteArray(fShaderBuilder.uniforms().get());
    buffer.writeString(fChildShaderName.c_str());
    for (size_t x = 0; x < fShaderBuilder.numChildren(); x++) {
        buffer.writeFlattenable(fShaderBuilder.children()[x].flattenable());
    }
    fShaderBuilderLock.release();
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

    // lock the mutation of the builder and creation of the shader so that the builder's state is
    // const and is safe for multi-threaded access.
    fShaderBuilderLock.acquire();
    fShaderBuilder.child(fChildShaderName.c_str()) = inputShader;
    sk_sp<SkShader>   shader = fShaderBuilder.makeShader(nullptr, false);
    // Remove the shader from the builder to avoid unnecessarily prolonging the shader's lifetime
    fShaderBuilder.child(fChildShaderName.c_str()) = nullptr;
    fShaderBuilderLock.release();

    SkASSERT(shader.get());

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

sk_sp<SkImageFilter> SkImageFilters::RuntimeShader(const SkRuntimeShaderBuilder& builder,
                                                   const char* childShaderName,
                                                   sk_sp<SkImageFilter> input) {
    // if no childShaderName is provided check to see if we can implicitly assign it to the only
    // child in the effect
    if (childShaderName == nullptr) {
        auto children = builder.effect()->children();
        if (children.size() != 1) {
            return nullptr;
        }
        childShaderName = children.front().name.c_str();
    } else if (builder.effect()->findChild(childShaderName) == nullptr) {
        // there was no child declared in the runtime effect that matches the provided name
        return nullptr;
    }

    return sk_sp<SkImageFilter>(
            new SkRuntimeImageFilter(builder, childShaderName, std::move(input)));
}

#endif  // SK_ENABLE_SKSL
