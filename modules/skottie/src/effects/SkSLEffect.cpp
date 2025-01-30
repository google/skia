/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkSpan_impl.h"
#include "modules/jsonreader/SkJSONReader.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/src/Adapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/animator/Animator.h"
#include "modules/skottie/src/effects/Effects.h"
#include "modules/skresources/include/SkResources.h"
#include "modules/sksg/include/SkSGColorFilter.h"
#include "modules/sksg/include/SkSGNode.h"
#include "modules/sksg/include/SkSGRenderNode.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

struct SkPoint;

namespace sksg {
class InvalidationController;
}

namespace skottie::internal {

// https://g-issues.chromium.org/issues/40064011
#if defined(SK_ENABLE_SKOTTIE_SKSLEFFECT)

class SkSLShaderNode final : public sksg::CustomRenderNode {
public:
    explicit SkSLShaderNode(sk_sp<RenderNode> child, const SkSize& content_size)
        : INHERITED({std::move(child)})
        , fContentSize(content_size) {}

    sk_sp<SkShader> contentShader() {
        if (!fContentShader || this->hasChildrenInval()) {
            const auto& child = this->children()[0];
            child->revalidate(nullptr, SkMatrix::I());

            SkPictureRecorder recorder;
            child->render(recorder.beginRecording(SkRect::MakeSize(fContentSize)));

            fContentShader = recorder.finishRecordingAsPicture()
                    ->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, SkFilterMode::kLinear,
                                 nullptr, nullptr);
        }

        return fContentShader;
    }

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
    sk_sp<SkShader> fContentShader;
    const SkSize fContentSize;

    using INHERITED = sksg::CustomRenderNode;
};

class SkSLEffectBase {
public:
    SkSLEffectBase(const skjson::ArrayValue& jprops,
                   const AnimationBuilder& abuilder)
    {
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
    }
protected:
    enum : size_t {
        kSkSL_index = 0,
        kFirstUniform_index = 1,
    };

    enum : size_t {
        kSkSLProp_uniform = 0,   // Maps to the integer value 1
        kSkSLProp_image = 98, // Maps to the integer value 98
        kSkSLProp_layer = 99  // Maps to the integer value 99
    };

    struct ChildData {
        int type;
        SkString name;
        SkRuntimeEffect::ChildPtr child;
    };

    void bindUniforms(const skjson::ArrayValue& jprops,
                      const AnimationBuilder& abuilder,
                      AnimatablePropertyContainer * const &container) {
        // construct dynamic uniform list from jprops, skip SkSL property
        for (size_t i = kFirstUniform_index; i < jprops.size(); i++) {
            const skjson::ObjectValue* jprop = jprops[i];
            if (!jprop) { continue; }
            const skjson::StringValue* uniformName = (*jprop)["nm"];
            if (!uniformName) { continue; }
            int type = ParseDefault<int>((*jprop)["ty"], kSkSLProp_uniform);
            if (type == kSkSLProp_uniform) {
                auto uniformTuple = std::make_tuple(SkString(uniformName->begin(),
                                                            uniformName->size()),
                                                    std::make_unique<VectorValue>());
                fUniforms.push_back(std::move(uniformTuple));
                container->bind(abuilder, (*jprop)["v"], std::get<1>(fUniforms.back()).get());
            } else if (type == kSkSLProp_image) {
                const skjson::ObjectValue* jimageRef = (*jprop)["v"];
                if (!jimageRef) {
                    continue;
                }

                const AnimationBuilder::ScopedAssetRef footageAsset(&abuilder, *jimageRef);
                const auto* asset_info = abuilder.loadFootageAsset(*footageAsset);
                if (asset_info && asset_info->fAsset) {
                    // TODO: instead of resolving shaders here, save a collection of footage assets
                    // onSync, grab the correct frameData and create a shader then
                    auto frameData = asset_info->fAsset->getFrameData(0);
                    SkSamplingOptions sampling(SkFilterMode::kLinear);
                    fChildren.push_back({type, SkString(uniformName->begin(), uniformName->size()),
                                            frameData.image->makeShader(sampling)});
                } else {
                    SkDebugf("cannot find asset for custom shader effect");
                }
            } else if (type == kSkSLProp_layer) { /* layer content */
                fChildren.push_back({type, SkString(uniformName->begin(), uniformName->size()),
                    SkRuntimeEffect::ChildPtr()});
            }
        }
    }

    sk_sp<SkData> buildUniformData() const {
        auto uniformData = SkData::MakeZeroInitialized(fEffect->uniformSize());
        SkASSERT(uniformData);
        for (const auto& uniform : fUniforms) {
            const auto& name = std::get<0>(uniform);
            const auto& data = std::get<1>(uniform);
            auto metadata = fEffect->findUniform(name.c_str());
            if (metadata && metadata->count == static_cast<int>(data->size())) {
                auto dst = reinterpret_cast<uint8_t*>(uniformData->writable_data())
                    + metadata->offset;
                memcpy(reinterpret_cast<void*>(dst), data->data(), data->size() * sizeof(float));
            } else {
                SkDebugf("cannot set malformed uniform: %s\n", name.c_str());
            }
        }
        return uniformData;
    }

    std::vector<SkRuntimeEffect::ChildPtr> buildChildrenData(sk_sp<SkSLShaderNode> node) const {
        std::vector<SkRuntimeEffect::ChildPtr> childrenData(fEffect->children().size());
        for (const auto& childData : fChildren) {
            auto metadata = fEffect->findChild(childData.name.c_str());
            if (childData.type == kSkSLProp_layer) {
                childrenData[metadata->index] = (node->contentShader());
            } else if (childData.type == kSkSLProp_image) {
                childrenData[metadata->index] = childData.child;
            }
        }
        return childrenData;
    }
    sk_sp<SkRuntimeEffect> fEffect;
    std::vector<std::tuple<SkString, std::unique_ptr<VectorValue>>> fUniforms;
    std::vector<ChildData> fChildren;
};

class SkSLShaderAdapter final : public DiscardableAdapterBase<SkSLShaderAdapter,
                                                              SkSLShaderNode>,
                                public SkSLEffectBase {
public:
    SkSLShaderAdapter(const skjson::ArrayValue& jprops,
                      const AnimationBuilder& abuilder,
                      sk_sp<SkSLShaderNode> node)
        : DiscardableAdapterBase<SkSLShaderAdapter, SkSLShaderNode>(std::move(node))
        , SkSLEffectBase(jprops, abuilder)
    {
        this->bindUniforms(jprops, abuilder, this);
    }

private:
    void onSync() override {
        if (!fEffect) {
            return;
        }
        sk_sp<SkShader> shader =
                fEffect->makeShader(buildUniformData(), SkSpan(buildChildrenData(this->node())));
        this->node()->setShader(std::move(shader));
    }
};

class SkSLColorFilterAdapter final : public DiscardableAdapterBase<SkSLColorFilterAdapter,
                                                             sksg::ExternalColorFilter>
                                   , public SkSLEffectBase {
public:
    SkSLColorFilterAdapter(const skjson::ArrayValue& jprops,
                      const AnimationBuilder& abuilder,
                      sk_sp<sksg::ExternalColorFilter> node)
        : DiscardableAdapterBase<SkSLColorFilterAdapter, sksg::ExternalColorFilter>(std::move(node))
        , SkSLEffectBase(jprops, abuilder)
    {
        this->bindUniforms(jprops, abuilder, this);
    }

private:
    void onSync() override {
        if (!fEffect) {
            return;
        }
        auto cf = fEffect->makeColorFilter(buildUniformData());
        this->node()->setColorFilter(std::move(cf));
    }
};

#endif  // SK_ENABLE_SKOTTIE_SKSLEFFECT

sk_sp<sksg::RenderNode> EffectBuilder::attachSkSLShader(const skjson::ArrayValue& jprops,
                                                        sk_sp<sksg::RenderNode> layer) const {
#if defined(SK_ENABLE_SKOTTIE_SKSLEFFECT)
    auto shaderNode = sk_make_sp<SkSLShaderNode>(std::move(layer), fLayerSize);
    return fBuilder->attachDiscardableAdapter<SkSLShaderAdapter>(jprops, *fBuilder,
                                                                 std::move(shaderNode));
#else
    return layer;
#endif
}

sk_sp<sksg::RenderNode> EffectBuilder::attachSkSLColorFilter(const skjson::ArrayValue& jprops,
                                                             sk_sp<sksg::RenderNode> layer) const {
#if defined(SK_ENABLE_SKOTTIE_SKSLEFFECT)
    auto cfNode = sksg::ExternalColorFilter::Make(std::move(layer));
    return fBuilder->attachDiscardableAdapter<SkSLColorFilterAdapter>(jprops, *fBuilder,
                                                                      std::move(cfNode));
#else
    return layer;
#endif
}

} // namespace skottie::internal
