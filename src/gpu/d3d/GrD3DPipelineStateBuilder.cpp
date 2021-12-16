/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

//#include <d3dcompiler.h>

#include "src/gpu/d3d/GrD3DPipelineStateBuilder.h"

#include "include/gpu/GrDirectContext.h"
#include "include/gpu/d3d/GrD3DTypes.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GrAutoLocaleSetter.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrPersistentCacheUtils.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DPipeline.h"
#include "src/gpu/d3d/GrD3DRenderTarget.h"
#include "src/gpu/d3d/GrD3DRootSignature.h"
#include "src/gpu/d3d/GrD3DUtil.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/utils/SkShaderUtils.h"

#include <d3dcompiler.h>

std::unique_ptr<GrD3DPipelineState> GrD3DPipelineStateBuilder::MakePipelineState(
        GrD3DGpu* gpu,
        GrD3DRenderTarget* renderTarget,
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
                                                     GrD3DRenderTarget* renderTarget,
                                                     const GrProgramDesc& desc,
                                                     const GrProgramInfo& programInfo)
        : INHERITED(desc, programInfo)
        , fGpu(gpu)
        , fVaryingHandler(this)
        , fUniformHandler(this)
        , fRenderTarget(renderTarget) {}

const GrCaps* GrD3DPipelineStateBuilder::caps() const {
    return fGpu->caps();
}

SkSL::Compiler* GrD3DPipelineStateBuilder::shaderCompiler() const {
    return fGpu->shaderCompiler();
}

void GrD3DPipelineStateBuilder::finalizeFragmentOutputColor(GrShaderVar& outputColor) {
    outputColor.addLayoutQualifier("location = 0, index = 0");
}

void GrD3DPipelineStateBuilder::finalizeFragmentSecondaryColor(GrShaderVar& outputColor) {
    outputColor.addLayoutQualifier("location = 0, index = 1");
}

// Print the source code for all shaders generated.
static const bool gPrintSKSL = false;
static const bool gPrintHLSL = false;

static gr_cp<ID3DBlob> GrCompileHLSLShader(GrD3DGpu* gpu,
                                           const SkSL::String& hlsl,
                                           SkSL::ProgramKind kind) {
    TRACE_EVENT0("skia.shaders", "driver_compile_shader");
    const char* compileTarget = nullptr;
    switch (kind) {
        case SkSL::ProgramKind::kVertex:
            compileTarget = "vs_5_1";
            break;
        case SkSL::ProgramKind::kFragment:
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

    gr_cp<ID3DBlob> shader;
    gr_cp<ID3DBlob> errors;
    HRESULT hr = D3DCompile(hlsl.c_str(), hlsl.length(), nullptr, nullptr, nullptr, "main",
                            compileTarget, compileFlags, 0, &shader, &errors);
    if (!SUCCEEDED(hr)) {
        gpu->getContext()->priv().getShaderErrorHandler()->compileError(
                hlsl.c_str(), reinterpret_cast<char*>(errors->GetBufferPointer()));
    }
    return shader;
}

bool GrD3DPipelineStateBuilder::loadHLSLFromCache(SkReadBuffer* reader, gr_cp<ID3DBlob> shaders[]) {

    SkSL::String hlsl[kGrShaderTypeCount];
    SkSL::Program::Inputs inputs[kGrShaderTypeCount];

    if (!GrPersistentCacheUtils::UnpackCachedShaders(reader, hlsl, inputs, kGrShaderTypeCount)) {
        return false;
    }

    auto compile = [&](SkSL::ProgramKind kind, GrShaderType shaderType) {
        if (inputs[shaderType].fUseFlipRTUniform) {
            this->addRTFlipUniform(SKSL_RTFLIP_NAME);
        }
        shaders[shaderType] = GrCompileHLSLShader(fGpu, hlsl[shaderType], kind);
        return shaders[shaderType].get();
    };

    return compile(SkSL::ProgramKind::kVertex, kVertex_GrShaderType) &&
           compile(SkSL::ProgramKind::kFragment, kFragment_GrShaderType);
}

gr_cp<ID3DBlob> GrD3DPipelineStateBuilder::compileD3DProgram(
        SkSL::ProgramKind kind,
        const SkSL::String& sksl,
        const SkSL::Program::Settings& settings,
        SkSL::Program::Inputs* outInputs,
        SkSL::String* outHLSL) {
#ifdef SK_DEBUG
    SkSL::String src = SkShaderUtils::PrettyPrint(sksl);
#else
    const SkSL::String& src = sksl;
#endif

    std::unique_ptr<SkSL::Program> program = fGpu->shaderCompiler()->convertProgram(
            kind, src, settings);
    if (!program || !fGpu->shaderCompiler()->toHLSL(*program, outHLSL)) {
        auto errorHandler = fGpu->getContext()->priv().getShaderErrorHandler();
        errorHandler->compileError(src.c_str(),
                                   fGpu->shaderCompiler()->errorText().c_str());
        return gr_cp<ID3DBlob>();
    }
    *outInputs = program->fInputs;

    if (gPrintSKSL || gPrintHLSL) {
        SkShaderUtils::PrintShaderBanner(kind);
        if (gPrintSKSL) {
            SkDebugf("SKSL:\n");
            SkShaderUtils::PrintLineByLine(SkShaderUtils::PrettyPrint(sksl));
        }
        if (gPrintHLSL) {
            SkDebugf("HLSL:\n");
            SkShaderUtils::PrintLineByLine(SkShaderUtils::PrettyPrint(*outHLSL));
        }
    }

    if (program->fInputs.fUseFlipRTUniform) {
        this->addRTFlipUniform(SKSL_RTFLIP_NAME);
    }

    return GrCompileHLSLShader(fGpu, *outHLSL, kind);
}

static DXGI_FORMAT attrib_type_to_format(GrVertexAttribType type) {
    switch (type) {
    case kFloat_GrVertexAttribType:
        return DXGI_FORMAT_R32_FLOAT;
    case kFloat2_GrVertexAttribType:
        return DXGI_FORMAT_R32G32_FLOAT;
    case kFloat3_GrVertexAttribType:
        return DXGI_FORMAT_R32G32B32_FLOAT;
    case kFloat4_GrVertexAttribType:
        return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case kHalf_GrVertexAttribType:
        return DXGI_FORMAT_R16_FLOAT;
    case kHalf2_GrVertexAttribType:
        return DXGI_FORMAT_R16G16_FLOAT;
    case kHalf4_GrVertexAttribType:
        return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case kInt2_GrVertexAttribType:
        return DXGI_FORMAT_R32G32_SINT;
    case kInt3_GrVertexAttribType:
        return DXGI_FORMAT_R32G32B32_SINT;
    case kInt4_GrVertexAttribType:
        return DXGI_FORMAT_R32G32B32A32_SINT;
    case kByte_GrVertexAttribType:
        return DXGI_FORMAT_R8_SINT;
    case kByte2_GrVertexAttribType:
        return DXGI_FORMAT_R8G8_SINT;
    case kByte4_GrVertexAttribType:
        return DXGI_FORMAT_R8G8B8A8_SINT;
    case kUByte_GrVertexAttribType:
        return DXGI_FORMAT_R8_UINT;
    case kUByte2_GrVertexAttribType:
        return DXGI_FORMAT_R8G8_UINT;
    case kUByte4_GrVertexAttribType:
        return DXGI_FORMAT_R8G8B8A8_UINT;
    case kUByte_norm_GrVertexAttribType:
        return DXGI_FORMAT_R8_UNORM;
    case kUByte4_norm_GrVertexAttribType:
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    case kShort2_GrVertexAttribType:
        return DXGI_FORMAT_R16G16_SINT;
    case kShort4_GrVertexAttribType:
        return DXGI_FORMAT_R16G16B16A16_SINT;
    case kUShort2_GrVertexAttribType:
        return DXGI_FORMAT_R16G16_UINT;
    case kUShort2_norm_GrVertexAttribType:
        return DXGI_FORMAT_R16G16_UNORM;
    case kInt_GrVertexAttribType:
        return DXGI_FORMAT_R32_SINT;
    case kUInt_GrVertexAttribType:
        return DXGI_FORMAT_R32_UINT;
    case kUShort_norm_GrVertexAttribType:
        return DXGI_FORMAT_R16_UNORM;
    case kUShort4_norm_GrVertexAttribType:
        return DXGI_FORMAT_R16G16B16A16_UNORM;
    }
    SK_ABORT("Unknown vertex attrib type");
}

static void setup_vertex_input_layout(const GrGeometryProcessor& geomProc,
                                      D3D12_INPUT_ELEMENT_DESC* inputElements) {
    unsigned int slotNumber = 0;
    unsigned int vertexSlot = 0;
    unsigned int instanceSlot = 0;
    if (geomProc.hasVertexAttributes()) {
        vertexSlot = slotNumber++;
    }
    if (geomProc.hasInstanceAttributes()) {
        instanceSlot = slotNumber++;
    }

    unsigned int currentAttrib = 0;

    for (auto attrib : geomProc.vertexAttributes()) {
        // When using SPIRV-Cross it converts the location modifier in SPIRV to be
        // TEXCOORD<N> where N is the location value for eveery vertext attribute
        inputElements[currentAttrib] = { "TEXCOORD", currentAttrib,
                                        attrib_type_to_format(attrib.cpuType()),
                                        vertexSlot, SkToU32(*attrib.offset()),
                                        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
        currentAttrib++;
    }

    for (auto attrib : geomProc.instanceAttributes()) {
        // When using SPIRV-Cross it converts the location modifier in SPIRV to be
        // TEXCOORD<N> where N is the location value for eveery vertext attribute
        inputElements[currentAttrib] = { "TEXCOORD", currentAttrib,
                                        attrib_type_to_format(attrib.cpuType()),
                                        instanceSlot, SkToU32(*attrib.offset()),
                                        D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
        currentAttrib++;
    }
}

static D3D12_BLEND blend_coeff_to_d3d_blend(GrBlendCoeff coeff) {
    switch (coeff) {
    case kZero_GrBlendCoeff:
        return D3D12_BLEND_ZERO;
    case kOne_GrBlendCoeff:
        return D3D12_BLEND_ONE;
    case kSC_GrBlendCoeff:
        return D3D12_BLEND_SRC_COLOR;
    case kISC_GrBlendCoeff:
        return D3D12_BLEND_INV_SRC_COLOR;
    case kDC_GrBlendCoeff:
        return D3D12_BLEND_DEST_COLOR;
    case kIDC_GrBlendCoeff:
        return D3D12_BLEND_INV_DEST_COLOR;
    case kSA_GrBlendCoeff:
        return D3D12_BLEND_SRC_ALPHA;
    case kISA_GrBlendCoeff:
        return D3D12_BLEND_INV_SRC_ALPHA;
    case kDA_GrBlendCoeff:
        return D3D12_BLEND_DEST_ALPHA;
    case kIDA_GrBlendCoeff:
        return D3D12_BLEND_INV_DEST_ALPHA;
    case kConstC_GrBlendCoeff:
        return D3D12_BLEND_BLEND_FACTOR;
    case kIConstC_GrBlendCoeff:
        return D3D12_BLEND_INV_BLEND_FACTOR;
    case kS2C_GrBlendCoeff:
        return D3D12_BLEND_SRC1_COLOR;
    case kIS2C_GrBlendCoeff:
        return D3D12_BLEND_INV_SRC1_COLOR;
    case kS2A_GrBlendCoeff:
        return D3D12_BLEND_SRC1_ALPHA;
    case kIS2A_GrBlendCoeff:
        return D3D12_BLEND_INV_SRC1_ALPHA;
    case kIllegal_GrBlendCoeff:
        return D3D12_BLEND_ZERO;
    }
    SkUNREACHABLE;
}

static D3D12_BLEND blend_coeff_to_d3d_blend_for_alpha(GrBlendCoeff coeff) {
    switch (coeff) {
        // Force all srcColor used in alpha slot to alpha version.
    case kSC_GrBlendCoeff:
        return D3D12_BLEND_SRC_ALPHA;
    case kISC_GrBlendCoeff:
        return D3D12_BLEND_INV_SRC_ALPHA;
    case kDC_GrBlendCoeff:
        return D3D12_BLEND_DEST_ALPHA;
    case kIDC_GrBlendCoeff:
        return D3D12_BLEND_INV_DEST_ALPHA;
    case kS2C_GrBlendCoeff:
        return D3D12_BLEND_SRC1_ALPHA;
    case kIS2C_GrBlendCoeff:
        return D3D12_BLEND_INV_SRC1_ALPHA;

    default:
        return blend_coeff_to_d3d_blend(coeff);
    }
}


static D3D12_BLEND_OP blend_equation_to_d3d_op(GrBlendEquation equation) {
    switch (equation) {
    case kAdd_GrBlendEquation:
        return D3D12_BLEND_OP_ADD;
    case kSubtract_GrBlendEquation:
        return D3D12_BLEND_OP_SUBTRACT;
    case kReverseSubtract_GrBlendEquation:
        return D3D12_BLEND_OP_REV_SUBTRACT;
    default:
        SkUNREACHABLE;
    }
}

static void fill_in_blend_state(const GrPipeline& pipeline, D3D12_BLEND_DESC* blendDesc) {
    blendDesc->AlphaToCoverageEnable = false;
    blendDesc->IndependentBlendEnable = false;

    const GrXferProcessor::BlendInfo& blendInfo = pipeline.getXferProcessor().getBlendInfo();

    GrBlendEquation equation = blendInfo.fEquation;
    GrBlendCoeff srcCoeff = blendInfo.fSrcBlend;
    GrBlendCoeff dstCoeff = blendInfo.fDstBlend;
    bool blendOff = GrBlendShouldDisable(equation, srcCoeff, dstCoeff);

    auto& rtBlend = blendDesc->RenderTarget[0];
    rtBlend.BlendEnable = !blendOff;
    if (!blendOff) {
        rtBlend.SrcBlend = blend_coeff_to_d3d_blend(srcCoeff);
        rtBlend.DestBlend = blend_coeff_to_d3d_blend(dstCoeff);
        rtBlend.BlendOp = blend_equation_to_d3d_op(equation);
        rtBlend.SrcBlendAlpha = blend_coeff_to_d3d_blend_for_alpha(srcCoeff);
        rtBlend.DestBlendAlpha = blend_coeff_to_d3d_blend_for_alpha(dstCoeff);
        rtBlend.BlendOpAlpha = blend_equation_to_d3d_op(equation);
    }

    if (!blendInfo.fWriteColor) {
        rtBlend.RenderTargetWriteMask = 0;
    } else {
        rtBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    }
}

static void fill_in_rasterizer_state(const GrPipeline& pipeline,
                                     bool multisampleEnable,
                                     const GrCaps* caps,
                                     D3D12_RASTERIZER_DESC* rasterizer) {
    rasterizer->FillMode = (caps->wireframeMode() || pipeline.isWireframe()) ?
        D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
    rasterizer->CullMode = D3D12_CULL_MODE_NONE;
    rasterizer->FrontCounterClockwise = true;
    rasterizer->DepthBias = 0;
    rasterizer->DepthBiasClamp = 0.0f;
    rasterizer->SlopeScaledDepthBias = 0.0f;
    rasterizer->DepthClipEnable = false;
    rasterizer->MultisampleEnable = multisampleEnable;
    rasterizer->AntialiasedLineEnable = false;
    rasterizer->ForcedSampleCount = 0;
    rasterizer->ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
}

static D3D12_STENCIL_OP stencil_op_to_d3d_op(GrStencilOp op) {
    switch (op) {
    case GrStencilOp::kKeep:
        return D3D12_STENCIL_OP_KEEP;
    case GrStencilOp::kZero:
        return D3D12_STENCIL_OP_ZERO;
    case GrStencilOp::kReplace:
        return D3D12_STENCIL_OP_REPLACE;
    case GrStencilOp::kInvert:
        return D3D12_STENCIL_OP_INVERT;
    case GrStencilOp::kIncWrap:
        return D3D12_STENCIL_OP_INCR;
    case GrStencilOp::kDecWrap:
        return D3D12_STENCIL_OP_DECR;
    case GrStencilOp::kIncClamp:
        return D3D12_STENCIL_OP_INCR_SAT;
    case GrStencilOp::kDecClamp:
        return D3D12_STENCIL_OP_DECR_SAT;
    }
    SkUNREACHABLE;
}

static D3D12_COMPARISON_FUNC stencil_test_to_d3d_func(GrStencilTest test) {
    switch (test) {
    case GrStencilTest::kAlways:
        return D3D12_COMPARISON_FUNC_ALWAYS;
    case GrStencilTest::kNever:
        return D3D12_COMPARISON_FUNC_NEVER;
    case GrStencilTest::kGreater:
        return D3D12_COMPARISON_FUNC_GREATER;
    case GrStencilTest::kGEqual:
        return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
    case GrStencilTest::kLess:
        return D3D12_COMPARISON_FUNC_LESS;
    case GrStencilTest::kLEqual:
        return D3D12_COMPARISON_FUNC_LESS_EQUAL;
    case GrStencilTest::kEqual:
        return D3D12_COMPARISON_FUNC_EQUAL;
    case GrStencilTest::kNotEqual:
        return D3D12_COMPARISON_FUNC_NOT_EQUAL;
    }
    SkUNREACHABLE;
}

static void setup_stencilop_desc(D3D12_DEPTH_STENCILOP_DESC* desc,
                                 const GrStencilSettings::Face& stencilFace) {
    desc->StencilFailOp = stencil_op_to_d3d_op(stencilFace.fFailOp);
    desc->StencilDepthFailOp = desc->StencilFailOp;
    desc->StencilPassOp = stencil_op_to_d3d_op(stencilFace.fPassOp);
    desc->StencilFunc = stencil_test_to_d3d_func(stencilFace.fTest);
}

static void fill_in_depth_stencil_state(const GrProgramInfo& programInfo,
                                        D3D12_DEPTH_STENCIL_DESC* dsDesc) {
    GrStencilSettings stencilSettings = programInfo.nonGLStencilSettings();
    GrSurfaceOrigin origin = programInfo.origin();

    dsDesc->DepthEnable = false;
    dsDesc->DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    dsDesc->DepthFunc = D3D12_COMPARISON_FUNC_NEVER;
    dsDesc->StencilEnable = !stencilSettings.isDisabled();
    if (!stencilSettings.isDisabled()) {
        if (stencilSettings.isTwoSided()) {
            const auto& frontFace = stencilSettings.postOriginCCWFace(origin);
            const auto& backFace = stencilSettings.postOriginCWFace(origin);

            SkASSERT(frontFace.fTestMask == backFace.fTestMask);
            SkASSERT(frontFace.fWriteMask == backFace.fWriteMask);
            dsDesc->StencilReadMask = frontFace.fTestMask;
            dsDesc->StencilWriteMask = frontFace.fWriteMask;

            setup_stencilop_desc(&dsDesc->FrontFace, frontFace);
            setup_stencilop_desc(&dsDesc->BackFace, backFace);
        } else {
            dsDesc->StencilReadMask = stencilSettings.singleSidedFace().fTestMask;
            dsDesc->StencilWriteMask = stencilSettings.singleSidedFace().fWriteMask;
            setup_stencilop_desc(&dsDesc->FrontFace, stencilSettings.singleSidedFace());
            dsDesc->BackFace = dsDesc->FrontFace;
        }
    }
}

static D3D12_PRIMITIVE_TOPOLOGY_TYPE gr_primitive_type_to_d3d(GrPrimitiveType primitiveType) {
    switch (primitiveType) {
        case GrPrimitiveType::kTriangles:
        case GrPrimitiveType::kTriangleStrip: //fall through
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        case GrPrimitiveType::kPoints:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
        case GrPrimitiveType::kLines: // fall through
        case GrPrimitiveType::kLineStrip:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
        case GrPrimitiveType::kPatches: // fall through, unsupported
        case GrPrimitiveType::kPath: // fall through, unsupported
        default:
            SkUNREACHABLE;
    }
}

gr_cp<ID3D12PipelineState> create_pipeline_state(
        GrD3DGpu* gpu, const GrProgramInfo& programInfo, const sk_sp<GrD3DRootSignature>& rootSig,
        gr_cp<ID3DBlob> vertexShader, gr_cp<ID3DBlob> pixelShader,
        DXGI_FORMAT renderTargetFormat, DXGI_FORMAT depthStencilFormat,
        unsigned int sampleQualityPattern) {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

    psoDesc.pRootSignature = rootSig->rootSignature();

    psoDesc.VS = { reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()),
                   vertexShader->GetBufferSize() };
    psoDesc.PS = { reinterpret_cast<UINT8*>(pixelShader->GetBufferPointer()),
                   pixelShader->GetBufferSize() };

    psoDesc.StreamOutput = { nullptr, 0, nullptr, 0, 0 };

    fill_in_blend_state(programInfo.pipeline(), &psoDesc.BlendState);
    psoDesc.SampleMask = UINT_MAX;

    fill_in_rasterizer_state(programInfo.pipeline(), programInfo.numSamples() > 1, gpu->caps(),
                             &psoDesc.RasterizerState);

    fill_in_depth_stencil_state(programInfo, &psoDesc.DepthStencilState);

    unsigned int totalAttributeCnt = programInfo.geomProc().numVertexAttributes() +
                                     programInfo.geomProc().numInstanceAttributes();
    SkAutoSTArray<4, D3D12_INPUT_ELEMENT_DESC> inputElements(totalAttributeCnt);
    setup_vertex_input_layout(programInfo.geomProc(), inputElements.get());

    psoDesc.InputLayout = { inputElements.get(), totalAttributeCnt };

    psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

    // This is for geometry or hull shader primitives
    psoDesc.PrimitiveTopologyType = gr_primitive_type_to_d3d(programInfo.primitiveType());

    psoDesc.NumRenderTargets = 1;

    psoDesc.RTVFormats[0] = renderTargetFormat;

    psoDesc.DSVFormat = depthStencilFormat;

    unsigned int numSamples = programInfo.numSamples();
    psoDesc.SampleDesc = { numSamples, sampleQualityPattern };

    // Only used for multi-adapter systems.
    psoDesc.NodeMask = 0;

    psoDesc.CachedPSO = { nullptr, 0 };
    psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    gr_cp<ID3D12PipelineState> pipelineState;
    {
        TRACE_EVENT0("skia.shaders", "CreateGraphicsPipelineState");
        GR_D3D_CALL_ERRCHECK(
                gpu->device()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));
    }

    return pipelineState;
}

static constexpr SkFourByteTag kHLSL_Tag = SkSetFourByteTag('H', 'L', 'S', 'L');
static constexpr SkFourByteTag kSKSL_Tag = SkSetFourByteTag('S', 'K', 'S', 'L');

std::unique_ptr<GrD3DPipelineState> GrD3DPipelineStateBuilder::finalize() {
    TRACE_EVENT0("skia.shaders", TRACE_FUNC);

    this->finalizeShaders();

    SkSL::Program::Settings settings;
    settings.fSharpenTextures =
        this->gpu()->getContext()->priv().options().fSharpenMipmappedTextures;
    settings.fRTFlipOffset = fUniformHandler.getRTFlipOffset();
    settings.fRTFlipBinding = 0;
    settings.fRTFlipSet = 0;

    sk_sp<SkData> cached;
    SkReadBuffer reader;
    SkFourByteTag shaderType = 0;
    auto persistentCache = fGpu->getContext()->priv().getPersistentCache();
    if (persistentCache) {
        // Shear off the D3D-specific portion of the Desc to get the persistent key. We only cache
        // shader code, not entire pipelines.
        sk_sp<SkData> key =
                SkData::MakeWithoutCopy(this->desc().asKey(), this->desc().initialKeyLength());
        cached = persistentCache->load(*key);
        if (cached) {
            reader.setMemory(cached->data(), cached->size());
            shaderType = GrPersistentCacheUtils::GetType(&reader);
        }
    }

    const GrGeometryProcessor& geomProc = this->geometryProcessor();
    gr_cp<ID3DBlob> shaders[kGrShaderTypeCount];

    if (kHLSL_Tag == shaderType && this->loadHLSLFromCache(&reader, shaders)) {
        // We successfully loaded and compiled HLSL
    } else {
        SkSL::Program::Inputs inputs[kGrShaderTypeCount];
        SkSL::String* sksl[kGrShaderTypeCount] = {
            &fVS.fCompilerString,
            &fFS.fCompilerString,
        };
        SkSL::String cached_sksl[kGrShaderTypeCount];
        SkSL::String hlsl[kGrShaderTypeCount];

        if (kSKSL_Tag == shaderType) {
            if (GrPersistentCacheUtils::UnpackCachedShaders(&reader, cached_sksl, inputs,
                                                            kGrShaderTypeCount)) {
                for (int i = 0; i < kGrShaderTypeCount; ++i) {
                    sksl[i] = &cached_sksl[i];
                }
            }
        }

        auto compile = [&](SkSL::ProgramKind kind, GrShaderType shaderType) {
            shaders[shaderType] = this->compileD3DProgram(kind, *sksl[shaderType], settings,
                                                          &inputs[shaderType], &hlsl[shaderType]);
            return shaders[shaderType].get();
        };

        if (!compile(SkSL::ProgramKind::kVertex, kVertex_GrShaderType) ||
            !compile(SkSL::ProgramKind::kFragment, kFragment_GrShaderType)) {
            return nullptr;
        }

        if (persistentCache && !cached) {
            const bool cacheSkSL = fGpu->getContext()->priv().options().fShaderCacheStrategy ==
                                   GrContextOptions::ShaderCacheStrategy::kSkSL;
            if (cacheSkSL) {
                // Replace the HLSL with formatted SkSL to be cached. This looks odd, but this is
                // the last time we're going to use these strings, so it's safe.
                for (int i = 0; i < kGrShaderTypeCount; ++i) {
                    hlsl[i] = SkShaderUtils::PrettyPrint(*sksl[i]);
                }
            }
            sk_sp<SkData> key =
                    SkData::MakeWithoutCopy(this->desc().asKey(), this->desc().initialKeyLength());
            SkString description = GrProgramDesc::Describe(fProgramInfo, *this->caps());
            sk_sp<SkData> data = GrPersistentCacheUtils::PackCachedShaders(
                    cacheSkSL ? kSKSL_Tag : kHLSL_Tag, hlsl, inputs, kGrShaderTypeCount);
            persistentCache->store(*key, *data, description);
        }
    }

    sk_sp<GrD3DRootSignature> rootSig =
            fGpu->resourceProvider().findOrCreateRootSignature(fUniformHandler.fTextures.count());
    if (!rootSig) {
        return nullptr;
    }

    const GrD3DRenderTarget* rt = static_cast<const GrD3DRenderTarget*>(fRenderTarget);
    gr_cp<ID3D12PipelineState> pipelineState = create_pipeline_state(
            fGpu, fProgramInfo, rootSig, std::move(shaders[kVertex_GrShaderType]),
            std::move(shaders[kFragment_GrShaderType]),
            rt->dxgiFormat(), rt->stencilDxgiFormat(), rt->sampleQualityPattern());
    sk_sp<GrD3DPipeline> pipeline = GrD3DPipeline::Make(std::move(pipelineState));

    return std::unique_ptr<GrD3DPipelineState>(
            new GrD3DPipelineState(std::move(pipeline),
                                   std::move(rootSig),
                                   fUniformHandles,
                                   fUniformHandler.fUniforms,
                                   fUniformHandler.fCurrentUBOOffset,
                                   fUniformHandler.fSamplers.count(),
                                   std::move(fGPImpl),
                                   std::move(fXPImpl),
                                   std::move(fFPImpls),
                                   geomProc.vertexStride(),
                                   geomProc.instanceStride()));
}


sk_sp<GrD3DPipeline> GrD3DPipelineStateBuilder::MakeComputePipeline(GrD3DGpu* gpu,
                                                                    GrD3DRootSignature* rootSig,
                                                                    const char* shader) {
    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = rootSig->rootSignature();

    // compile shader
    gr_cp<ID3DBlob> shaderBlob;
    {
        TRACE_EVENT0("skia.shaders", "driver_compile_shader");
        uint32_t compileFlags = 0;
#ifdef SK_DEBUG
        // Enable better shader debugging with the graphics debugging tools.
        compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        gr_cp<ID3DBlob> errors;
        HRESULT hr = D3DCompile(shader, strlen(shader), nullptr, nullptr, nullptr, "main",
                                "cs_5_1", compileFlags, 0, &shaderBlob, &errors);
        if (!SUCCEEDED(hr)) {
            gpu->getContext()->priv().getShaderErrorHandler()->compileError(
                shader, reinterpret_cast<char*>(errors->GetBufferPointer()));
            return nullptr;
        }
        psoDesc.CS = { reinterpret_cast<UINT8*>(shaderBlob->GetBufferPointer()),
                       shaderBlob->GetBufferSize() };
    }

    // Only used for multi-adapter systems.
    psoDesc.NodeMask = 0;

    psoDesc.CachedPSO = { nullptr, 0 };
    psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    gr_cp<ID3D12PipelineState> pipelineState;
    {
        TRACE_EVENT0("skia.shaders", "CreateComputePipelineState");
        GR_D3D_CALL_ERRCHECK(
            gpu->device()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));
    }

    return GrD3DPipeline::Make(std::move(pipelineState));
}

