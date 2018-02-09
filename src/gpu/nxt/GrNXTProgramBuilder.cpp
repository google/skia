/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNXTProgramBuilder.h"

#include "nxt/GrNXTGpu.h"
#include "GrRenderTarget.h"
#include "SkSLCompiler.h"

#include "GrSKSLPrettyPrint.h"
namespace {

SkSL::String sksl_to_spirv(const GrNXTGpu* gpu, const char* shaderString, SkSL::Program::Kind kind, SkSL::Program::Inputs* inputs) {
    SkSL::Program::Settings settings;
    std::unique_ptr<SkSL::Program> program = gpu->shaderCompiler()->convertProgram(
        kind,
        shaderString,
        settings);
    if (!program) {
        SkDebugf("SkSL error:\n%s\n", gpu->shaderCompiler()->errorText().c_str());
        SkASSERT(false);
        return "";
    }
    *inputs = program->fInputs;
    SkSL::String code;
    if (!gpu->shaderCompiler()->toSPIRV(*program, &code)) {
        return "";
    }
    return code;
}

nxt::BlendFactor to_nxt_blend_factor(GrBlendCoeff coeff) {
    switch (coeff) {
    case kZero_GrBlendCoeff:
        return nxt::BlendFactor::Zero;
    case kOne_GrBlendCoeff:
        return nxt::BlendFactor::One;
    case kSC_GrBlendCoeff:
        return nxt::BlendFactor::SrcColor;
    case kISC_GrBlendCoeff:
        return nxt::BlendFactor::OneMinusSrcColor;
    case kDC_GrBlendCoeff:
        return nxt::BlendFactor::DstColor;
    case kIDC_GrBlendCoeff:
        return nxt::BlendFactor::OneMinusDstColor;
    case kSA_GrBlendCoeff:
        return nxt::BlendFactor::SrcAlpha;
    case kISA_GrBlendCoeff:
        return nxt::BlendFactor::OneMinusSrcAlpha;
    case kDA_GrBlendCoeff:
        return nxt::BlendFactor::DstAlpha;
    case kIDA_GrBlendCoeff:
        return nxt::BlendFactor::OneMinusDstAlpha;
    case kConstC_GrBlendCoeff:
        return nxt::BlendFactor::BlendColor;
    case kIConstC_GrBlendCoeff:
        return nxt::BlendFactor::OneMinusBlendColor;
    case kConstA_GrBlendCoeff:
    case kIConstA_GrBlendCoeff:
    case kS2C_GrBlendCoeff:
    case kIS2C_GrBlendCoeff:
    case kS2A_GrBlendCoeff:
    case kIS2A_GrBlendCoeff:
    default:
        SkASSERT(!"unsupported blend coefficient");
        return nxt::BlendFactor::One;
    }
}

nxt::BlendOperation to_nxt_blend_operation(GrBlendEquation equation) {
    switch (equation) {
    case kAdd_GrBlendEquation:
        return nxt::BlendOperation::Add;
    case kSubtract_GrBlendEquation:
        return nxt::BlendOperation::Subtract;
    default:
        SkASSERT(!"unsupported blend equation");
        return nxt::BlendOperation::Add;
    }
}

nxt::BlendState create_blend_state(const GrNXTGpu* gpu, const GrPipeline& pipeline) {
    GrXferProcessor::BlendInfo blendInfo;
    pipeline.getXferProcessor().getBlendInfo(&blendInfo);
    GrBlendEquation equation = blendInfo.fEquation;
    GrBlendCoeff srcCoeff = blendInfo.fSrcBlend;
    GrBlendCoeff dstCoeff = blendInfo.fDstBlend;
    bool blendOff = (kAdd_GrBlendEquation == equation || kSubtract_GrBlendEquation == equation) &&
                    kOne_GrBlendCoeff == srcCoeff && kZero_GrBlendCoeff == dstCoeff;

    nxt::BlendFactor srcFactor = to_nxt_blend_factor(srcCoeff);
    nxt::BlendFactor dstFactor = to_nxt_blend_factor(dstCoeff);
    nxt::BlendOperation operation = to_nxt_blend_operation(equation);

    return gpu->device().CreateBlendStateBuilder()
        .SetBlendEnabled(!blendOff)
        .SetAlphaBlend(operation, srcFactor, dstFactor)
        .SetColorBlend(operation, srcFactor, dstFactor)
        .GetResult();
}

}

/////////////////////////////////////////////////////////////////////////////

sk_sp<GrNXTProgram> GrNXTProgramBuilder::Build(GrNXTGpu* gpu,
                                               const GrPipeline& pipeline,
                                               const GrPrimitiveProcessor& primProc,
                                               GrProgramDesc* desc) {
    GrNXTProgramBuilder builder(gpu, pipeline, primProc, desc);
    if (!builder.emitAndInstallProcs()) {
        return nullptr;
    }

    builder.fVS.extensions().appendf("#extension GL_ARB_separate_shader_objects : enable\n");
    builder.fFS.extensions().appendf("#extension GL_ARB_separate_shader_objects : enable\n");
    builder.fVS.extensions().appendf("#extension GL_ARB_shading_language_420pack : enable\n");
    builder.fFS.extensions().appendf("#extension GL_ARB_shading_language_420pack : enable\n");

    builder.finalizeShaders();

    SkSL::Program::Inputs vertInputs, fragInputs;
    GrNXTUniformHandler::UniformInfoArray& uniforms = builder.fUniformHandler.fUniforms;
    uint32_t geometryUniformSize = builder.fUniformHandler.fCurrentGeometryUBOOffset;
    uint32_t fragmentUniformSize = builder.fUniformHandler.fCurrentFragmentUBOOffset;
    sk_sp<GrNXTProgram> result(
        new GrNXTProgram(uniforms, geometryUniformSize, fragmentUniformSize));
    result->fVSModule = builder.CreateShaderModule(gpu->device(), builder.fVS,
                                                   SkSL::Program::kVertex_Kind, &vertInputs);
    result->fFSModule = builder.CreateShaderModule(gpu->device(), builder.fFS,
                                                   SkSL::Program::kFragment_Kind, &fragInputs);
    result->fGeometryProcessor = std::move(builder.fGeometryProcessor);
    result->fXferProcessor = std::move(builder.fXferProcessor);
    result->fFragmentProcessors = builder.fFragmentProcessors;
    auto bindGroupLayoutBuilder = gpu->device().CreateBindGroupLayoutBuilder();
    nxt::BufferView bufferViews[2];
    int numBuffers = 0;
    if (0 != geometryUniformSize) {
        result->fGeometryUniformBuffer = gpu->device().CreateBufferBuilder()
            .SetAllowedUsage(nxt::BufferUsageBit::Uniform | nxt::BufferUsageBit::TransferDst)
            .SetInitialUsage(nxt::BufferUsageBit::TransferDst)
            .SetSize(geometryUniformSize)
            .GetResult();
        }
        bufferViews[numBuffers++] = result->fGeometryUniformBuffer.CreateBufferViewBuilder()
            .SetExtent(0, geometryUniformSize)
            .GetResult();
        bindGroupLayoutBuilder.SetBindingsType(nxt::ShaderStageBit::Vertex, nxt::BindingType::UniformBuffer, 0, 1);
    if (0 != fragmentUniformSize) {
        result->fFragmentUniformBuffer = gpu->device().CreateBufferBuilder()
            .SetAllowedUsage(nxt::BufferUsageBit::Uniform | nxt::BufferUsageBit::TransferDst)
            .SetInitialUsage(nxt::BufferUsageBit::TransferDst)
            .SetSize(fragmentUniformSize)
            .GetResult();
        bufferViews[numBuffers++] = result->fFragmentUniformBuffer.CreateBufferViewBuilder()
            .SetExtent(0, fragmentUniformSize)
            .GetResult();
        bindGroupLayoutBuilder.SetBindingsType(nxt::ShaderStageBit::Fragment, nxt::BindingType::UniformBuffer, 1, 1);
        }
    auto bindGroupLayout = bindGroupLayoutBuilder.GetResult();
    result->fUniformBindGroup = gpu->device().CreateBindGroupBuilder()
        .SetLayout(bindGroupLayout)
        .SetUsage(nxt::BindGroupUsage::Frozen)
        .SetBufferViews(0, numBuffers, bufferViews)
        .GetResult();
    result->fPipelineLayout = gpu->device().CreatePipelineLayoutBuilder()
        .SetBindGroupLayout(0, bindGroupLayout)
        .GetResult();
    result->fBuiltinUniformHandles = builder.fUniformHandles;
    result->fBlendState = create_blend_state(gpu, pipeline);
    return result;
}

GrNXTProgramBuilder::GrNXTProgramBuilder(GrNXTGpu* gpu,
                                         const GrPipeline& pipeline,
                                         const GrPrimitiveProcessor& primProc,
                                         GrProgramDesc* desc)
    : INHERITED(pipeline, primProc, desc)
    , fGpu(gpu)
    , fVaryingHandler(this)
    , fUniformHandler(this) {
}

nxt::ShaderModule GrNXTProgramBuilder::CreateShaderModule(nxt::Device device, const GrGLSLShaderBuilder& builder, SkSL::Program::Kind kind, SkSL::Program::Inputs* inputs) {
    SkString source;
    for (int i = 0; i < builder.fCompilerStrings.count(); i++) {
        source.append(builder.fCompilerStrings[i]);
        source.append("\n");
    }

#if 0
    SkSL::String sksl = GrSKSLPrettyPrint::PrettyPrint((const char**) builder.fCompilerStrings.begin(), (int*) builder.fCompilerStringLengths.begin(), builder.fCompilerStrings.count(), false);
    printf("converting program:\n%s\n", sksl.c_str());
#endif

    SkSL::String spirvSource = sksl_to_spirv(fGpu, source.c_str(), kind, inputs);

    return device.CreateShaderModuleBuilder()
        .SetSource(spirvSource.size() / 4, reinterpret_cast<const uint32_t*>(spirvSource.c_str()))
        .GetResult();
};

const GrCaps* GrNXTProgramBuilder::caps() const {
    return fGpu->caps();
}

void GrNXTProgram::setRenderTargetState(const GrRenderTargetProxy* proxy) {
    GrRenderTarget* rt = proxy->priv().peekRenderTarget();

    // Load the RT height uniform if it is needed to y-flip gl_FragCoord.
    if (fBuiltinUniformHandles.fRTHeightUni.isValid() &&
        fRenderTargetState.fRenderTargetSize.fHeight != rt->height()) {
        fDataManager.set1f(fBuiltinUniformHandles.fRTHeightUni, SkIntToScalar(rt->height()));
    }

    // set RT adjustment
    SkISize size;
    size.set(rt->width(), rt->height());
    SkASSERT(fBuiltinUniformHandles.fRTAdjustmentUni.isValid());
    if (fRenderTargetState.fRenderTargetOrigin != proxy->origin() ||
        fRenderTargetState.fRenderTargetSize != size) {
        fRenderTargetState.fRenderTargetSize = size;
        fRenderTargetState.fRenderTargetOrigin = proxy->origin();

        float rtAdjustmentVec[4];
        fRenderTargetState.getRTAdjustmentVec(rtAdjustmentVec);
        fDataManager.set4fv(fBuiltinUniformHandles.fRTAdjustmentUni, 1, rtAdjustmentVec);
    }
}

void GrNXTProgram::setData(const GrPrimitiveProcessor& primProc,
                           const GrPipeline& pipeline) {
    this->setRenderTargetState(pipeline.proxy());
    fGeometryProcessor->setData(fDataManager, primProc,
                                GrFragmentProcessor::CoordTransformIter(pipeline));
    GrFragmentProcessor::Iter iter(pipeline);
    GrGLSLFragmentProcessor::Iter glslIter(fFragmentProcessors.begin(),
                                           fFragmentProcessors.count());
    const GrFragmentProcessor* fp = iter.next();
    GrGLSLFragmentProcessor* glslFP = glslIter.next();
    while (fp && glslFP) {
        glslFP->setData(fDataManager, *fp);
        fp = iter.next();
        glslFP = glslIter.next();
    }
    fDataManager.uploadUniformBuffers(fGeometryUniformBuffer.Clone(),
                                      fFragmentUniformBuffer.Clone());
}
