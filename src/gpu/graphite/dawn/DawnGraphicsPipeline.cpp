/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnGraphicsPipeline.h"

#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/PipelineUtils.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/dawn/DawnCaps.h"
#include "src/gpu/graphite/dawn/DawnErrorChecker.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtilsPriv.h"
#include "src/gpu/graphite/dawn/DawnResourceProvider.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#include "src/gpu/graphite/dawn/DawnUtilsPriv.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLProgram.h"

#include <vector>

namespace skgpu::graphite {

namespace {

inline wgpu::VertexFormat attribute_type_to_dawn(VertexAttribType type) {
    switch (type) {
        case VertexAttribType::kFloat:
            return wgpu::VertexFormat::Float32;
        case VertexAttribType::kFloat2:
            return wgpu::VertexFormat::Float32x2;
        case VertexAttribType::kFloat3:
            return wgpu::VertexFormat::Float32x3;
        case VertexAttribType::kFloat4:
            return wgpu::VertexFormat::Float32x4;
        case VertexAttribType::kHalf:
            return wgpu::VertexFormat::Undefined;
        case VertexAttribType::kHalf2:
            return wgpu::VertexFormat::Float16x2;
        case VertexAttribType::kHalf4:
            return wgpu::VertexFormat::Float16x4;
        case VertexAttribType::kInt2:
            return wgpu::VertexFormat::Sint32x2;
        case VertexAttribType::kInt3:
            return wgpu::VertexFormat::Sint32x3;
        case VertexAttribType::kInt4:
            return wgpu::VertexFormat::Sint32x4;
        case VertexAttribType::kByte:
            return wgpu::VertexFormat::Undefined;
        case VertexAttribType::kByte2:
            return wgpu::VertexFormat::Sint8x2;
        case VertexAttribType::kByte4:
            return wgpu::VertexFormat::Sint8x4;
        case VertexAttribType::kUByte:
            return wgpu::VertexFormat::Undefined;
        case VertexAttribType::kUByte2:
            return wgpu::VertexFormat::Uint8x2;
        case VertexAttribType::kUByte4:
            return wgpu::VertexFormat::Uint8x4;
        case VertexAttribType::kUByte_norm:
            return wgpu::VertexFormat::Undefined;
        case VertexAttribType::kUByte4_norm:
            return wgpu::VertexFormat::Unorm8x4;
        case VertexAttribType::kShort2:
            return wgpu::VertexFormat::Sint16x2;
        case VertexAttribType::kShort4:
            return wgpu::VertexFormat::Sint16x4;
        case VertexAttribType::kUShort2:
            return wgpu::VertexFormat::Uint16x2;
        case VertexAttribType::kUShort2_norm:
            return wgpu::VertexFormat::Unorm16x2;
        case VertexAttribType::kInt:
            return wgpu::VertexFormat::Sint32;
        case VertexAttribType::kUInt:
            return wgpu::VertexFormat::Uint32;
        case VertexAttribType::kUShort_norm:
            return wgpu::VertexFormat::Undefined;
        case VertexAttribType::kUShort4_norm:
            return wgpu::VertexFormat::Unorm16x4;
    }
    SkUNREACHABLE;
}

wgpu::CompareFunction compare_op_to_dawn(CompareOp op) {
    switch (op) {
        case CompareOp::kAlways:
            return wgpu::CompareFunction::Always;
        case CompareOp::kNever:
            return wgpu::CompareFunction::Never;
        case CompareOp::kGreater:
            return wgpu::CompareFunction::Greater;
        case CompareOp::kGEqual:
            return wgpu::CompareFunction::GreaterEqual;
        case CompareOp::kLess:
            return wgpu::CompareFunction::Less;
        case CompareOp::kLEqual:
            return wgpu::CompareFunction::LessEqual;
        case CompareOp::kEqual:
            return wgpu::CompareFunction::Equal;
        case CompareOp::kNotEqual:
            return wgpu::CompareFunction::NotEqual;
    }
    SkUNREACHABLE;
}

wgpu::StencilOperation stencil_op_to_dawn(StencilOp op) {
    switch (op) {
        case StencilOp::kKeep:
            return wgpu::StencilOperation::Keep;
        case StencilOp::kZero:
            return wgpu::StencilOperation::Zero;
        case StencilOp::kReplace:
            return wgpu::StencilOperation::Replace;
        case StencilOp::kInvert:
            return wgpu::StencilOperation::Invert;
        case StencilOp::kIncWrap:
            return wgpu::StencilOperation::IncrementWrap;
        case StencilOp::kDecWrap:
            return wgpu::StencilOperation::DecrementWrap;
        case StencilOp::kIncClamp:
            return wgpu::StencilOperation::IncrementClamp;
        case StencilOp::kDecClamp:
            return wgpu::StencilOperation::DecrementClamp;
    }
    SkUNREACHABLE;
}

wgpu::StencilFaceState stencil_face_to_dawn(DepthStencilSettings::Face face) {
    wgpu::StencilFaceState state;
    state.compare = compare_op_to_dawn(face.fCompareOp);
    state.failOp = stencil_op_to_dawn(face.fStencilFailOp);
    state.depthFailOp = stencil_op_to_dawn(face.fDepthFailOp);
    state.passOp = stencil_op_to_dawn(face.fDepthStencilPassOp);
    return state;
}

size_t create_vertex_attributes(SkSpan<const Attribute> attrs,
                                int shaderLocationOffset,
                                std::vector<wgpu::VertexAttribute>* out) {
    SkASSERT(out && out->empty());
    out->resize(attrs.size());
    size_t vertexAttributeOffset = 0;
    int attributeIndex = 0;
    for (const auto& attr : attrs) {
        wgpu::VertexAttribute& vertexAttribute =  (*out)[attributeIndex];
        vertexAttribute.format = attribute_type_to_dawn(attr.cpuType());
        SkASSERT(vertexAttribute.format != wgpu::VertexFormat::Undefined);
        vertexAttribute.offset = vertexAttributeOffset;
        vertexAttribute.shaderLocation = shaderLocationOffset + attributeIndex;
        vertexAttributeOffset += attr.sizeAlign4();
        attributeIndex++;
    }
    return vertexAttributeOffset;
}

// TODO: share this w/ Ganesh dawn backend?
static wgpu::BlendFactor blend_coeff_to_dawn_blend(const DawnCaps& caps, skgpu::BlendCoeff coeff) {
#if defined(__EMSCRIPTEN__)
#define VALUE_IF_DSB_OR_ZERO(VALUE) wgpu::BlendFactor::Zero
#else
#define VALUE_IF_DSB_OR_ZERO(VALUE) \
    ((caps.shaderCaps()->fDualSourceBlendingSupport) ? (VALUE) : wgpu::BlendFactor::Zero)
#endif
    switch (coeff) {
        case skgpu::BlendCoeff::kZero:
            return wgpu::BlendFactor::Zero;
        case skgpu::BlendCoeff::kOne:
            return wgpu::BlendFactor::One;
        case skgpu::BlendCoeff::kSC:
            return wgpu::BlendFactor::Src;
        case skgpu::BlendCoeff::kISC:
            return wgpu::BlendFactor::OneMinusSrc;
        case skgpu::BlendCoeff::kDC:
            return wgpu::BlendFactor::Dst;
        case skgpu::BlendCoeff::kIDC:
            return wgpu::BlendFactor::OneMinusDst;
        case skgpu::BlendCoeff::kSA:
            return wgpu::BlendFactor::SrcAlpha;
        case skgpu::BlendCoeff::kISA:
            return wgpu::BlendFactor::OneMinusSrcAlpha;
        case skgpu::BlendCoeff::kDA:
            return wgpu::BlendFactor::DstAlpha;
        case skgpu::BlendCoeff::kIDA:
            return wgpu::BlendFactor::OneMinusDstAlpha;
        case skgpu::BlendCoeff::kConstC:
            return wgpu::BlendFactor::Constant;
        case skgpu::BlendCoeff::kIConstC:
            return wgpu::BlendFactor::OneMinusConstant;
        case skgpu::BlendCoeff::kS2C:
            return VALUE_IF_DSB_OR_ZERO(wgpu::BlendFactor::Src1);
        case skgpu::BlendCoeff::kIS2C:
            return VALUE_IF_DSB_OR_ZERO(wgpu::BlendFactor::OneMinusSrc1);
        case skgpu::BlendCoeff::kS2A:
            return VALUE_IF_DSB_OR_ZERO(wgpu::BlendFactor::Src1Alpha);
        case skgpu::BlendCoeff::kIS2A:
            return VALUE_IF_DSB_OR_ZERO(wgpu::BlendFactor::OneMinusSrc1Alpha);
        case skgpu::BlendCoeff::kIllegal:
            return wgpu::BlendFactor::Zero;
    }
    SkUNREACHABLE;
#undef VALUE_IF_DSB_OR_ZERO
}

static wgpu::BlendFactor blend_coeff_to_dawn_blend_for_alpha(const DawnCaps& caps,
                                                             skgpu::BlendCoeff coeff) {
    switch (coeff) {
        // Force all srcColor used in alpha slot to alpha version.
        case skgpu::BlendCoeff::kSC:
            return wgpu::BlendFactor::SrcAlpha;
        case skgpu::BlendCoeff::kISC:
            return wgpu::BlendFactor::OneMinusSrcAlpha;
        case skgpu::BlendCoeff::kDC:
            return wgpu::BlendFactor::DstAlpha;
        case skgpu::BlendCoeff::kIDC:
            return wgpu::BlendFactor::OneMinusDstAlpha;
        default:
            return blend_coeff_to_dawn_blend(caps, coeff);
    }
}

// TODO: share this w/ Ganesh Metal backend?
static wgpu::BlendOperation blend_equation_to_dawn_blend_op(skgpu::BlendEquation equation) {
    static const wgpu::BlendOperation gTable[] = {
            wgpu::BlendOperation::Add,              // skgpu::BlendEquation::kAdd
            wgpu::BlendOperation::Subtract,         // skgpu::BlendEquation::kSubtract
            wgpu::BlendOperation::ReverseSubtract,  // skgpu::BlendEquation::kReverseSubtract
    };
    static_assert(std::size(gTable) == (int)skgpu::BlendEquation::kFirstAdvanced);
    static_assert(0 == (int)skgpu::BlendEquation::kAdd);
    static_assert(1 == (int)skgpu::BlendEquation::kSubtract);
    static_assert(2 == (int)skgpu::BlendEquation::kReverseSubtract);

    SkASSERT((unsigned)equation < skgpu::kBlendEquationCnt);
    return gTable[(int)equation];
}

struct AsyncPipelineCreationBase {
    wgpu::RenderPipeline fRenderPipeline;
    bool fFinished = false;
};

} // anonymous namespace

#if defined(__EMSCRIPTEN__)
// For wasm, we don't use async compilation.
struct DawnGraphicsPipeline::AsyncPipelineCreation : public AsyncPipelineCreationBase {};
#else
struct DawnGraphicsPipeline::AsyncPipelineCreation : public AsyncPipelineCreationBase {
    wgpu::Future fFuture;
};
#endif

// static
sk_sp<DawnGraphicsPipeline> DawnGraphicsPipeline::Make(const DawnSharedContext* sharedContext,
                                                       DawnResourceProvider* resourceProvider,
                                                       const RuntimeEffectDictionary* runtimeDict,
                                                       const GraphicsPipelineDesc& pipelineDesc,
                                                       const RenderPassDesc& renderPassDesc) {
    const DawnCaps& caps = *static_cast<const DawnCaps*>(sharedContext->caps());
    const auto& device = sharedContext->device();

    SkSL::Program::Interface vsInterface, fsInterface;
    SkSL::ProgramSettings settings;

    settings.fForceNoRTFlip = true;

    ShaderErrorHandler* errorHandler = caps.shaderErrorHandler();

    const RenderStep* step = sharedContext->rendererProvider()->lookup(pipelineDesc.renderStepID());
    const bool useStorageBuffers = caps.storageBufferPreferred();

    std::string vsCode, fsCode;
    wgpu::ShaderModule fsModule, vsModule;

    // Some steps just render depth buffer but not color buffer, so the fragment
    // shader is null.
    UniquePaintParamsID paintID = pipelineDesc.paintParamsID();
    FragSkSLInfo fsSkSLInfo = BuildFragmentSkSL(&caps,
                                                sharedContext->shaderCodeDictionary(),
                                                runtimeDict,
                                                step,
                                                paintID,
                                                useStorageBuffers,
                                                renderPassDesc.fWriteSwizzle);
    std::string& fsSkSL = fsSkSLInfo.fSkSL;
    const BlendInfo& blendInfo = fsSkSLInfo.fBlendInfo;
    const bool localCoordsNeeded = fsSkSLInfo.fRequiresLocalCoords;
    const int numTexturesAndSamplers = fsSkSLInfo.fNumTexturesAndSamplers;

    bool hasFragmentSkSL = !fsSkSL.empty();
    if (hasFragmentSkSL) {
        if (!skgpu::SkSLToWGSL(caps.shaderCaps(),
                               fsSkSL,
                               SkSL::ProgramKind::kGraphiteFragment,
                               settings,
                               &fsCode,
                               &fsInterface,
                               errorHandler)) {
            return {};
        }
        if (!DawnCompileWGSLShaderModule(sharedContext, fsSkSLInfo.fLabel.c_str(), fsCode,
                                         &fsModule, errorHandler)) {
            return {};
        }
    }

    VertSkSLInfo vsSkSLInfo = BuildVertexSkSL(caps.resourceBindingRequirements(),
                                              step,
                                              useStorageBuffers,
                                              localCoordsNeeded);
    const std::string& vsSkSL = vsSkSLInfo.fSkSL;
    if (!skgpu::SkSLToWGSL(caps.shaderCaps(),
                           vsSkSL,
                           SkSL::ProgramKind::kGraphiteVertex,
                           settings,
                           &vsCode,
                           &vsInterface,
                           errorHandler)) {
        return {};
    }
    if (!DawnCompileWGSLShaderModule(sharedContext, vsSkSLInfo.fLabel.c_str(), vsCode,
                                     &vsModule, errorHandler)) {
        return {};
    }

    std::string pipelineLabel =
            GetPipelineLabel(sharedContext->shaderCodeDictionary(), renderPassDesc, step, paintID);
    wgpu::RenderPipelineDescriptor descriptor;
    // Always set the label for pipelines, dawn may need it for tracing.
    descriptor.label = pipelineLabel.c_str();

    // Fragment state
    skgpu::BlendEquation equation = blendInfo.fEquation;
    skgpu::BlendCoeff srcCoeff = blendInfo.fSrcBlend;
    skgpu::BlendCoeff dstCoeff = blendInfo.fDstBlend;
    bool blendOn = !skgpu::BlendShouldDisable(equation, srcCoeff, dstCoeff);

    wgpu::BlendState blend;
    if (blendOn) {
        blend.color.operation = blend_equation_to_dawn_blend_op(equation);
        blend.color.srcFactor = blend_coeff_to_dawn_blend(caps, srcCoeff);
        blend.color.dstFactor = blend_coeff_to_dawn_blend(caps, dstCoeff);
        blend.alpha.operation = blend_equation_to_dawn_blend_op(equation);
        blend.alpha.srcFactor = blend_coeff_to_dawn_blend_for_alpha(caps, srcCoeff);
        blend.alpha.dstFactor = blend_coeff_to_dawn_blend_for_alpha(caps, dstCoeff);
    }

    wgpu::ColorTargetState colorTarget;
    colorTarget.format =
            renderPassDesc.fColorAttachment.fTextureInfo.dawnTextureSpec().getViewFormat();
    colorTarget.blend = blendOn ? &blend : nullptr;
    colorTarget.writeMask = blendInfo.fWritesColor && hasFragmentSkSL ? wgpu::ColorWriteMask::All
                                                                      : wgpu::ColorWriteMask::None;

    wgpu::FragmentState fragment;
    // Dawn doesn't allow having a color attachment but without fragment shader, so have to use a
    // noop fragment shader, if fragment shader is null.
    fragment.module = hasFragmentSkSL ? std::move(fsModule) : sharedContext->noopFragment();
    fragment.entryPoint = "main";
    fragment.targetCount = 1;
    fragment.targets = &colorTarget;
    descriptor.fragment = &fragment;

    // Depth stencil state
    const auto& depthStencilSettings = step->depthStencilSettings();
    SkASSERT(depthStencilSettings.fDepthTestEnabled ||
             depthStencilSettings.fDepthCompareOp == CompareOp::kAlways);
    wgpu::DepthStencilState depthStencil;
    if (renderPassDesc.fDepthStencilAttachment.fTextureInfo.isValid()) {
        wgpu::TextureFormat dsFormat =
                renderPassDesc.fDepthStencilAttachment.fTextureInfo.dawnTextureSpec()
                        .getViewFormat();
        depthStencil.format =
                DawnFormatIsDepthOrStencil(dsFormat) ? dsFormat : wgpu::TextureFormat::Undefined;
        if (depthStencilSettings.fDepthTestEnabled) {
            depthStencil.depthWriteEnabled = depthStencilSettings.fDepthWriteEnabled;
        }
        depthStencil.depthCompare = compare_op_to_dawn(depthStencilSettings.fDepthCompareOp);

        // Dawn validation fails if the stencil state is non-default and the
        // format doesn't have the stencil aspect.
        if (DawnFormatIsStencil(dsFormat) && depthStencilSettings.fStencilTestEnabled) {
            depthStencil.stencilFront = stencil_face_to_dawn(depthStencilSettings.fFrontStencil);
            depthStencil.stencilBack = stencil_face_to_dawn(depthStencilSettings.fBackStencil);
            depthStencil.stencilReadMask = depthStencilSettings.fFrontStencil.fReadMask;
            depthStencil.stencilWriteMask = depthStencilSettings.fFrontStencil.fWriteMask;
        }

        descriptor.depthStencil = &depthStencil;
    }

    // Pipeline layout
    BindGroupLayouts groupLayouts;
    {
        groupLayouts[0] = resourceProvider->getOrCreateUniformBuffersBindGroupLayout();
        if (!groupLayouts[0]) {
            return {};
        }

        bool hasFragmentSamplers = hasFragmentSkSL && numTexturesAndSamplers > 0;
        if (hasFragmentSamplers) {
            if (numTexturesAndSamplers == 2) {
                // Common case: single texture + sampler.
                groupLayouts[1] =
                        resourceProvider->getOrCreateSingleTextureSamplerBindGroupLayout();
            } else {
                std::vector<wgpu::BindGroupLayoutEntry> entries(numTexturesAndSamplers);
                for (int i = 0; i < numTexturesAndSamplers;) {
                    entries[i].binding = static_cast<uint32_t>(i);
                    entries[i].visibility = wgpu::ShaderStage::Fragment;
                    entries[i].sampler.type = wgpu::SamplerBindingType::Filtering;
                    ++i;
                    entries[i].binding = i;
                    entries[i].visibility = wgpu::ShaderStage::Fragment;
                    entries[i].texture.sampleType = wgpu::TextureSampleType::Float;
                    entries[i].texture.viewDimension = wgpu::TextureViewDimension::e2D;
                    entries[i].texture.multisampled = false;
                    ++i;
                }

                wgpu::BindGroupLayoutDescriptor groupLayoutDesc;
                if (sharedContext->caps()->setBackendLabels()) {
                    groupLayoutDesc.label = vsSkSLInfo.fLabel.c_str();
                }
                groupLayoutDesc.entryCount = entries.size();
                groupLayoutDesc.entries = entries.data();
                groupLayouts[1] = device.CreateBindGroupLayout(&groupLayoutDesc);
            }
            if (!groupLayouts[1]) {
                return {};
            }
        }

        wgpu::PipelineLayoutDescriptor layoutDesc;
        if (sharedContext->caps()->setBackendLabels()) {
            layoutDesc.label = fsSkSLInfo.fLabel.c_str();
        }
        layoutDesc.bindGroupLayoutCount =
            hasFragmentSamplers ? groupLayouts.size() : groupLayouts.size() - 1;
        layoutDesc.bindGroupLayouts = groupLayouts.data();
        auto layout = device.CreatePipelineLayout(&layoutDesc);
        if (!layout) {
            return {};
        }
        descriptor.layout = std::move(layout);
    }

    // Vertex state
    std::array<wgpu::VertexBufferLayout, kNumVertexBuffers> vertexBufferLayouts;
    // Vertex buffer layout
    std::vector<wgpu::VertexAttribute> vertexAttributes;
    {
        auto arrayStride = create_vertex_attributes(step->vertexAttributes(),
                                                    0,
                                                    &vertexAttributes);
        auto& layout = vertexBufferLayouts[kVertexBufferIndex];
        if (arrayStride) {
            layout.arrayStride = arrayStride;
            layout.stepMode = wgpu::VertexStepMode::Vertex;
            layout.attributeCount = vertexAttributes.size();
            layout.attributes = vertexAttributes.data();
        } else {
            layout.arrayStride = 0;
            layout.stepMode = wgpu::VertexStepMode::VertexBufferNotUsed;
            layout.attributeCount = 0;
            layout.attributes = nullptr;
        }
    }

    // Instance buffer layout
    std::vector<wgpu::VertexAttribute> instanceAttributes;
    {
        auto arrayStride = create_vertex_attributes(step->instanceAttributes(),
                                                    step->vertexAttributes().size(),
                                                    &instanceAttributes);
        auto& layout = vertexBufferLayouts[kInstanceBufferIndex];
        if (arrayStride) {
            layout.arrayStride = arrayStride;
            layout.stepMode = wgpu::VertexStepMode::Instance;
            layout.attributeCount = instanceAttributes.size();
            layout.attributes = instanceAttributes.data();
        } else {
            layout.arrayStride = 0;
            layout.stepMode = wgpu::VertexStepMode::VertexBufferNotUsed;
            layout.attributeCount = 0;
            layout.attributes = nullptr;
        }
    }

    auto& vertex = descriptor.vertex;
    vertex.module = std::move(vsModule);
    vertex.entryPoint = "main";
    vertex.constantCount = 0;
    vertex.constants = nullptr;
    vertex.bufferCount = vertexBufferLayouts.size();
    vertex.buffers = vertexBufferLayouts.data();

    // Other state
    descriptor.primitive.frontFace = wgpu::FrontFace::CCW;
    descriptor.primitive.cullMode = wgpu::CullMode::None;
    switch(step->primitiveType()) {
        case PrimitiveType::kTriangles:
            descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
            break;
        case PrimitiveType::kTriangleStrip:
            descriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleStrip;
            descriptor.primitive.stripIndexFormat = wgpu::IndexFormat::Uint16;
            break;
        case PrimitiveType::kPoints:
            descriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;
            break;
    }

    // Multisampled state
    descriptor.multisample.count = renderPassDesc.fSampleCount;
    [[maybe_unused]] bool isMSAARenderToSingleSampled =
            renderPassDesc.fSampleCount > 1 &&
            renderPassDesc.fColorAttachment.fTextureInfo.isValid() &&
            renderPassDesc.fColorAttachment.fTextureInfo.numSamples() == 1;
#if defined(__EMSCRIPTEN__)
    SkASSERT(!isMSAARenderToSingleSampled);
#else
    wgpu::DawnMultisampleStateRenderToSingleSampled pipelineMSAARenderToSingleSampledDesc;
    if (isMSAARenderToSingleSampled) {
        // If render pass is multi sampled but the color attachment is single sampled, we need
        // to activate multisampled render to single sampled feature for this graphics pipeline.
        SkASSERT(device.HasFeature(wgpu::FeatureName::MSAARenderToSingleSampled));

        descriptor.multisample.nextInChain = &pipelineMSAARenderToSingleSampledDesc;
        pipelineMSAARenderToSingleSampledDesc.enabled = true;
    }
#endif

    descriptor.multisample.mask = 0xFFFFFFFF;
    descriptor.multisample.alphaToCoverageEnabled = false;

    auto asyncCreation = std::make_unique<AsyncPipelineCreation>();

    if (caps.useAsyncPipelineCreation()) {
#if defined(__EMSCRIPTEN__)
        // We shouldn't use CreateRenderPipelineAsync in wasm.
        SKGPU_LOG_F("CreateRenderPipelineAsync shouldn't be used in WASM");
#else
        wgpu::CreateRenderPipelineAsyncCallbackInfo callbackInfo{};
        callbackInfo.mode = wgpu::CallbackMode::WaitAnyOnly;
        callbackInfo.userdata = asyncCreation.get();
        callbackInfo.callback = [](WGPUCreatePipelineAsyncStatus status,
                                   WGPURenderPipeline pipeline,
                                   char const* message,
                                   void* userdata) {
            auto* arg = static_cast<AsyncPipelineCreation*>(userdata);

            if (status != WGPUCreatePipelineAsyncStatus_Success) {
                SKGPU_LOG_E("Failed to create render pipeline (%d): %s", status, message);
                // invalidate AsyncPipelineCreation pointer to signal that this pipeline has failed.
                arg->fRenderPipeline = nullptr;
            } else {
                arg->fRenderPipeline = wgpu::RenderPipeline::Acquire(pipeline);
            }

            arg->fFinished = true;
        };

        asyncCreation->fFuture = device.CreateRenderPipelineAsync(&descriptor, callbackInfo);
#endif
    } else {
        std::optional<DawnErrorChecker> errorChecker;
        if (sharedContext->dawnCaps()->allowScopedErrorChecks()) {
            errorChecker.emplace(sharedContext);
        }
        asyncCreation->fRenderPipeline = device.CreateRenderPipeline(&descriptor);
        asyncCreation->fFinished = true;

        if (errorChecker.has_value() && errorChecker->popErrorScopes() != DawnErrorType::kNoError) {
            asyncCreation->fRenderPipeline = nullptr;
        }
    }
#if defined(GRAPHITE_TEST_UTILS)
    GraphicsPipeline::PipelineInfo pipelineInfo = {pipelineDesc.renderStepID(),
                                                   pipelineDesc.paintParamsID(),
                                                   std::move(vsSkSL),
                                                   std::move(fsSkSL),
                                                   std::move(vsCode),
                                                   std::move(fsCode)};
    GraphicsPipeline::PipelineInfo* pipelineInfoPtr = &pipelineInfo;
#else
    GraphicsPipeline::PipelineInfo* pipelineInfoPtr = nullptr;
#endif

    return sk_sp<DawnGraphicsPipeline>(
            new DawnGraphicsPipeline(sharedContext,
                                     pipelineInfoPtr,
                                     std::move(asyncCreation),
                                     std::move(groupLayouts),
                                     step->primitiveType(),
                                     depthStencilSettings.fStencilReferenceValue,
                                     /*hasStepUniforms=*/!step->uniforms().empty(),
                                     /*hasPaintUniforms=*/fsSkSLInfo.fNumPaintUniforms > 0,
                                     numTexturesAndSamplers));
}

DawnGraphicsPipeline::DawnGraphicsPipeline(const skgpu::graphite::SharedContext* sharedContext,
                                           PipelineInfo* pipelineInfo,
                                           std::unique_ptr<AsyncPipelineCreation> asyncCreationInfo,
                                           BindGroupLayouts groupLayouts,
                                           PrimitiveType primitiveType,
                                           uint32_t refValue,
                                           bool hasStepUniforms,
                                           bool hasPaintUniforms,
                                           int numFragmentTexturesAndSamplers)
        : GraphicsPipeline(sharedContext, pipelineInfo)
        , fAsyncPipelineCreation(std::move(asyncCreationInfo))
        , fGroupLayouts(std::move(groupLayouts))
        , fPrimitiveType(primitiveType)
        , fStencilReferenceValue(refValue)
        , fHasStepUniforms(hasStepUniforms)
        , fHasPaintUniforms(hasPaintUniforms)
        , fNumFragmentTexturesAndSamplers(numFragmentTexturesAndSamplers) {}

DawnGraphicsPipeline::~DawnGraphicsPipeline() {
    this->freeGpuData();
}

void DawnGraphicsPipeline::freeGpuData() {
    // Wait for async creation to finish before we can destroy this object.
    (void)this->dawnRenderPipeline();
    fAsyncPipelineCreation = nullptr;
}

const wgpu::RenderPipeline& DawnGraphicsPipeline::dawnRenderPipeline() const {
    if (!fAsyncPipelineCreation) {
        static const wgpu::RenderPipeline kNullPipeline = nullptr;
        return kNullPipeline;
    }
    if (fAsyncPipelineCreation->fFinished) {
        return fAsyncPipelineCreation->fRenderPipeline;
    }
#if defined(__EMSCRIPTEN__)
    // We shouldn't use CreateRenderPipelineAsync in wasm.
    SKGPU_LOG_F("CreateRenderPipelineAsync shouldn't be used in WASM");
#else
    wgpu::FutureWaitInfo waitInfo{};
    waitInfo.future = fAsyncPipelineCreation->fFuture;
    const auto& instance = static_cast<const DawnSharedContext*>(sharedContext())
                                   ->device()
                                   .GetAdapter()
                                   .GetInstance();

    [[maybe_unused]] auto status =
            instance.WaitAny(1, &waitInfo, /*timeoutNS=*/std::numeric_limits<uint64_t>::max());
    SkASSERT(status == wgpu::WaitStatus::Success);
    SkASSERT(waitInfo.completed);
#endif

    return fAsyncPipelineCreation->fRenderPipeline;
}

} // namespace skgpu::graphite
