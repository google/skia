/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnGraphiteUtils.h"

#include "include/gpu/ShaderErrorHandler.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/dawn/DawnBackendContext.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/graphite/dawn/DawnQueueManager.h"
#include "src/gpu/graphite/dawn/DawnSharedContext.h"

namespace skgpu::graphite {

namespace ContextFactory {
std::unique_ptr<Context> MakeDawn(const DawnBackendContext& backendContext,
                                  const ContextOptions& options) {
    sk_sp<SharedContext> sharedContext = DawnSharedContext::Make(backendContext, options);
    if (!sharedContext) {
        return nullptr;
    }

    auto queueManager =
            std::make_unique<DawnQueueManager>(backendContext.fQueue, sharedContext.get());
    if (!queueManager) {
        return nullptr;
    }

    auto context = ContextCtorAccessor::MakeContext(std::move(sharedContext),
                                                    std::move(queueManager),
                                                    options);
    SkASSERT(context);
    return context;
}
} // namespace ContextFactory

SkTextureCompressionType DawnFormatToCompressionType(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::ETC2RGB8Unorm: return SkTextureCompressionType::kETC2_RGB8_UNORM;
        case wgpu::TextureFormat::BC1RGBAUnorm:  return SkTextureCompressionType::kBC1_RGBA8_UNORM;
        default:                                 return SkTextureCompressionType::kNone;
    }
}

bool DawnFormatIsDepthOrStencil(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::Stencil8:             [[fallthrough]];
        case wgpu::TextureFormat::Depth16Unorm:
        case wgpu::TextureFormat::Depth32Float:
        case wgpu::TextureFormat::Depth24PlusStencil8:
        case wgpu::TextureFormat::Depth32FloatStencil8:
            return true;
        default:
            return false;
    }

    SkUNREACHABLE;
}

bool DawnFormatIsDepth(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::Depth16Unorm:         [[fallthrough]];
        case wgpu::TextureFormat::Depth32Float:
        case wgpu::TextureFormat::Depth24PlusStencil8:
        case wgpu::TextureFormat::Depth32FloatStencil8:
            return true;
        default:
            return false;
    }

    SkUNREACHABLE;
}

bool DawnFormatIsStencil(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::Stencil8:             [[fallthrough]];
        case wgpu::TextureFormat::Depth24PlusStencil8:
        case wgpu::TextureFormat::Depth32FloatStencil8:
            return true;
        default:
            return false;
    }

    SkUNREACHABLE;
}

wgpu::TextureFormat DawnDepthStencilFlagsToFormat(SkEnumBitMask<DepthStencilFlags> mask) {
    // TODO: Decide if we want to change this to always return a combined depth and stencil format
    // to allow more sharing of depth stencil allocations.
    if (mask == DepthStencilFlags::kDepth) {
        // If needed for workarounds or performance, Depth32Float is also available but requires 2x
        // the amount of memory.
        return wgpu::TextureFormat::Depth16Unorm;
    } else if (mask == DepthStencilFlags::kStencil) {
        return wgpu::TextureFormat::Stencil8;
    } else if (mask == DepthStencilFlags::kDepthStencil) {
        return wgpu::TextureFormat::Depth24PlusStencil8;
    }
    SkASSERT(false);
    return wgpu::TextureFormat::Undefined;
}

TextureFormat DawnFormatToTextureFormat(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::R8Unorm:                    return TextureFormat::kR8;
        case wgpu::TextureFormat::R16Float:                   return TextureFormat::kR16F;
        case wgpu::TextureFormat::R32Float:                   return TextureFormat::kR32F;
        case wgpu::TextureFormat::RG8Unorm:                   return TextureFormat::kRG8;
        case wgpu::TextureFormat::RG16Float:                  return TextureFormat::kRG16F;
        case wgpu::TextureFormat::RG32Float:                  return TextureFormat::kRG32F;
        case wgpu::TextureFormat::RGBA8Unorm:                 return TextureFormat::kRGBA8;
        case wgpu::TextureFormat::RGBA16Float:                return TextureFormat::kRGBA16F;
        case wgpu::TextureFormat::RGBA32Float:                return TextureFormat::kRGBA32F;
        case wgpu::TextureFormat::RGB10A2Unorm:               return TextureFormat::kRGB10_A2;
        case wgpu::TextureFormat::RGBA8UnormSrgb:             return TextureFormat::kRGBA8_sRGB;
        case wgpu::TextureFormat::BGRA8Unorm:                 return TextureFormat::kBGRA8;
        case wgpu::TextureFormat::BGRA8UnormSrgb:             return TextureFormat::kBGRA8_sRGB;
        case wgpu::TextureFormat::ETC2RGB8Unorm:              return TextureFormat::kRGB8_ETC2;
        case wgpu::TextureFormat::ETC2RGB8UnormSrgb:          return TextureFormat::kRGB8_ETC2_sRGB;
        case wgpu::TextureFormat::BC1RGBAUnorm:               return TextureFormat::kRGBA8_BC1;
        case wgpu::TextureFormat::BC1RGBAUnormSrgb:           return TextureFormat::kRGBA8_BC1_sRGB;
        case wgpu::TextureFormat::Stencil8:                   return TextureFormat::kS8;
        case wgpu::TextureFormat::Depth16Unorm:               return TextureFormat::kD16;
        case wgpu::TextureFormat::Depth32Float:               return TextureFormat::kD32F;
        case wgpu::TextureFormat::Depth24PlusStencil8:        return TextureFormat::kD24_S8;
        case wgpu::TextureFormat::Depth32FloatStencil8:       return TextureFormat::kD32F_S8;
#if !defined(__EMSCRIPTEN__)
        case wgpu::TextureFormat::R16Unorm:                   return TextureFormat::kR16;
        case wgpu::TextureFormat::RG16Unorm:                  return TextureFormat::kRG16;
        case wgpu::TextureFormat::RGBA16Unorm:                return TextureFormat::kRGBA16;
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:      return TextureFormat::kYUV8_P2_420;
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
            return TextureFormat::kYUV10x6_P2_420;
        case wgpu::TextureFormat::External:                   return TextureFormat::kExternal;
#endif
        default: return TextureFormat::kUnsupported;
    }
}

static bool check_shader_module([[maybe_unused]] const DawnSharedContext* sharedContext,
                                wgpu::ShaderModule* module,
                                const char* shaderText,
                                ShaderErrorHandler* errorHandler) {
    // Prior to emsdk 3.1.51 wgpu::ShaderModule::GetCompilationInfo is unimplemented.
#if defined(__EMSCRIPTEN__)                                      &&  \
        ((__EMSCRIPTEN_major__ <  3                               || \
         (__EMSCRIPTEN_major__ == 3 && __EMSCRIPTEN_minor__ <  1) || \
         (__EMSCRIPTEN_major__ == 3 && __EMSCRIPTEN_minor__ == 1 && __EMSCRIPTEN_tiny__ < 51)))
    return true;
#else
    struct Handler {
        static void Fn(WGPUCompilationInfoRequestStatus status,
                       const WGPUCompilationInfo* info,
                       void* userdata) {
            Handler* self = reinterpret_cast<Handler*>(userdata);
            SkASSERT(status == WGPUCompilationInfoRequestStatus_Success);

            // Walk the message list and check for hard errors.
            self->fSuccess = true;
            for (size_t index = 0; index < info->messageCount; ++index) {
                const WGPUCompilationMessage& entry = info->messages[index];
                if (entry.type == WGPUCompilationMessageType_Error) {
                    self->fSuccess = false;
                    break;
                }
            }

            // If we found a hard error, report the compilation messages to the error handler.
            if (!self->fSuccess) {
                std::string errors;
                for (size_t index = 0; index < info->messageCount; ++index) {
                    const WGPUCompilationMessage& entry = info->messages[index];
#if defined(WGPU_BREAKING_CHANGE_STRING_VIEW_OUTPUT_STRUCTS)
                    std::string messageString(entry.message.data, entry.message.length);
#else   // defined(WGPU_BREAKING_CHANGE_STRING_VIEW_OUTPUT_STRUCTS)
                    std::string messageString(entry.message);
#endif  // defined(WGPU_BREAKING_CHANGE_STRING_VIEW_OUTPUT_STRUCTS)
                    errors += "line " + std::to_string(entry.lineNum) + ':' +
                              std::to_string(entry.linePos) + ' ' + messageString + '\n';
                }
                self->fErrorHandler->compileError(
                        self->fShaderText, errors.c_str(), /*shaderWasCached=*/false);
            }
        }

        const char* fShaderText;
        ShaderErrorHandler* fErrorHandler;
        bool fSuccess = false;
    };

    Handler handler;
    handler.fShaderText = shaderText;
    handler.fErrorHandler = errorHandler;
#if defined(__EMSCRIPTEN__)
    // Deprecated function.
    module->GetCompilationInfo(&Handler::Fn, &handler);
#else
    // New API.
    wgpu::FutureWaitInfo waitInfo{};
    waitInfo.future = module->GetCompilationInfo(
            wgpu::CallbackMode::WaitAnyOnly,
            [handlerPtr = &handler](wgpu::CompilationInfoRequestStatus status,
                                    const wgpu::CompilationInfo* info) {
                Handler::Fn(static_cast<WGPUCompilationInfoRequestStatus>(status),
                            reinterpret_cast<const WGPUCompilationInfo*>(info),
                            handlerPtr);
            });

    const auto& instance = static_cast<const DawnSharedContext*>(sharedContext)
                                   ->device()
                                   .GetAdapter()
                                   .GetInstance();
    [[maybe_unused]] auto status =
            instance.WaitAny(1, &waitInfo, /*timeoutNS=*/std::numeric_limits<uint64_t>::max());
    SkASSERT(status == wgpu::WaitStatus::Success);
#endif  // defined(__EMSCRIPTEN__)

    return handler.fSuccess;
#endif
}

bool DawnCompileWGSLShaderModule(const DawnSharedContext* sharedContext,
                                 const char* label,
                                 const std::string& wgsl,
                                 wgpu::ShaderModule* module,
                                 ShaderErrorHandler* errorHandler) {
#if defined(__EMSCRIPTEN__)
    wgpu::ShaderModuleWGSLDescriptor wgslDesc;
#else
    wgpu::ShaderSourceWGSL wgslDesc;
#endif
    wgslDesc.code = wgsl.c_str();

    wgpu::ShaderModuleDescriptor desc;
    desc.nextInChain = &wgslDesc;
    if (sharedContext->caps()->setBackendLabels()) {
        desc.label = label;
    }

    *module = sharedContext->device().CreateShaderModule(&desc);

    return check_shader_module(sharedContext, module, wgsl.c_str(), errorHandler);
}

#if !defined(__EMSCRIPTEN__)

namespace {

static constexpr int kUsesExternalFormatBits  = 1;
static constexpr int kYcbcrModelBits          = 3;
static constexpr int kYcbcrRangeBits          = 1;
static constexpr int kXChromaOffsetBits       = 1;
static constexpr int kYChromaOffsetBits       = 1;
// wgpu::FilterMode contains Undefined/Nearest/Linear entries (Linear is 2).
static constexpr int kChromaFilterBits        = 2;
static constexpr int kForceExplicitReconBits  = 1;
static constexpr int kComponentBits           = 3;

static constexpr int kUsesExternalFormatShift = 0;
static constexpr int kYcbcrModelShift         = kUsesExternalFormatShift + kUsesExternalFormatBits;
static constexpr int kYcbcrRangeShift         = kYcbcrModelShift         + kYcbcrModelBits;
static constexpr int kXChromaOffsetShift      = kYcbcrRangeShift         + kYcbcrRangeBits;
static constexpr int kYChromaOffsetShift      = kXChromaOffsetShift      + kXChromaOffsetBits;
static constexpr int kChromaFilterShift       = kYChromaOffsetShift      + kYChromaOffsetBits;
static constexpr int kForceExplicitReconShift = kChromaFilterShift       + kChromaFilterBits;
static constexpr int kComponentRShift         = kForceExplicitReconShift + kForceExplicitReconBits;
static constexpr int kComponentGShift         = kComponentRShift         + kComponentBits;
static constexpr int kComponentBShift         = kComponentGShift         + kComponentBits;
static constexpr int kComponentAShift         = kComponentBShift         + kComponentBits;

static constexpr uint32_t kUseExternalFormatMask =
        ((1 << kUsesExternalFormatBits) - 1) << kUsesExternalFormatShift;
static constexpr uint32_t kYcbcrModelMask =
        ((1 << kYcbcrModelBits) - 1) << kYcbcrModelShift;
static constexpr uint32_t kYcbcrRangeMask =
        ((1 << kYcbcrRangeBits) - 1) << kYcbcrRangeShift;
static constexpr uint32_t kXChromaOffsetMask =
        ((1 << kXChromaOffsetBits) - 1) << kXChromaOffsetShift;
static constexpr uint32_t kYChromaOffsetMask =
        ((1 << kYChromaOffsetBits) - 1) << kYChromaOffsetShift;
static constexpr uint32_t kChromaFilterMask =
        ((1 << kChromaFilterBits) - 1) << kChromaFilterShift;
static constexpr uint32_t kForceExplicitReconMask =
        ((1 << kForceExplicitReconBits) - 1) << kForceExplicitReconShift;
static constexpr uint32_t kComponentRMask = ((1 << kComponentBits) - 1) << kComponentRShift;
static constexpr uint32_t kComponentBMask = ((1 << kComponentBits) - 1) << kComponentBShift;
static constexpr uint32_t kComponentGMask = ((1 << kComponentBits) - 1) << kComponentGShift;
static constexpr uint32_t kComponentAMask = ((1 << kComponentBits) - 1) << kComponentAShift;

} // anonymous namespace

bool DawnDescriptorsAreEquivalent(const wgpu::YCbCrVkDescriptor& desc1,
                                  const wgpu::YCbCrVkDescriptor& desc2) {
    return desc1.vkFormat                    == desc2.vkFormat                    &&
           desc1.vkYCbCrModel                == desc2.vkYCbCrModel                &&
           desc1.vkYCbCrRange                == desc2.vkYCbCrRange                &&
           desc1.vkComponentSwizzleRed       == desc2.vkComponentSwizzleRed       &&
           desc1.vkComponentSwizzleGreen     == desc2.vkComponentSwizzleGreen     &&
           desc1.vkComponentSwizzleBlue      == desc2.vkComponentSwizzleBlue      &&
           desc1.vkComponentSwizzleAlpha     == desc2.vkComponentSwizzleAlpha     &&
           desc1.vkXChromaOffset             == desc2.vkXChromaOffset             &&
           desc1.vkYChromaOffset             == desc2.vkYChromaOffset             &&
           desc1.vkChromaFilter              == desc2.vkChromaFilter              &&
           desc1.forceExplicitReconstruction == desc2.forceExplicitReconstruction &&
           desc1.externalFormat              == desc2.externalFormat;
}

bool DawnDescriptorIsValid(const wgpu::YCbCrVkDescriptor& desc) {
    static const wgpu::YCbCrVkDescriptor kDefaultYcbcrDescriptor = {};
    return !DawnDescriptorsAreEquivalent(desc, kDefaultYcbcrDescriptor);
}

bool DawnDescriptorUsesExternalFormat(const wgpu::YCbCrVkDescriptor& desc) {
    SkASSERT(desc.externalFormat != 0 || desc.vkFormat != 0);
    return desc.externalFormat != 0;
}

ImmutableSamplerInfo DawnDescriptorToImmutableSamplerInfo(const wgpu::YCbCrVkDescriptor& desc) {
    static_assert(kComponentAShift + kComponentBits <= 32);
    SkASSERT(desc.vkYCbCrModel                          < (1u << kYcbcrModelBits    ));
    SkASSERT(desc.vkYCbCrRange                          < (1u << kYcbcrRangeBits    ));
    SkASSERT(desc.vkXChromaOffset                       < (1u << kXChromaOffsetBits ));
    SkASSERT(desc.vkYChromaOffset                       < (1u << kYChromaOffsetBits ));
    SkASSERT(static_cast<uint32_t>(desc.vkChromaFilter) < (1u << kChromaFilterBits  ));
    SkASSERT(desc.vkComponentSwizzleRed                 < (1u << kComponentBits     ));
    SkASSERT(desc.vkComponentSwizzleGreen               < (1u << kComponentBits     ));
    SkASSERT(desc.vkComponentSwizzleBlue                < (1u << kComponentBits     ));
    SkASSERT(desc.vkComponentSwizzleAlpha               < (1u << kComponentBits     ));
    SkASSERT(static_cast<uint32_t>(desc.forceExplicitReconstruction)
             < (1u << kForceExplicitReconBits));

    const bool usesExternalFormat = DawnDescriptorUsesExternalFormat(desc);

    ImmutableSamplerInfo info;
    info.fNonFormatYcbcrConversionInfo =
            (((uint32_t)(usesExternalFormat               ) << kUsesExternalFormatShift) |
             ((uint32_t)(desc.vkYCbCrModel                ) << kYcbcrModelShift        ) |
             ((uint32_t)(desc.vkYCbCrRange                ) << kYcbcrRangeShift        ) |
             ((uint32_t)(desc.vkXChromaOffset             ) << kXChromaOffsetShift     ) |
             ((uint32_t)(desc.vkYChromaOffset             ) << kYChromaOffsetShift     ) |
             ((uint32_t)(desc.vkChromaFilter              ) << kChromaFilterShift      ) |
             ((uint32_t)(desc.forceExplicitReconstruction ) << kForceExplicitReconShift) |
             ((uint32_t)(desc.vkComponentSwizzleRed       ) << kComponentRShift        ) |
             ((uint32_t)(desc.vkComponentSwizzleGreen     ) << kComponentGShift        ) |
             ((uint32_t)(desc.vkComponentSwizzleBlue      ) << kComponentBShift        ) |
             ((uint32_t)(desc.vkComponentSwizzleAlpha     ) << kComponentAShift        ));
    info.fFormat = usesExternalFormat ? desc.externalFormat : desc.vkFormat;
    return info;
}

wgpu::YCbCrVkDescriptor DawnDescriptorFromImmutableSamplerInfo(ImmutableSamplerInfo info) {
    const uint32_t nonFormatInfo = info.fNonFormatYcbcrConversionInfo;

    wgpu::YCbCrVkDescriptor desc;
    const bool usesExternalFormat =
            (nonFormatInfo >> kUsesExternalFormatShift) & kUseExternalFormatMask;
    if (usesExternalFormat) {
        desc.vkFormat = 0;
        desc.externalFormat = info.fFormat;
    } else {
        desc.vkFormat = (uint32_t) info.fFormat;
        desc.externalFormat = 0;
    }

    desc.vkYCbCrModel                = (nonFormatInfo & kYcbcrModelMask)    >> kYcbcrModelShift;
    desc.vkYCbCrRange                = (nonFormatInfo & kYcbcrRangeMask)    >> kYcbcrRangeShift;
    desc.vkComponentSwizzleRed       = (nonFormatInfo & kComponentRMask)    >> kComponentRShift;
    desc.vkComponentSwizzleGreen     = (nonFormatInfo & kComponentGMask)    >> kComponentGShift;
    desc.vkComponentSwizzleBlue      = (nonFormatInfo & kComponentBMask)    >> kComponentBShift;
    desc.vkComponentSwizzleAlpha     = (nonFormatInfo & kComponentAMask)    >> kComponentAShift;
    desc.vkXChromaOffset             = (nonFormatInfo & kXChromaOffsetMask) >> kXChromaOffsetShift;
    desc.vkYChromaOffset             = (nonFormatInfo & kYChromaOffsetMask) >> kYChromaOffsetShift;
    desc.vkChromaFilter              = static_cast<wgpu::FilterMode>(
            (nonFormatInfo & kChromaFilterMask) >> kChromaFilterShift);
    desc.forceExplicitReconstruction =
            (nonFormatInfo & kForceExplicitReconMask) >> kForceExplicitReconShift;
    return desc;
}

#endif // !defined(__EMSCRIPTEN__)

} // namespace skgpu::graphite
