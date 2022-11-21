/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnUtils.h"

#include "include/gpu/ShaderErrorHandler.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/utils/SkShaderUtils.h"

namespace skgpu::graphite {

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
#if defined(SK_DEBUG)
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

    wgpu::ShaderModuleDescriptor desc;
    desc.nextInChain = &spirvDesc;
    return sharedContext->device().CreateShaderModule(&desc);
}

} // namespace skgpu::graphite
