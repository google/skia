/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlResourceProvider.h"

#include "experimental/graphite/include/BackendTexture.h"
#include "experimental/graphite/src/GraphicsPipelineDesc.h"
#include "experimental/graphite/src/mtl/MtlBuffer.h"
#include "experimental/graphite/src/mtl/MtlCommandBuffer.h"
#include "experimental/graphite/src/mtl/MtlGpu.h"
#include "experimental/graphite/src/mtl/MtlGraphicsPipeline.h"
#include "experimental/graphite/src/mtl/MtlTexture.h"

#import <Metal/Metal.h>

namespace skgpu::mtl {

ResourceProvider::ResourceProvider(const skgpu::Gpu* gpu)
    : skgpu::ResourceProvider(gpu) {
}

const Gpu* ResourceProvider::mtlGpu() {
    return static_cast<const Gpu*>(fGpu);
}

sk_sp<skgpu::CommandBuffer> ResourceProvider::createCommandBuffer() {
    return CommandBuffer::Make(this->mtlGpu());
}

sk_sp<skgpu::GraphicsPipeline> ResourceProvider::onCreateGraphicsPipeline(
        const GraphicsPipelineDesc& desc) {
    return GraphicsPipeline::Make(this->mtlGpu(), desc);
}

sk_sp<skgpu::Texture> ResourceProvider::createTexture(SkISize dimensions,
                                                      const skgpu::TextureInfo& info) {
    return Texture::Make(this->mtlGpu(), dimensions, info);
}

sk_sp<skgpu::Texture> ResourceProvider::createWrappedTexture(const BackendTexture& texture) {
    mtl::Handle mtlHandleTexture = texture.getMtlTexture();
    if (!mtlHandleTexture) {
        return nullptr;
    }
    sk_cfp<id<MTLTexture>> mtlTexture = sk_ret_cfp((id<MTLTexture>)mtlHandleTexture);
    return Texture::MakeWrapped(this->mtlGpu(),
                                texture.dimensions(),
                                texture.info(),
                                std::move(mtlTexture));
}

sk_sp<skgpu::Buffer> ResourceProvider::createBuffer(size_t size,
                                                    BufferType type,
                                                    PrioritizeGpuReads prioritizeGpuReads) {
    return Buffer::Make(this->mtlGpu(), size, type, prioritizeGpuReads);
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

id<MTLDepthStencilState> ResourceProvider::findOrCreateCompatibleDepthStencilState(
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
                [this->mtlGpu()->device() newDepthStencilStateWithDescriptor: desc]);
        depthStencilState = fDepthStencilStates.set(depthStencilSettings, std::move(dss));
    }

    SkASSERT(depthStencilState);
    return depthStencilState->get();
}

} // namespace skgpu::mtl
