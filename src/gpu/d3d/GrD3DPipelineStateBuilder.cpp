/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

//#include <d3dcompiler.h>

#include "src/gpu/d3d/GrD3DPipelineStateBuilder.h"

#include "include/gpu/GrContext.h"
#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GrAutoLocaleSetter.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrShaderUtils.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DRenderTarget.h"
#include "src/gpu/d3d/GrD3DRootSignature.h"
#include "src/sksl/SkSLCompiler.h"

#include <d3dcompiler.h>

typedef size_t shader_size;

sk_sp<GrD3DPipelineState> GrD3DPipelineStateBuilder::MakePipelineState(
        GrD3DGpu* gpu,
        GrRenderTarget* renderTarget,
        const GrProgramDesc& desc,
        const GrProgramInfo& programInfo) {
    // ensure that we use "." as a decimal separator when creating SkSL code
    GrAutoLocaleSetter als("C");

    // create a builder.  This will be handed off to effects so they can use it to add
    // uniforms, varyings, textures, etc
    GrD3DPipelineStateBuilder builder(gpu, renderTarget, desc, programInfo);

    if (!builder.emitAndInstallProcs()) {
        return nullptr;
    }

    return builder.finalize();
}

GrD3DPipelineStateBuilder::GrD3DPipelineStateBuilder(GrD3DGpu* gpu,
                                                     GrRenderTarget* renderTarget,
                                                     const GrProgramDesc& desc,
                                                     const GrProgramInfo& programInfo)
        : INHERITED(renderTarget, desc, programInfo)
        , fGpu(gpu)
        , fVaryingHandler(this)
        , fUniformHandler(this) {}

const GrCaps* GrD3DPipelineStateBuilder::caps() const {
    return fGpu->caps();
}

void GrD3DPipelineStateBuilder::finalizeFragmentOutputColor(GrShaderVar& outputColor) {
    outputColor.addLayoutQualifier("location = 0, index = 0");
}

void GrD3DPipelineStateBuilder::finalizeFragmentSecondaryColor(GrShaderVar& outputColor) {
    outputColor.addLayoutQualifier("location = 0, index = 1");
}

void GrD3DPipelineStateBuilder::compileD3DProgram(SkSL::Program::Kind kind,
                                                  const SkSL::String& sksl,
                                                  const SkSL::Program::Settings& settings,
                                                  ID3DBlob** shader,
                                                  SkSL::Program::Inputs* outInputs) {
    auto errorHandler = fGpu->getContext()->priv().getShaderErrorHandler();
    std::unique_ptr<SkSL::Program> program = fGpu->shaderCompiler()->convertProgram(
            kind, sksl, settings);
    if (!program) {
        errorHandler->compileError(sksl.c_str(),
                                   fGpu->shaderCompiler()->errorText().c_str());
        return;
    }
    *outInputs = program->fInputs;
    SkSL::String outHLSL;
    if (!fGpu->shaderCompiler()->toHLSL(*program, &outHLSL)) {
        errorHandler->compileError(sksl.c_str(),
                                   fGpu->shaderCompiler()->errorText().c_str());
        return;
    }

    const char* compileTarget = nullptr;
    switch (kind) {
        case SkSL::Program::kVertex_Kind:
            compileTarget = "vs_5_1";
            break;
        case SkSL::Program::kGeometry_Kind:
            compileTarget = "gs_5_1";
            break;
        case SkSL::Program::kFragment_Kind:
            compileTarget = "ps_5_1";
            break;
        default:
            SkUNREACHABLE;
    }

    uint32_t compileFlags = 0;
#ifdef SK_DEBUG
    // Enable better shader debugging with the graphics debugging tools.
    compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    // SPRIV-cross does matrix multiplication expecting row major matrices
    compileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

    // TODO: D3D Static Function
    gr_cp<ID3DBlob> errors;
    HRESULT hr = D3DCompile(outHLSL.c_str(), outHLSL.length(), nullptr, nullptr, nullptr, "main",
                            compileTarget, compileFlags, 0, shader, &errors);
    if (!SUCCEEDED(hr)) {
        errorHandler->compileError(outHLSL.c_str(),
                                   reinterpret_cast<char*>(errors->GetBufferPointer()));
    }
}

sk_sp<GrD3DPipelineState> GrD3DPipelineStateBuilder::finalize() {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    // We need to enable the following extensions so that the compiler can correctly make spir-v
    // from our glsl shaders.
    fVS.extensions().appendf("#extension GL_ARB_separate_shader_objects : enable\n");
    fFS.extensions().appendf("#extension GL_ARB_separate_shader_objects : enable\n");
    fVS.extensions().appendf("#extension GL_ARB_shading_language_420pack : enable\n");
    fFS.extensions().appendf("#extension GL_ARB_shading_language_420pack : enable\n");

    this->finalizeShaders();

    SkSL::Program::Settings settings;
    settings.fCaps = this->caps()->shaderCaps();
    settings.fFlipY = this->origin() != kTopLeft_GrSurfaceOrigin;
    settings.fSharpenTextures =
        this->gpu()->getContext()->priv().options().fSharpenMipmappedTextures;
    settings.fRTHeightOffset = fUniformHandler.getRTHeightOffset();
    settings.fRTHeightBinding = 0;
    settings.fRTHeightSet = 0;

    gr_cp<ID3DBlob> vertexShader;
    gr_cp<ID3DBlob> geometryShader;
    gr_cp<ID3DBlob> pixelShader;
    SkSL::Program::Inputs vertInputs, fragInputs, geomInputs;

    this->compileD3DProgram(SkSL::Program::kVertex_Kind, fVS.fCompilerString, settings,
                            &vertexShader, &vertInputs);
    this->compileD3DProgram(SkSL::Program::kFragment_Kind, fFS.fCompilerString, settings,
                            &pixelShader, &fragInputs);

    if (!vertexShader.get() || !pixelShader.get()) {
        return nullptr;
    }

    if (this->primitiveProcessor().willUseGeoShader()) {
        this->compileD3DProgram(SkSL::Program::kGeometry_Kind, fGS.fCompilerString, settings,
                                &geometryShader, &geomInputs);
        if (!geometryShader.get()) {
            return nullptr;
        }
    }

    sk_sp<GrD3DRootSignature> rootSig =
            fGpu->resourceProvider().findOrCreateRootSignature(fUniformHandler.fTextures.count());
    if (!rootSig) {
        return nullptr;
    }

    const GrD3DRenderTarget* rt = static_cast<const GrD3DRenderTarget*>(fRenderTarget);
    return GrD3DPipelineState::Make(fGpu, fProgramInfo, std::move(rootSig),
                                    std::move(vertexShader), std::move(geometryShader),
                                    std::move(pixelShader), rt->dxgiFormat(),
                                    rt->stencilDxgiFormat(), rt->sampleQualityLevel());
}
