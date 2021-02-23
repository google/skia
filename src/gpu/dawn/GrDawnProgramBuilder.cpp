/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnProgramBuilder.h"

#include "src/gpu/GrAutoLocaleSetter.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrShaderUtils.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/dawn/GrDawnGpu.h"
#include "src/gpu/dawn/GrDawnTexture.h"

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

static wgpu::BindGroupEntry make_bind_group_entry(uint32_t binding,
                                                  const wgpu::Sampler& sampler,
                                                  const wgpu::TextureView& textureView) {
    wgpu::BindGroupEntry result;
    result.binding = binding;
    result.buffer = nullptr;
    result.offset = 0;
    result.size = 0;
    result.sampler = sampler;
    result.textureView = textureView;
    return result;
}

static wgpu::BindGroupEntry make_bind_group_entry(uint32_t binding,
                                                  const wgpu::Sampler& sampler) {
    return make_bind_group_entry(binding, sampler, nullptr);
}

static wgpu::BindGroupEntry make_bind_group_entry(uint32_t binding,
                                                  const wgpu::TextureView& textureView) {
    return make_bind_group_entry(binding, nullptr, textureView);
}

sk_sp<GrDawnProgram> GrDawnProgramBuilder::Build(GrDawnGpu* gpu,
                                                 GrRenderTarget* renderTarget,
                                                 const GrProgramInfo& programInfo,
                                                 wgpu::TextureFormat colorFormat,
                                                 bool hasDepthStencil,
                                                 wgpu::TextureFormat depthStencilFormat,
                                                 GrProgramDesc* desc) {
    GrAutoLocaleSetter als("C");

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
    auto vsModule = builder.createShaderModule(builder.fVS, SkSL::ProgramKind::kVertex, flipY,
                                               &vertInputs);
    auto fsModule = builder.createShaderModule(builder.fFS, SkSL::ProgramKind::kFragment, flipY,
                                               &fragInputs);
    GrSPIRVUniformHandler::UniformInfoArray& uniforms = builder.fUniformHandler.fUniforms;
    uint32_t uniformBufferSize = builder.fUniformHandler.fCurrentUBOOffset;
    sk_sp<GrDawnProgram> result(new GrDawnProgram(uniforms, uniformBufferSize));
    result->fGeometryProcessor = std::move(builder.fGeometryProcessor);
    result->fXferProcessor = std::move(builder.fXferProcessor);
    result->fFPImpls = std::move(builder.fFPImpls);
    std::vector<wgpu::BindGroupLayoutEntry> uniformLayoutEntries;
    if (0 != uniformBufferSize) {
        wgpu::BindGroupLayoutEntry entry;
        entry.binding = GrSPIRVUniformHandler::kUniformBinding;
        entry.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
        entry.type = wgpu::BindingType::UniformBuffer;
        uniformLayoutEntries.push_back(std::move(entry));
    }
    wgpu::BindGroupLayoutDescriptor uniformBindGroupLayoutDesc;
    uniformBindGroupLayoutDesc.entryCount = uniformLayoutEntries.size();
    uniformBindGroupLayoutDesc.entries = uniformLayoutEntries.data();
    result->fBindGroupLayouts.push_back(
        gpu->device().CreateBindGroupLayout(&uniformBindGroupLayoutDesc));
    uint32_t binding = 0;
    std::vector<wgpu::BindGroupLayoutEntry> textureLayoutEntries;
    int textureCount = builder.fUniformHandler.fSamplers.count();
    if (textureCount > 0) {
        for (int i = 0; i < textureCount; ++i)  {
            {
                wgpu::BindGroupLayoutEntry entry;
                entry.binding = binding++;
                entry.visibility = wgpu::ShaderStage::Fragment;
                entry.type = wgpu::BindingType::Sampler;
                textureLayoutEntries.push_back(std::move(entry));
            }
            {
                wgpu::BindGroupLayoutEntry entry;
                entry.binding = binding++;
                entry.visibility = wgpu::ShaderStage::Fragment;
                entry.type = wgpu::BindingType::SampledTexture;
                textureLayoutEntries.push_back(std::move(entry));
            }
        }
        wgpu::BindGroupLayoutDescriptor textureBindGroupLayoutDesc;
        textureBindGroupLayoutDesc.entryCount = textureLayoutEntries.size();
        textureBindGroupLayoutDesc.entries = textureLayoutEntries.data();
        result->fBindGroupLayouts.push_back(
            gpu->device().CreateBindGroupLayout(&textureBindGroupLayoutDesc));
    }
    wgpu::PipelineLayoutDescriptor pipelineLayoutDesc;
    pipelineLayoutDesc.bindGroupLayoutCount = result->fBindGroupLayouts.size();
    pipelineLayoutDesc.bindGroupLayouts = result->fBindGroupLayouts.data();
    auto pipelineLayout = gpu->device().CreatePipelineLayout(&pipelineLayoutDesc);
    result->fBuiltinUniformHandles = builder.fUniformHandles;
    const GrPipeline& pipeline = programInfo.pipeline();
    auto colorState = create_color_state(gpu, pipeline, colorFormat);
    wgpu::DepthStencilStateDescriptor depthStencilState;

#ifdef SK_DEBUG
    if (programInfo.isStencilEnabled()) {
        SkASSERT(renderTarget->numStencilBits() == 8);
    }
#endif
    depthStencilState = create_depth_stencil_state(programInfo, depthStencilFormat);

    std::vector<wgpu::VertexBufferLayoutDescriptor> inputs;

    std::vector<wgpu::VertexAttributeDescriptor> vertexAttributes;
    const GrPrimitiveProcessor& primProc = programInfo.primProc();
    int i = 0;
    if (primProc.numVertexAttributes() > 0) {
        size_t offset = 0;
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
    if (programInfo.primitiveType() == GrPrimitiveType::kTriangleStrip ||
        programInfo.primitiveType() == GrPrimitiveType::kLineStrip) {
        vertexState.indexFormat = wgpu::IndexFormat::Uint16;
    }
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
                                                            SkSL::ProgramKind kind,
                                                            bool flipY,
                                                            SkSL::Program::Inputs* inputs) {
    wgpu::Device device = fGpu->device();
    SkString source(builder.fCompilerString.c_str());

#if 0
    SkSL::String sksl = GrShaderUtils::PrettyPrint(builder.fCompilerString);
    printf("converting program:\n%s\n", sksl.c_str());
#endif

    SkSL::String spirvSource = fGpu->SkSLToSPIRV(source.c_str(), kind, flipY,
                                                 fUniformHandler.getRTHeightOffset(), inputs);
    if (inputs->fRTHeight) {
        this->addRTHeightUniform(SKSL_RTHEIGHT_NAME);
    }

    return fGpu->createShaderModule(spirvSource);
};

const GrCaps* GrDawnProgramBuilder::caps() const {
    return fGpu->caps();
}

SkSL::Compiler* GrDawnProgramBuilder::shaderCompiler() const {
    return fGpu->shaderCompiler();
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
                        std::vector<wgpu::BindGroupEntry>* bindings, int* binding) {
    // FIXME: could probably cache samplers in GrDawnProgram
    wgpu::Sampler sampler = gpu->getOrCreateSampler(state);
    bindings->push_back(make_bind_group_entry((*binding)++, sampler));
    GrDawnTexture* tex = static_cast<GrDawnTexture*>(texture);
    wgpu::TextureViewDescriptor viewDesc;
    // Note that a mipLevelCount of zero here means to expose all available levels.
    viewDesc.mipLevelCount = GrSamplerState::MipmapMode::kNone == state.mipmapMode() ? 1 : 0;
    wgpu::TextureView textureView = tex->texture().CreateView(&viewDesc);
    bindings->push_back(make_bind_group_entry((*binding)++, textureView));
}

wgpu::BindGroup GrDawnProgram::setUniformData(GrDawnGpu* gpu, const GrRenderTarget* renderTarget,
                                              const GrProgramInfo& programInfo) {
    if (0 == fDataManager.uniformBufferSize()) {
        return nullptr;
    }
    this->setRenderTargetState(renderTarget, programInfo.origin());
    const GrPipeline& pipeline = programInfo.pipeline();
    const GrPrimitiveProcessor& primProc = programInfo.primProc();
    fGeometryProcessor->setData(fDataManager, primProc);

    for (int i = 0; i < programInfo.pipeline().numFragmentProcessors(); ++i) {
        auto& fp = programInfo.pipeline().getFragmentProcessor(i);
        for (auto [fp, impl] : GrGLSLFragmentProcessor::ParallelRange(fp, *fFPImpls[i])) {
            impl.setData(fDataManager, fp);
        }
    }

    SkIPoint offset;
    GrTexture* dstTexture = pipeline.peekDstTexture(&offset);
    fXferProcessor->setData(fDataManager, pipeline.getXferProcessor(), dstTexture, offset);
    return fDataManager.uploadUniformBuffers(gpu, fBindGroupLayouts[0]);
}

wgpu::BindGroup GrDawnProgram::setTextures(GrDawnGpu* gpu,
                                           const GrPrimitiveProcessor& primProc,
                                           const GrPipeline& pipeline,
                                           const GrSurfaceProxy* const primProcTextures[]) {
    if (fBindGroupLayouts.size() < 2) {
        return nullptr;
    }
    std::vector<wgpu::BindGroupEntry> bindings;
    int binding = 0;
    if (primProcTextures) {
        for (int i = 0; i < primProc.numTextureSamplers(); ++i) {
            SkASSERT(primProcTextures[i]->asTextureProxy());
            auto& sampler = primProc.textureSampler(i);
            set_texture(gpu, sampler.samplerState(), primProcTextures[i]->peekTexture(), &bindings,
                        &binding);
        }
    }

    pipeline.visitTextureEffects([&](const GrTextureEffect& te) {
        set_texture(gpu, te.samplerState(), te.texture(), &bindings, &binding);
    });

    SkIPoint offset;
    if (GrTexture* dstTexture = pipeline.peekDstTexture(&offset)) {
        set_texture(gpu, GrSamplerState::Filter::kNearest, dstTexture, &bindings, &binding);
    }
    wgpu::BindGroupDescriptor descriptor;
    descriptor.layout = fBindGroupLayouts[1];
    descriptor.entryCount = bindings.size();
    descriptor.entries = bindings.data();
    return gpu->device().CreateBindGroup(&descriptor);
}
