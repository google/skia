/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/mtl/MtlResourceProvider.h"

#include "include/gpu/ShaderErrorHandler.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/mtl/MtlGraphiteTypes.h"
#include "src/sksl/SkSLProgramKind.h"

#include "src/core/SkSLTypeShared.h"
#include "src/gpu/Blend.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/graphite/GlobalCache.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/mtl/MtlBuffer.h"
#include "src/gpu/graphite/mtl/MtlCommandBuffer.h"
#include "src/gpu/graphite/mtl/MtlComputePipeline.h"
#include "src/gpu/graphite/mtl/MtlGraphicsPipeline.h"
#include "src/gpu/graphite/mtl/MtlSampler.h"
#include "src/gpu/graphite/mtl/MtlSharedContext.h"
#include "src/gpu/graphite/mtl/MtlTexture.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"

#import <Metal/Metal.h>

namespace skgpu::graphite {

MtlResourceProvider::MtlResourceProvider(SharedContext* sharedContext,
                                         SingleOwner* singleOwner,
                                         uint32_t recorderID,
                                         size_t resourceBudget)
        : ResourceProvider(sharedContext, singleOwner, recorderID, resourceBudget) {}

const MtlSharedContext* MtlResourceProvider::mtlSharedContext() {
    return static_cast<const MtlSharedContext*>(fSharedContext);
}

sk_sp<MtlGraphicsPipeline> MtlResourceProvider::findOrCreateLoadMSAAPipeline(
        const RenderPassDesc& renderPassDesc) {
    uint32_t renderPassKey =
            this->mtlSharedContext()->mtlCaps().getRenderPassDescKey(renderPassDesc);
    sk_sp<MtlGraphicsPipeline> pipeline = fLoadMSAAPipelines[renderPassKey];
    if (!pipeline) {
        pipeline  = MtlGraphicsPipeline::MakeLoadMSAAPipeline(this->mtlSharedContext(),
                                                              renderPassDesc);
        if (pipeline) {
            fLoadMSAAPipelines.set(renderPassKey, pipeline);
        }
    }

    return pipeline;
}

sk_sp<ComputePipeline> MtlResourceProvider::createComputePipeline(
        const ComputePipelineDesc& pipelineDesc) {
    return MtlComputePipeline::Make(this->mtlSharedContext(), pipelineDesc);
}

sk_sp<Texture> MtlResourceProvider::createTexture(SkISize dimensions,
                                                  const TextureInfo& info) {
    return MtlTexture::Make(this->mtlSharedContext(), dimensions, info);
}

sk_sp<Texture> MtlResourceProvider::onCreateWrappedTexture(const BackendTexture& texture) {
    CFTypeRef mtlHandleTexture = BackendTextures::GetMtlTexture(texture);
    if (!mtlHandleTexture) {
        return nullptr;
    }
    sk_cfp<id<MTLTexture>> mtlTexture = sk_ret_cfp((id<MTLTexture>)mtlHandleTexture);
    return MtlTexture::MakeWrapped(this->mtlSharedContext(), texture.dimensions(), texture.info(),
                                   std::move(mtlTexture));
}

sk_sp<Buffer> MtlResourceProvider::createBuffer(size_t size,
                                                BufferType type,
                                                AccessPattern accessPattern) {
    return MtlBuffer::Make(this->mtlSharedContext(), size, type, accessPattern);
}

sk_sp<Sampler> MtlResourceProvider::createSampler(const SamplerDesc& samplerDesc) {
    return MtlSampler::Make(this->mtlSharedContext(),
                            samplerDesc.samplingOptions(),
                            samplerDesc.tileModeX(),
                            samplerDesc.tileModeY());
}

BackendTexture MtlResourceProvider::onCreateBackendTexture(SkISize dimensions,
                                                           const TextureInfo& info) {
    sk_cfp<id<MTLTexture>> texture = MtlTexture::MakeMtlTexture(this->mtlSharedContext(),
                                                                dimensions,
                                                                info);
    if (!texture) {
        return {};
    }
    return BackendTextures::MakeMetal(dimensions, (CFTypeRef)texture.release());
}

void MtlResourceProvider::onDeleteBackendTexture(const BackendTexture& texture) {
    SkASSERT(texture.backend() == BackendApi::kMetal);
    CFTypeRef texHandle = BackendTextures::GetMtlTexture(texture);
    SkCFSafeRelease(texHandle);
}

MtlThreadSafeResourceProvider::MtlThreadSafeResourceProvider(
        std::unique_ptr<ResourceProvider> resourceProvider)
    : ThreadSafeResourceProvider(std::move(resourceProvider)) {}

} // namespace skgpu::graphite
