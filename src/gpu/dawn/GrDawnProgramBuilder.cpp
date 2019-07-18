/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDawnProgramBuilder.h"

#include "src/gpu/dawn/GrDawnGpu.h"
#include "src/gpu/dawn/GrDawnTexture.h"
#include "include/gpu/GrRenderTarget.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/sksl/SkSLCompiler.h"

#include "src/gpu/GrShaderUtils.h"

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
        return dawn::VertexFormat::Float;
    case kFloat2_GrVertexAttribType:
    case kHalf2_GrVertexAttribType:
        return dawn::VertexFormat::Float2;
    case kFloat3_GrVertexAttribType:
    case kHalf3_GrVertexAttribType:
        return dawn::VertexFormat::Float3;
    case kFloat4_GrVertexAttribType:
    case kHalf4_GrVertexAttribType:
        return dawn::VertexFormat::Float4;
    case kUShort2_GrVertexAttribType:
        return dawn::VertexFormat::UShort2;
    case kInt_GrVertexAttribType:
        return dawn::VertexFormat::Int;
    case kUByte4_norm_GrVertexAttribType:
        return dawn::VertexFormat::UChar4Norm;
    default:
        SkASSERT(!"unsupported vertex format");
        return dawn::VertexFormat::Float4;
    }
}

dawn::ColorStateDescriptor create_color_state(const GrDawnGpu* gpu, const GrPipeline& pipeline,
                                              dawn::TextureFormat colorFormat) {
    GrXferProcessor::BlendInfo blendInfo;
    pipeline.getXferProcessor().getBlendInfo(&blendInfo);
    GrBlendEquation equation = blendInfo.fEquation;
    GrBlendCoeff srcCoeff = blendInfo.fSrcBlend;
    GrBlendCoeff dstCoeff = blendInfo.fDstBlend;

    dawn::BlendFactor srcFactor = to_dawn_blend_factor(srcCoeff);
    dawn::BlendFactor dstFactor = to_dawn_blend_factor(dstCoeff);
    dawn::BlendFactor srcFactorAlpha = to_dawn_blend_factor_for_alpha(srcCoeff);
    dawn::BlendFactor dstFactorAlpha = to_dawn_blend_factor_for_alpha(dstCoeff);
    dawn::BlendOperation operation = to_dawn_blend_operation(equation);
    auto mask = blendInfo.fWriteColor ? dawn::ColorWriteMask::All : dawn::ColorWriteMask::None;

    dawn::BlendDescriptor colorDesc = {operation, srcFactor, dstFactor};
    dawn::BlendDescriptor alphaDesc = {operation, srcFactorAlpha, dstFactorAlpha};

    dawn::ColorStateDescriptor descriptor;
    descriptor.format = colorFormat;
    descriptor.alphaBlend = alphaDesc;
    descriptor.colorBlend = colorDesc;
    descriptor.nextInChain = nullptr;
    descriptor.writeMask = mask;

    return descriptor;
}

dawn::StencilStateFaceDescriptor to_stencil_state_face(const GrStencilSettings::Face& face) {
     dawn::StencilStateFaceDescriptor desc;
     desc.compare = to_dawn_compare_function(face.fTest);
     desc.failOp = desc.depthFailOp = to_dawn_stencil_operation(face.fFailOp);
     desc.passOp = to_dawn_stencil_operation(face.fPassOp);
     return desc;
}

dawn::DepthStencilStateDescriptor create_depth_stencil_state(const GrStencilSettings& stencilSettings, dawn::TextureFormat depthStencilFormat, GrSurfaceOrigin origin) {
    dawn::DepthStencilStateDescriptor state;
    state.format = depthStencilFormat;
    state.depthWriteEnabled = false;
    state.depthCompare = dawn::CompareFunction::Always;
    if (stencilSettings.isDisabled()) {
        dawn::StencilStateFaceDescriptor stencilFace;
        stencilFace.compare = dawn::CompareFunction::Always;
        stencilFace.failOp = dawn::StencilOperation::Keep;
        stencilFace.depthFailOp = dawn::StencilOperation::Keep;
        stencilFace.passOp = dawn::StencilOperation::Keep;
        state.stencilReadMask = state.stencilWriteMask = 0x0;
        state.stencilBack = state.stencilFront = stencilFace;
    } else {
        const GrStencilSettings::Face& front = stencilSettings.front(origin);
        state.stencilReadMask = front.fTestMask;
        state.stencilWriteMask = front.fWriteMask;
        state.stencilFront = to_stencil_state_face(stencilSettings.front(origin));
        if (stencilSettings.isTwoSided()) {
            state.stencilBack = to_stencil_state_face(stencilSettings.back(origin));
        } else {
            state.stencilBack = state.stencilFront;
        }
    }
    return state;
}

};

/////////////////////////////////////////////////////////////////////////////

static dawn::BindGroupBinding MakeBindGroupBinding(uint32_t binding, const dawn::Buffer& buffer,
                                                   uint32_t offset, uint32_t size, const
                                                   dawn::Sampler& sampler,
                                                   const dawn::TextureView& textureView) {
    dawn::BindGroupBinding result;
    result.binding = binding;
    result.buffer = buffer;
    result.offset = offset;
    result.size = size;
    result.sampler = sampler;
    result.textureView = textureView;
    return result;
}

static dawn::BindGroupBinding MakeBindGroupBinding(uint32_t binding, const dawn::Buffer& buffer,
                                                   uint32_t offset, uint32_t size) {
    return MakeBindGroupBinding(binding, buffer, offset, size, nullptr, nullptr);
}

static dawn::BindGroupBinding MakeBindGroupBinding(uint32_t binding, const dawn::Sampler& sampler) {
    return MakeBindGroupBinding(binding, nullptr, 0, 0, sampler, nullptr);
}

static dawn::BindGroupBinding MakeBindGroupBinding(uint32_t binding, const dawn::TextureView& textureView) {
    return MakeBindGroupBinding(binding, nullptr, 0, 0, nullptr, textureView);
}

sk_sp<GrDawnProgram> GrDawnProgramBuilder::Build(GrDawnGpu* gpu,
                                                 GrRenderTarget* renderTarget,
                                                 GrSurfaceOrigin origin,
                                                 const GrPipeline& pipeline,
                                                 const GrPrimitiveProcessor& primProc,
                                                 const GrTextureProxy* const primProcProxies[],
                                                 GrPrimitiveType primitiveType,
                                                 dawn::TextureFormat colorFormat,
                                                 bool hasDepthStencil,
                                                 dawn::TextureFormat depthStencilFormat,
                                                 GrProgramDesc* desc) {
    GrDawnProgramBuilder builder(gpu, renderTarget, origin, primProc, primProcProxies, pipeline, desc);
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
    bindGroupLayoutDesc.bindingCount = layoutBindings.size();
    bindGroupLayoutDesc.bindings = layoutBindings.data();
    result->fBindGroupLayout = gpu->device().CreateBindGroupLayout(&bindGroupLayoutDesc);
    dawn::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = &result->fBindGroupLayout;
    auto pipelineLayout = gpu->device().CreatePipelineLayout(&pipelineLayoutDesc);
    result->fBuiltinUniformHandles = builder.fUniformHandles;
    auto colorState = create_color_state(gpu, pipeline, colorFormat);
    dawn::DepthStencilStateDescriptor depthStencilState;
    GrStencilSettings stencil;
    if (pipeline.isStencilEnabled()) {
        int numStencilBits = renderTarget->renderTargetPriv().numStencilBits();
        stencil.reset(*pipeline.getUserStencil(), pipeline.hasStencilClip(), numStencilBits);
    }
    depthStencilState = create_depth_stencil_state(stencil, depthStencilFormat, origin);

    std::vector<dawn::VertexBufferDescriptor> inputs;

    std::vector<dawn::VertexAttributeDescriptor> vertexAttributes;
    if (primProc.numVertexAttributes() > 0) {
        size_t offset = 0;
        int i = 0;
        for (const auto& attrib : primProc.vertexAttributes()) {
            dawn::VertexAttributeDescriptor attribute;
            attribute.shaderLocation = i;
            attribute.offset = offset;
            attribute.format = to_dawn_vertex_format(attrib.cpuType());
            vertexAttributes.push_back(attribute);
            offset += attrib.sizeAlign4();
            i++;
        }
        result->fVertexStride = offset;
        dawn::VertexBufferDescriptor input;
        input.stride = offset;
        input.stepMode = dawn::InputStepMode::Vertex;
        input.attributeCount = vertexAttributes.size();
        input.attributes = &vertexAttributes.front();
        inputs.push_back(input);
    }
    std::vector<dawn::VertexAttributeDescriptor> instanceAttributes;
    if (primProc.numInstanceAttributes() > 0) {
        size_t offset = 0;
        int i = 0;
        for (const auto& attrib : primProc.instanceAttributes()) {
            dawn::VertexAttributeDescriptor attribute;
            attribute.shaderLocation = i;
            attribute.offset = offset;
            attribute.format = to_dawn_vertex_format(attrib.cpuType());
            instanceAttributes.push_back(attribute);
            offset += attrib.sizeAlign4();
            i++;
        }
        dawn::VertexBufferDescriptor input;
        input.stride = offset;
        input.stepMode = dawn::InputStepMode::Instance;
        input.attributeCount = instanceAttributes.size();
        input.attributes = &instanceAttributes.front();
        inputs.push_back(input);
    }
    dawn::VertexInputDescriptor vertexInput;
    vertexInput.bufferCount = inputs.size();
    vertexInput.buffers = &inputs.front();
    vertexInput.indexFormat = dawn::IndexFormat::Uint16;

    dawn::PipelineStageDescriptor vsDesc;
    vsDesc.module = vsModule;
    vsDesc.entryPoint = "main";

    dawn::PipelineStageDescriptor fsDesc;
    fsDesc.module = fsModule;
    fsDesc.entryPoint = "main";

    dawn::RasterizationStateDescriptor rastDesc;

    rastDesc.frontFace = dawn::FrontFace::CW;
    rastDesc.cullMode = dawn::CullMode::None;
    rastDesc.depthBias = 0;
    rastDesc.depthBiasSlopeScale = 0.0f;
    rastDesc.depthBiasClamp = 0.0f;

    dawn::RenderPipelineDescriptor rpDesc;
    rpDesc.layout = pipelineLayout;
    rpDesc.vertexStage = &vsDesc;
    rpDesc.fragmentStage = &fsDesc;
    rpDesc.vertexInput = &vertexInput;
    rpDesc.primitiveTopology = to_dawn_primitive_topology(primitiveType);
    rpDesc.rasterizationState = &rastDesc;
    rpDesc.sampleCount = 1;
    rpDesc.depthStencilState = hasDepthStencil ? &depthStencilState : nullptr;
    rpDesc.colorStateCount = 1;
    dawn::ColorStateDescriptor* colorStatesPtr[] = { &colorState };
    rpDesc.colorStates = colorStatesPtr;
    result->fRenderPipeline = gpu->device().CreateRenderPipeline(&rpDesc);
    return result;
}

GrDawnProgramBuilder::GrDawnProgramBuilder(GrDawnGpu* gpu,
                                           GrRenderTarget* renderTarget,
                                           GrSurfaceOrigin origin,
                                           const GrPrimitiveProcessor& primProc,
                                           const GrTextureProxy* const primProcProxies[],
                                           const GrPipeline& pipeline,
                                           GrProgramDesc* desc)
    : INHERITED(renderTarget, origin, primProc, primProcProxies, pipeline, desc)
    , fGpu(gpu)
    , fVaryingHandler(this)
    , fUniformHandler(this) {
}

dawn::ShaderModule GrDawnProgramBuilder::CreateShaderModule(dawn::Device device,
                                                            const GrGLSLShaderBuilder& builder,
                                                            SkSL::Program::Kind kind,
                                                            SkSL::Program::Inputs* inputs) {
    SkString source;
    source.append(builder.fCompilerString.c_str());

#if 0
    SkSL::String sksl = GrShaderUtils::PrettyPrint(builder.fCompilerString);
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

void GrDawnProgram::setRenderTargetState(const GrRenderTarget* rt, GrSurfaceOrigin origin) {
    // Load the RT height uniform if it is needed to y-flip gl_FragCoord.
    if (fBuiltinUniformHandles.fRTHeightUni.isValid() &&
        fRenderTargetState.fRenderTargetSize.fHeight != rt->height()) {
        fDataManager.set1f(fBuiltinUniformHandles.fRTHeightUni, SkIntToScalar(rt->height()));
    }

    // set RT adjustment
    SkISize size;
    size.set(rt->width(), rt->height());
    SkASSERT(fBuiltinUniformHandles.fRTAdjustmentUni.isValid());
    if (fRenderTargetState.fRenderTargetOrigin != origin ||
        fRenderTargetState.fRenderTargetSize != size) {
        fRenderTargetState.fRenderTargetSize = size;
        fRenderTargetState.fRenderTargetOrigin = origin;

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

dawn::BindGroup GrDawnProgram::setData(GrDawnGpu* gpu, const GrRenderTarget* renderTarget,
                                       GrSurfaceOrigin origin,
                                       const GrPrimitiveProcessor& primProc,
                                       const GrPipeline& pipeline,
                                       const GrTextureProxy* const primProcTextures[]) {
    this->setRenderTargetState(renderTarget, origin);
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
    descriptor.bindingCount = bindings.size();
    descriptor.bindings = bindings.data();
    value.fBindGroup = gpu->device().CreateBindGroup(&descriptor);
    fBindGroupCache.insert(key, value);
    return value.fBindGroup;
}
