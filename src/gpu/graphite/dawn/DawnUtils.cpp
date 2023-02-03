/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/dawn/DawnUtils.h"
#include "src/gpu/graphite/dawn/DawnUtilsPriv.h"

#include "include/gpu/ShaderErrorHandler.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/dawn/DawnBackendContext.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/dawn/DawnQueueManager.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/utils/SkShaderUtils.h"

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
        case wgpu::TextureFormat::Depth32FloatStencil8:
            return true;
        default:
            return false;
    }
}

bool DawnFormatIsDepth(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::Depth32Float:
        case wgpu::TextureFormat::Depth32FloatStencil8:
            return true;
        default:
            return false;
    }
}

bool DawnFormatIsStencil(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::Stencil8: // fallthrough
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
        // wgpu::TextureFormatDepth24Unorm_Stencil8 is supported on Mac family GPUs.
        return wgpu::TextureFormat::Depth32FloatStencil8;
    }
    SkASSERT(false);
    return wgpu::TextureFormat::Undefined;
}

// Print the source code for all shaders generated.
#ifdef SK_PRINT_SKSL_SHADERS
static constexpr bool gPrintSKSL  = true;
#else
static constexpr bool gPrintSKSL  = false;
#endif
bool SkSLToSPIRV(SkSL::Compiler* compiler,
                 const std::string& sksl,
                 SkSL::ProgramKind programKind,
                 const SkSL::ProgramSettings& settings,
                 std::string* spirv,
                 SkSL::Program::Inputs* outInputs,
                 ShaderErrorHandler* errorHandler) {
#ifdef SK_DEBUG
    std::string src = SkShaderUtils::PrettyPrint(sksl);
#else
    const std::string& src = sksl;
#endif
    std::unique_ptr<SkSL::Program> program = compiler->convertProgram(programKind,
                                                                      src,
                                                                      settings);
    if (!program || !compiler->toSPIRV(*program, spirv)) {
        errorHandler->compileError(src.c_str(), compiler->errorText().c_str());
        return false;
    }

    if (gPrintSKSL) {
        SkShaderUtils::PrintShaderBanner(programKind);
        SkDebugf("SKSL:\n");
        SkShaderUtils::PrintLineByLine(SkShaderUtils::PrettyPrint(sksl));
    }

    *outInputs = program->fInputs;
    return true;
}

wgpu::ShaderModule DawnCompileSPIRVShaderModule(const DawnSharedContext* sharedContext,
                                                const std::string& spirv,
                                                ShaderErrorHandler* errorHandler) {
    wgpu::ShaderModuleSPIRVDescriptor spirvDesc;
    spirvDesc.codeSize = spirv.size() / 4;
    spirvDesc.code = reinterpret_cast<const uint32_t*>(spirv.c_str());

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

} // namespace skgpu::graphite
