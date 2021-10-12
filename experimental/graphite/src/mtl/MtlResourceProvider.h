/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlResourceProvider_DEFINED
#define skgpu_MtlResourceProvider_DEFINED

#include "experimental/graphite/src/ResourceProvider.h"

#import <Metal/Metal.h>

namespace skgpu {
class CommandBuffer;
}

namespace skgpu::mtl {

class Gpu;

class ResourceProvider final : public skgpu::ResourceProvider {
public:
    ResourceProvider(const skgpu::Gpu* gpu);
    ~ResourceProvider() override {}

private:
    const Gpu* mtlGpu();

    sk_sp<skgpu::CommandBuffer> createCommandBuffer() override;
    std::unique_ptr<skgpu::RenderPipeline> onCreateRenderPipeline(
            const RenderPipelineDesc&) override;
    sk_sp<skgpu::Texture> createTexture(SkISize, const skgpu::TextureInfo&) override;
};

} // namespace skgpu::mtl

#endif // skgpu_MtlResourceProvider_DEFINED
