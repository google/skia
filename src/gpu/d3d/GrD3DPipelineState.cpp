/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DPipelineState.h"

#include "include/private/SkTemplates.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/d3d/GrD3DGpu.h"
#include "src/gpu/d3d/GrD3DRootSignature.h"

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
        case kUint_GrVertexAttribType:
            return DXGI_FORMAT_R32_UINT;
        case kUShort_norm_GrVertexAttribType:
            return DXGI_FORMAT_R16_UNORM;
        case kUShort4_norm_GrVertexAttribType:
            return DXGI_FORMAT_R16G16B16A16_UNORM;
    }
    SK_ABORT("Unknown vertex attrib type");
}

static void setup_vertex_input_layout(const GrPrimitiveProcessor& primProc,
                                      D3D12_INPUT_ELEMENT_DESC* inputElements) {
    unsigned int slotNumber = 0;
    unsigned int vertexSlot = 0;
    unsigned int instanceSlot = 0;
    if (primProc.hasVertexAttributes()) {
        vertexSlot = slotNumber++;
    }
    if (primProc.hasInstanceAttributes()) {
        instanceSlot = slotNumber++;
    }

    unsigned int currentAttrib = 0;
    unsigned int vertexAttributeOffset = 0;

    for (const auto& attrib : primProc.vertexAttributes()) {
        // When using SPIRV-Cross it converts the location modifier in SPIRV to be
        // TEXCOORD<N> where N is the location value for eveery vertext attribute
        inputElements[currentAttrib] = {"TEXCOORD", currentAttrib,
                                        attrib_type_to_format(attrib.cpuType()),
                                        vertexSlot, vertexAttributeOffset,
                                        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
        vertexAttributeOffset += attrib.sizeAlign4();
        currentAttrib++;
    }
    SkASSERT(vertexAttributeOffset == primProc.vertexStride());

    unsigned int instanceAttributeOffset = 0;
    for (const auto& attrib : primProc.instanceAttributes()) {
        // When using SPIRV-Cross it converts the location modifier in SPIRV to be
        // TEXCOORD<N> where N is the location value for eveery vertext attribute
        inputElements[currentAttrib] = {"TEXCOORD", currentAttrib,
                                        attrib_type_to_format(attrib.cpuType()),
                                        instanceSlot, instanceAttributeOffset,
                                        D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0};
        instanceAttributeOffset += attrib.sizeAlign4();
        currentAttrib++;
    }
    SkASSERT(instanceAttributeOffset == primProc.instanceStride());
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

static void fill_in_rasterizer_state(const GrPipeline& pipeline, const GrCaps* caps,
                                     D3D12_RASTERIZER_DESC* rasterizer) {
    rasterizer->FillMode = (caps->wireframeMode() || pipeline.isWireframe()) ?
                           D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
    rasterizer->CullMode = D3D12_CULL_MODE_NONE;
    rasterizer->FrontCounterClockwise = true;
    rasterizer->DepthBias = 0;
    rasterizer->DepthBiasClamp = 0.0f;
    rasterizer->SlopeScaledDepthBias = 0.0f;
    rasterizer->DepthClipEnable = false;
    rasterizer->MultisampleEnable = pipeline.isHWAntialiasState();
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
            const auto& backFace = stencilSettings.postOriginCCWFace(origin);

            SkASSERT(frontFace.fTestMask == backFace.fTestMask);
            SkASSERT(frontFace.fWriteMask == backFace.fWriteMask);
            dsDesc->StencilReadMask = frontFace.fTestMask;
            dsDesc->StencilWriteMask = frontFace.fWriteMask;

            setup_stencilop_desc(&dsDesc->FrontFace, frontFace);
            setup_stencilop_desc(&dsDesc->BackFace, backFace);
        } else {
            dsDesc->StencilReadMask = stencilSettings.singleSidedFace().fTestMask;
            dsDesc->StencilWriteMask = stencilSettings.singleSidedFace().fTestMask;
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

sk_sp<GrD3DPipelineState> GrD3DPipelineState::Make(GrD3DGpu* gpu,
                                                   const GrProgramInfo& programInfo,
                                                   sk_sp<GrD3DRootSignature> rootSig,
                                                   gr_cp<ID3DBlob> vertexShader,
                                                   gr_cp<ID3DBlob> geometryShader,
                                                   gr_cp<ID3DBlob> pixelShader,
                                                   DXGI_FORMAT renderTargetFormat,
                                                   DXGI_FORMAT depthStencilFormat,
                                                   unsigned int sampleQualityLevel) {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

    psoDesc.pRootSignature = rootSig->rootSignature();

    psoDesc.VS = { reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()),
                   vertexShader->GetBufferSize() };
    psoDesc.PS = { reinterpret_cast<UINT8*>(pixelShader->GetBufferPointer()),
                   pixelShader->GetBufferSize() };

    if (geometryShader.get()) {
        psoDesc.GS = { reinterpret_cast<UINT8*>(geometryShader->GetBufferPointer()),
                       geometryShader->GetBufferSize() };
    }

    psoDesc.StreamOutput = {nullptr, 0, nullptr, 0, 0};

    fill_in_blend_state(programInfo.pipeline(), &psoDesc.BlendState);
    psoDesc.SampleMask = UINT_MAX;

    fill_in_rasterizer_state(programInfo.pipeline(), gpu->caps(), &psoDesc.RasterizerState);

    fill_in_depth_stencil_state(programInfo, &psoDesc.DepthStencilState);

    unsigned int totalAttributeCnt = programInfo.primProc().numVertexAttributes() +
            programInfo.primProc().numInstanceAttributes();
    SkAutoSTArray<4, D3D12_INPUT_ELEMENT_DESC> inputElements(totalAttributeCnt);
    setup_vertex_input_layout(programInfo.primProc(), inputElements.get());

    psoDesc.InputLayout = { inputElements.get(), totalAttributeCnt };

    psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

    // This is for geometry or hull shader primitives
    psoDesc.PrimitiveTopologyType = gr_primitive_type_to_d3d(programInfo.primitiveType());

    psoDesc.NumRenderTargets = 1;

    psoDesc.RTVFormats[0] = renderTargetFormat;

    psoDesc.DSVFormat = depthStencilFormat;

    unsigned int numRasterSamples = programInfo.numRasterSamples();
    psoDesc.SampleDesc = {numRasterSamples, sampleQualityLevel};

    // Only used for multi-adapter systems.
    psoDesc.NodeMask = 0;

    psoDesc.CachedPSO = {nullptr, 0};
    psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    gr_cp<ID3D12PipelineState> pipelineState;
    SkDEBUGCODE(HRESULT hr = )gpu->device()->CreateGraphicsPipelineState(
            &psoDesc, IID_PPV_ARGS(&pipelineState));
    SkASSERT(SUCCEEDED(hr));

    return sk_sp<GrD3DPipelineState>(new GrD3DPipelineState(std::move(pipelineState)));
}

GrD3DPipelineState::GrD3DPipelineState(gr_cp<ID3D12PipelineState> pipelineState)
        : fPipelineState(std::move(pipelineState)) {}
