/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/Composition.h"

#include "include/core/SkCanvas.h"
#include "modules/skottie/src/SkottieAdapter.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/sksg/include/SkSGGroup.h"
#include "modules/sksg/include/SkSGTransform.h"

#include <algorithm>

namespace skottie {
namespace internal {

sk_sp<sksg::RenderNode> AnimationBuilder::attachNestedAnimation(const char* name) const {
    class SkottieSGAdapter final : public sksg::RenderNode {
    public:
        explicit SkottieSGAdapter(sk_sp<Animation> animation)
            : fAnimation(std::move(animation)) {
            SkASSERT(fAnimation);
        }

    protected:
        SkRect onRevalidate(sksg::InvalidationController*, const SkMatrix&) override {
            return SkRect::MakeSize(fAnimation->size());
        }

        const RenderNode* onNodeAt(const SkPoint&) const override { return nullptr; }

        void onRender(SkCanvas* canvas, const RenderContext* ctx) const override {
            const auto local_scope =
                ScopedRenderContext(canvas, ctx).setIsolation(this->bounds(),
                                                              canvas->getTotalMatrix(),
                                                              true);
            fAnimation->render(canvas);
        }

    private:
        const sk_sp<Animation> fAnimation;
    };

    class SkottieAnimatorAdapter final : public sksg::Animator {
    public:
        SkottieAnimatorAdapter(sk_sp<Animation> animation, float time_scale)
            : fAnimation(std::move(animation))
            , fTimeScale(time_scale) {
            SkASSERT(fAnimation);
        }

    protected:
        void onTick(float t) {
            // TODO: we prolly need more sophisticated timeline mapping for nested animations.
            fAnimation->seek(t * fTimeScale);
        }

    private:
        const sk_sp<Animation> fAnimation;
        const float            fTimeScale;
    };

    const auto data = fResourceProvider->load("", name);
    if (!data) {
        this->log(Logger::Level::kError, nullptr, "Could not load: %s.", name);
        return nullptr;
    }

    auto animation = Animation::Builder()
            .setResourceProvider(fResourceProvider)
            .setFontManager(fLazyFontMgr.getMaybeNull())
            .make(static_cast<const char*>(data->data()), data->size());
    if (!animation) {
        this->log(Logger::Level::kError, nullptr, "Could not parse nested animation: %s.", name);
        return nullptr;
    }

    fCurrentAnimatorScope->push_back(
            sk_make_sp<SkottieAnimatorAdapter>(animation, animation->duration() / fDuration));

    return sk_make_sp<SkottieSGAdapter>(std::move(animation));
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachAssetRef(
    const skjson::ObjectValue& jlayer,
    const std::function<sk_sp<sksg::RenderNode>(const skjson::ObjectValue&)>& func) const {

    const auto refId = ParseDefault<SkString>(jlayer["refId"], SkString());
    if (refId.isEmpty()) {
        this->log(Logger::Level::kError, nullptr, "Layer missing refId.");
        return nullptr;
    }

    if (refId.startsWith("$")) {
        return this->attachNestedAnimation(refId.c_str() + 1);
    }

    const auto* asset_info = fAssets.find(refId);
    if (!asset_info) {
        this->log(Logger::Level::kError, nullptr, "Asset not found: '%s'.", refId.c_str());
        return nullptr;
    }

    if (asset_info->fIsAttaching) {
        this->log(Logger::Level::kError, nullptr,
                  "Asset cycle detected for: '%s'", refId.c_str());
        return nullptr;
    }

    asset_info->fIsAttaching = true;
    auto asset = func(*asset_info->fAsset);
    asset_info->fIsAttaching = false;

    return asset;
}

CompositionBuilder::CompositionBuilder(const AnimationBuilder& abuilder,
                                       const SkSize& size,
                                       const skjson::ObjectValue& jcomp)
    : fSize(size) {

    // Optional motion blur params.
    if (const skjson::ObjectValue* jmb = jcomp["mb"]) {
        static constexpr size_t kMaxSamplesPerFrame = 64;
        fMotionBlurSamples = std::min(ParseDefault<size_t>((*jmb)["spf"], 1ul),
                                      kMaxSamplesPerFrame);
        fMotionBlurAngle = SkTPin(ParseDefault((*jmb)["sa"], 0.0f),    0.0f, 720.0f);
        fMotionBlurPhase = SkTPin(ParseDefault((*jmb)["sp"], 0.0f), -360.0f, 360.0f);
    }

    int camera_builder_index = -1;

    // Prepare layer builders.
    if (const skjson::ArrayValue* jlayers = jcomp["layers"]) {
        fLayerBuilders.reserve(SkToInt(jlayers->size()));
        for (const skjson::ObjectValue* jlayer : *jlayers) {
            if (!jlayer) continue;

            const auto  lbuilder_index = fLayerBuilders.size();
            const auto& lbuilder       = fLayerBuilders.emplace_back(*jlayer);

            fLayerIndexMap.set(lbuilder.index(), lbuilder_index);

            // Keep track of the camera builder.
            if (lbuilder.isCamera()) {
                // We only support one (first) camera for now.
                if (camera_builder_index < 0) {
                    camera_builder_index = SkToInt(lbuilder_index);
                } else {
                    abuilder.log(Logger::Level::kWarning, jlayer,
                                 "Ignoring duplicate camera layer.");
                }
            }
        }
    }

    // Attach a camera transform upfront, if needed (required to build
    // all other 3D transform chains).
    if (camera_builder_index >= 0) {
        // Explicit camera.
        fCameraTransform = fLayerBuilders[camera_builder_index].buildTransform(abuilder, this);
    } else if (ParseDefault<int>(jcomp["ddd"], 0)) {
        // Default/implicit camera when 3D layers are present.
        fCameraTransform = CameraAdapter::MakeDefault(fSize)->refTransform();
    }
}

CompositionBuilder::~CompositionBuilder() = default;

LayerBuilder* CompositionBuilder::layerBuilder(int layer_index) {
    if (layer_index < 0) {
        return nullptr;
    }

    if (const auto* idx = fLayerIndexMap.find(layer_index)) {
        return &fLayerBuilders[SkToInt(*idx)];
    }

    return nullptr;
}

sk_sp<sksg::RenderNode> CompositionBuilder::build(const AnimationBuilder& abuilder) {
    // First pass - transitively attach layer transform chains.
    for (auto& lbuilder : fLayerBuilders) {
        lbuilder.buildTransform(abuilder, this);
    }

    // Second pass - attach actual layer contents and finalize the layer render tree.
    std::vector<sk_sp<sksg::RenderNode>> layers;
    layers.reserve(fLayerBuilders.size());

    LayerBuilder* prev_layer = nullptr;
    for (auto& lbuilder : fLayerBuilders) {
        if (auto layer = lbuilder.buildRenderTree(abuilder, this, prev_layer)) {
            layers.push_back(std::move(layer));
        }
        prev_layer = &lbuilder;
    }

    if (layers.empty()) {
        return nullptr;
    }

    if (layers.size() == 1) {
        return std::move(layers[0]);
    }

    // Layers are painted in bottom->top order.
    std::reverse(layers.begin(), layers.end());
    layers.shrink_to_fit();

    return sksg::Group::Make(std::move(layers));
}

} // namespace internal
} // namespace skottie
