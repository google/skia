/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNXTProgramBuilder.h"

#include "nxt/GrNXTGpu.h"
#include "nxt/GrNXTTexture.h"
#include "GrRenderTarget.h"
#include "GrStencilSettings.h"
#include "SkSLCompiler.h"

#include "GrSKSLPrettyPrint.h"
namespace {

SkSL::String sksl_to_spirv(const GrNXTGpu* gpu, const char* shaderString, SkSL::Program::Kind kind, SkSL::Program::Inputs* inputs) {
    SkSL::Program::Settings settings;
    settings.fCaps = gpu->caps()->shaderCaps();
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
    case kReverseSubtract_GrBlendEquation:
        return nxt::BlendOperation::ReverseSubtract;
    default:
        SkASSERT(!"unsupported blend equation");
        return nxt::BlendOperation::Add;
    }
}

nxt::CompareFunction to_nxt_compare_function(GrStencilTest test) {
    switch (test) {
        case GrStencilTest::kAlways:
            return nxt::CompareFunction::Always;
        case GrStencilTest::kNever:
            return nxt::CompareFunction::Never;
        case GrStencilTest::kGreater:
            return nxt::CompareFunction::Greater;
        case GrStencilTest::kGEqual:
            return nxt::CompareFunction::GreaterEqual;
        case GrStencilTest::kLess:
            return nxt::CompareFunction::Less;
        case GrStencilTest::kLEqual:
            return nxt::CompareFunction::LessEqual;
        case GrStencilTest::kEqual:
            return nxt::CompareFunction::Equal;
        case GrStencilTest::kNotEqual:
            return nxt::CompareFunction::NotEqual;
        default:
            SkASSERT(!"unsupported stencil test");
            return nxt::CompareFunction::Always;
    }
}

nxt::StencilOperation to_nxt_stencil_operation(GrStencilOp op) {
    switch (op) {
        case GrStencilOp::kKeep:
            return nxt::StencilOperation::Keep;
        case GrStencilOp::kZero:
            return nxt::StencilOperation::Zero;
        case GrStencilOp::kReplace:
            return nxt::StencilOperation::Replace;
        case GrStencilOp::kInvert:
            return nxt::StencilOperation::Invert;
        case GrStencilOp::kIncClamp:
            return nxt::StencilOperation::IncrementClamp;
        case GrStencilOp::kDecClamp:
            return nxt::StencilOperation::DecrementClamp;
        case GrStencilOp::kIncWrap:
            return nxt::StencilOperation::IncrementWrap;
        case GrStencilOp::kDecWrap:
            return nxt::StencilOperation::DecrementWrap;
        default:
            SkASSERT(!"unsupported stencil function");
            return nxt::StencilOperation::Keep;
    }
}

nxt::FilterMode to_nxt_filter_mode(GrSamplerState::Filter filter) {
    switch (filter) {
        case GrSamplerState::Filter::kNearest:
            return nxt::FilterMode::Nearest;
        case GrSamplerState::Filter::kBilerp:
        case GrSamplerState::Filter::kMipMap:
            return nxt::FilterMode::Linear;
        default:
            SkASSERT(!"unsupported filter mode");
            return nxt::FilterMode::Nearest;
    }
}

nxt::AddressMode to_nxt_address_mode(GrSamplerState::WrapMode wrapMode) {
    switch (wrapMode) {
        case GrSamplerState::WrapMode::kClamp:
            return nxt::AddressMode::ClampToEdge;
        case GrSamplerState::WrapMode::kRepeat:
            return nxt::AddressMode::Repeat;
        case GrSamplerState::WrapMode::kMirrorRepeat:
            return nxt::AddressMode::MirroredRepeat;
    }
    SkASSERT(!"unsupported address mode");
    return nxt::AddressMode::ClampToEdge;

}

static nxt::PrimitiveTopology to_nxt_primitive_topology(GrPrimitiveType primitiveType) {
    switch (primitiveType) {
        case GrPrimitiveType::kTriangles:
            return nxt::PrimitiveTopology::TriangleList;
        case GrPrimitiveType::kTriangleStrip:
            return nxt::PrimitiveTopology::TriangleStrip;
        case GrPrimitiveType::kPoints:
            return nxt::PrimitiveTopology::PointList;
        case GrPrimitiveType::kLines:
            return nxt::PrimitiveTopology::LineList;
        case GrPrimitiveType::kLineStrip:
            return nxt::PrimitiveTopology::LineStrip;
        case GrPrimitiveType::kLinesAdjacency:
        case GrPrimitiveType::kTriangleFan:
        default:
            SkASSERT(!"unsupported primitive topology");
            return nxt::PrimitiveTopology::TriangleList;
    }
}

static nxt::VertexFormat to_nxt_vertex_format(GrVertexAttribType type) {
    switch (type) {
    case kFloat_GrVertexAttribType:
    case kHalf_GrVertexAttribType:
        return nxt::VertexFormat::FloatR32;
    case kFloat2_GrVertexAttribType:
    case kHalf2_GrVertexAttribType:
        return nxt::VertexFormat::FloatR32G32;
    case kFloat3_GrVertexAttribType:
    case kHalf3_GrVertexAttribType:
        return nxt::VertexFormat::FloatR32G32B32;
    case kFloat4_GrVertexAttribType:
    case kHalf4_GrVertexAttribType:
        return nxt::VertexFormat::FloatR32G32B32A32;
    case kUShort2_GrVertexAttribType:
        return nxt::VertexFormat::UshortR16G16;
    case kUByte4_norm_GrVertexAttribType:
        return nxt::VertexFormat::UnormR8G8B8A8;
    default:
        SkASSERT(!"unsupported vertex format");
        return nxt::VertexFormat::FloatR32G32B32A32;
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
    auto mask = blendInfo.fWriteColor ? nxt::ColorWriteMask::All : nxt::ColorWriteMask::None;

    return gpu->device().CreateBlendStateBuilder()
        .SetBlendEnabled(!blendOff)
        .SetAlphaBlend(operation, srcFactor, dstFactor)
        .SetColorBlend(operation, srcFactor, dstFactor)
        .SetColorWriteMask(mask)
        .GetResult();
}

void set_stencil_function(nxt::DepthStencilStateBuilder& builder, nxt::Face nxtFace, const GrStencilSettings::Face& face) {
     builder.SetStencilFunction(nxtFace,
                                to_nxt_compare_function(face.fTest),
                                to_nxt_stencil_operation(face.fFailOp),
                                to_nxt_stencil_operation(face.fFailOp),
                                to_nxt_stencil_operation(face.fPassOp));
}

nxt::DepthStencilState create_depth_stencil_state(const GrNXTGpu* gpu, const GrStencilSettings& stencilSettings) {
    const GrStencilSettings::Face& front = stencilSettings.front();
    auto readMask = front.fTestMask;
    auto writeMask = front.fWriteMask;
    auto builder = gpu->device().CreateDepthStencilStateBuilder();
    builder.SetDepthWriteEnabled(false)
           .SetStencilMask(readMask, writeMask);
    set_stencil_function(builder, nxt::Face::Front, stencilSettings.front());
    if (stencilSettings.isTwoSided()) {
        set_stencil_function(builder, nxt::Face::Back, stencilSettings.back());
    } else {
        set_stencil_function(builder, nxt::Face::Back, stencilSettings.front());
    }
    return builder.GetResult();
}

nxt::Sampler create_sampler(const GrNXTGpu* gpu, const GrSamplerState& samplerState) {
    auto filterMode = to_nxt_filter_mode(samplerState.filter());
    auto addressModeU = to_nxt_address_mode(samplerState.wrapModeX());
    auto addressModeV = to_nxt_address_mode(samplerState.wrapModeY());
    auto addressModeW = nxt::AddressMode::ClampToEdge;
    return gpu->device().CreateSamplerBuilder()
        .SetFilterMode(filterMode, filterMode, nxt::FilterMode::Linear)
        .SetAddressMode(addressModeU, addressModeV, addressModeW)
        .GetResult();
}

};

/////////////////////////////////////////////////////////////////////////////

sk_sp<GrNXTProgram> GrNXTProgramBuilder::Build(GrNXTGpu* gpu,
                                               const GrPipeline& pipeline,
                                               const GrPrimitiveProcessor& primProc,
                                               GrPrimitiveType primitiveType,
                                               nxt::RenderPass renderPass,
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
    auto vsModule = builder.CreateShaderModule(gpu->device(), builder.fVS,
                                               SkSL::Program::kVertex_Kind, &vertInputs);
    auto fsModule = builder.CreateShaderModule(gpu->device(), builder.fFS,
                                               SkSL::Program::kFragment_Kind, &fragInputs);
    result->fGeometryProcessor = std::move(builder.fGeometryProcessor);
    result->fXferProcessor = std::move(builder.fXferProcessor);
    result->fFragmentProcessors = builder.fFragmentProcessors;
    auto bindGroupLayoutBuilder = gpu->device().CreateBindGroupLayoutBuilder();
    if (0 != geometryUniformSize) {
        bindGroupLayoutBuilder.SetBindingsType(nxt::ShaderStageBit::Vertex, nxt::BindingType::UniformBuffer, GrNXTUniformHandler::kGeometryBinding, 1);
    }
    if (0 != fragmentUniformSize) {
        bindGroupLayoutBuilder.SetBindingsType(nxt::ShaderStageBit::Fragment, nxt::BindingType::UniformBuffer, GrNXTUniformHandler::kFragBinding, 1);
    }
    int binding = GrNXTUniformHandler::kSamplerBindingBase;
    for (int i = 0; i < primProc.numTextureSamplers(); ++i) {
        bindGroupLayoutBuilder.SetBindingsType(nxt::ShaderStageBit::Fragment,
                                               nxt::BindingType::Sampler,
                                               binding++,
                                               1);
        bindGroupLayoutBuilder.SetBindingsType(nxt::ShaderStageBit::Fragment,
                                               nxt::BindingType::SampledTexture,
                                               binding++,
                                               1);
    }
    GrFragmentProcessor::Iter iter(pipeline);
    const GrFragmentProcessor* fp = iter.next();
    int j = 0;
    while (fp) {
        for (int i = 0; i < fp->numTextureSamplers(); ++i) {
            bindGroupLayoutBuilder.SetBindingsType(nxt::ShaderStageBit::Fragment,
                                                   nxt::BindingType::Sampler,
                                                   binding++,
                                                   1);
            bindGroupLayoutBuilder.SetBindingsType(nxt::ShaderStageBit::Fragment,
                                                   nxt::BindingType::SampledTexture,
                                                   binding++,
                                                   1);
        }
        fp = iter.next();
        j++;
    }
    result->fBindGroupLayout = bindGroupLayoutBuilder.GetResult();
    auto pipelineLayout = gpu->device().CreatePipelineLayoutBuilder()
        .SetBindGroupLayout(0, result->fBindGroupLayout)
        .GetResult();
    result->fBuiltinUniformHandles = builder.fUniformHandles;
    auto blendState = create_blend_state(gpu, pipeline);
    nxt::DepthStencilState depthStencilState;
    if (pipeline.isStencilEnabled()) {
        GrStencilSettings stencil;
        int numStencilBits = 8; // FIXME
        stencil.reset(*pipeline.getUserStencil(), pipeline.hasStencilClip(), numStencilBits);
        depthStencilState = create_depth_stencil_state(gpu, stencil);
    }

    auto inputStateBuilder = gpu->device().CreateInputStateBuilder();
    int vertexBindingSlot = 0, instanceBindingSlot = 1;
    if (primProc.hasVertexAttribs()) {
        inputStateBuilder.SetInput(vertexBindingSlot, primProc.getVertexStride(), nxt::InputStepMode::Vertex);
    }
    if (primProc.hasInstanceAttribs()) {
        inputStateBuilder.SetInput(instanceBindingSlot, primProc.getVertexStride(), nxt::InputStepMode::Instance);
    }
    for (int i = 0; i < primProc.numAttribs(); i++) {
        const GrGeometryProcessor::Attribute& attrib = primProc.getAttrib(i);
        int input = attrib.fInputRate == GrPrimitiveProcessor::Attribute::InputRate::kPerVertex ? vertexBindingSlot : instanceBindingSlot;
        inputStateBuilder
            .SetAttribute(i, input, to_nxt_vertex_format(attrib.fType), attrib.fOffsetInRecord);
    }
    auto inputState = inputStateBuilder.GetResult();

    result->fRenderPipeline = gpu->device().CreateRenderPipelineBuilder()
        .SetSubpass(renderPass, 0)
        .SetLayout(pipelineLayout)
        .SetStage(nxt::ShaderStage::Vertex, vsModule, "main")
        .SetStage(nxt::ShaderStage::Fragment, fsModule, "main")
        .SetIndexFormat(nxt::IndexFormat::Uint16)
        .SetInputState(inputState)
        .SetColorAttachmentBlendState(0, blendState)
        .SetDepthStencilState(depthStencilState)
        .SetPrimitiveTopology(to_nxt_primitive_topology(primitiveType))
        .GetResult();
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

nxt::BindGroup GrNXTProgram::setData(GrNXTGpu* gpu, const GrPrimitiveProcessor& primProc, const GrPipeline& pipeline, nxt::CommandBufferBuilder cmdBuilder) {
    auto bindGroupBuilder = gpu->device().CreateBindGroupBuilder();
    bindGroupBuilder.SetLayout(fBindGroupLayout)
                    .SetUsage(nxt::BindGroupUsage::Frozen);
    GrNXTRingBuffer::Slice geom, frag;
    uint32_t geometryUniformSize = fDataManager.geometryUniformSize();
    uint32_t fragmentUniformSize = fDataManager.fragmentUniformSize();
    if (0 != geometryUniformSize) {
        geom = gpu->allocateUniformRingBufferSlice(geometryUniformSize);
        auto view = geom.fBuffer.CreateBufferViewBuilder()
            .SetExtent(geom.fOffset, geometryUniformSize)
            .GetResult();
        bindGroupBuilder.SetBufferViews(GrNXTUniformHandler::kGeometryBinding, 1, &view);
    }
    if (0 != fragmentUniformSize) {
        frag = gpu->allocateUniformRingBufferSlice(fragmentUniformSize);
        auto view = frag.fBuffer.CreateBufferViewBuilder()
            .SetExtent(frag.fOffset, fragmentUniformSize)
            .GetResult();
        bindGroupBuilder.SetBufferViews(GrNXTUniformHandler::kFragBinding, 1, &view);
    }
    this->setRenderTargetState(pipeline.proxy());
    fGeometryProcessor->setData(fDataManager, primProc,
                                GrFragmentProcessor::CoordTransformIter(pipeline));
    int binding = GrNXTUniformHandler::kSamplerBindingBase;
    for (int i = 0; i < primProc.numTextureSamplers(); ++i) {
        // FIXME: could probably cache samplers in GrNXTProgram
        nxt::Sampler sampler = create_sampler(gpu, primProc.textureSampler(i).samplerState());
        bindGroupBuilder.SetSamplers(binding++, 1, &sampler);
        GrNXTTexture* tex = static_cast<GrNXTTexture*>(primProc.textureSampler(i).peekTexture());
        nxt::TextureView textureView = tex->textureView();
        bindGroupBuilder.SetTextureViews(binding++, 1, &textureView);
    }
    GrFragmentProcessor::Iter iter(pipeline);
    GrGLSLFragmentProcessor::Iter glslIter(fFragmentProcessors.begin(),
                                           fFragmentProcessors.count());
    const GrFragmentProcessor* fp = iter.next();
    GrGLSLFragmentProcessor* glslFP = glslIter.next();
    while (fp && glslFP) {
        glslFP->setData(fDataManager, *fp);
        for (int i = 0; i < fp->numTextureSamplers(); ++i) {
            // FIXME: could probably cache samplers in GrNXTProgram
            nxt::Sampler sampler = create_sampler(gpu, fp->textureSampler(i).samplerState());
            bindGroupBuilder.SetSamplers(binding++, 1, &sampler);
            GrNXTTexture* tex = static_cast<GrNXTTexture*>(fp->textureSampler(i).peekTexture());
            nxt::TextureView textureView = tex->textureView();
            bindGroupBuilder.SetTextureViews(binding++, 1, &textureView);
        }
        fp = iter.next();
        glslFP = glslIter.next();
    }
    fDataManager.uploadUniformBuffers(geom, frag);
    if (0 != geometryUniformSize) {
        cmdBuilder.TransitionBufferUsage(geom.fBuffer, nxt::BufferUsageBit::Uniform);
    }
    if (0 != fragmentUniformSize) {
        cmdBuilder.TransitionBufferUsage(frag.fBuffer, nxt::BufferUsageBit::Uniform);
    }
    return bindGroupBuilder.GetResult();
}
