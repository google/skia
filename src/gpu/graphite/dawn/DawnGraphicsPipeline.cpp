/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnGraphicsPipeline.h"

#include "include/gpu/graphite/TextureInfo.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/dawn/DawnResourceProvider.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#include "src/gpu/graphite/dawn/DawnUtilsPriv.h"
#include "src/sksl/SkSLProgramSettings.h"
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
static wgpu::BlendFactor blend_coeff_to_dawn_blend(skgpu::BlendCoeff coeff) {
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
        case skgpu::BlendCoeff::kIS2C:
        case skgpu::BlendCoeff::kS2A:
        case skgpu::BlendCoeff::kIS2A:
        case skgpu::BlendCoeff::kIllegal:
            return wgpu::BlendFactor::Zero;
    }
    SkUNREACHABLE;
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

} // anonymous namespace

// static
sk_sp<DawnGraphicsPipeline> DawnGraphicsPipeline::Make(const DawnSharedContext* sharedContext,
                                                       SkSL::Compiler* compiler,
                                                       const RuntimeEffectDictionary* runtimeDict,
                                                       const GraphicsPipelineDesc& pipelineDesc,
                                                       const RenderPassDesc& renderPassDesc) {
    const auto& device = sharedContext->device();

    SkSL::Program::Inputs vsInputs, fsInputs;
    SkSL::ProgramSettings settings;

    settings.fForceNoRTFlip = true;
    settings.fSPIRVDawnCompatMode = true;

    ShaderErrorHandler* errorHandler = sharedContext->caps()->shaderErrorHandler();

    const RenderStep* step =
            sharedContext->rendererProvider()->lookup(pipelineDesc.renderStepID());

    bool useShadingSsboIndex =
            sharedContext->caps()->storageBufferPreferred() && step->performsShading();

    std::string vsSPIRV, fsSPIRV;
    wgpu::ShaderModule fsModule, vsModule;

    // Some steps just render depth buffer but not color buffer, so the fragment
    // shader is null.
    const FragSkSLInfo fsSkSLInfo = GetSkSLFS(sharedContext->caps()->resourceBindingRequirements(),
                                              sharedContext->shaderCodeDictionary(),
                                              runtimeDict,
                                              step,
                                              pipelineDesc.paintParamsID(),
                                              useShadingSsboIndex);
    const std::string& fsSKSL = fsSkSLInfo.fSkSL;
    const BlendInfo& blendInfo = fsSkSLInfo.fBlendInfo;
    const bool localCoordsNeeded = fsSkSLInfo.fRequiresLocalCoords;
    const int numTexturesAndSamplers = fsSkSLInfo.fNumTexturesAndSamplers;

    bool hasFragment = !fsSKSL.empty();
    if (hasFragment) {
        if (!SkSLToSPIRV(compiler,
                         fsSKSL,
                         SkSL::ProgramKind::kGraphiteFragment,
                         settings,
                         &fsSPIRV,
                         &fsInputs,
                         errorHandler)) {
            return {};
        }
        fsModule = DawnCompileSPIRVShaderModule(sharedContext,
                                                fsSPIRV,
                                                errorHandler);
        if (!fsModule) {
            return {};
        }
    }

    if (!SkSLToSPIRV(compiler,
                     GetSkSLVS(sharedContext->caps()->resourceBindingRequirements(),
                               step,
                               useShadingSsboIndex,
                               localCoordsNeeded),
                     SkSL::ProgramKind::kGraphiteVertex,
                     settings,
                     &vsSPIRV,
                     &vsInputs,
                     errorHandler)) {
        return {};
    }

    vsModule = DawnCompileSPIRVShaderModule(sharedContext, vsSPIRV, errorHandler);
    if (!vsModule) {
        return {};
    }

    wgpu::RenderPipelineDescriptor descriptor;
#if defined(SK_DEBUG)
    descriptor.label = step->name();
#endif

    // Fragment state
    skgpu::BlendEquation equation = blendInfo.fEquation;
    skgpu::BlendCoeff srcCoeff = blendInfo.fSrcBlend;
    skgpu::BlendCoeff dstCoeff = blendInfo.fDstBlend;
    bool blendOn = !skgpu::BlendShouldDisable(equation, srcCoeff, dstCoeff);

    wgpu::BlendState blend;
    if (blendOn) {
        blend.color.operation = blend_equation_to_dawn_blend_op(equation);
        blend.color.srcFactor = blend_coeff_to_dawn_blend(srcCoeff);
        blend.color.dstFactor = blend_coeff_to_dawn_blend(dstCoeff);
        blend.alpha.operation = blend_equation_to_dawn_blend_op(equation);
        blend.alpha.srcFactor = blend_coeff_to_dawn_blend(srcCoeff);
        blend.alpha.dstFactor = blend_coeff_to_dawn_blend(dstCoeff);
    }

    wgpu::ColorTargetState colorTarget;
    colorTarget.format = renderPassDesc.fColorAttachment.fTextureInfo.dawnTextureSpec().fFormat;
    colorTarget.blend = blendOn ? &blend : nullptr;
    colorTarget.writeMask = blendInfo.fWritesColor && hasFragment ? wgpu::ColorWriteMask::All
                                                                  : wgpu::ColorWriteMask::None;

    wgpu::FragmentState fragment;
    // Dawn doesn't allow having a color attachment but without fragment shader, so have to use a
    // noop fragment shader, if fragment shader is null.
    fragment.module = hasFragment ? std::move(fsModule) : sharedContext->noopFragment();
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
                renderPassDesc.fDepthStencilAttachment.fTextureInfo.dawnTextureSpec().fFormat;
        depthStencil.format =
                DawnFormatIsDepthOrStencil(dsFormat) ? dsFormat : wgpu::TextureFormat::Undefined;
        if (depthStencilSettings.fDepthTestEnabled) {
            depthStencil.depthWriteEnabled = depthStencilSettings.fDepthWriteEnabled;
        }
        depthStencil.depthCompare = compare_op_to_dawn(depthStencilSettings.fDepthCompareOp);
        depthStencil.stencilFront = stencil_face_to_dawn(depthStencilSettings.fFrontStencil);
        depthStencil.stencilBack = stencil_face_to_dawn(depthStencilSettings.fBackStencil);
        depthStencil.stencilReadMask = depthStencilSettings.fFrontStencil.fReadMask;
        depthStencil.stencilWriteMask = depthStencilSettings.fFrontStencil.fWriteMask;

        descriptor.depthStencil = &depthStencil;
    }

    // Pipeline layout
    {
        std::array<wgpu::BindGroupLayout, 2> groupLayouts;
        {
            std::array<wgpu::BindGroupLayoutEntry, 3> entries;
            entries[0].binding = kIntrinsicUniformBufferIndex;
            entries[0].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
            entries[0].buffer.type = wgpu::BufferBindingType::Uniform;
            entries[0].buffer.hasDynamicOffset = false;
            entries[0].buffer.minBindingSize = 0;

            uint32_t numBuffers = 1;

            if (!step->uniforms().empty()) {
                entries[numBuffers].binding = kRenderStepUniformBufferIndex;
                entries[numBuffers].visibility =
                        wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
                entries[numBuffers].buffer.type = wgpu::BufferBindingType::Uniform;
                entries[numBuffers].buffer.hasDynamicOffset = false;
                entries[numBuffers].buffer.minBindingSize = 0;
                ++numBuffers;
            }

            if (hasFragment) {
                entries[numBuffers].binding = kPaintUniformBufferIndex;
                entries[numBuffers].visibility = wgpu::ShaderStage::Fragment;
                entries[numBuffers].buffer.type = wgpu::BufferBindingType::Uniform;
                entries[numBuffers].buffer.hasDynamicOffset = false;
                entries[numBuffers].buffer.minBindingSize = 0;
                ++numBuffers;
            }

            wgpu::BindGroupLayoutDescriptor groupLayoutDesc;
#if defined(SK_DEBUG)
            groupLayoutDesc.label = step->name();
#endif

            groupLayoutDesc.entryCount = numBuffers;
            groupLayoutDesc.entries = entries.data();
            groupLayouts[0] = device.CreateBindGroupLayout(&groupLayoutDesc);
            if (!groupLayouts[0]) {
                return {};
            }
        }

        bool hasFragmentSamplers = hasFragment && numTexturesAndSamplers > 0;
        if (hasFragmentSamplers) {
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
#if defined(SK_DEBUG)
            groupLayoutDesc.label = step->name();
#endif
            groupLayoutDesc.entryCount = entries.size();
            groupLayoutDesc.entries = entries.data();
            groupLayouts[1] = device.CreateBindGroupLayout(&groupLayoutDesc);
            if (!groupLayouts[1]) {
                return {};
            }
        }

        wgpu::PipelineLayoutDescriptor layoutDesc;
#if defined(SK_DEBUG)
        layoutDesc.label = step->name();
#endif
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
            break;
        case PrimitiveType::kPoints:
            descriptor.primitive.topology = wgpu::PrimitiveTopology::PointList;
            break;
    }
    descriptor.primitive.stripIndexFormat = wgpu::IndexFormat::Uint16;

    descriptor.multisample.count = renderPassDesc.fColorAttachment.fTextureInfo.numSamples();
    descriptor.multisample.mask = 0xFFFFFFFF;
    descriptor.multisample.alphaToCoverageEnabled = false;

    auto pipeline = device.CreateRenderPipeline(&descriptor);
    if (!pipeline) {
        return {};
    }

    return sk_sp<DawnGraphicsPipeline>(
            new DawnGraphicsPipeline(sharedContext,
                                     std::move(pipeline),
                                     step->primitiveType(),
                                     depthStencilSettings.fStencilReferenceValue,
                                     !step->uniforms().empty(),
                                     hasFragment));
}

void DawnGraphicsPipeline::freeGpuData() {
    fRenderPipeline = nullptr;
}

const wgpu::RenderPipeline& DawnGraphicsPipeline::dawnRenderPipeline() const {
    return fRenderPipeline;
}

} // namespace skgpu::graphite
