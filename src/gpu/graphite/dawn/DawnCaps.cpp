/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnCaps.h"

#include <algorithm>
#include <string>

#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/gpu/graphite/dawn/DawnBackendContext.h"
#include "include/gpu/graphite/dawn/DawnGraphiteTypes.h"
#include "src/gpu/SwizzlePriv.h"
#include "src/gpu/graphite/ComputePipelineDesc.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/GraphiteResourceKey.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/dawn/DawnGraphicsPipeline.h"
#include "src/gpu/graphite/dawn/DawnGraphiteUtils.h"
#include "src/sksl/SkSLUtil.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten/version.h>
#endif

namespace skgpu::graphite {

namespace {

skgpu::UniqueKey::Domain get_pipeline_domain() {
    static const skgpu::UniqueKey::Domain kDawnGraphicsPipelineDomain =
            skgpu::UniqueKey::GenerateDomain();

    return kDawnGraphicsPipelineDomain;
}

bool is_valid_view(const DawnTextureInfo& dawnInfo) {
#if !defined(__EMSCRIPTEN__)
    switch (dawnInfo.fFormat) {
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar422Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar444Unorm:
            if (dawnInfo.fAspect == wgpu::TextureAspect::Plane0Only) {
                return dawnInfo.getViewFormat() == wgpu::TextureFormat::R8Unorm;
            } else if (dawnInfo.fAspect == wgpu::TextureAspect::Plane1Only) {
                return dawnInfo.getViewFormat() == wgpu::TextureFormat::RG8Unorm;
            }
            break; // else fall through to validate All aspect

        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm:
            if (dawnInfo.fAspect == wgpu::TextureAspect::Plane0Only) {
                return dawnInfo.getViewFormat() == wgpu::TextureFormat::R16Unorm;
            } else if (dawnInfo.fAspect == wgpu::TextureAspect::Plane1Only) {
                return dawnInfo.getViewFormat() == wgpu::TextureFormat::RG16Unorm;
            }
            break; // else fall through to validate All aspect

        // There are not yet triplanar variants for 10-bit YUV formats.
        case wgpu::TextureFormat::R8BG8A8Triplanar420Unorm:
            if (dawnInfo.fAspect == wgpu::TextureAspect::Plane0Only ||
                dawnInfo.fAspect == wgpu::TextureAspect::Plane2Only) {
                return dawnInfo.getViewFormat() == wgpu::TextureFormat::R8Unorm;
            } else if (dawnInfo.fAspect == wgpu::TextureAspect::Plane1Only) {
                return dawnInfo.getViewFormat() == wgpu::TextureFormat::RG8Unorm;
            }
            break; // else fall through to validate All aspect

        default:
            // Not a multiplanar format, fall through
            break;
    }
#endif

    // No multiplanar formats in pure WebGPU, so require that aspect == All and view format and
    // base format are the same. We allow the format to be Undefined if the view format is set,
    // which can arise with promise images.
    return dawnInfo.fAspect == wgpu::TextureAspect::All &&
           dawnInfo.getViewFormat() != wgpu::TextureFormat::Undefined &&
           (dawnInfo.fFormat == dawnInfo.getViewFormat() ||
                    dawnInfo.fFormat == wgpu::TextureFormat::Undefined);
}

}  // anonymous namespace

DawnCaps::DawnCaps(const DawnBackendContext& backendContext, const ContextOptions& options)
        : Caps() {
    this->initCaps(backendContext, options);
    this->initShaderCaps(backendContext.fDevice);
    this->initFormatTable(backendContext.fDevice);
    this->finishInitialization(options);
}

DawnCaps::~DawnCaps() = default;

void DawnCaps::initFormatTable(const wgpu::Device& device) {
    for (int i = 0; i < kTextureFormatCount; ++i) {
        TextureFormat tf = static_cast<TextureFormat>(i);
        wgpu::TextureFormat format = TextureFormatToDawnFormat(tf);
        SkEnumBitMask<DawnFormatFlag> formatCaps = DawnTextureFormatSupport(device, format);

        // The Dawn backend currently only supports optimal tiling
        auto& [supportedUsage, supportedSampleCounts] = fFormatSupport[(int) Tiling::kOptimal][i];
        if (formatCaps == DawnFormatFlag::None) {
            SkASSERT(!SkToBool(supportedUsage) && !SkToBool(supportedSampleCounts));
            continue;
        }

        // At this point, we can claim at least 1 sample is supported
        supportedSampleCounts = SampleCount::k1;
        if (formatCaps & DawnFormatFlag::Filter) {
            supportedUsage |= TextureUsage::kSample;
        }

        if (formatCaps & DawnFormatFlag::Render) {
            supportedUsage |= TextureUsage::kRender;

            SkEnumBitMask<DawnFormatFlag> msaaFlag = DawnFormatFlag::MSAA;
            if (!TextureFormatIsDepthOrStencil(tf)) {
                // We never resolve depth/stencil, but Graphite assumes we resolve to color formats
                msaaFlag |= DawnFormatFlag::Resolve;
            }
            if ((formatCaps & msaaFlag) == msaaFlag) {
                // WebGPU only supports 1x and 4x MSAA
                supportedSampleCounts |= SampleCount::k4;
                if (this->msaaRenderToSingleSampledSupport() &&
                    !TextureFormatIsDepthOrStencil(tf)) {
                    // If WebGPU exposes the MSRTSS extension, assume that all color formats that
                    // support MSAA can support MSRTSS.
                    supportedUsage |= TextureUsage::kMSRTSS;
                }
            }
        }

        // For now, support kStorage usage if the format has both read-only and write-only
        // capabilities, but we don't require that it supports simultaneous read/write in a binding.
        if ((formatCaps & DawnFormatFlag::ReadOnly) && (formatCaps & DawnFormatFlag::WriteOnly)) {
            supportedUsage |= TextureUsage::kStorage;
        }

        if (TextureFormatCompressionType(tf) != SkTextureCompressionType::kNone) {
            // Compressed textures can be copied into, but disallow copying out
            supportedUsage |= TextureUsage::kCopyDst;
        } else if (tf != TextureFormat::kExternal) {
            // Plain textures can be copied into and out of
            supportedUsage |= TextureUsage::kCopySrc | TextureUsage::kCopyDst;
        } // else no copying for external texture formats
    }
}

std::pair<SkEnumBitMask<TextureUsage>, Tiling> DawnCaps::getTextureUsage(
        const TextureInfo& info) const {
    const auto& dawnInfo = TextureInfoPriv::Get<DawnTextureInfo>(info);

    SkEnumBitMask<TextureUsage> usage;
    if (is_valid_view(dawnInfo)) {
        if (dawnInfo.fUsage & wgpu::TextureUsage::RenderAttachment) {
            usage |= TextureUsage::kRender;
            // All color renderable formats can be used with MSRTSS when the extension is available
            if (this->msaaRenderToSingleSampledSupport() &&
                !TextureFormatIsDepthOrStencil(TextureInfoPriv::ViewFormat(info))) {
                usage |= TextureUsage::kMSRTSS;
            }
        }
        if (dawnInfo.fUsage & wgpu::TextureUsage::TextureBinding) {
            usage |= TextureUsage::kSample;
        }
        if (dawnInfo.fUsage & wgpu::TextureUsage::CopySrc) {
            usage |= TextureUsage::kCopySrc;
        }
        if (dawnInfo.fUsage & wgpu::TextureUsage::CopyDst) {
            usage |= TextureUsage::kCopyDst;
        }
        if (dawnInfo.fUsage & wgpu::TextureUsage::StorageBinding) {
            usage |= TextureUsage::kStorage;
        }
        // NOTE: No support for TextureUsage::kHostCopy yet
    }

    return {usage, Tiling::kOptimal};
}

TextureInfo DawnCaps::onGetDefaultTextureInfo(SkEnumBitMask<TextureUsage> usage,
                                              TextureFormat format,
                                              SampleCount sampleCount,
                                              Mipmapped mipmapped,
                                              Protected,
                                              Discardable discardable) const {
    wgpu::TextureFormat dawnFormat = TextureFormatToDawnFormat(format);
    SkASSERT(dawnFormat != wgpu::TextureFormat::Undefined); // should have been caught by Caps first

    wgpu::TextureUsage dawnUsage = wgpu::TextureUsage::None;

    if (usage & TextureUsage::kSample) {
        dawnUsage |= wgpu::TextureUsage::TextureBinding;
    }
    if (usage & TextureUsage::kStorage) {
        dawnUsage |= wgpu::TextureUsage::StorageBinding;
    }
    if (usage & TextureUsage::kCopySrc) {
        dawnUsage |= wgpu::TextureUsage::CopySrc;
    }
    if (usage & TextureUsage::kCopyDst) {
        dawnUsage |= wgpu::TextureUsage::CopyDst;
    }
    if (usage & TextureUsage::kRender) {
        dawnUsage |= wgpu::TextureUsage::RenderAttachment;
        // Use transient attachments if possible for discardable textures
        if (fSupportedTransientAttachmentUsage != wgpu::TextureUsage::None &&
            discardable == Discardable::kYes) {
            dawnUsage |= fSupportedTransientAttachmentUsage;
        }
        if (fEmulateLoadStoreResolve && !TextureFormatIsDepthOrStencil(format)) {
            // When emulating the store, the color attachment is sampled into the resolve so add
            // this usage even if higher-up Graphite logic wasn't expecting to sample it.
            dawnUsage |= wgpu::TextureUsage::TextureBinding;
        }
    }
    // NOTE: kMSRTSS is ignored since it's implicitly available on any wgpu::Texture if the
    // extension is available. kHostCopy should not be requested from Caps since it's unsupported.
    SkASSERT(!SkToBool(usage & TextureUsage::kHostCopy));

    DawnTextureInfo info;
    info.fSampleCount = sampleCount;
    info.fMipmapped = mipmapped;
    info.fFormat = dawnFormat;
    info.fUsage = dawnUsage;

    return TextureInfos::MakeDawn(info);
}

SkISize DawnCaps::getDepthAttachmentDimensions(const TextureInfo& textureInfo,
                                               const SkISize colorAttachmentDimensions) const {
#if !defined(__EMSCRIPTEN__)
    // For multiplanar textures, texture->textureInfo() uses the format of planes instead of
    // textures (R8, R8G8, vs R8BG8Biplanar420Unorm), so we have to query texture format from
    // wgpu::Texture object, and then use it reconstruct the full dimensions.
    const auto& dawnInfo = TextureInfoPriv::Get<DawnTextureInfo>(textureInfo);
    switch (dawnInfo.fFormat) {
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
        case wgpu::TextureFormat::R8BG8A8Triplanar420Unorm:
            if (dawnInfo.fAspect == wgpu::TextureAspect::Plane1Only) {
                // Dawn requires depth attachment to match the size of Y plane (texture size).
                return SkISize::Make(colorAttachmentDimensions.width() * 2,
                                     colorAttachmentDimensions.height() * 2);
            }

            // Otherwise this is the Y or A plane, so no adjustment needed
            [[fallthrough]];
        default:
            // Not multiplanar, so no adjustment needed
            break;
    }
#endif

    return colorAttachmentDimensions;
}

void DawnCaps::initCaps(const DawnBackendContext& backendContext, const ContextOptions& options) {
    // GetAdapter() is not available in WASM and there's no way to get AdapterInfo off of
    // the WGPUDevice directly.
#if !defined(__EMSCRIPTEN__)
    wgpu::AdapterInfo info;
    backendContext.fDevice.GetAdapter().GetInfo(&info);

#if defined(GPU_TEST_UTILS)
    this->setDeviceName(std::string(info.device));
#endif
#endif // defined(__EMSCRIPTEN__)

#if defined(__EMSCRIPTEN__)
    wgpu::SupportedLimits supportedLimits;
    // TODO(crbug.com/42241199): Update to use wgpu::Status when webgpu.h in Emscripten is updated.
    [[maybe_unused]] bool limitsSucceeded = backendContext.fDevice.GetLimits(&supportedLimits);
    SkASSERT(limitsSucceeded);
    wgpu::Limits& limits = supportedLimits.limits;
#else
    wgpu::CompatibilityModeLimits compatLimits;
    wgpu::Limits limits{.nextInChain = &compatLimits};
    wgpu::DawnTexelCopyBufferRowAlignmentLimits alignmentLimits{};
    if (backendContext.fDevice.HasFeature(wgpu::FeatureName::DawnTexelCopyBufferRowAlignment)) {
        compatLimits.nextInChain = &alignmentLimits;
    }
    [[maybe_unused]] wgpu::Status status = backendContext.fDevice.GetLimits(&limits);
    SkASSERT(status == wgpu::Status::Success);
#endif  // defined(__EMSCRIPTEN__)

    fMaxTextureSize = limits.maxTextureDimension2D;

    fRequiredTransferBufferAlignment = 4;
    fRequiredUniformBufferAlignment = limits.minUniformBufferOffsetAlignment;
    fRequiredStorageBufferAlignment = limits.minStorageBufferOffsetAlignment;

    fMaxVaryings = limits.maxInterStageShaderVariables;

    // Dawn requires 256 bytes per row alignment for buffer texture copies.
    fTextureDataRowBytesAlignment = 256;
#if !defined(__EMSCRIPTEN__)
    // If the device supports the DawnTexelCopyBufferRowAlignment feature, the alignment can be
    // queried from its limits.
    if (backendContext.fDevice.HasFeature(wgpu::FeatureName::DawnTexelCopyBufferRowAlignment)) {
        fTextureDataRowBytesAlignment = alignmentLimits.minTexelCopyBufferRowAlignment;
    }
#endif

    fSupportsHalfPrecision = backendContext.fDevice.HasFeature(wgpu::FeatureName::ShaderF16);
    fResourceBindingReqs.fBackendApi = BackendApi::kDawn;
    // The WGSL generator assumes tightly packed std430 layout for SSBOs which is also the default
    // for all types outside the uniform address space in WGSL (which is emulated to 140 by SkSL's
    // WGSL generation). If ShaderF16 is supported, we switch the layout to upload half data.
    fResourceBindingReqs.fUniformBufferLayout = fSupportsHalfPrecision ? Layout::kStd140_F16
                                                                       : Layout::kStd140;
    fResourceBindingReqs.fStorageBufferLayout = fSupportsHalfPrecision ? Layout::kStd430_F16
                                                                       : Layout::kStd430;
    fResourceBindingReqs.fSeparateTextureAndSamplerBinding = true;

#if !defined(__EMSCRIPTEN__)
    // We need 32 bytes push constant for 2 vectors worth of intrinsic data.
    fResourceBindingReqs.fUsePushConstantsForIntrinsicConstants =
            limits.maxImmediateSize >= DawnGraphicsPipeline::kIntrinsicUniformSize;
#endif

    fResourceBindingReqs.fUniformsSetIdx = DawnGraphicsPipeline::kUniformBufferBindGroupIndex;
    fResourceBindingReqs.fTextureSamplerSetIdx = DawnGraphicsPipeline::kTextureBindGroupIndex;

    fResourceBindingReqs.fIntrinsicBufferBinding =
            DawnGraphicsPipeline::kIntrinsicUniformBufferIndex;
    fResourceBindingReqs.fCombinedUniformBufferBinding =
            DawnGraphicsPipeline::kCombinedUniformIndex;
    fResourceBindingReqs.fGradientBufferBinding = DawnGraphicsPipeline::kGradientBufferIndex;

#if !defined(__EMSCRIPTEN__)
    // We need at least 4 SSBOs for intrinsic, render step, paint & gradient buffers.
    // TODO(b/418235681): Enable SSBOs after fixing performance regressions for Dawn/Vulkan.
    fStorageBufferSupport = info.backendType != wgpu::BackendType::OpenGL &&
                            info.backendType != wgpu::BackendType::OpenGLES &&
                            info.backendType != wgpu::BackendType::Vulkan &&
                            compatLimits.maxStorageBuffersInVertexStage >= 4 &&
                            compatLimits.maxStorageBuffersInFragmentStage >= 4;
#else
    // WASM doesn't provide a way to query the backend, so can't tell if we are on a backend that
    // needs to have SSBOs disabled. Pessimistically assume we could be. Once the above conditions
    // go away in Dawn-native, then we can assume SSBOs are always supported in pure WebGPU too.
    fStorageBufferSupport = false;
#endif

    fDrawBufferCanBeMapped = false;

    fComputeSupport = true;

    // TODO: support clamp to border.
    fClampToBorderSupport = false;

    // We use async map.
    fBufferMapsAreAsync = true;

#if defined(GPU_TEST_UTILS)
    fDrawBufferCanBeMappedForReadback = false;
#endif

#if !defined(__EMSCRIPTEN__)
    fDrawBufferCanBeMapped =
            backendContext.fDevice.HasFeature(wgpu::FeatureName::BufferMapExtendedUsages);

    fMSAARenderToSingleSampledSupport =
            backendContext.fDevice.HasFeature(wgpu::FeatureName::MSAARenderToSingleSampled);

    if (backendContext.fDevice.HasFeature(wgpu::FeatureName::TransientAttachments)) {
        fSupportedTransientAttachmentUsage = wgpu::TextureUsage::TransientAttachment;
    }
#endif

#if !defined(__EMSCRIPTEN__)
    if (backendContext.fDevice.HasFeature(wgpu::FeatureName::DawnLoadResolveTexture)) {
        fSupportedResolveTextureLoadOp = wgpu::LoadOp::ExpandResolveTexture;
        fSupportsPartialLoadResolve =
                backendContext.fDevice.HasFeature(wgpu::FeatureName::DawnPartialLoadResolveTexture);
        fDifferentResolveAttachmentSizeSupport = fSupportsPartialLoadResolve;
    }

    fSupportsRenderPassRenderArea =
            backendContext.fDevice.HasFeature(wgpu::FeatureName::RenderPassRenderArea);
#endif

    if (!fSupportsPartialLoadResolve &&
        fSupportedTransientAttachmentUsage == wgpu::TextureUsage::None) {
        // If the device doesn't support partial resolve nor transient attachments, we will emulate
        // load/resolve using separate render passes. This helps reuse MSAA textures better to
        // reduce memory usage.
        fEmulateLoadStoreResolve = true;
        fDifferentResolveAttachmentSizeSupport = true;

        // On hardware that doesn't support transient attachments or partial resolve, we
        // force-disable the ExpandResolveTexture loadOp. This is done because, under emulation,
        // ExpandResolveTexture isn't used, and fully disabling it prevents the precompilation API
        // from generating duplicate pipeline permutations (one for load resolve texture, one for
        // others).
        fSupportedResolveTextureLoadOp = std::nullopt;
    }

    if (backendContext.fDevice.HasFeature(wgpu::FeatureName::TimestampQuery)) {
        // Native Dawn has an API for writing timestamps on command buffers. WebGPU only supports
        // begin and end timestamps on render and compute passes.
#if !defined(__EMSCRIPTEN__)
        // TODO(b/42240559): On Apple silicon, the timer queries don't have the correct dependencies
        // to measure all the encoders that the start/end commands encapsulate in the commandbuffer.
        // We would prefer to keep this API as it lets us measure our texture uploads. If either
        // this is fixed in Dawn, we can unconditionally take this approach for dawn-native; or
        // the WebGPU API can hopefully be extended to capture blit passes.
        fSupportsCommandBufferTimestamps = info.backendType != wgpu::BackendType::Metal;
#endif

        // The emscripten C/C++ interface before 3.1.48 for timestamp query writes on render and
        // compute passes is different than on current emsdk. The older API isn't correctly
        // translated to the current JS WebGPU API in emsdk. So we require 3.1.48+.
#if !defined(__EMSCRIPTEN__)                                                                   \
        || (__EMSCRIPTEN_major__ > 3)                                                          \
        || (__EMSCRIPTEN_major__ == 3 && __EMSCRIPTEN_minor__ > 1)                             \
        || (__EMSCRIPTEN_major__ == 3 && __EMSCRIPTEN_minor__ == 1 && __EMSCRIPTEN_tiny__ >= 48)
        fSupportedGpuStats |= GpuStatsFlags::kElapsedTime;
#endif
    }

    if (!backendContext.fTick) {
        fAllowCpuSync = false;
        // This seems paradoxical. However, if we use the async pipeline creation methods (e.g
        // Device::CreateRenderPipelineAsync) then we may have to synchronize before a submit that
        // uses the pipeline. If we use the methods that look synchronous (e.g.
        // Device::CreateRenderPipeline) they actually operate asynchronously on WebGPU but the
        // browser becomes responsible for synchronizing when we call submit.
        fUseAsyncPipelineCreation = false;

        // The implementation busy waits after popping.
        fAllowScopedErrorChecks = false;
    }

    fFullCompressedUploadSizeMustAlignToBlockDims = true;
}

void DawnCaps::initShaderCaps(const wgpu::Device& device) {
    SkSL::ShaderCaps* shaderCaps = fShaderCaps.get();

    // WGSL does not actually support infinities regardless of hardware support. There are
    // discussions around enabling it using an extension in the future.
    shaderCaps->fInfinitySupport = false;

    if (device.HasFeature(wgpu::FeatureName::DualSourceBlending)) {
        shaderCaps->fDualSourceBlendingSupport = true;
    }
#if !defined(__EMSCRIPTEN__)
    if (device.HasFeature(wgpu::FeatureName::FramebufferFetch)) {
        shaderCaps->fFBFetchSupport = true;
    }
#endif
}

// TextureFormat is backed by a uint8_t, so 8 bits are always sufficient (including using
// kUnsupported) to represent an unused attachment. To make room for the load-from-resolve bit, we
// reduce the uint8_t of fSampleCount to 3 bits with the SampleToKey function (x2 attachments)
static constexpr int kFormatBits = 8; // x2 attachments (color & depthStencil formats)
static constexpr int kResolveBits = 1;

static_assert(2*(kFormatBits + kNumSampleKeyBits) + kResolveBits <= 32);
static_assert(kTextureFormatCount < 1 << kFormatBits);

static constexpr int kDepthStencilNumSamplesOffset = /*loadResolveOffset=0 + */   kResolveBits;
static constexpr int kDepthStencilFormatOffset  = kDepthStencilNumSamplesOffset + kNumSampleKeyBits;
static constexpr int kColorNumSamplesOffset     = kDepthStencilFormatOffset     + kFormatBits;
static constexpr int kColorFormatOffset         = kColorNumSamplesOffset        + kNumSampleKeyBits;
static constexpr int kAdditionalFlagOffset      = kColorFormatOffset            + kFormatBits;
static_assert(kAdditionalFlagOffset <= 31);

static constexpr uint32_t kFormatMask     = (1 << kFormatBits) - 1;
static constexpr uint32_t kNumSamplesMask = (1 << kNumSampleKeyBits) - 1;
static constexpr uint32_t kResolveMask    = (1 << kResolveBits) - 1;

uint32_t DawnCaps::getRenderPassDescKeyForPipeline(const RenderPassDesc& renderPassDesc,
                                                   bool additionalFlag) const {
    // The color attachment should be valid; the depth-stencil attachment may not be if it's not
    // being used. The full resolve attachment (if present) does not need to be included.
    const auto& color = renderPassDesc.fColorAttachment;
    const auto& depthStencil = renderPassDesc.fDepthStencilAttachment;
    SkASSERT(color.fFormat != TextureFormat::kUnsupported);

    // Note: if Dawn supports ExpandResolveTexture load op and the render pass uses it to load the
    // resolve texture, a render pipeline will need to be created with
    // wgpu::ColorTargetStateExpandResolveTextureDawn chained struct in order to be compatible.
    // Hence a render pipeline created for a render pass using ExpandResolveTexture load op will be
    // different from the one created for a render pass not using that load op. So we need to
    // include a bit flag to differentiate the two kinds of pipelines.
    const bool shouldIncludeLoadResolveAttachmentBit = this->loadOpAffectsMSAAPipelines();
    uint32_t loadResolveAttachmentKey = 0;
    if (shouldIncludeLoadResolveAttachmentBit &&
        renderPassDesc.fColorResolveAttachment.fFormat != TextureFormat::kUnsupported &&
        renderPassDesc.fColorResolveAttachment.fLoadOp == LoadOp::kLoad) {
        loadResolveAttachmentKey = 1;
    }

    SkASSERT(SamplesToKey(color.fSampleCount) < (1 << kNumSampleKeyBits));
    SkASSERT(SamplesToKey(depthStencil.fSampleCount) < (1 << kNumSampleKeyBits));
    SkASSERT(loadResolveAttachmentKey < (1 << kResolveBits));
    uint32_t additionalFlagKey = additionalFlag ? 1 : 0;

    return (additionalFlagKey                           << kAdditionalFlagOffset)         |
           (static_cast<uint32_t>(color.fFormat)        << kColorFormatOffset)            |
           (SamplesToKey(color.fSampleCount)            << kColorNumSamplesOffset)        |
           (static_cast<uint32_t>(depthStencil.fFormat) << kDepthStencilFormatOffset)     |
           (SamplesToKey(depthStencil.fSampleCount)     << kDepthStencilNumSamplesOffset) |
           loadResolveAttachmentKey;
}

static constexpr uint16_t kDawnGraphicsPipelineKeyData32Count = 4;

UniqueKey DawnCaps::makeGraphicsPipelineKey(const GraphicsPipelineDesc& pipelineDesc,
                                            const RenderPassDesc& renderPassDesc) const {
    UniqueKey pipelineKey;
    {
        // 4 uint32_t's (render step id, paint id, uint32 RenderPassDesc, uint16 write swizzle key)
        UniqueKey::Builder builder(&pipelineKey, get_pipeline_domain(),
                                   kDawnGraphicsPipelineKeyData32Count, "DawnGraphicsPipeline");
        // Add GraphicsPipelineDesc key.
        builder[0] = static_cast<uint32_t>(pipelineDesc.renderStepID());
        builder[1] = pipelineDesc.paintParamsID().asUInt();

        // Add RenderPassDesc key and write swizzle (which is separate from the RenderPassDescKey
        // because it is applied in the program writing to the target, and is not actually part of
        // the underlying GPU render pass config).
        builder[2] = this->getRenderPassDescKeyForPipeline(renderPassDesc);
        builder[3] = renderPassDesc.fWriteSwizzle.asKey();
        builder.finish();
    }

    return pipelineKey;
}

bool DawnCaps::extractGraphicsDescs(const UniqueKey& key,
                                    GraphicsPipelineDesc* pipelineDesc,
                                    RenderPassDesc* renderPassDesc,
                                    const RendererProvider* rendererProvider) const {
    SkASSERT(key.domain() == get_pipeline_domain());
    SkASSERT(key.dataSize() == 4 * kDawnGraphicsPipelineKeyData32Count);

    const uint32_t* rawKeyData = key.data();

    SkASSERT(RenderStep::IsValidRenderStepID(rawKeyData[0]));
    RenderStep::RenderStepID renderStepID = static_cast<RenderStep::RenderStepID>(rawKeyData[0]);

    *pipelineDesc = GraphicsPipelineDesc(renderStepID, UniquePaintParamsID(rawKeyData[1]));

    const uint32_t rpDescBits = rawKeyData[2];
    TextureFormat colorFormat =
            static_cast<TextureFormat>((rpDescBits >> kColorFormatOffset) & kFormatMask);
    SampleCount colorSamples =
            KeyToSamples((rpDescBits >> kColorNumSamplesOffset) & kNumSamplesMask);

    TextureFormat depthStencilFormat =
            static_cast<TextureFormat>((rpDescBits >> kDepthStencilFormatOffset) & kFormatMask);
    SampleCount depthStencilSamples =
            KeyToSamples((rpDescBits >> kDepthStencilNumSamplesOffset) & kNumSamplesMask);

    if ((colorSamples > SampleCount::k1 && this->avoidMSAA()) ||
        (depthStencilFormat != TextureFormat::kUnsupported && this->avoidDepthMode())) {
        return false;
    }

    const bool loadFromResolve = (rpDescBits & kResolveMask) != 0;
    // This bit should only be set if Dawn supports ExpandResolveTexture load op
    SkASSERT(!loadFromResolve || this->loadOpAffectsMSAAPipelines());

    // Recreate the RenderPassDesc, assuming that if there is MSAA on the color attachment that
    // a resolve attachment is also required. The resolve attachment's load op will match the key.
    SkASSERT(colorSamples == depthStencilSamples ||
             depthStencilFormat == TextureFormat::kUnsupported);
    *renderPassDesc = {};
    renderPassDesc->fColorAttachment = {colorFormat,
                                        LoadOp::kClear,
                                        StoreOp::kStore,
                                        colorSamples};
    if (!this->avoidDepthMode()) {
        renderPassDesc->fDepthStencilAttachment = {depthStencilFormat,
                                                   LoadOp::kClear,
                                                   StoreOp::kDiscard,
                                                   depthStencilSamples};
    }
    if (colorSamples > SampleCount::k1) {
        renderPassDesc->fColorResolveAttachment = {colorFormat,
                                                   loadFromResolve ? LoadOp::kLoad : LoadOp::kClear,
                                                   StoreOp::kStore,
                                                   SampleCount::k1};
        renderPassDesc->fColorAttachment.fStoreOp = StoreOp::kDiscard;
    }

    renderPassDesc->fSampleCount = colorSamples;
    renderPassDesc->fWriteSwizzle = SwizzleCtorAccessor::Make(rawKeyData[3]);
    renderPassDesc->fDstReadStrategy = this->getDstReadStrategy();

    return true;
}

UniqueKey DawnCaps::makeComputePipelineKey(const ComputePipelineDesc& pipelineDesc) const {
    UniqueKey pipelineKey;
    {
        static const skgpu::UniqueKey::Domain kComputePipelineDomain = UniqueKey::GenerateDomain();
        // The key is made up of a single uint32_t corresponding to the compute step ID.
        UniqueKey::Builder builder(&pipelineKey, kComputePipelineDomain, 1, "ComputePipeline");
        builder[0] = pipelineDesc.computeStep()->uniqueID();

        // TODO(b/240615224): The local work group size should factor into the key here since it is
        // specified in the shader text on Dawn/SPIR-V. This is not a problem right now since
        // ComputeSteps don't vary their workgroup size dynamically.

        builder.finish();
    }
    return pipelineKey;
}

ImmutableSamplerInfo DawnCaps::getImmutableSamplerInfo(const TextureInfo& textureInfo) const {
#if !defined(__EMSCRIPTEN__)
    const wgpu::YCbCrVkDescriptor& ycbcrConversionInfo =
            TextureInfoPriv::Get<DawnTextureInfo>(textureInfo).fYcbcrVkDescriptor;

    if (DawnDescriptorIsValid(ycbcrConversionInfo)) {
        return DawnDescriptorToImmutableSamplerInfo(ycbcrConversionInfo);
    }
#endif

    // If the YCbCr conversion for is invalid, then return a default ImmutableSamplerInfo struct.
    return {};
}

#if !defined(__EMSCRIPTEN__)
static constexpr const char* filter_mode_to_str(wgpu::FilterMode mode) {
    switch (mode) {
        case wgpu::FilterMode::Undefined: return "undefined";
        case wgpu::FilterMode::Nearest:   return "nearest";
        case wgpu::FilterMode::Linear:    return "linear";
    }
    SkUNREACHABLE;
}

static constexpr const char* model_to_str(uint32_t c) {
    switch (c) {
        case 0 /* VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY */:   return "RGB-I";
        case 1 /* VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_IDENTITY */: return "YCbCr-I";
        case 2 /* VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709 */:      return "709";
        case 3 /* VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601 */:      return "601";
        case 4 /* VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020 */:     return "2020";
        default:                                                       return "unknown";
    }
    SkUNREACHABLE;
}

static constexpr const char* range_to_str(uint32_t r) {
    switch (r) {
        case 0 /* VK_SAMPLER_YCBCR_RANGE_ITU_FULL */:   return "full";
        case 1 /* VK_SAMPLER_YCBCR_RANGE_ITU_NARROW */: return "narrow";
        default:                                        return "unknown";
    }
    SkUNREACHABLE;
}

static constexpr char swizzle_to_str(uint32_t c, char identityAnswer) {
    switch (c) {
        case 0 /* VK_COMPONENT_SWIZZLE_IDENTITY */: return identityAnswer;
        case 1 /* VK_COMPONENT_SWIZZLE_ZERO */:     return '0';
        case 2 /* VK_COMPONENT_SWIZZLE_ONE */:      return '1';
        case 3 /* VK_COMPONENT_SWIZZLE_R */:        return 'r';
        case 4 /* VK_COMPONENT_SWIZZLE_G */:        return 'g';
        case 5 /* VK_COMPONENT_SWIZZLE_B */:        return 'b';
        case 6 /* VK_COMPONENT_SWIZZLE_A */:        return 'a';
        default:                                    return '?';
    }
    SkUNREACHABLE;
}
#endif

std::string DawnCaps::toString(const ImmutableSamplerInfo& immutableSamplerInfo) const {
#if defined(__EMSCRIPTEN__)
    return "";
#else
    const wgpu::YCbCrVkDescriptor info =
                DawnDescriptorFromImmutableSamplerInfo(immutableSamplerInfo);
    if (!DawnDescriptorIsValid(info)) {
        return "";
    }

    std::string result;

    if (info.vkFormat == 0) {
        result += 'x';
        result += std::to_string(info.externalFormat);
    } else {
        result += std::to_string(info.vkFormat);
    }

    result += " ";
    result += model_to_str(info.vkYCbCrModel);
    result += "+";
    result += range_to_str(info.vkYCbCrRange);
    result += info.vkXChromaOffset ? " mid"  : " cos";  // midpoint or cosited-even
    result += info.vkYChromaOffset ? " mid " : " cos "; // midpoint or cosited-even
    result += filter_mode_to_str(info.vkChromaFilter);
    result += info.forceExplicitReconstruction ? " T " : " F ";
    result += swizzle_to_str(info.vkComponentSwizzleRed,   'r');
    result += swizzle_to_str(info.vkComponentSwizzleGreen, 'g');
    result += swizzle_to_str(info.vkComponentSwizzleBlue,  'b');
    result += swizzle_to_str(info.vkComponentSwizzleAlpha, 'a');
    return result;
#endif
}

void DawnCaps::buildKeyForTexture(SkISize dimensions,
                                  const TextureInfo& info,
                                  ResourceType type,
                                  GraphiteResourceKey* key) const {
    const auto& dawnInfo = TextureInfoPriv::Get<DawnTextureInfo>(info);

    SkASSERT(!dimensions.isEmpty());

    SkASSERT(dawnInfo.getViewFormat() != wgpu::TextureFormat::Undefined);
    // FIXME we can reduce this by packing format into samplesKey and then we're back down to 5 ints
    // we could go further if we said textures were likely to be under 65kx65kf...
    uint32_t formatKey = static_cast<uint32_t>(dawnInfo.getViewFormat());

    uint32_t samplesKey = SamplesToKey(info.sampleCount());
    // We don't have to key the number of mip levels because it is inherit in the combination of
    // isMipped and dimensions.
    bool isMipped = info.mipmapped() == Mipmapped::kYes;

    // Confirm all the below parts of the key can fit in a single uint32_t. The sum of the shift
    // amounts in the asserts must be less than or equal to 32.
    SkASSERT(samplesKey                             < (1u << 3));  // sample key is first 3 bits
    SkASSERT(static_cast<uint32_t>(isMipped)        < (1u << 1));  // isMapped is 4th bit
    SkASSERT(static_cast<uint32_t>(dawnInfo.fUsage) < (1u << 28)); // usage is remaining 28 bits

    // We need two uint32_ts for dimensions, 1 for format, and 1 for the rest of the key;
    uint16_t num32DataCnt = 2 + 1 + 1;
    bool hasYcbcrInfo = false;
#if !defined(__EMSCRIPTEN__)
    // If we are using ycbcr texture/sampling, more key information is needed.
    if ((hasYcbcrInfo = DawnDescriptorIsValid(dawnInfo.fYcbcrVkDescriptor))) {
        num32DataCnt += 3; // non-format flags and 64-bit format
    }
#endif
    GraphiteResourceKey::Builder builder(key, type, num32DataCnt);

    builder[0] = dimensions.width();
    builder[1] = dimensions.height();
    builder[2] = formatKey;
    builder[3] = (samplesKey                                   << 0) |
                 (static_cast<uint32_t>(isMipped)              << 3) |
                 (static_cast<uint32_t>(dawnInfo.fUsage)       << 4);

#if !defined(__EMSCRIPTEN__)
    if (hasYcbcrInfo) {
        ImmutableSamplerInfo packedInfo =
                DawnDescriptorToImmutableSamplerInfo(dawnInfo.fYcbcrVkDescriptor);
        builder[4] = packedInfo.fNonFormatYcbcrConversionInfo;
        // Even though we already have formatKey appended to the texture key, we still need to add
        // fYcbcrVkDescriptor's vkFormat or externalFormat. The latter two are distinct from
        // dawnSpec's wgpu::TextureFormat.
        builder[5] = (uint32_t) packedInfo.fFormat;
        builder[6] = (uint32_t) (packedInfo.fFormat >> 32);
    }
#endif
}

} // namespace skgpu::graphite
