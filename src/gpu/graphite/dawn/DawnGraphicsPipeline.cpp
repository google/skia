/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnGraphicsPipeline.h"

#include "include/gpu/graphite/TextureInfo.h"
#include "include/gpu/graphite/dawn/DawnGraphiteTypes.h"
#include "include/private/base/SkTemplates.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/SkSLToBackend.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/ShaderInfo.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/UniformManager.h"
#include "src/gpu/graphite/dawn/DawnCaps.h"
#include "src/gpu/graphite/dawn/DawnErrorChecker.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtils.h"
#include "src/gpu/graphite/dawn/DawnResourceProvider.h"
#include "src/gpu/graphite/dawn/DawnSampler.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLProgram.h"

#include <atomic>
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
        case VertexAttribType::kUInt2:
            return wgpu::VertexFormat::Uint32x2;
        case VertexAttribType::kByte2:
            return wgpu::VertexFormat::Sint8x2;
        case VertexAttribType::kByte4:
            return wgpu::VertexFormat::Sint8x4;
        case VertexAttribType::kUByte2:
            return wgpu::VertexFormat::Uint8x2;
        case VertexAttribType::kUByte4:
            return wgpu::VertexFormat::Uint8x4;
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
        case VertexAttribType::kUShort4_norm:
            return wgpu::VertexFormat::Unorm16x4;
        case VertexAttribType::kHalf:
        case VertexAttribType::kByte:
        case VertexAttribType::kUByte:
        case VertexAttribType::kUByte_norm:
        case VertexAttribType::kUShort_norm:
            // Not supported.
            break;
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
    AsyncPipelineCreationBase(const UniqueKey& key) : fKey(key) {}

    wgpu::RenderPipeline fRenderPipeline;
    std::atomic<bool> fFinished = false;
    UniqueKey fKey; // for logging the wait to resolve a Pipeline future in dawnRenderPipeline
#if SK_HISTOGRAMS_ENABLED
    // We need these three for the Graphite.PipelineCreationTimes.* histograms (cf.
    // log_pipeline_creation)
    skgpu::StdSteadyClock::time_point fStartTime;
    bool fFromPrecompile;
    bool fAsynchronous = false;
#endif
};

void log_pipeline_creation(const AsyncPipelineCreationBase* apcb) {
#if SK_HISTOGRAMS_ENABLED
    [[maybe_unused]] static constexpr int kBucketCount = 100;
    [[maybe_unused]] static constexpr int kOneSecInUS = 1000000;

    SkASSERT(apcb->fFinished);

    if (!apcb->fRenderPipeline) {
        // A null fRenderPipeline means Pipeline creation failed
        return; // TODO: log failures to their own UMA stat
    }

    [[maybe_unused]] auto micros_since = [](skgpu::StdSteadyClock::time_point start) {
        skgpu::StdSteadyClock::duration elapsed = skgpu::StdSteadyClock::now() - start;
        return std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    };

    if (apcb->fFromPrecompile) {
        SkASSERT(!apcb->fAsynchronous);     // precompile is done synchronously on a thread
        SK_HISTOGRAM_CUSTOM_MICROSECONDS_TIMES(
            "Graphite.PipelineCreationTimes.Precompile",
            micros_since(apcb->fStartTime),
            /* minUSec= */ 1,
            /* maxUSec= */ kOneSecInUS,
            kBucketCount);
    } else if (apcb->fAsynchronous) {
        SK_HISTOGRAM_CUSTOM_MICROSECONDS_TIMES(
            "Graphite.PipelineCreationTimes.Asynchronous",
            micros_since(apcb->fStartTime),
            /* minUSec= */ 1,
            /* maxUSec= */ kOneSecInUS,
            kBucketCount);
    } else {
        SK_HISTOGRAM_CUSTOM_MICROSECONDS_TIMES(
            "Graphite.PipelineCreationTimes.Synchronous",
            micros_since(apcb->fStartTime),
            /* minUSec= */ 1,
            /* maxUSec= */ kOneSecInUS,
            kBucketCount);
    }
#endif // SK_HISTOGRAMS_ENABLED
}

} // anonymous namespace

#if defined(__EMSCRIPTEN__)
// For wasm, we don't use async compilation.
struct DawnGraphicsPipeline::AsyncPipelineCreation : public AsyncPipelineCreationBase {
    AsyncPipelineCreation(const UniqueKey& key) : AsyncPipelineCreationBase(key) {}
};
#else
struct DawnGraphicsPipeline::AsyncPipelineCreation : public AsyncPipelineCreationBase {
    AsyncPipelineCreation(const UniqueKey& key) : AsyncPipelineCreationBase(key) {}

    wgpu::Future fFuture;
};
#endif

// static
sk_sp<DawnGraphicsPipeline> DawnGraphicsPipeline::Make(
        const DawnSharedContext* sharedContext,
        DawnResourceProvider* resourceProvider,
        const RuntimeEffectDictionary* runtimeDict,
        const UniqueKey& pipelineKey,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc,
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags,
        uint32_t compilationID) {
    const DawnCaps& caps = *static_cast<const DawnCaps*>(sharedContext->caps());
    const auto& device = sharedContext->device();

    SkSL::Program::Interface vsInterface, fsInterface;

    SkSL::ProgramSettings settings;
    settings.fSharpenTextures = true;
    settings.fForceNoRTFlip = true;

    ShaderErrorHandler* errorHandler = caps.shaderErrorHandler();

    const RenderStep* step = sharedContext->rendererProvider()->lookup(pipelineDesc.renderStepID());
    const bool useStorageBuffers = caps.storageBufferSupport();

    SkSL::NativeShader vsCode, fsCode;
    wgpu::ShaderModule fsModule, vsModule;

    // Some steps just render depth buffer but not color buffer, so the fragment
    // shader is null.
    UniquePaintParamsID paintID = pipelineDesc.paintParamsID();

    skia_private::TArray<SamplerDesc>* samplerDescArrPtr = nullptr;
#if !defined(__EMSCRIPTEN__)
    skia_private::TArray<SamplerDesc> samplerDescArr {};
    samplerDescArrPtr = &samplerDescArr;
#endif

    std::unique_ptr<ShaderInfo> shaderInfo =
            ShaderInfo::Make(&caps,
                             sharedContext->shaderCodeDictionary(),
                             runtimeDict,
                             step,
                             paintID,
                             useStorageBuffers,
                             renderPassDesc.fColorAttachment.fFormat,
                             renderPassDesc.fWriteSwizzle,
                             renderPassDesc.fDstReadStrategy,
                             samplerDescArrPtr);

    const std::string& fsSkSL = shaderInfo->fragmentSkSL();
    const BlendInfo& blendInfo = shaderInfo->blendInfo();
    const int numTexturesAndSamplers = shaderInfo->numFragmentTexturesAndSamplers();

    const bool hasFragmentSkSL = !fsSkSL.empty();
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
        if (!DawnCompileWGSLShaderModule(sharedContext, shaderInfo->fsLabel().c_str(), fsCode,
                                         &fsModule, errorHandler)) {
            return {};
        }
    }

    const std::string& vsSkSL = shaderInfo->vertexSkSL();
    if (!skgpu::SkSLToWGSL(caps.shaderCaps(),
                           vsSkSL,
                           SkSL::ProgramKind::kGraphiteVertex,
                           settings,
                           &vsCode,
                           &vsInterface,
                           errorHandler)) {
        return {};
    }
    if (!DawnCompileWGSLShaderModule(sharedContext, shaderInfo->vsLabel().c_str(), vsCode,
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
    colorTarget.format = TextureFormatToDawnFormat(renderPassDesc.fColorAttachment.fFormat);
    colorTarget.blend = blendOn ? &blend : nullptr;
    colorTarget.writeMask = blendInfo.fWritesColor && hasFragmentSkSL ? wgpu::ColorWriteMask::All
                                                                      : wgpu::ColorWriteMask::None;

#if !defined(__EMSCRIPTEN__)
    const bool loadMsaaFromResolve =
            renderPassDesc.fColorResolveAttachment.fFormat != TextureFormat::kUnsupported &&
            renderPassDesc.fColorResolveAttachment.fLoadOp == LoadOp::kLoad;
    // Special case: a render pass loading resolve texture requires additional settings for the
    // pipeline to make it compatible.
    wgpu::ColorTargetStateExpandResolveTextureDawn pipelineMSAALoadResolveTextureDesc;
    if (loadMsaaFromResolve && sharedContext->dawnCaps()->loadOpAffectsMSAAPipelines()) {
        SkASSERT(device.HasFeature(wgpu::FeatureName::DawnLoadResolveTexture));
        colorTarget.nextInChain = &pipelineMSAALoadResolveTextureDesc;
        pipelineMSAALoadResolveTextureDesc.enabled = true;
    }
#endif

    wgpu::FragmentState fragment;
    // Dawn doesn't allow having a color attachment but without fragment shader, so have to use a
    // noop fragment shader, if fragment shader is null.
    fragment.module = hasFragmentSkSL ? std::move(fsModule) : sharedContext->noopFragment();
    fragment.entryPoint = "main";
    fragment.constantCount = 0;
    fragment.constants = nullptr;
    fragment.targetCount = 1;
    fragment.targets = &colorTarget;
    descriptor.fragment = &fragment;

    // Depth stencil state
    const auto& depthStencilSettings = step->depthStencilSettings();
    SkASSERT(depthStencilSettings.fDepthTestEnabled ||
             depthStencilSettings.fDepthCompareOp == CompareOp::kAlways);
    TextureFormat dsFormat = renderPassDesc.fDepthStencilAttachment.fFormat;
    wgpu::DepthStencilState depthStencil;
    if (dsFormat != TextureFormat::kUnsupported) {
        SkASSERT(TextureFormatIsDepthOrStencil(dsFormat));
        depthStencil.format = TextureFormatToDawnFormat(dsFormat);
        if (depthStencilSettings.fDepthTestEnabled) {
            depthStencil.depthWriteEnabled = depthStencilSettings.fDepthWriteEnabled;
        }
        depthStencil.depthCompare = compare_op_to_dawn(depthStencilSettings.fDepthCompareOp);

        // Dawn validation fails if the stencil state is non-default and the
        // format doesn't have the stencil aspect.
        if (TextureFormatHasStencil(dsFormat) && depthStencilSettings.fStencilTestEnabled) {
            depthStencil.stencilFront = stencil_face_to_dawn(depthStencilSettings.fFrontStencil);
            depthStencil.stencilBack = stencil_face_to_dawn(depthStencilSettings.fBackStencil);
            depthStencil.stencilReadMask = depthStencilSettings.fFrontStencil.fReadMask;
            depthStencil.stencilWriteMask = depthStencilSettings.fFrontStencil.fWriteMask;
        }

        descriptor.depthStencil = &depthStencil;
    }

    // Determine the BindGroupLayouts that will be used to make up the pipeline layout.
    BindGroupLayouts groupLayouts;
    // If immutable samplers are used with this pipeline, they must be included in the pipeline
    // layout and passed in to the pipline constructor for lifetime management.
    skia_private::TArray<sk_sp<DawnSampler>> immutableSamplers;
    {
        SkASSERT(resourceProvider);
        groupLayouts[0] = resourceProvider->getOrCreateUniformBuffersBindGroupLayout();
        if (!groupLayouts[0]) {
            return {};
        }

        bool hasFragmentSamplers = hasFragmentSkSL && numTexturesAndSamplers > 0;
        if (hasFragmentSamplers) {
            // Check if we can optimize for the common case of a single texture + 1 dynamic sampler
            if (numTexturesAndSamplers == 2 &&
                !(samplerDescArrPtr && samplerDescArrPtr->at(0).isImmutable())) {
                groupLayouts[1] =
                        resourceProvider->getOrCreateSingleTextureSamplerBindGroupLayout();
            } else {
                std::vector<wgpu::BindGroupLayoutEntry> entries(numTexturesAndSamplers);
#if !defined(__EMSCRIPTEN__)
                // Static sampler layouts are passed into Dawn by address and therefore must stay
                // alive until the BindGroupLayoutDescriptor is created. So, store them outside of
                // the loop that iterates over each BindGroupLayoutEntry.
                skia_private::TArray<wgpu::StaticSamplerBindingLayout> staticSamplerLayouts;

                // Note that the number of samplers is equivalent to numTexturesAndSamplers / 2. So,
                // a sampler's index within any container that only pertains to sampler information
                // (as in, excludes textures) is equivalent to 1/2 of that sampler's binding index
                // within the BindGroupLayout. Assert that we have analyzed the appropriate number
                // of samplers by equating samplerDescArr size to sampler quantity.
                SkASSERT(samplerDescArrPtr && samplerDescArr.size() == numTexturesAndSamplers / 2);
                immutableSamplers.reset(samplerDescArr.size());
#endif

                for (int i = 0; i < numTexturesAndSamplers;) {
                    entries[i].binding = i;
                    entries[i].visibility = wgpu::ShaderStage::Fragment;
#if !defined(__EMSCRIPTEN__)
                    // Index of sampler information = 1/2 of cumulative texture and sampler index.
                    // If we have a non-default-initialized SamplerDesc at that index,
                    // fetch an immutable sampler that matches that description to include in the
                    // pipeline layout.
                    const SamplerDesc& samplerDesc = samplerDescArr.at(i/2);
                    if (samplerDesc.isImmutable()) {
                        sk_sp<Sampler> immutableSampler =
                                resourceProvider->findOrCreateCompatibleSampler(samplerDesc);
                        if (!immutableSampler) {
                            SKGPU_LOG_E("Failed to find/create immutable sampler for pipeline");
                            return {};
                        }
                        sk_sp<DawnSampler> dawnImmutableSampler = sk_ref_sp<DawnSampler>(
                                static_cast<DawnSampler*>(immutableSampler.get()));
                        SkASSERT(dawnImmutableSampler);

                        wgpu::StaticSamplerBindingLayout& immutableSamplerBinding =
                                staticSamplerLayouts.emplace_back();
                        immutableSamplerBinding.sampler = dawnImmutableSampler->dawnSampler();
                        // Static samplers sample from the subsequent texture in the BindGroupLayout
                        immutableSamplerBinding.sampledTextureBinding = i + 1;

                        immutableSamplers[i/2] = std::move(dawnImmutableSampler);
                        entries[i].nextInChain = &immutableSamplerBinding;
                    } else {
#endif
                        entries[i].sampler.type = wgpu::SamplerBindingType::Filtering;
#if !defined(__EMSCRIPTEN__)
                    }
#endif
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
                    groupLayoutDesc.label = shaderInfo->vsLabel().c_str();
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
            layoutDesc.label = shaderInfo->fsLabel().c_str();
        }
        layoutDesc.bindGroupLayoutCount =
            hasFragmentSamplers ? groupLayouts.size() : groupLayouts.size() - 1;
        layoutDesc.bindGroupLayouts = groupLayouts.data();
#if !defined(__EMSCRIPTEN__)
        if (sharedContext->caps()
                    ->resourceBindingRequirements()
                    .fUsePushConstantsForIntrinsicConstants) {
            layoutDesc.immediateSize = kIntrinsicUniformSize;
        }
#endif
        auto layout = device.CreatePipelineLayout(&layoutDesc);
        if (!layout) {
            return {};
        }
        descriptor.layout = std::move(layout);
    }

    // Vertex state
    std::array<wgpu::VertexBufferLayout, kNumVertexBuffers> vertexBufferLayouts;
    // Static data buffer layout
    std::vector<wgpu::VertexAttribute> staticDataAttributes;
    {
        auto arrayStride = create_vertex_attributes(step->staticAttributes(),
                                                    0,
                                                    &staticDataAttributes);
        auto& layout = vertexBufferLayouts[kStaticDataBufferIndex];
        if (arrayStride) {
            layout.arrayStride = arrayStride;
            layout.stepMode = wgpu::VertexStepMode::Vertex;
            layout.attributeCount = staticDataAttributes.size();
            layout.attributes = staticDataAttributes.data();
        } else {
            layout.arrayStride = 0;
#if defined(__EMSCRIPTEN__)
            layout.stepMode = wgpu::VertexStepMode::VertexBufferNotUsed;
#else
            layout.stepMode = wgpu::VertexStepMode::Undefined;
#endif
            layout.attributeCount = 0;
            layout.attributes = nullptr;
        }
    }

    // Append data buffer layout
    std::vector<wgpu::VertexAttribute> appendDataAttributes;
    {
        // Note: the shaderLocationOffset in this function call needs to be the staticAttributeSize
        auto arrayStride = create_vertex_attributes(step->appendAttributes(),
                                                    step->staticAttributes().size(),
                                                    &appendDataAttributes);
        auto& layout = vertexBufferLayouts[kAppendDataBufferIndex];
        if (arrayStride) {
            layout.arrayStride = arrayStride;
            layout.stepMode = step->appendsVertices() ? wgpu::VertexStepMode::Vertex :
                                                        wgpu::VertexStepMode::Instance;
            layout.attributeCount = appendDataAttributes.size();
            layout.attributes = appendDataAttributes.data();
        } else {
            layout.arrayStride = 0;
#if defined(__EMSCRIPTEN__)
            layout.stepMode = wgpu::VertexStepMode::VertexBufferNotUsed;
#else
            layout.stepMode = wgpu::VertexStepMode::Undefined;
#endif
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
    switch (step->primitiveType()) {
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
    descriptor.multisample.mask = 0xFFFFFFFF;
    descriptor.multisample.alphaToCoverageEnabled = false;

    const bool forPrecompilation =
            SkToBool(pipelineCreationFlags & PipelineCreationFlags::kForPrecompilation);
    // For Dawn, we want Precompilation to happen synchronously
    const bool useAsync = caps.useAsyncPipelineCreation() && !forPrecompilation;

    auto asyncCreation = std::make_unique<AsyncPipelineCreation>(pipelineKey);
#if SK_HISTOGRAMS_ENABLED
    asyncCreation->fStartTime = skgpu::StdSteadyClock::now();
    asyncCreation->fFromPrecompile = forPrecompilation;
    asyncCreation->fAsynchronous = useAsync;
#endif

    if (useAsync) {
#if defined(__EMSCRIPTEN__)
        // We shouldn't use CreateRenderPipelineAsync in wasm.
        SKGPU_LOG_F("CreateRenderPipelineAsync shouldn't be used in WASM");
#else
        asyncCreation->fFuture = device.CreateRenderPipelineAsync(
                &descriptor,
                wgpu::CallbackMode::WaitAnyOnly,
                [asyncCreationPtr = asyncCreation.get()](wgpu::CreatePipelineAsyncStatus status,
                                                         wgpu::RenderPipeline pipeline,
                                                         wgpu::StringView message) {
                    if (status != wgpu::CreatePipelineAsyncStatus::Success) {
                        SKGPU_LOG_E("Failed to create render pipeline (%d): %.*s",
                                    static_cast<int>(status),
                                    static_cast<int>(message.length),
                                    message.data);
                        // invalidate AsyncPipelineCreation pointer to signal that this pipeline has
                        // failed.
                        asyncCreationPtr->fRenderPipeline = nullptr;
                    } else {
                        asyncCreationPtr->fRenderPipeline = std::move(pipeline);
                    }

                    asyncCreationPtr->fFinished = true;

                    log_pipeline_creation(asyncCreationPtr);
                });
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

        // Fail ASAP for synchronous pipeline creation so it affects the Recording snap instead of
        // being detected later at insertRecording().
        if (!asyncCreation->fRenderPipeline) {
            return nullptr;
        }

        log_pipeline_creation(asyncCreation.get());
    }

    PipelineInfo pipelineInfo{ *shaderInfo, pipelineCreationFlags,
                               pipelineKey.hash(), compilationID };
#if defined(GPU_TEST_UTILS)
    pipelineInfo.fNativeVertexShader = std::move(vsCode.fText);
    pipelineInfo.fNativeFragmentShader = std::move(fsCode.fText);
#endif

    return sk_sp<DawnGraphicsPipeline>(
            new DawnGraphicsPipeline(sharedContext,
                                     pipelineInfo,
                                     std::move(asyncCreation),
                                     std::move(groupLayouts),
                                     step->primitiveType(),
                                     depthStencilSettings.fStencilReferenceValue,
                                     std::move(immutableSamplers)));
}

DawnGraphicsPipeline::DawnGraphicsPipeline(
        const skgpu::graphite::SharedContext* sharedContext,
        const PipelineInfo& pipelineInfo,
        std::unique_ptr<AsyncPipelineCreation> asyncCreationInfo,
        BindGroupLayouts groupLayouts,
        PrimitiveType primitiveType,
        uint32_t refValue,
        skia_private::TArray<sk_sp<DawnSampler>> immutableSamplers)
    : GraphicsPipeline(sharedContext, pipelineInfo)
    , fAsyncPipelineCreation(std::move(asyncCreationInfo))
    , fGroupLayouts(std::move(groupLayouts))
    , fPrimitiveType(primitiveType)
    , fStencilReferenceValue(refValue)
    , fImmutableSamplers(std::move(immutableSamplers)) {}

DawnGraphicsPipeline::~DawnGraphicsPipeline() {
    this->freeGpuData();
}

void DawnGraphicsPipeline::freeGpuData() {
    // Wait for async creation to finish before we can destroy this object.
    (void)this->dawnRenderPipeline();
    fAsyncPipelineCreation = nullptr;
}

bool DawnGraphicsPipeline::didAsyncCompilationFail() const {
    return fAsyncPipelineCreation &&
           fAsyncPipelineCreation->fFinished &&
           !fAsyncPipelineCreation->fRenderPipeline;
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

#if defined(SK_PIPELINE_LIFETIME_LOGGING)
    TRACE_EVENT_INSTANT2("skia.gpu",
                         TRACE_STR_STATIC("WaitBeginN"),
                         TRACE_EVENT_SCOPE_THREAD,
                         "key", fAsyncPipelineCreation->fKey.hash(),
                         "compilationID", this->getPipelineInfo().fCompilationID);
#endif

    wgpu::FutureWaitInfo waitInfo{};
    waitInfo.future = fAsyncPipelineCreation->fFuture;
    const auto& instance = static_cast<const DawnSharedContext*>(sharedContext())->instance();

    [[maybe_unused]] auto status =
            instance.WaitAny(1, &waitInfo, /*timeoutNS=*/std::numeric_limits<uint64_t>::max());
    SkASSERT(status == wgpu::WaitStatus::Success);
    SkASSERT(waitInfo.completed);

#if defined(SK_PIPELINE_LIFETIME_LOGGING)
    TRACE_EVENT_INSTANT2("skia.gpu",
                         TRACE_STR_STATIC("WaitEndN"),
                         TRACE_EVENT_SCOPE_THREAD,
                         "key", fAsyncPipelineCreation->fKey.hash(),
                         "compilationID", this->getPipelineInfo().fCompilationID);
#endif

#endif

    return fAsyncPipelineCreation->fRenderPipeline;
}

} // namespace skgpu::graphite
