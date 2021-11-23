/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkMalloc.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/sksg/include/SkSGColorFilter.h"

namespace skottie::internal {

#ifdef SK_ENABLE_SKSL

namespace  {
class SkSLShaderNode final : public sksg::CustomRenderNode {
public:
    explicit SkSLShaderNode(sk_sp<RenderNode> child) : INHERITED({std::move(child)}) {}

    SG_ATTRIBUTE(Shader, sk_sp<SkShader>, fEffectShader)
private:
    SkRect onRevalidate(sksg::InvalidationController* ic, const SkMatrix& ctm) override {
        const auto& child = this->children()[0];
        return child->revalidate(ic, ctm);
    }

    void onRender(SkCanvas* canvas, const RenderContext* ctx) const override {
        const auto& bounds = this->bounds();
        const auto local_ctx = ScopedRenderContext(canvas, ctx)
                .setIsolation(bounds, canvas->getTotalMatrix(), true);

        canvas->saveLayer(&bounds, nullptr);
        this->children()[0]->render(canvas, local_ctx);

        SkPaint effect_paint;
        effect_paint.setShader(fEffectShader);
        effect_paint.setBlendMode(SkBlendMode::kSrcIn);

        canvas->drawPaint(effect_paint);
    }

    const RenderNode* onNodeAt(const SkPoint&) const override { return nullptr; } // no hit-testing

    sk_sp<SkShader> fEffectShader;

    using INHERITED = sksg::CustomRenderNode;
};

class SkSLEffectAdapter final : public DiscardableAdapterBase<SkSLEffectAdapter,
                                                             SkSLShaderNode> {
public:
    SkSLEffectAdapter(const skjson::ArrayValue& jprops,
                      const AnimationBuilder& abuilder,
                      sk_sp<SkSLShaderNode> node)
        : INHERITED(std::move(node))
    {
        enum : size_t {
            kSkSL_index = 0,
            kFirstUniform_index = 1,
        };
        if (jprops.size() < 1) {
            return;
        }
        const skjson::ObjectValue* jSkSL = jprops[kSkSL_index];
        if (!jSkSL) {
            return;
        }
        const skjson::StringValue* jShader = (*jSkSL)["sh"];
        if (!jShader) {
            return;
        }
        SkString shader = SkString(jShader->begin(), jShader->size());
        auto result = SkRuntimeEffect::MakeForShader(shader, {});
        if (!result.effect) {
            abuilder.log(Logger::Level::kError, nullptr, "Failed to parse SkSL shader: %s",
               result.errorText.c_str());
            return;
        }
        fEffect = std::move(result.effect);

        // construct dynamic uniform list from jprops, skip SkSL property
        for (size_t i = kFirstUniform_index; i < jprops.size(); i++) {
            const skjson::ObjectValue* jprop = jprops[i];
            if (!jprop) { continue; }
            const skjson::StringValue* uniformName = (*jprop)["nm"];
            if (!uniformName) { continue; }
            auto uniformTuple = std::make_tuple(SkString(uniformName->begin(),
                                                         uniformName->size()),
                                                std::make_unique<VectorValue>());
            fUniforms.push_back(std::move(uniformTuple));
            this->bind(abuilder, (*jprop)["v"], std::get<1>(fUniforms.back()).get());
        }
    }

private:
    void onSync() override {
        if (!fEffect) {
            return;
        }
        sk_sp<SkShader> shader = fEffect->makeShader(buildUniformData(), {/* TODO: child support */}, &SkMatrix::I(), false);
        this->node()->setShader(std::move(shader));
    }

    sk_sp<SkData> buildUniformData() const {
        auto uniformData = SkData::MakeUninitialized(fEffect->uniformSize());
        SkASSERT(uniformData);
        sk_bzero(uniformData->writable_data(), uniformData->size());
        for (const auto& uniform : fUniforms) {
            const auto& name = std::get<0>(uniform);
            const auto& data = std::get<1>(uniform);
            auto metadata = fEffect->findUniform(name.c_str());
            if (metadata && metadata->count == static_cast<int>(data->size())) {
                auto dst = reinterpret_cast<uint8_t*>(uniformData->writable_data()) + metadata->offset;
                memcpy(reinterpret_cast<void*>(dst), data->data(), data->size() * sizeof(float));
            } else {
                SkDebugf("cannot set malformed uniform: %s", name.c_str());
            }
        }
        return uniformData;
    }
    sk_sp<SkRuntimeEffect> fEffect;
    std::vector<std::tuple<SkString, std::unique_ptr<VectorValue>>> fUniforms;
    using INHERITED = DiscardableAdapterBase<SkSLEffectAdapter, SkSLShaderNode>;
};

class SkSLColorFilterAdapter final : public DiscardableAdapterBase<SkSLColorFilterAdapter,
                                                             sksg::ExternalColorFilter> {
public:
    SkSLColorFilterAdapter(const skjson::ArrayValue& jprops,
                      const AnimationBuilder& abuilder,
                      sk_sp<sksg::ExternalColorFilter> node)
        : INHERITED(std::move(node))
    {
        enum : size_t {
            kSkSL_index = 0,
            kFirstUniform_index = 1,
        };
        if (jprops.size() < 1) {
            return;
        }
        const skjson::ObjectValue* jSkSL = jprops[kSkSL_index];
        if (!jSkSL) {
            return;
        }
        const skjson::StringValue* jShader = (*jSkSL)["sh"];
        if (!jShader) {
            return;
        }
        SkString skSL = SkString(jShader->begin(), jShader->size());
        auto result = SkRuntimeEffect::MakeForColorFilter(skSL, {});
        if (!result.effect) {
            abuilder.log(Logger::Level::kError, nullptr, "Failed to parse SkSL color filter: %s",
               result.errorText.c_str());
            return;
        }
        fEffect = std::move(result.effect);

        // construct dynamic uniform list from jprops, skip SkSL property
        for (size_t i = kFirstUniform_index; i < jprops.size(); i++) {
            const skjson::ObjectValue* jprop = jprops[i];
            if (!jprop) { continue; }
            const skjson::StringValue* uniformName = (*jprop)["nm"];
            if (!uniformName) { continue; }
            auto uniformTuple = std::make_tuple(SkString(uniformName->begin(),
                                                         uniformName->size()),
                                                std::make_unique<VectorValue>());
            fUniforms.push_back(std::move(uniformTuple));
            this->bind(abuilder, (*jprop)["v"], std::get<1>(fUniforms.back()).get());
        }
    }

private:
    void onSync() override {
        if (!fEffect) {
            return;
        }
        auto cf = fEffect->makeColorFilter(buildUniformData());
        this->node()->setColorFilter(std::move(cf));
    }

    sk_sp<SkData> buildUniformData() const {
        auto uniformData = SkData::MakeUninitialized(fEffect->uniformSize());
        SkASSERT(uniformData);
        sk_bzero(uniformData->writable_data(), uniformData->size());
        for (const auto& uniform : fUniforms) {
            const auto& name = std::get<0>(uniform);
            const auto& data = std::get<1>(uniform);
            auto metadata = fEffect->findUniform(name.c_str());
            if (metadata && metadata->count == static_cast<int>(data->size())) {
                auto dst = reinterpret_cast<uint8_t*>(uniformData->writable_data()) + metadata->offset;
                memcpy(reinterpret_cast<void*>(dst), data->data(), data->size() * sizeof(float));
            } else {
                SkDebugf("cannot set malformed uniform: %s", name.c_str());
            }
        }
        return uniformData;
    }
    sk_sp<SkRuntimeEffect> fEffect;
    std::vector<std::tuple<SkString, std::unique_ptr<VectorValue>>> fUniforms;
    using INHERITED = DiscardableAdapterBase<SkSLColorFilterAdapter, sksg::ExternalColorFilter>;
};

} // namespace

#endif  // SK_ENABLE_SKSL

sk_sp<sksg::RenderNode> EffectBuilder::attachSkSLShader(const skjson::ArrayValue& jprops,
                                                        sk_sp<sksg::RenderNode> layer) const {
#ifdef SK_ENABLE_SKSL
    auto shaderNode = sk_make_sp<SkSLShaderNode>(std::move(layer));
    return fBuilder->attachDiscardableAdapter<SkSLEffectAdapter>(jprops, *fBuilder, std::move(shaderNode));
#else
    return layer;
#endif
}

sk_sp<sksg::RenderNode> EffectBuilder::attachSkSLColorFilter(const skjson::ArrayValue& jprops,
                                                             sk_sp<sksg::RenderNode> layer) const {
#ifdef SK_ENABLE_SKSL
    auto cfNode = sksg::ExternalColorFilter::Make(std::move(layer));
    return fBuilder->attachDiscardableAdapter<SkSLColorFilterAdapter>(jprops, *fBuilder, std::move(cfNode));
#else
    return layer;
#endif
}

} // namespace skottie::internal
