/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/effects/imagefilters/SkRuntimeImageFilter.h"

#ifdef SK_ENABLE_SKSL

#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkShader.h"
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkSpinlock.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkWriteBuffer.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

class SkRuntimeImageFilter final : public SkImageFilter_Base {
public:
    SkRuntimeImageFilter(sk_sp<SkRuntimeEffect> effect,
                         sk_sp<SkData> uniforms,
                         sk_sp<SkImageFilter> input)
            : INHERITED(&input, 1, /*cropRect=*/nullptr)
            , fShaderBuilder(std::move(effect), std::move(uniforms)) {
        std::string_view childName = fShaderBuilder.effect()->children().front().name;
        fChildShaderNames.push_back(SkString(childName));
    }
    SkRuntimeImageFilter(const SkRuntimeShaderBuilder& builder,
                         std::string_view childShaderNames[],
                         const sk_sp<SkImageFilter> inputs[],
                         int inputCount)
            : INHERITED(inputs, inputCount, /*cropRect=*/nullptr)
            , fShaderBuilder(builder) {
        fChildShaderNames.reserve_back(inputCount);
        for (int i = 0; i < inputCount; i++) {
            fChildShaderNames.push_back(SkString(childShaderNames[i]));
        }
    }

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
    SkSTArray<1, SkString> fChildShaderNames;

    using INHERITED = SkImageFilter_Base;
};

sk_sp<SkImageFilter> SkMakeRuntimeImageFilter(sk_sp<SkRuntimeEffect> effect,
                                              sk_sp<SkData> uniforms,
                                              sk_sp<SkImageFilter> input) {
    // Rather than replicate all of the checks from makeShader here, just try to create a shader
    // once, to determine if everything is valid.
    sk_sp<SkShader> child = nullptr;
    auto shader = effect->makeShader(uniforms, &child, 1);
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
    // We don't know how many inputs to expect yet. Passing -1 allows any number of children.
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, -1);
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

    // Read the child shader names
    SkSTArray<4, std::string_view> childShaderNames;
    SkSTArray<4, SkString> childShaderNameStrings;
    childShaderNames.resize(common.inputCount());
    childShaderNameStrings.resize(common.inputCount());
    for (int i = 0; i < common.inputCount(); i++) {
        buffer.readString(&childShaderNameStrings[i]);
        childShaderNames[i] = childShaderNameStrings[i].c_str();
    }

    SkRuntimeShaderBuilder builder(std::move(effect), std::move(uniforms));

    // Populate the builder with the corresponding children
    for (const SkRuntimeEffect::Child& child : builder.effect()->children()) {
        std::string_view name = child.name;
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

    return SkImageFilters::RuntimeShader(builder, childShaderNames.data(),
                                         common.inputs(), common.inputCount());
}

void SkRuntimeImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    fShaderBuilderLock.acquire();
    buffer.writeString(fShaderBuilder.effect()->source().c_str());
    buffer.writeDataAsByteArray(fShaderBuilder.uniforms().get());
    for (const SkString& name : fChildShaderNames) {
        buffer.writeString(name.c_str());
    }
    for (size_t x = 0; x < fShaderBuilder.children().size(); x++) {
        buffer.writeFlattenable(fShaderBuilder.children()[x].flattenable());
    }
    fShaderBuilderLock.release();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkSpecialImage> SkRuntimeImageFilter::onFilterImage(const Context& ctx,
                                                          SkIPoint* offset) const {
    SkIRect outputBounds = SkIRect(ctx.desiredOutput());
    sk_sp<SkSpecialSurface> surf(ctx.makeSurface(outputBounds.size()));
    if (!surf) {
        return nullptr;
    }

    SkMatrix ctm = ctx.ctm();
    SkMatrix inverse;
    SkAssertResult(ctm.invert(&inverse));

    const int inputCount = this->countInputs();
    SkASSERT(inputCount == fChildShaderNames.size());

    SkSTArray<1, sk_sp<SkShader>> inputShaders;
    for (int i = 0; i < inputCount; i++) {
        SkIPoint inputOffset = SkIPoint::Make(0, 0);
        sk_sp<SkSpecialImage> input(this->filterInput(i, ctx, &inputOffset));
        if (!input) {
            return nullptr;
        }

        SkMatrix localM = inverse * SkMatrix::Translate(inputOffset);
        sk_sp<SkShader> inputShader =
                input->asShader(SkSamplingOptions(SkFilterMode::kLinear), localM);
        SkASSERT(inputShader);
        inputShaders.push_back(std::move(inputShader));
    }

    // lock the mutation of the builder and creation of the shader so that the builder's state is
    // const and is safe for multi-threaded access.
    fShaderBuilderLock.acquire();
    for (int i = 0; i < inputCount; i++) {
        fShaderBuilder.child(fChildShaderNames[i].c_str()) = inputShaders[i];
    }
    sk_sp<SkShader> shader = fShaderBuilder.makeShader();
    // Remove the inputs from the builder to avoid unnecessarily prolonging the shader's lifetime
    for (int i = 0; i < inputCount; i++) {
        fShaderBuilder.child(fChildShaderNames[i].c_str()) = nullptr;
    }
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

static bool child_is_shader(const SkRuntimeEffect::Child* child) {
    return child && child->type == SkRuntimeEffect::ChildType::kShader;
}

sk_sp<SkImageFilter> SkImageFilters::RuntimeShader(const SkRuntimeShaderBuilder& builder,
                                                   std::string_view childShaderName,
                                                   sk_sp<SkImageFilter> input) {
    // If no childShaderName is provided, check to see if we can implicitly assign it to the only
    // child in the effect.
    if (childShaderName.empty()) {
        auto children = builder.effect()->children();
        if (children.size() != 1) {
            return nullptr;
        }
        childShaderName = children.front().name;
    }

    return SkImageFilters::RuntimeShader(builder, &childShaderName, &input, 1);
}

sk_sp<SkImageFilter> SkImageFilters::RuntimeShader(const SkRuntimeShaderBuilder& builder,
                                                   std::string_view childShaderNames[],
                                                   const sk_sp<SkImageFilter> inputs[],
                                                   int inputCount) {
    for (int i = 0; i < inputCount; i++) {
        std::string_view name = childShaderNames[i];
        // All names must be non-empty, and present as a child shader in the effect:
        if (name.empty() || !child_is_shader(builder.effect()->findChild(name))) {
            return nullptr;
        }

        // We don't allow duplicates, either:
        for (int j = 0; j < i; j++) {
            if (name == childShaderNames[j]) {
                return nullptr;
            }
        }
    }

    return sk_sp<SkImageFilter>(new SkRuntimeImageFilter(builder, childShaderNames,
                                                         inputs, inputCount));
}

#endif  // SK_ENABLE_SKSL
