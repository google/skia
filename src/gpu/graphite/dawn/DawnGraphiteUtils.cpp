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

#define DAWN_FORMAT_MAPPING(M) \
        M(TextureFormat::kR8,             wgpu::TextureFormat::R8Unorm)                     \
        /*TextureFormat::kR16             Native-only */                                    \
        M(TextureFormat::kR16F,           wgpu::TextureFormat::R16Float)                    \
        M(TextureFormat::kR32F,           wgpu::TextureFormat::R32Float)                    \
        /*TextureFormat::kA8              unsupported */                                    \
        M(TextureFormat::kRG8,            wgpu::TextureFormat::RG8Unorm)                    \
        /*TextureFormat::kR16             Native-only */                                    \
        M(TextureFormat::kRG16F,          wgpu::TextureFormat::RG16Float)                   \
        M(TextureFormat::kRG32F,          wgpu::TextureFormat::RG32Float)                   \
        /*TextureFormat::kRGB8            unsupported */                                    \
        /*TextureFormat::kBGR8            unsupported */                                    \
        /*TextureFormat::kB5_G6_R5,       unsupported */                                    \
        /*TextureFormat::kR5_G6_B5        unsupported */                                    \
        /*TextureFormat::kRGB16           unsupported */                                    \
        /*TextureFormat::kRGB16F          unsupported */                                    \
        /*TextureFormat::kRGB32F          unsupported */                                    \
        /*TextureFormat::kRGB8_sRGB       unsupported */                                    \
        /*TextureFormat::kBGR10_XR        unsupported */                                    \
        M(TextureFormat::kRGBA8,          wgpu::TextureFormat::RGBA8Unorm)                  \
        /*TextureFormat::kRGBA16          Native-only */                                    \
        M(TextureFormat::kRGBA16F,        wgpu::TextureFormat::RGBA16Float)                 \
        M(TextureFormat::kRGBA32F,        wgpu::TextureFormat::RGBA32Float)                 \
        M(TextureFormat::kRGB10_A2,       wgpu::TextureFormat::RGB10A2Unorm)                \
        /*TextureFormat::kRGBA10x6        unsupported */                                    \
        M(TextureFormat::kRGBA8_sRGB,     wgpu::TextureFormat::RGBA8UnormSrgb)              \
        M(TextureFormat::kBGRA8,          wgpu::TextureFormat::BGRA8Unorm)                  \
        /*TextureFormat::kBGR10_A2        unsupported */                                    \
        M(TextureFormat::kBGRA8_sRGB,     wgpu::TextureFormat::BGRA8UnormSrgb)              \
        /*TextureFormat::kABGR4,          unsupported */                                    \
        /*TextureFormat::kARGB4           unsupported */                                    \
        /*TextureFormat::kBGRA10x6_XR     unsupported */                                    \
        M(TextureFormat::kRGB8_ETC2,      wgpu::TextureFormat::ETC2RGB8Unorm)               \
        M(TextureFormat::kRGB8_ETC2_sRGB, wgpu::TextureFormat::ETC2RGB8UnormSrgb)           \
        /*TextureFormat::kRGB_BC1         unsupported */                                    \
        M(TextureFormat::kRGBA8_BC1,      wgpu::TextureFormat::BC1RGBAUnorm)                \
        M(TextureFormat::kRGBA8_BC1_sRGB, wgpu::TextureFormat::BC1RGBAUnormSrgb)            \
        /*TextureFormat::kYUV8_P2_420     Native-only */                                    \
        /*TextureFormat::kYUV8_P3_420     unsupported */                                    \
        /*TextureFormat::kYUV10x6_P2_420  Native-only */                                    \
        /*TextureFormat::kExternal        Native-only */                                    \
        M(TextureFormat::kS8,             wgpu::TextureFormat::Stencil8)                    \
        M(TextureFormat::kD16,            wgpu::TextureFormat::Depth16Unorm)                \
        M(TextureFormat::kD32F,           wgpu::TextureFormat::Depth32Float)                \
        M(TextureFormat::kD24_S8,         wgpu::TextureFormat::Depth24PlusStencil8)         \
        M(TextureFormat::kD32F_S8,        wgpu::TextureFormat::Depth32FloatStencil8)        \

#if !defined(__EMSCRIPTEN__)
#define DAWN_FORMAT_MAPPING_NATIVE_ONLY(M) \
        M(TextureFormat::kR16,            wgpu::TextureFormat::R16Unorm)                    \
        M(TextureFormat::kRG16,           wgpu::TextureFormat::RG16Unorm)                   \
        M(TextureFormat::kRGBA16,         wgpu::TextureFormat::RGBA16Unorm)                 \
        M(TextureFormat::kYUV8_P2_420,    wgpu::TextureFormat::R8BG8Biplanar420Unorm)       \
        M(TextureFormat::kYUV10x6_P2_420, wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm) \
        M(TextureFormat::kExternal,       wgpu::TextureFormat::External)
#else
#define DAWN_FORMAT_MAPPING_NATIVE_ONLY(M)
#endif

TextureFormat DawnFormatToTextureFormat(wgpu::TextureFormat format) {
#define M(TF, WGPU) case WGPU: return TF;
    switch(format) {
        DAWN_FORMAT_MAPPING(M)
        DAWN_FORMAT_MAPPING_NATIVE_ONLY(M)
        default: return TextureFormat::kUnsupported;
    }
#undef M
}
wgpu::TextureFormat TextureFormatToDawnFormat(TextureFormat format) {
#define M(TF, WGPU) case TF: return WGPU;
    switch(format) {
        DAWN_FORMAT_MAPPING(M)
        DAWN_FORMAT_MAPPING_NATIVE_ONLY(M)
        default: return wgpu::TextureFormat::Undefined;
    }
#undef M
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
                    std::string messageString(entry.message.data, entry.message.length);
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

    const auto& instance = static_cast<const DawnSharedContext*>(sharedContext)->instance();
    [[maybe_unused]] auto status =
            instance.WaitAny(1, &waitInfo, /*timeoutNS=*/std::numeric_limits<uint64_t>::max());
    SkASSERT(status == wgpu::WaitStatus::Success);
#endif  // defined(__EMSCRIPTEN__)

    return handler.fSuccess;
#endif
}

bool DawnCompileWGSLShaderModule(const DawnSharedContext* sharedContext,
                                 const char* label,
                                 const SkSL::NativeShader& wgsl,
                                 wgpu::ShaderModule* module,
                                 ShaderErrorHandler* errorHandler) {
#if defined(__EMSCRIPTEN__)
    wgpu::ShaderModuleWGSLDescriptor wgslDesc;
#else
    wgpu::ShaderSourceWGSL wgslDesc;
#endif
    wgslDesc.code = wgsl.fText.c_str();

    wgpu::ShaderModuleDescriptor desc;
    desc.nextInChain = &wgslDesc;
    if (sharedContext->caps()->setBackendLabels()) {
        desc.label = label;
    }

    *module = sharedContext->device().CreateShaderModule(&desc);

    return check_shader_module(sharedContext, module, wgsl.fText.c_str(), errorHandler);
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
