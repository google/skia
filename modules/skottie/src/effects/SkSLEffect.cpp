/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/effects/Effects.h"

#include "include/effects/SkRuntimeEffect.h"
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
        this->node()->setShader(buildEffectShader());
    }

    sk_sp<SkShader> buildEffectShader() const {
        if (!fEffect) {
            return nullptr;
        }
        // TODO: consider dumping builder and work with lower level API
        SkRuntimeShaderBuilder builder = SkRuntimeShaderBuilder(fEffect);
        for (const auto& uniform : fUniforms) {
            const auto& name = std::get<0>(uniform);
            const auto& data = std::get<1>(uniform);
            auto metadata = fEffect->findUniform(name.c_str());
            // TODO: build SkData from SkRuntimeEffect::Uniform data
            switch (metadata->type) {
                case SkRuntimeEffect::Uniform::Type::kFloat:
                    builder.uniform(name.c_str()) = data->at(0); break;
                default:
                    printf("!!! %s\n", "uniform data type not supported");
            }
        }
        return builder.makeShader(&SkMatrix::I(), false);
    }
    sk_sp<SkRuntimeEffect> fEffect;
    std::vector<std::tuple<SkString, std::unique_ptr<VectorValue>>> fUniforms;
    using INHERITED = DiscardableAdapterBase<SkSLEffectAdapter, SkSLShaderNode>;
};

} // namespace

#endif  // SK_ENABLE_SKSL

sk_sp<sksg::RenderNode> EffectBuilder::attachSkSLEffect(const skjson::ArrayValue& jprops,
                                                             sk_sp<sksg::RenderNode> layer) const {
#ifdef SK_ENABLE_SKSL
    auto shaderNode = sk_make_sp<SkSLShaderNode>(std::move(layer));
    return fBuilder->attachDiscardableAdapter<SkSLEffectAdapter>(jprops, *fBuilder, shaderNode);
#else
    return layer;
#endif
}

} // namespace skottie::internal
