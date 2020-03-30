/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnProgramBuilder.h"

#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrShaderUtils.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/dawn/GrDawnGpu.h"
#include "src/gpu/dawn/GrDawnTexture.h"
#include "src/sksl/SkSLCompiler.h"

static SkSL::String sksl_to_spirv(const GrDawnGpu* gpu, const char* shaderString,
                                  SkSL::Program::Kind kind, bool flipY, uint32_t rtHeightOffset,
                                  SkSL::Program::Inputs* inputs) {
    SkSL::Program::Settings settings;
    settings.fCaps = gpu->caps()->shaderCaps();
    settings.fFlipY = flipY;
    settings.fRTHeightOffset = rtHeightOffset;
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

static wgpu::BlendFactor to_dawn_blend_factor(GrBlendCoeff coeff) {
    switch (coeff) {
        case kZero_GrBlendCoeff:
            return wgpu::BlendFactor::Zero;
        case kOne_GrBlendCoeff:
            return wgpu::BlendFactor::One;
        case kSC_GrBlendCoeff:
            return wgpu::BlendFactor::SrcColor;
        case kISC_GrBlendCoeff:
            return wgpu::BlendFactor::OneMinusSrcColor;
        case kDC_GrBlendCoeff:
            return wgpu::BlendFactor::DstColor;
        case kIDC_GrBlendCoeff:
            return wgpu::BlendFactor::OneMinusDstColor;
        case kSA_GrBlendCoeff:
            return wgpu::BlendFactor::SrcAlpha;
        case kISA_GrBlendCoeff:
            return wgpu::BlendFactor::OneMinusSrcAlpha;
        case kDA_GrBlendCoeff:
            return wgpu::BlendFactor::DstAlpha;
        case kIDA_GrBlendCoeff:
            return wgpu::BlendFactor::OneMinusDstAlpha;
        case kConstC_GrBlendCoeff:
            return wgpu::BlendFactor::BlendColor;
        case kIConstC_GrBlendCoeff:
            return wgpu::BlendFactor::OneMinusBlendColor;
        case kS2C_GrBlendCoeff:
        case kIS2C_GrBlendCoeff:
        case kS2A_GrBlendCoeff:
        case kIS2A_GrBlendCoeff:
        default:
            SkASSERT(!"unsupported blend coefficient");
            return wgpu::BlendFactor::One;
        }
}

static wgpu::BlendFactor to_dawn_blend_factor_for_alpha(GrBlendCoeff coeff) {
    switch (coeff) {
    // Force all srcColor used in alpha slot to alpha version.
    case kSC_GrBlendCoeff:
        return wgpu::BlendFactor::SrcAlpha;
    case kISC_GrBlendCoeff:
        return wgpu::BlendFactor::OneMinusSrcAlpha;
    case kDC_GrBlendCoeff:
        return wgpu::BlendFactor::DstAlpha;
    case kIDC_GrBlendCoeff:
        return wgpu::BlendFactor::OneMinusDstAlpha;
    default:
        return to_dawn_blend_factor(coeff);
    }
}

static wgpu::BlendOperation to_dawn_blend_operation(GrBlendEquation equation) {
    switch (equation) {
    case kAdd_GrBlendEquation:
        return wgpu::BlendOperation::Add;
    case kSubtract_GrBlendEquation:
        return wgpu::BlendOperation::Subtract;
    case kReverseSubtract_GrBlendEquation:
        return wgpu::BlendOperation::ReverseSubtract;
    default:
        SkASSERT(!"unsupported blend equation");
        return wgpu::BlendOperation::Add;
    }
}

static wgpu::CompareFunction to_dawn_compare_function(GrStencilTest test) {
    switch (test) {
        case GrStencilTest::kAlways:
            return wgpu::CompareFunction::Always;
        case GrStencilTest::kNever:
            return wgpu::CompareFunction::Never;
        case GrStencilTest::kGreater:
            return wgpu::CompareFunction::Greater;
        case GrStencilTest::kGEqual:
            return wgpu::CompareFunction::GreaterEqual;
        case GrStencilTest::kLess:
            return wgpu::CompareFunction::Less;
        case GrStencilTest::kLEqual:
            return wgpu::CompareFunction::LessEqual;
        case GrStencilTest::kEqual:
            return wgpu::CompareFunction::Equal;
        case GrStencilTest::kNotEqual:
            return wgpu::CompareFunction::NotEqual;
        default:
            SkASSERT(!"unsupported stencil test");
            return wgpu::CompareFunction::Always;
    }
}

static wgpu::StencilOperation to_dawn_stencil_operation(GrStencilOp op) {
    switch (op) {
        case GrStencilOp::kKeep:
            return wgpu::StencilOperation::Keep;
        case GrStencilOp::kZero:
            return wgpu::StencilOperation::Zero;
        case GrStencilOp::kReplace:
            return wgpu::StencilOperation::Replace;
        case GrStencilOp::kInvert:
            return wgpu::StencilOperation::Invert;
        case GrStencilOp::kIncClamp:
            return wgpu::StencilOperation::IncrementClamp;
        case GrStencilOp::kDecClamp:
            return wgpu::StencilOperation::DecrementClamp;
        case GrStencilOp::kIncWrap:
            return wgpu::StencilOperation::IncrementWrap;
        case GrStencilOp::kDecWrap:
            return wgpu::StencilOperation::DecrementWrap;
        default:
            SkASSERT(!"unsupported stencil function");
            return wgpu::StencilOperation::Keep;
    }
}

static wgpu::PrimitiveTopology to_dawn_primitive_topology(GrPrimitiveType primitiveType) {
    switch (primitiveType) {
        case GrPrimitiveType::kTriangles:
            return wgpu::PrimitiveTopology::TriangleList;
        case GrPrimitiveType::kTriangleStrip:
            return wgpu::PrimitiveTopology::TriangleStrip;
        case GrPrimitiveType::kPoints:
            return wgpu::PrimitiveTopology::PointList;
        case GrPrimitiveType::kLines:
            return wgpu::PrimitiveTopology::LineList;
        case GrPrimitiveType::kLineStrip:
            return wgpu::PrimitiveTopology::LineStrip;
        case GrPrimitiveType::kPath:
        default:
            SkASSERT(!"unsupported primitive topology");
            return wgpu::PrimitiveTopology::TriangleList;
    }
}

static wgpu::VertexFormat to_dawn_vertex_format(GrVertexAttribType type) {
    switch (type) {
    case kFloat_GrVertexAttribType:
    case kHalf_GrVertexAttribType:
        return wgpu::VertexFormat::Float;
    case kFloat2_GrVertexAttribType:
    case kHalf2_GrVertexAttribType:
        return wgpu::VertexFormat::Float2;
    case kFloat3_GrVertexAttribType:
        return wgpu::VertexFormat::Float3;
    case kFloat4_GrVertexAttribType:
    case kHalf4_GrVertexAttribType:
        return wgpu::VertexFormat::Float4;
    case kUShort2_GrVertexAttribType:
        return wgpu::VertexFormat::UShort2;
    case kInt_GrVertexAttribType:
        return wgpu::VertexFormat::Int;
    case kUByte4_norm_GrVertexAttribType:
        return wgpu::VertexFormat::UChar4Norm;
    default:
        SkASSERT(!"unsupported vertex format");
        return wgpu::VertexFormat::Float4;
    }
}

static wgpu::ColorStateDescriptor create_color_state(const GrDawnGpu* gpu,
                                                     const GrPipeline& pipeline,
                                                     wgpu::TextureFormat colorFormat) {
    GrXferProcessor::BlendInfo blendInfo = pipeline.getXferProcessor().getBlendInfo();
    GrBlendEquation equation = blendInfo.fEquation;
    GrBlendCoeff srcCoeff = blendInfo.fSrcBlend;
    GrBlendCoeff dstCoeff = blendInfo.fDstBlend;

    wgpu::BlendFactor srcFactor = to_dawn_blend_factor(srcCoeff);
    wgpu::BlendFactor dstFactor = to_dawn_blend_factor(dstCoeff);
    wgpu::BlendFactor srcFactorAlpha = to_dawn_blend_factor_for_alpha(srcCoeff);
    wgpu::BlendFactor dstFactorAlpha = to_dawn_blend_factor_for_alpha(dstCoeff);
    wgpu::BlendOperation operation = to_dawn_blend_operation(equation);
    auto mask = blendInfo.fWriteColor ? wgpu::ColorWriteMask::All : wgpu::ColorWriteMask::None;

    wgpu::BlendDescriptor colorDesc = {operation, srcFactor, dstFactor};
    wgpu::BlendDescriptor alphaDesc = {operation, srcFactorAlpha, dstFactorAlpha};

    wgpu::ColorStateDescriptor descriptor;
    descriptor.format = colorFormat;
    descriptor.alphaBlend = alphaDesc;
    descriptor.colorBlend = colorDesc;
    descriptor.nextInChain = nullptr;
    descriptor.writeMask = mask;

    return descriptor;
}

static wgpu::StencilStateFaceDescriptor to_stencil_state_face(const GrStencilSettings::Face& face) {
     wgpu::StencilStateFaceDescriptor desc;
     desc.compare = to_dawn_compare_function(face.fTest);
     desc.failOp = desc.depthFailOp = to_dawn_stencil_operation(face.fFailOp);
     desc.passOp = to_dawn_stencil_operation(face.fPassOp);
     return desc;
}

static wgpu::DepthStencilStateDescriptor create_depth_stencil_state(
        const GrProgramInfo& programInfo,
        wgpu::TextureFormat depthStencilFormat) {
    GrStencilSettings stencilSettings = programInfo.nonGLStencilSettings();
    GrSurfaceOrigin origin = programInfo.origin();

    wgpu::DepthStencilStateDescriptor state;
    state.format = depthStencilFormat;
    if (!stencilSettings.isDisabled()) {
        if (stencilSettings.isTwoSided()) {
            auto front = stencilSettings.postOriginCCWFace(origin);
            auto back = stencilSettings.postOriginCWFace(origin);
            state.stencilFront = to_stencil_state_face(front);
            state.stencilBack = to_stencil_state_face(back);
            state.stencilReadMask = front.fTestMask;
            state.stencilWriteMask = front.fWriteMask;
        } else {
            auto frontAndBack = stencilSettings.singleSidedFace();
            state.stencilBack = state.stencilFront = to_stencil_state_face(frontAndBack);
            state.stencilReadMask = frontAndBack.fTestMask;
            state.stencilWriteMask = frontAndBack.fWriteMask;
        }
    }
    return state;
}

static wgpu::BindGroupBinding make_bind_group_binding(uint32_t binding, const wgpu::Buffer& buffer,
                                                      uint32_t offset, uint32_t size, const
                                                      wgpu::Sampler& sampler,
                                                      const wgpu::TextureView& textureView) {
    wgpu::BindGroupBinding result;
    result.binding = binding;
    result.buffer = buffer;
    result.offset = offset;
    result.size = size;
    result.sampler = sampler;
    result.textureView = textureView;
    return result;
}

static wgpu::BindGroupBinding make_bind_group_binding(uint32_t binding, const wgpu::Buffer& buffer,
                                                      uint32_t offset, uint32_t size) {
    return make_bind_group_binding(binding, buffer, offset, size, nullptr, nullptr);
}

static wgpu::BindGroupBinding make_bind_group_binding(uint32_t binding,
                                                      const wgpu::Sampler& sampler) {
    return make_bind_group_binding(binding, nullptr, 0, 0, sampler, nullptr);
}

static wgpu::BindGroupBinding make_bind_group_binding(uint32_t binding,
                                                      const wgpu::TextureView& textureView) {
    return make_bind_group_binding(binding, nullptr, 0, 0, nullptr, textureView);
}

sk_sp<GrDawnProgram> GrDawnProgramBuilder::Build(GrDawnGpu* gpu,
                                                 GrRenderTarget* renderTarget,
                                                 const GrProgramInfo& programInfo,
                                                 wgpu::TextureFormat colorFormat,
                                                 bool hasDepthStencil,
                                                 wgpu::TextureFormat depthStencilFormat,
                                                 GrProgramDesc* desc) {
    GrDawnProgramBuilder builder(gpu, renderTarget, programInfo, desc);
    if (!builder.emitAndInstallProcs()) {
        return nullptr;
    }

    builder.fVS.extensions().appendf("#extension GL_ARB_separate_shader_objects : enable\n");
    builder.fFS.extensions().appendf("#extension GL_ARB_separate_shader_objects : enable\n");
    builder.fVS.extensions().appendf("#extension GL_ARB_shading_language_420pack : enable\n");
    builder.fFS.extensions().appendf("#extension GL_ARB_shading_language_420pack : enable\n");

    builder.finalizeShaders();

    SkSL::Program::Inputs vertInputs, fragInputs;
    bool flipY = programInfo.origin() != kTopLeft_GrSurfaceOrigin;
    auto vsModule = builder.createShaderModule(builder.fVS, SkSL::Program::kVertex_Kind, flipY,
                                               &vertInputs);
    auto fsModule = builder.createShaderModule(builder.fFS, SkSL::Program::kFragment_Kind, flipY,
                                               &fragInputs);
    GrDawnUniformHandler::UniformInfoArray& uniforms = builder.fUniformHandler.fUniforms;
    uint32_t uniformBufferSize = builder.fUniformHandler.fCurrentUBOOffset;
    sk_sp<GrDawnProgram> result(new GrDawnProgram(uniforms, uniformBufferSize));
    result->fGeometryProcessor = std::move(builder.fGeometryProcessor);
    result->fXferProcessor = std::move(builder.fXferProcessor);
    result->fFragmentProcessors = std::move(builder.fFragmentProcessors);
    result->fFragmentProcessorCnt = builder.fFragmentProcessorCnt;
    std::vector<wgpu::BindGroupLayoutBinding> uniformLayoutBindings;
    if (0 != uniformBufferSize) {
        uniformLayoutBindings.push_back({ GrDawnUniformHandler::kUniformBinding,
                                          wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment,
                                          wgpu::BindingType::UniformBuffer});
    }
    wgpu::BindGroupLayoutDescriptor uniformBindGroupLayoutDesc;
    uniformBindGroupLayoutDesc.bindingCount = uniformLayoutBindings.size();
    uniformBindGroupLayoutDesc.bindings = uniformLayoutBindings.data();
    result->fBindGroupLayouts[0] =
        gpu->device().CreateBindGroupLayout(&uniformBindGroupLayoutDesc);
    uint32_t binding = 0;
    std::vector<wgpu::BindGroupLayoutBinding> textureLayoutBindings;
    for (int i = 0; i < builder.fUniformHandler.fSamplers.count(); ++i) {
        textureLayoutBindings.push_back({ binding++, wgpu::ShaderStage::Fragment,
                                          wgpu::BindingType::Sampler});
        textureLayoutBindings.push_back({ binding++, wgpu::ShaderStage::Fragment,
                                          wgpu::BindingType::SampledTexture});
    }
    wgpu::BindGroupLayoutDescriptor textureBindGroupLayoutDesc;
    textureBindGroupLayoutDesc.bindingCount = textureLayoutBindings.size();
    textureBindGroupLayoutDesc.bindings = textureLayoutBindings.data();
    result->fBindGroupLayouts[1] =
        gpu->device().CreateBindGroupLayout(&textureBindGroupLayoutDesc);
    wgpu::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindGroupLayoutCount = 2;
    pipelineLayoutDesc.bindGroupLayouts = &result->fBindGroupLayouts[0];
    auto pipelineLayout = gpu->device().CreatePipelineLayout(&pipelineLayoutDesc);
    result->fBuiltinUniformHandles = builder.fUniformHandles;
    const GrPipeline& pipeline = programInfo.pipeline();
    auto colorState = create_color_state(gpu, pipeline, colorFormat);
    wgpu::DepthStencilStateDescriptor depthStencilState;

#ifdef SK_DEBUG
    if (pipeline.isStencilEnabled()) {
        SkASSERT(renderTarget->renderTargetPriv().numStencilBits() == 8);
    }
#endif
    depthStencilState = create_depth_stencil_state(programInfo, depthStencilFormat);

    std::vector<wgpu::VertexBufferLayoutDescriptor> inputs;

    std::vector<wgpu::VertexAttributeDescriptor> vertexAttributes;
    const GrPrimitiveProcessor& primProc = programInfo.primProc();
    if (primProc.numVertexAttributes() > 0) {
        size_t offset = 0;
        int i = 0;
        for (const auto& attrib : primProc.vertexAttributes()) {
            wgpu::VertexAttributeDescriptor attribute;
            attribute.shaderLocation = i;
            attribute.offset = offset;
            attribute.format = to_dawn_vertex_format(attrib.cpuType());
            vertexAttributes.push_back(attribute);
            offset += attrib.sizeAlign4();
            i++;
        }
        wgpu::VertexBufferLayoutDescriptor input;
        input.arrayStride = offset;
        input.stepMode = wgpu::InputStepMode::Vertex;
        input.attributeCount = vertexAttributes.size();
        input.attributes = &vertexAttributes.front();
        inputs.push_back(input);
    }
    std::vector<wgpu::VertexAttributeDescriptor> instanceAttributes;
    if (primProc.numInstanceAttributes() > 0) {
        size_t offset = 0;
        int i = 0;
        for (const auto& attrib : primProc.instanceAttributes()) {
            wgpu::VertexAttributeDescriptor attribute;
            attribute.shaderLocation = i;
            attribute.offset = offset;
            attribute.format = to_dawn_vertex_format(attrib.cpuType());
            instanceAttributes.push_back(attribute);
            offset += attrib.sizeAlign4();
            i++;
        }
        wgpu::VertexBufferLayoutDescriptor input;
        input.arrayStride = offset;
        input.stepMode = wgpu::InputStepMode::Instance;
        input.attributeCount = instanceAttributes.size();
        input.attributes = &instanceAttributes.front();
        inputs.push_back(input);
    }
    wgpu::VertexStateDescriptor vertexState;
    vertexState.indexFormat = wgpu::IndexFormat::Uint16;
    vertexState.vertexBufferCount = inputs.size();
    vertexState.vertexBuffers = &inputs.front();

    wgpu::ProgrammableStageDescriptor vsDesc;
    vsDesc.module = vsModule;
    vsDesc.entryPoint = "main";

    wgpu::ProgrammableStageDescriptor fsDesc;
    fsDesc.module = fsModule;
    fsDesc.entryPoint = "main";

    wgpu::RenderPipelineDescriptor rpDesc;
    rpDesc.layout = pipelineLayout;
    rpDesc.vertexStage = vsDesc;
    rpDesc.fragmentStage = &fsDesc;
    rpDesc.vertexState = &vertexState;
    rpDesc.primitiveTopology = to_dawn_primitive_topology(programInfo.primitiveType());
    if (hasDepthStencil) {
        rpDesc.depthStencilState = &depthStencilState;
    }
    rpDesc.colorStateCount = 1;
    rpDesc.colorStates = &colorState;
    result->fRenderPipeline = gpu->device().CreateRenderPipeline(&rpDesc);
    return result;
}

GrDawnProgramBuilder::GrDawnProgramBuilder(GrDawnGpu* gpu,
                                           GrRenderTarget* renderTarget,
                                           const GrProgramInfo& programInfo,
                                           GrProgramDesc* desc)
    : INHERITED(renderTarget, *desc, programInfo)
    , fGpu(gpu)
    , fVaryingHandler(this)
    , fUniformHandler(this) {
}

wgpu::ShaderModule GrDawnProgramBuilder::createShaderModule(const GrGLSLShaderBuilder& builder,
                                                            SkSL::Program::Kind kind,
                                                            bool flipY,
                                                            SkSL::Program::Inputs* inputs) {
    wgpu::Device device = fGpu->device();
    SkString source(builder.fCompilerString.c_str());

#if 0
    SkSL::String sksl = GrShaderUtils::PrettyPrint(builder.fCompilerString);
    printf("converting program:\n%s\n", sksl.c_str());
#endif

    SkSL::String spirvSource = sksl_to_spirv(fGpu, source.c_str(), kind, flipY,
                                             fUniformHandler.getRTHeightOffset(), inputs);
    if (inputs->fRTHeight) {
        this->addRTHeightUniform(SKSL_RTHEIGHT_NAME);
    }

    wgpu::ShaderModuleDescriptor desc;
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
    SkISize dimensions = rt->dimensions();
    SkASSERT(fBuiltinUniformHandles.fRTAdjustmentUni.isValid());
    if (fRenderTargetState.fRenderTargetOrigin != origin ||
        fRenderTargetState.fRenderTargetSize != dimensions) {
        fRenderTargetState.fRenderTargetSize = dimensions;
        fRenderTargetState.fRenderTargetOrigin = origin;

        float rtAdjustmentVec[4];
        fRenderTargetState.getRTAdjustmentVec(rtAdjustmentVec);
        fDataManager.set4fv(fBuiltinUniformHandles.fRTAdjustmentUni, 1, rtAdjustmentVec);
    }
}

static void set_texture(GrDawnGpu* gpu, GrSamplerState state, GrTexture* texture,
                        std::vector<wgpu::BindGroupBinding>* bindings, int* binding) {
    // FIXME: could probably cache samplers in GrDawnProgram
    wgpu::Sampler sampler = gpu->getOrCreateSampler(state);
    bindings->push_back(make_bind_group_binding((*binding)++, sampler));
    GrDawnTexture* tex = static_cast<GrDawnTexture*>(texture);
    wgpu::TextureView textureView = tex->textureView();
    bindings->push_back(make_bind_group_binding((*binding)++, textureView));
}

wgpu::BindGroup GrDawnProgram::setUniformData(GrDawnGpu* gpu, const GrRenderTarget* renderTarget,
                                              const GrProgramInfo& programInfo) {
    std::vector<wgpu::BindGroupBinding> bindings;
    GrDawnRingBuffer::Slice slice;
    uint32_t uniformBufferSize = fDataManager.uniformBufferSize();
    if (0 != uniformBufferSize) {
        slice = gpu->allocateUniformRingBufferSlice(uniformBufferSize);
        bindings.push_back(make_bind_group_binding(GrDawnUniformHandler::kUniformBinding,
                                                   slice.fBuffer, slice.fOffset,
                                                   uniformBufferSize));
    }
    this->setRenderTargetState(renderTarget, programInfo.origin());
    const GrPipeline& pipeline = programInfo.pipeline();
    const GrPrimitiveProcessor& primProc = programInfo.primProc();
    GrFragmentProcessor::PipelineCoordTransformRange transformRange(pipeline);
    fGeometryProcessor->setData(fDataManager, primProc, transformRange);
    GrFragmentProcessor::CIter fpIter(pipeline);
    GrGLSLFragmentProcessor::Iter glslIter(fFragmentProcessors.get(), fFragmentProcessorCnt);
    for (; fpIter && glslIter; ++fpIter, ++glslIter) {
        glslIter->setData(fDataManager, *fpIter);
    }
    SkIPoint offset;
    GrTexture* dstTexture = pipeline.peekDstTexture(&offset);
    fXferProcessor->setData(fDataManager, pipeline.getXferProcessor(), dstTexture, offset);
    if (0 != uniformBufferSize) {
        fDataManager.uploadUniformBuffers(slice.fData);
    }
    wgpu::BindGroupDescriptor descriptor;
    descriptor.layout = fBindGroupLayouts[0];
    descriptor.bindingCount = bindings.size();
    descriptor.bindings = bindings.data();
    return gpu->device().CreateBindGroup(&descriptor);
}

wgpu::BindGroup GrDawnProgram::setTextures(GrDawnGpu* gpu,
                                           const GrPrimitiveProcessor& primProc,
                                           const GrPipeline& pipeline,
                                           const GrSurfaceProxy* const primProcTextures[]) {
    std::vector<wgpu::BindGroupBinding> bindings;
    int binding = 0;
    if (primProcTextures) {
        for (int i = 0; i < primProc.numTextureSamplers(); ++i) {
            SkASSERT(primProcTextures[i]->asTextureProxy());
            auto& sampler = primProc.textureSampler(i);
            set_texture(gpu, sampler.samplerState(), primProcTextures[i]->peekTexture(), &bindings,
                        &binding);
        }
    }
    GrFragmentProcessor::CIter fpIter(pipeline);
    GrGLSLFragmentProcessor::Iter glslIter(fFragmentProcessors.get(), fFragmentProcessorCnt);
    for (; fpIter && glslIter; ++fpIter, ++glslIter) {
        for (int i = 0; i < fpIter->numTextureSamplers(); ++i) {
            auto& s = fpIter->textureSampler(i);
            set_texture(gpu, s.samplerState(), s.peekTexture(), &bindings, &binding);
        }
    }
    SkIPoint offset;
    if (GrTexture* dstTexture = pipeline.peekDstTexture(&offset)) {
        set_texture(gpu, GrSamplerState::Filter::kNearest, dstTexture, &bindings, &binding);
    }
    wgpu::BindGroupDescriptor descriptor;
    descriptor.layout = fBindGroupLayouts[1];
    descriptor.bindingCount = bindings.size();
    descriptor.bindings = bindings.data();
    return gpu->device().CreateBindGroup(&descriptor);
}
