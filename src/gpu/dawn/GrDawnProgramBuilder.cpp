/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDawnProgramBuilder.h"

#include "dawn/GrDawnGpu.h"
#include "dawn/GrDawnTexture.h"
#include "GrRenderTarget.h"
#include "GrStencilSettings.h"
#include "SkSLCompiler.h"

#include "GrSKSLPrettyPrint.h"

const int kMaxBindGroupCacheEntries = 4096;

namespace {

SkSL::String sksl_to_spirv(const GrDawnGpu* gpu, const char* shaderString, SkSL::Program::Kind kind, SkSL::Program::Inputs* inputs) {
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
        SkASSERT(false);
        return "";
    }
    return code;
}

dawn::BlendFactor to_dawn_blend_factor(GrBlendCoeff coeff) {
    switch (coeff) {
    case kZero_GrBlendCoeff:
        return dawn::BlendFactor::Zero;
    case kOne_GrBlendCoeff:
        return dawn::BlendFactor::One;
    case kSC_GrBlendCoeff:
        return dawn::BlendFactor::SrcColor;
    case kISC_GrBlendCoeff:
        return dawn::BlendFactor::OneMinusSrcColor;
    case kDC_GrBlendCoeff:
        return dawn::BlendFactor::DstColor;
    case kIDC_GrBlendCoeff:
        return dawn::BlendFactor::OneMinusDstColor;
    case kSA_GrBlendCoeff:
        return dawn::BlendFactor::SrcAlpha;
    case kISA_GrBlendCoeff:
        return dawn::BlendFactor::OneMinusSrcAlpha;
    case kDA_GrBlendCoeff:
        return dawn::BlendFactor::DstAlpha;
    case kIDA_GrBlendCoeff:
        return dawn::BlendFactor::OneMinusDstAlpha;
    case kConstC_GrBlendCoeff:
        return dawn::BlendFactor::BlendColor;
    case kIConstC_GrBlendCoeff:
        return dawn::BlendFactor::OneMinusBlendColor;
    case kConstA_GrBlendCoeff:
    case kIConstA_GrBlendCoeff:
    case kS2C_GrBlendCoeff:
    case kIS2C_GrBlendCoeff:
    case kS2A_GrBlendCoeff:
    case kIS2A_GrBlendCoeff:
    default:
        SkASSERT(!"unsupported blend coefficient");
        return dawn::BlendFactor::One;
    }
}

dawn::BlendFactor to_dawn_blend_factor_for_alpha(GrBlendCoeff coeff) {
    switch (coeff) {
    // Force all srcColor used in alpha slot to alpha version.
    case kSC_GrBlendCoeff:
        return dawn::BlendFactor::SrcAlpha;
    case kISC_GrBlendCoeff:
        return dawn::BlendFactor::OneMinusSrcAlpha;
    case kDC_GrBlendCoeff:
        return dawn::BlendFactor::DstAlpha;
    case kIDC_GrBlendCoeff:
        return dawn::BlendFactor::OneMinusDstAlpha;
    default:
        return to_dawn_blend_factor(coeff);
    }
}

dawn::BlendOperation to_dawn_blend_operation(GrBlendEquation equation) {
    switch (equation) {
    case kAdd_GrBlendEquation:
        return dawn::BlendOperation::Add;
    case kSubtract_GrBlendEquation:
        return dawn::BlendOperation::Subtract;
    case kReverseSubtract_GrBlendEquation:
        return dawn::BlendOperation::ReverseSubtract;
    default:
        SkASSERT(!"unsupported blend equation");
        return dawn::BlendOperation::Add;
    }
}

dawn::CompareFunction to_dawn_compare_function(GrStencilTest test) {
    switch (test) {
        case GrStencilTest::kAlways:
            return dawn::CompareFunction::Always;
        case GrStencilTest::kNever:
            return dawn::CompareFunction::Never;
        case GrStencilTest::kGreater:
            return dawn::CompareFunction::Greater;
        case GrStencilTest::kGEqual:
            return dawn::CompareFunction::GreaterEqual;
        case GrStencilTest::kLess:
            return dawn::CompareFunction::Less;
        case GrStencilTest::kLEqual:
            return dawn::CompareFunction::LessEqual;
        case GrStencilTest::kEqual:
            return dawn::CompareFunction::Equal;
        case GrStencilTest::kNotEqual:
            return dawn::CompareFunction::NotEqual;
        default:
            SkASSERT(!"unsupported stencil test");
            return dawn::CompareFunction::Always;
    }
}

dawn::StencilOperation to_dawn_stencil_operation(GrStencilOp op) {
    switch (op) {
        case GrStencilOp::kKeep:
            return dawn::StencilOperation::Keep;
        case GrStencilOp::kZero:
            return dawn::StencilOperation::Zero;
        case GrStencilOp::kReplace:
            return dawn::StencilOperation::Replace;
        case GrStencilOp::kInvert:
            return dawn::StencilOperation::Invert;
        case GrStencilOp::kIncClamp:
            return dawn::StencilOperation::IncrementClamp;
        case GrStencilOp::kDecClamp:
            return dawn::StencilOperation::DecrementClamp;
        case GrStencilOp::kIncWrap:
            return dawn::StencilOperation::IncrementWrap;
        case GrStencilOp::kDecWrap:
            return dawn::StencilOperation::DecrementWrap;
        default:
            SkASSERT(!"unsupported stencil function");
            return dawn::StencilOperation::Keep;
    }
}

static dawn::PrimitiveTopology to_dawn_primitive_topology(GrPrimitiveType primitiveType) {
    switch (primitiveType) {
        case GrPrimitiveType::kTriangles:
            return dawn::PrimitiveTopology::TriangleList;
        case GrPrimitiveType::kTriangleStrip:
            return dawn::PrimitiveTopology::TriangleStrip;
        case GrPrimitiveType::kPoints:
            return dawn::PrimitiveTopology::PointList;
        case GrPrimitiveType::kLines:
            return dawn::PrimitiveTopology::LineList;
        case GrPrimitiveType::kLineStrip:
            return dawn::PrimitiveTopology::LineStrip;
        case GrPrimitiveType::kLinesAdjacency:
        default:
            SkASSERT(!"unsupported primitive topology");
            return dawn::PrimitiveTopology::TriangleList;
    }
}

static dawn::VertexFormat to_dawn_vertex_format(GrVertexAttribType type) {
    switch (type) {
    case kFloat_GrVertexAttribType:
    case kHalf_GrVertexAttribType:
        return dawn::VertexFormat::FloatR32;
    case kFloat2_GrVertexAttribType:
    case kHalf2_GrVertexAttribType:
        return dawn::VertexFormat::FloatR32G32;
    case kFloat3_GrVertexAttribType:
    case kHalf3_GrVertexAttribType:
        return dawn::VertexFormat::FloatR32G32B32;
    case kFloat4_GrVertexAttribType:
    case kHalf4_GrVertexAttribType:
        return dawn::VertexFormat::FloatR32G32B32A32;
    case kUShort2_GrVertexAttribType:
        return dawn::VertexFormat::UshortR16G16;
    case kInt_GrVertexAttribType:
        return dawn::VertexFormat::IntR32;
    case kUByte4_norm_GrVertexAttribType:
        return dawn::VertexFormat::UnormR8G8B8A8;
    default:
        SkASSERT(!"unsupported vertex format");
        return dawn::VertexFormat::FloatR32G32B32A32;
    }
}

dawn::BlendState create_blend_state(const GrDawnGpu* gpu, const GrPipeline& pipeline) {
    GrXferProcessor::BlendInfo blendInfo;
    pipeline.getXferProcessor().getBlendInfo(&blendInfo);
    GrBlendEquation equation = blendInfo.fEquation;
    GrBlendCoeff srcCoeff = blendInfo.fSrcBlend;
    GrBlendCoeff dstCoeff = blendInfo.fDstBlend;
    bool blendOff = (kAdd_GrBlendEquation == equation || kSubtract_GrBlendEquation == equation) &&
                    kOne_GrBlendCoeff == srcCoeff && kZero_GrBlendCoeff == dstCoeff;

    dawn::BlendFactor srcFactor = to_dawn_blend_factor(srcCoeff);
    dawn::BlendFactor dstFactor = to_dawn_blend_factor(dstCoeff);
    dawn::BlendFactor srcFactorAlpha = to_dawn_blend_factor_for_alpha(srcCoeff);
    dawn::BlendFactor dstFactorAlpha = to_dawn_blend_factor_for_alpha(dstCoeff);
    dawn::BlendOperation operation = to_dawn_blend_operation(equation);
    auto mask = blendInfo.fWriteColor ? dawn::ColorWriteMask::All : dawn::ColorWriteMask::None;

    dawn::BlendDescriptor colorDesc = {operation, srcFactor, dstFactor};
    dawn::BlendDescriptor alphaDesc = {operation, srcFactorAlpha, dstFactorAlpha};

    return gpu->device().CreateBlendStateBuilder()
        .SetBlendEnabled(!blendOff)
        .SetColorBlend(&colorDesc)
        .SetAlphaBlend(&alphaDesc)
        .SetColorWriteMask(mask)
        .GetResult();
}

void set_stencil_function(dawn::DepthStencilStateBuilder& builder, dawn::Face dawnFace, const GrStencilSettings::Face& face) {
     dawn::StencilStateFaceDescriptor desc;
     desc.compare = to_dawn_compare_function(face.fTest);
     desc.stencilFailOp = desc.depthFailOp = to_dawn_stencil_operation(face.fFailOp);
     desc.passOp = to_dawn_stencil_operation(face.fPassOp);
     builder.SetStencilFunction(dawnFace, &desc);
}

dawn::DepthStencilState create_depth_stencil_state(const GrDawnGpu* gpu, const GrStencilSettings& stencilSettings) {
    const GrStencilSettings::Face& front = stencilSettings.front();
    auto readMask = front.fTestMask;
    auto writeMask = front.fWriteMask;
    auto builder = gpu->device().CreateDepthStencilStateBuilder();
    builder.SetDepthWriteEnabled(false)
           .SetStencilMask(readMask, writeMask);
    set_stencil_function(builder, dawn::Face::Front, stencilSettings.front());
    if (stencilSettings.isTwoSided()) {
        set_stencil_function(builder, dawn::Face::Back, stencilSettings.back());
    } else {
        set_stencil_function(builder, dawn::Face::Back, stencilSettings.front());
    }
    return builder.GetResult();
}

};

/////////////////////////////////////////////////////////////////////////////

static dawn::BindGroupBinding MakeBindGroupBinding(uint32_t binding, const dawn::Buffer& buffer, uint32_t offset, uint32_t size, const dawn::Sampler& sampler, const dawn::TextureView& textureView) {
    dawn::BindGroupBinding result;
    result.binding = binding;
    result.buffer = buffer;
    result.offset = offset;
    result.size = size;
    result.sampler = sampler;
    result.textureView = textureView;
    return result;
}

static dawn::BindGroupBinding MakeBindGroupBinding(uint32_t binding, const dawn::Buffer& buffer, uint32_t offset, uint32_t size) {
    return MakeBindGroupBinding(binding, buffer, offset, size, nullptr, nullptr);
}

static dawn::BindGroupBinding MakeBindGroupBinding(uint32_t binding, const dawn::Sampler& sampler) {
    return MakeBindGroupBinding(binding, nullptr, 0, 0, sampler, nullptr);
}

static dawn::BindGroupBinding MakeBindGroupBinding(uint32_t binding, const dawn::TextureView& textureView) {
    return MakeBindGroupBinding(binding, nullptr, 0, 0, nullptr, textureView);
}

sk_sp<GrDawnProgram> GrDawnProgramBuilder::Build(GrDawnGpu* gpu,
                                               const GrPipeline& pipeline,
                                               const GrPrimitiveProcessor& primProc,
                                               const GrTextureProxy* const primProcProxies[],
                                               GrPrimitiveType primitiveType,
                                               dawn::TextureFormat colorFormat,
                                               bool hasDepthStencil,
                                               dawn::TextureFormat depthStencilFormat,
                                               GrProgramDesc* desc) {
    GrDawnProgramBuilder builder(gpu, primProc, primProcProxies, pipeline, desc);
    if (!builder.emitAndInstallProcs()) {
        return nullptr;
    }

    builder.fVS.extensions().appendf("#extension GL_ARB_separate_shader_objects : enable\n");
    builder.fFS.extensions().appendf("#extension GL_ARB_separate_shader_objects : enable\n");
    builder.fVS.extensions().appendf("#extension GL_ARB_shading_language_420pack : enable\n");
    builder.fFS.extensions().appendf("#extension GL_ARB_shading_language_420pack : enable\n");

    builder.finalizeShaders();

    SkSL::Program::Inputs vertInputs, fragInputs;
    GrDawnUniformHandler::UniformInfoArray& uniforms = builder.fUniformHandler.fUniforms;
    uint32_t geometryUniformSize = builder.fUniformHandler.fCurrentGeometryUBOOffset;
    uint32_t fragmentUniformSize = builder.fUniformHandler.fCurrentFragmentUBOOffset;
    sk_sp<GrDawnProgram> result(
        new GrDawnProgram(uniforms, geometryUniformSize, fragmentUniformSize));
    auto vsModule = builder.CreateShaderModule(gpu->device(), builder.fVS,
                                               SkSL::Program::kVertex_Kind, &vertInputs);
    auto fsModule = builder.CreateShaderModule(gpu->device(), builder.fFS,
                                               SkSL::Program::kFragment_Kind, &fragInputs);
    result->fGeometryProcessor = std::move(builder.fGeometryProcessor);
    result->fXferProcessor = std::move(builder.fXferProcessor);
    result->fFragmentProcessors = std::move(builder.fFragmentProcessors);
    result->fFragmentProcessorCnt = builder.fFragmentProcessorCnt;
    std::vector<dawn::BindGroupLayoutBinding> layoutBindings;
    if (0 != geometryUniformSize) {
        layoutBindings.push_back({ GrDawnUniformHandler::kGeometryBinding, dawn::ShaderStageBit::Vertex, dawn::BindingType::UniformBuffer});
    }
    if (0 != fragmentUniformSize) {
        layoutBindings.push_back({ GrDawnUniformHandler::kFragBinding, dawn::ShaderStageBit::Fragment, dawn::BindingType::UniformBuffer});
    }
    uint32_t binding = GrDawnUniformHandler::kSamplerBindingBase;
    for (int i = 0; i < builder.fUniformHandler.fSamplers.count(); ++i) {
        layoutBindings.push_back({ binding++, dawn::ShaderStageBit::Fragment, dawn::BindingType::Sampler});
        layoutBindings.push_back({ binding++, dawn::ShaderStageBit::Fragment, dawn::BindingType::SampledTexture});
    }
    dawn::BindGroupLayoutDescriptor bindGroupLayoutDesc;
    bindGroupLayoutDesc.numBindings = layoutBindings.size();
    bindGroupLayoutDesc.bindings = layoutBindings.data();
    result->fBindGroupLayout = gpu->device().CreateBindGroupLayout(&bindGroupLayoutDesc);
    dawn::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.numBindGroupLayouts = 1;
    pipelineLayoutDesc.bindGroupLayouts = &result->fBindGroupLayout;
    auto pipelineLayout = gpu->device().CreatePipelineLayout(&pipelineLayoutDesc);
    result->fBuiltinUniformHandles = builder.fUniformHandles;
    auto blendState = create_blend_state(gpu, pipeline);
    dawn::DepthStencilState depthStencilState;
    if (pipeline.isStencilEnabled()) {
        GrStencilSettings stencil;
        int numStencilBits = 8; // FIXME
        stencil.reset(*pipeline.getUserStencil(), pipeline.hasStencilClip(), numStencilBits);
        depthStencilState = create_depth_stencil_state(gpu, stencil);
    } else {
        depthStencilState = gpu->device().CreateDepthStencilStateBuilder().GetResult();
    }

    auto inputStateBuilder = gpu->device().CreateInputStateBuilder();

    // FIXME: can we assign these dynamically instead?
    int vertexBindingSlot = 0, instanceBindingSlot = 1;
    if (primProc.numVertexAttributes() > 0) {
        size_t offset = 0;
        int i = 0;
        for (const auto& attrib : primProc.vertexAttributes()) {
            inputStateBuilder
                .SetAttribute(i, vertexBindingSlot, to_dawn_vertex_format(attrib.cpuType()), offset);
            offset += attrib.sizeAlign4();
            i++;
        }
        result->fVertexStride = offset;
        inputStateBuilder.SetInput(vertexBindingSlot, offset, dawn::InputStepMode::Vertex);
    }
    if (primProc.numInstanceAttributes() > 0) {
        size_t offset = 0;
        int i = 0;
        for (const auto& attrib : primProc.instanceAttributes()) {
            inputStateBuilder
                .SetAttribute(i, instanceBindingSlot, to_dawn_vertex_format(attrib.cpuType()), offset);
            i++;
        }
        inputStateBuilder.SetInput(instanceBindingSlot, offset, dawn::InputStepMode::Instance);
    }
    auto inputState = inputStateBuilder.GetResult();

    dawn::PipelineStageDescriptor vsDesc;
    vsDesc.module = vsModule;
    vsDesc.entryPoint = "main";

    dawn::PipelineStageDescriptor fsDesc;
    fsDesc.module = fsModule;
    fsDesc.entryPoint = "main";

    dawn::AttachmentDescriptor colorAttachmentDesc;
    colorAttachmentDesc.format = colorFormat;

    dawn::AttachmentDescriptor depthStencilAttachmentDesc;
    depthStencilAttachmentDesc.format = depthStencilFormat;

    dawn::AttachmentsStateDescriptor attachmentsDesc;
    attachmentsDesc.numColorAttachments = 1;
    attachmentsDesc.colorAttachments = &colorAttachmentDesc;
    attachmentsDesc.hasDepthStencilAttachment = hasDepthStencil;
    attachmentsDesc.depthStencilAttachment = &depthStencilAttachmentDesc;

    dawn::RenderPipelineDescriptor rpDesc;
    rpDesc.layout = pipelineLayout;
    rpDesc.vertexStage = &vsDesc;
    rpDesc.fragmentStage = &fsDesc;
    rpDesc.inputState = inputState;
    rpDesc.indexFormat = dawn::IndexFormat::Uint16;
    rpDesc.primitiveTopology = to_dawn_primitive_topology(primitiveType);
    rpDesc.attachmentsState = &attachmentsDesc;
    rpDesc.sampleCount = 1;
    rpDesc.depthStencilState = depthStencilState;
    rpDesc.numBlendStates = 1;
    rpDesc.blendStates = &blendState;
    result->fRenderPipeline = gpu->device().CreateRenderPipeline(&rpDesc);
    return result;
}

GrDawnProgramBuilder::GrDawnProgramBuilder(GrDawnGpu* gpu,
                                         const GrPrimitiveProcessor& primProc,
                                         const GrTextureProxy* const primProcProxies[],
                                         const GrPipeline& pipeline,
                                         GrProgramDesc* desc)
    : INHERITED(primProc, primProcProxies, pipeline, desc)
    , fGpu(gpu)
    , fVaryingHandler(this)
    , fUniformHandler(this) {
}

dawn::ShaderModule GrDawnProgramBuilder::CreateShaderModule(dawn::Device device, const GrGLSLShaderBuilder& builder, SkSL::Program::Kind kind, SkSL::Program::Inputs* inputs) {
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

    dawn::ShaderModuleDescriptor desc;
    desc.codeSize = spirvSource.size() / 4;
    desc.code = reinterpret_cast<const uint32_t*>(spirvSource.c_str());

    return device.CreateShaderModule(&desc);
};

const GrCaps* GrDawnProgramBuilder::caps() const {
    return fGpu->caps();
}

void GrDawnProgram::setRenderTargetState(const GrRenderTargetProxy* proxy) {
    GrRenderTarget* rt = proxy->peekRenderTarget();

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

static void setTexture(GrDawnGpu* gpu, const GrSamplerState& state, GrTexture* texture, std::vector<dawn::BindGroupBinding> *bindings, int* binding) {
    // FIXME: could probably cache samplers in GrDawnProgram
    dawn::Sampler sampler = gpu->getOrCreateSampler(state);
    bindings->push_back(MakeBindGroupBinding((*binding)++, sampler));
    GrDawnTexture* tex = static_cast<GrDawnTexture*>(texture);
    dawn::TextureView textureView = tex->textureView();
    bindings->push_back(MakeBindGroupBinding((*binding)++, textureView));
}

static void appendTextureSamplerKey(GrDawnProgram::BindGroupKey* key,
                                    GrTexture* texture,
                                    GrSamplerState samplerState) {
    GrDawnTexture* tex = static_cast<GrDawnTexture*>(texture);
    key->append(sizeof(tex), &tex);
    key->append(sizeof(samplerState), &samplerState);
}

GrDawnProgram::GrDawnProgram(const GrDawnUniformHandler::UniformInfoArray& uniforms,
                           uint32_t geometryUniformSize,
                           uint32_t fragmentUniformSize)
    : fDataManager(uniforms, geometryUniformSize, fragmentUniformSize)
    , fBindGroupCache(kMaxBindGroupCacheEntries) {
}

void GrDawnProgram::buildKey(BindGroupKey* key, const GrPrimitiveProcessor& primProc, const GrPipeline& pipeline, const GrTextureProxy* const primProcTextures[]) {
    key->append(fDataManager.geometryUniformSize(), fDataManager.geometryUniformData());
    key->append(fDataManager.fragmentUniformSize(), fDataManager.fragmentUniformData());
    for (int i = 0; i < primProc.numTextureSamplers(); ++i) {
        const GrPrimitiveProcessor::TextureSampler& sampler = primProc.textureSampler(i);
        appendTextureSamplerKey(key, primProcTextures[i]->peekTexture(), sampler.samplerState());
    }
    GrFragmentProcessor::Iter iter(pipeline);
    GrGLSLFragmentProcessor::Iter glslIter(fFragmentProcessors.get(), fFragmentProcessorCnt);
    const GrFragmentProcessor* fp = iter.next();
    GrGLSLFragmentProcessor* glslFP = glslIter.next();
    while (fp && glslFP) {
        glslFP->setData(fDataManager, *fp);
        for (int i = 0; i < fp->numTextureSamplers(); ++i) {
            const GrFragmentProcessor::TextureSampler& sampler = fp->textureSampler(i);
            appendTextureSamplerKey(key, sampler.peekTexture(), sampler.samplerState());
        }
        fp = iter.next();
        glslFP = glslIter.next();
    }
    if (GrTextureProxy* proxy = pipeline.dstTextureProxy()) {
        GrFragmentProcessor::TextureSampler sampler(sk_ref_sp(proxy));
        appendTextureSamplerKey(key, sampler.peekTexture(), sampler.samplerState());
    }
}

dawn::BindGroup GrDawnProgram::setData(GrDawnGpu* gpu, const GrPrimitiveProcessor& primProc, const GrPipeline& pipeline, const GrTextureProxy* const primProcTextures[]) {
    this->setRenderTargetState(pipeline.proxy());
    fGeometryProcessor->setData(fDataManager, primProc,
                                GrFragmentProcessor::CoordTransformIter(pipeline));
    SkIPoint offset;
    GrTexture* dstTexture = pipeline.peekDstTexture(&offset);
    fXferProcessor->setData(fDataManager, pipeline.getXferProcessor(), dstTexture, offset);
    BindGroupKey key;
    this->buildKey(&key, primProc, pipeline, primProcTextures);
    if (BindGroupValue* value = fBindGroupCache.find(key)) {
        return value->fBindGroup;
    }
    std::vector<dawn::BindGroupBinding> bindings;
    GrDawnRingBuffer::Slice geom, frag;
    uint32_t geometryUniformSize = fDataManager.geometryUniformSize();
    uint32_t fragmentUniformSize = fDataManager.fragmentUniformSize();
    BindGroupValue value;
    if (0 != geometryUniformSize) {
        geom = gpu->allocateUniformRingBufferSlice(geometryUniformSize);
        bindings.push_back(MakeBindGroupBinding(GrDawnUniformHandler::kGeometryBinding, geom.fBuffer, geom.fOffset, geometryUniformSize));
        value.fGeometryBuffer = geom.fBuffer;
    }
    if (0 != fragmentUniformSize) {
        frag = gpu->allocateUniformRingBufferSlice(fragmentUniformSize);
        bindings.push_back(MakeBindGroupBinding(GrDawnUniformHandler::kFragBinding, frag.fBuffer, frag.fOffset, fragmentUniformSize));
        value.fFragmentBuffer = frag.fBuffer;
    }
    int binding = GrDawnUniformHandler::kSamplerBindingBase;
    for (int i = 0; i < primProc.numTextureSamplers(); ++i) {
        auto& sampler = primProc.textureSampler(i);
        setTexture(gpu, sampler.samplerState(), primProcTextures[i]->peekTexture(), &bindings, &binding);
    }
    GrFragmentProcessor::Iter iter(pipeline);
    GrGLSLFragmentProcessor::Iter glslIter(fFragmentProcessors.get(), fFragmentProcessorCnt);
    const GrFragmentProcessor* fp = iter.next();
    GrGLSLFragmentProcessor* glslFP = glslIter.next();
    while (fp && glslFP) {
        for (int i = 0; i < fp->numTextureSamplers(); ++i) {
            auto& s = fp->textureSampler(i);
            setTexture(gpu, s.samplerState(), s.peekTexture(), &bindings, &binding);
        }
        fp = iter.next();
        glslFP = glslIter.next();
    }
    if (GrTextureProxy* proxy = pipeline.dstTextureProxy()) {
        GrFragmentProcessor::TextureSampler sampler(sk_ref_sp(proxy));
        setTexture(gpu, sampler.samplerState(), sampler.peekTexture(), &bindings, &binding);
    }
    fDataManager.uploadUniformBuffers(gpu, geom, frag);
    dawn::BindGroupDescriptor descriptor;
    descriptor.layout = fBindGroupLayout;
    descriptor.numBindings = bindings.size();
    descriptor.bindings = bindings.data();
    value.fBindGroup = gpu->device().CreateBindGroup(&descriptor);
    fBindGroupCache.insert(key, value);
    return value.fBindGroup;
}
