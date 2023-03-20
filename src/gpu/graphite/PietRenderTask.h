/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_PietRenderTask_DEFINED
#define skgpu_graphite_PietRenderTask_DEFINED

#include <vector>

#include "src/gpu/graphite/Task.h"
#include "src/gpu/piet/Scene.h"

namespace skgpu::graphite {

class TextureProxy;

class PietRenderInstance final {
public:
    PietRenderInstance(sk_sp<const skgpu::piet::Scene> scene, sk_sp<TextureProxy> targetProxy)
            : fScene(std::move(scene))
            , fTargetProxy(std::move(targetProxy)) {}

    // Make this type move-only.
    PietRenderInstance(const PietRenderInstance&) = delete;
    PietRenderInstance(PietRenderInstance&&) = default;

    bool prepareResources(ResourceProvider*);

    bool addCommands(CommandBuffer*);

private:
    sk_sp<const skgpu::piet::Scene> fScene;
    sk_sp<TextureProxy> fTargetProxy;
};

class PietRenderTask final : public Task {
public:
    explicit PietRenderTask(std::vector<PietRenderInstance> instances)
            : fInstances(std::move(instances)) {}

    ~PietRenderTask() override = default;

    bool prepareResources(ResourceProvider*, const RuntimeEffectDictionary*) override;

    bool addCommands(Context*, CommandBuffer*, ReplayTargetData) override;

private:
    std::vector<PietRenderInstance> fInstances;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_PietRenderTask_DEFINED
