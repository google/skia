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
        pipeline  = MtlGraphicsPipeline::MakeLoadMSAAPipeline(this->mtlSharedContext(), this,
                                                              renderPassDesc);
        if (pipeline) {
            fLoadMSAAPipelines.set(renderPassKey, pipeline);
        }
    }

    return pipeline;
}

sk_sp<GraphicsPipeline> MtlResourceProvider::createGraphicsPipeline(
        const RuntimeEffectDictionary* runtimeDict,
        const UniqueKey& pipelineKey,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc,
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags,
        uint32_t compilationID) {
    return MtlGraphicsPipeline::Make(this->mtlSharedContext(), this,
                                     runtimeDict, pipelineKey, pipelineDesc, renderPassDesc,
                                     pipelineCreationFlags, compilationID);
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

namespace {
MTLCompareFunction compare_op_to_mtl(CompareOp op) {
    switch (op) {
        case CompareOp::kAlways:
            return MTLCompareFunctionAlways;
        case CompareOp::kNever:
            return MTLCompareFunctionNever;
        case CompareOp::kGreater:
            return MTLCompareFunctionGreater;
        case CompareOp::kGEqual:
            return MTLCompareFunctionGreaterEqual;
        case CompareOp::kLess:
            return MTLCompareFunctionLess;
        case CompareOp::kLEqual:
            return MTLCompareFunctionLessEqual;
        case CompareOp::kEqual:
            return MTLCompareFunctionEqual;
        case CompareOp::kNotEqual:
            return MTLCompareFunctionNotEqual;
    }
}

MTLStencilOperation stencil_op_to_mtl(StencilOp op) {
    switch (op) {
        case StencilOp::kKeep:
            return MTLStencilOperationKeep;
        case StencilOp::kZero:
            return MTLStencilOperationZero;
        case StencilOp::kReplace:
            return MTLStencilOperationReplace;
        case StencilOp::kInvert:
            return MTLStencilOperationInvert;
        case StencilOp::kIncWrap:
            return MTLStencilOperationIncrementWrap;
        case StencilOp::kDecWrap:
            return MTLStencilOperationDecrementWrap;
        case StencilOp::kIncClamp:
            return MTLStencilOperationIncrementClamp;
        case StencilOp::kDecClamp:
            return MTLStencilOperationDecrementClamp;
    }
}

MTLStencilDescriptor* stencil_face_to_mtl(DepthStencilSettings::Face face) {
    MTLStencilDescriptor* result = [[MTLStencilDescriptor alloc] init];
    result.stencilCompareFunction = compare_op_to_mtl(face.fCompareOp);
    result.readMask = face.fReadMask;
    result.writeMask = face.fWriteMask;
    result.depthStencilPassOperation = stencil_op_to_mtl(face.fDepthStencilPassOp);
    result.stencilFailureOperation = stencil_op_to_mtl(face.fStencilFailOp);
    return result;
}
}  // anonymous namespace

sk_cfp<id<MTLDepthStencilState>> MtlResourceProvider::findOrCreateCompatibleDepthStencilState(
            const DepthStencilSettings& depthStencilSettings) {
    sk_cfp<id<MTLDepthStencilState>>* depthStencilState;
    depthStencilState = fDepthStencilStates.find(depthStencilSettings);
    if (!depthStencilState) {
        MTLDepthStencilDescriptor* desc = [[MTLDepthStencilDescriptor alloc] init];
        SkASSERT(depthStencilSettings.fDepthTestEnabled ||
                 depthStencilSettings.fDepthCompareOp == CompareOp::kAlways);
        desc.depthCompareFunction = compare_op_to_mtl(depthStencilSettings.fDepthCompareOp);
        if (depthStencilSettings.fDepthTestEnabled) {
            desc.depthWriteEnabled = depthStencilSettings.fDepthWriteEnabled;
        }
        if (depthStencilSettings.fStencilTestEnabled) {
            desc.frontFaceStencil = stencil_face_to_mtl(depthStencilSettings.fFrontStencil);
            desc.backFaceStencil = stencil_face_to_mtl(depthStencilSettings.fBackStencil);
        }

        sk_cfp<id<MTLDepthStencilState>> dss(
                [this->mtlSharedContext()->device() newDepthStencilStateWithDescriptor: desc]);
        depthStencilState = fDepthStencilStates.set(depthStencilSettings, std::move(dss));
    }

    SkASSERT(depthStencilState);
    return *depthStencilState;
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

} // namespace skgpu::graphite
