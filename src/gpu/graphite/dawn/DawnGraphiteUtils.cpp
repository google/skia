/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/dawn/DawnUtils.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtilsPriv.h"

#include "include/gpu/ShaderErrorHandler.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/dawn/DawnBackendContext.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/dawn/DawnQueueManager.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"

namespace skgpu::graphite {

namespace ContextFactory {
std::unique_ptr<Context> MakeDawn(const DawnBackendContext& backendContext,
                                  const ContextOptions& options) {
    sk_sp<SharedContext> sharedContext = DawnSharedContext::Make(backendContext, options);
    if (!sharedContext) {
        return nullptr;
    }

    auto queueManager =
            std::make_unique<DawnQueueManager>(backendContext.fQueue, sharedContext.get());
    if (!queueManager) {
        return nullptr;
    }

    auto context = ContextCtorAccessor::MakeContext(std::move(sharedContext),
                                                    std::move(queueManager),
                                                    options);
    SkASSERT(context);
    return context;
}
} // namespace ContextFactory

bool DawnFormatIsDepthOrStencil(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::Stencil8: // fallthrough
        case wgpu::TextureFormat::Depth32Float:
        case wgpu::TextureFormat::Depth24PlusStencil8:
        case wgpu::TextureFormat::Depth32FloatStencil8:
            return true;
        default:
            return false;
    }
}

bool DawnFormatIsDepth(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::Depth32Float:
        case wgpu::TextureFormat::Depth24PlusStencil8:
        case wgpu::TextureFormat::Depth32FloatStencil8:
            return true;
        default:
            return false;
    }
}

bool DawnFormatIsStencil(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::Stencil8: // fallthrough
        case wgpu::TextureFormat::Depth24PlusStencil8:
        case wgpu::TextureFormat::Depth32FloatStencil8:
            return true;
        default:
            return false;
    }
}

wgpu::TextureFormat DawnDepthStencilFlagsToFormat(SkEnumBitMask<DepthStencilFlags> mask) {
    // TODO: Decide if we want to change this to always return a combined depth and stencil format
    // to allow more sharing of depth stencil allocations.
    if (mask == DepthStencilFlags::kDepth) {
        // wgpu::TextureFormatDepth16Unorm is also a universally supported option here
        return wgpu::TextureFormat::Depth32Float;
    } else if (mask == DepthStencilFlags::kStencil) {
        return wgpu::TextureFormat::Stencil8;
    } else if (mask == DepthStencilFlags::kDepthStencil) {
        return wgpu::TextureFormat::Depth24PlusStencil8;
    }
    SkASSERT(false);
    return wgpu::TextureFormat::Undefined;
}

wgpu::ShaderModule DawnCompileSPIRVShaderModule(const DawnSharedContext* sharedContext,
                                                std::string_view spirv,
                                                ShaderErrorHandler*) {
    wgpu::ShaderModuleSPIRVDescriptor spirvDesc;
    spirvDesc.codeSize = spirv.size() / 4;
    spirvDesc.code = reinterpret_cast<const uint32_t*>(spirv.data());

    // Skia often generates shaders that select a texture/sampler conditionally based on an
    // attribute (specifically in the case of texture atlas indexing). We disable derivative
    // uniformity warnings as we expect Skia's behavior to result in well-defined values.
    wgpu::DawnShaderModuleSPIRVOptionsDescriptor dawnSpirvOptions;
    dawnSpirvOptions.allowNonUniformDerivatives = true;

    wgpu::ShaderModuleDescriptor desc;
    desc.nextInChain = &spirvDesc;
    spirvDesc.nextInChain = &dawnSpirvOptions;

    return sharedContext->device().CreateShaderModule(&desc);
}

wgpu::ShaderModule DawnCompileWGSLShaderModule(const DawnSharedContext* sharedContext,
                                               const std::string& wgsl,
                                               ShaderErrorHandler*) {
    wgpu::ShaderModuleWGSLDescriptor wgslDesc;
    wgslDesc.code = wgsl.c_str();

    wgpu::ShaderModuleDescriptor desc;
    desc.nextInChain = &wgslDesc;

    return sharedContext->device().CreateShaderModule(&desc);
}

} // namespace skgpu::graphite
