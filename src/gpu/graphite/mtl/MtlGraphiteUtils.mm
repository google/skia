/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "include/gpu/ShaderErrorHandler.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/mtl/MtlBackendContext.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/graphite/mtl/MtlGraphiteUtils.h"
#include "src/gpu/graphite/mtl/MtlQueueManager.h"
#include "src/gpu/graphite/mtl/MtlSharedContext.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"

#import <Metal/Metal.h>

namespace skgpu::graphite {

namespace ContextFactory {

std::unique_ptr<Context> MakeMetal(const MtlBackendContext& backendContext,
                                   const ContextOptions& options) {
    sk_sp<SharedContext> sharedContext = MtlSharedContext::Make(backendContext, options);
    if (!sharedContext) {
        return nullptr;
    }

    sk_cfp<id<MTLCommandQueue>> queue =
            sk_ret_cfp((id<MTLCommandQueue>)(backendContext.fQueue.get()));
    auto queueManager = std::make_unique<MtlQueueManager>(std::move(queue), sharedContext.get());
    if (!queueManager) {
        return nullptr;
    }

    return ContextCtorAccessor::MakeContext(std::move(sharedContext),
                                            std::move(queueManager),
                                            options);
}

} // namespace ContextFactory

sk_cfp<id<MTLLibrary>> MtlCompileShaderLibrary(const MtlSharedContext* sharedContext,
                                               std::string_view label,
                                               std::string_view msl,
                                               ShaderErrorHandler* errorHandler) {
    TRACE_EVENT0("skia.shaders", "driver_compile_shader");
    NSString* nsSource = [[NSString alloc] initWithBytesNoCopy:const_cast<char*>(msl.data())
                                                        length:msl.size()
                                                      encoding:NSUTF8StringEncoding
                                                  freeWhenDone:NO];
    if (!nsSource) {
        return nil;
    }
    MTLCompileOptions* options = [[MTLCompileOptions alloc] init];

    // Framebuffer fetch is supported in MSL 2.3 in MacOS 11+.
    if (@available(macOS 11.0, iOS 14.0, tvOS 14.0, *)) {
        options.languageVersion = MTLLanguageVersion2_3;

    // array<> is supported in MSL 2.0 on MacOS 10.13+ and iOS 11+,
    // and in MSL 1.2 on iOS 10+ (but not MacOS).
    } else if (@available(macOS 10.13, iOS 11.0, tvOS 11.0, *)) {
        options.languageVersion = MTLLanguageVersion2_0;
#if defined(SK_BUILD_FOR_IOS)
    } else if (@available(macOS 10.12, iOS 10.0, tvOS 10.0, *)) {
        options.languageVersion = MTLLanguageVersion1_2;
#endif
    }

    NSError* error = nil;
    // TODO: do we need a version with a timeout?
    sk_cfp<id<MTLLibrary>> compiledLibrary(
            [sharedContext->device() newLibraryWithSource:(NSString* _Nonnull)nsSource
                                                  options:options
                                                    error:&error]);
    if (!compiledLibrary) {
        std::string mslStr(msl);
        errorHandler->compileError(
                mslStr.c_str(), error.debugDescription.UTF8String, /*shaderWasCached=*/false);
        return nil;
    }

    NSString* nsLabel = [[NSString alloc] initWithBytesNoCopy:const_cast<char*>(label.data())
                                                       length:label.size()
                                                     encoding:NSUTF8StringEncoding
                                                 freeWhenDone:NO];
    compiledLibrary.get().label = nsLabel;
    return compiledLibrary;
}



// Most formats in MTL_FORMAT_MAPPING for TextureFormat are always available for Graphite's
// supported Mac and iOS versions. These formats are not. To simplify the mapping, these constants
// are extracted from Frameworks/Metal/MTLPixelFormat.h to avoid guarding with an @available check,
// although MtlCaps must still do so when enabling a particular format.

// Either unsupported on iOS or high min iOS, so considered "mac-only"
static constexpr MTLPixelFormat MTLPixelFormatBC1_RGBA_{130};
static constexpr MTLPixelFormat MTLPixelFormatBC1_RGBA_sRGB_{131};
static constexpr MTLPixelFormat MTLPixelFormatDepth24Unorm_Stencil8_{255};
// Requires Mac 11.0+, while we support 10.15+. All meet Graphite's minimum supported iOS version
static constexpr MTLPixelFormat MTLPixelFormatB5G6R5Unorm_{40};
static constexpr MTLPixelFormat MTLPixelFormatABGR4Unorm_{42};
static constexpr MTLPixelFormat MTLPixelFormatBGR10_XR_{554};
static constexpr MTLPixelFormat MTLPixelFormatBGRA10_XR_{552};
static constexpr MTLPixelFormat MTLPixelFormatETC2_RGB8_{180};
static constexpr MTLPixelFormat MTLPixelFormatETC2_RGB8_sRGB_{181};

static void validate_mtl_pixelformats() {
#if defined(SK_BUILD_FOR_MAC)
    SkASSERT(MTLPixelFormatBC1_RGBA_ == MTLPixelFormatBC1_RGBA);
    SkASSERT(MTLPixelFormatBC1_RGBA_sRGB_ == MTLPixelFormatBC1_RGBA_sRGB);
    SkASSERT(MTLPixelFormatDepth24Unorm_Stencil8_ == MTLPixelFormatDepth24Unorm_Stencil8);
#endif
    if (@available(macOS 11.0, iOS 10.0, *)) {
        SkASSERT(MTLPixelFormatB5G6R5Unorm_ == MTLPixelFormatB5G6R5Unorm);
        SkASSERT(MTLPixelFormatABGR4Unorm_ == MTLPixelFormatABGR4Unorm);
        SkASSERT(MTLPixelFormatBGR10_XR_ == MTLPixelFormatBGR10_XR);
        SkASSERT(MTLPixelFormatBGRA10_XR_ == MTLPixelFormatBGRA10_XR);
        SkASSERT(MTLPixelFormatETC2_RGB8_ == MTLPixelFormatETC2_RGB8);
        SkASSERT(MTLPixelFormatETC2_RGB8_sRGB_ == MTLPixelFormatETC2_RGB8_sRGB);
    }
}

#define MTL_FORMAT_MAPPING(M) \
    M(TextureFormat::kR8,             MTLPixelFormatR8Unorm)                \
    M(TextureFormat::kR16,            MTLPixelFormatR16Unorm)               \
    M(TextureFormat::kR16F,           MTLPixelFormatR16Float)               \
    M(TextureFormat::kR32F,           MTLPixelFormatR32Float)               \
    M(TextureFormat::kA8,             MTLPixelFormatA8Unorm)                \
    M(TextureFormat::kRG8,            MTLPixelFormatRG8Unorm)               \
    M(TextureFormat::kRG16,           MTLPixelFormatRG16Unorm)              \
    M(TextureFormat::kRG16F,          MTLPixelFormatRG16Float)              \
    M(TextureFormat::kRG32F,          MTLPixelFormatRG32Float)              \
    /*TextureFormat::kRGB8,           unsupported */                        \
    /*TextureFormat::kBGR8,           unsupported */                        \
    M(TextureFormat::kB5_G6_R5,       MTLPixelFormatB5G6R5Unorm_)           \
    /*TextureFormat::kR5_G6_B5,       unsupported */                        \
    /*TextureFormat::kRGB16,          unsupported */                        \
    /*TextureFormat::kRGB16F,         unsupported */                        \
    /*TextureFormat::kRGB32F,         unsupported */                        \
    /*TextureFormat::kRGB8_sRGB,      unsupported */                        \
    M(TextureFormat::kBGR10_XR,       MTLPixelFormatBGR10_XR_)              \
    M(TextureFormat::kRGBA8,          MTLPixelFormatRGBA8Unorm)             \
    M(TextureFormat::kRGBA16,         MTLPixelFormatRGBA16Unorm)            \
    M(TextureFormat::kRGBA16F,        MTLPixelFormatRGBA16Float)            \
    M(TextureFormat::kRGBA32F,        MTLPixelFormatRGBA32Float)            \
    M(TextureFormat::kRGB10_A2,       MTLPixelFormatRGB10A2Unorm)           \
    M(TextureFormat::kRGBA8_sRGB,     MTLPixelFormatRGBA8Unorm_sRGB)        \
    M(TextureFormat::kBGRA8,          MTLPixelFormatBGRA8Unorm)             \
    M(TextureFormat::kBGR10_A2,       MTLPixelFormatBGR10A2Unorm)           \
    M(TextureFormat::kBGRA8_sRGB,     MTLPixelFormatBGRA8Unorm_sRGB)        \
    M(TextureFormat::kABGR4,          MTLPixelFormatABGR4Unorm_)            \
    /*TextureFormat::kARGB4,          unsupported */                        \
    M(TextureFormat::kBGRA10x6_XR,    MTLPixelFormatBGRA10_XR_)             \
    M(TextureFormat::kRGB8_ETC2,      MTLPixelFormatETC2_RGB8_)             \
    M(TextureFormat::kRGB8_ETC2_sRGB, MTLPixelFormatETC2_RGB8_sRGB_)        \
    /*TextureFormat::kRGB8_BC1,       unsupported */                        \
    M(TextureFormat::kRGBA8_BC1,      MTLPixelFormatBC1_RGBA_)              \
    M(TextureFormat::kRGBA8_BC1_sRGB, MTLPixelFormatBC1_RGBA_sRGB_)         \
    /*TextureFormat::kYUV8_P2_420,    unsupported */                        \
    /*TextureFormat::kYUV8_P3_420,    unsupported */                        \
    /*TextureFormat::kYUV10x6_P2_420, unsupported */                        \
    /*TextureFormat::kExternal,       unsupported */                        \
    M(TextureFormat::kS8,             MTLPixelFormatStencil8)               \
    M(TextureFormat::kD16,            MTLPixelFormatDepth16Unorm)           \
    M(TextureFormat::kD32F,           MTLPixelFormatDepth32Float)           \
    M(TextureFormat::kD24_S8,         MTLPixelFormatDepth24Unorm_Stencil8_) \
    M(TextureFormat::kD32F_S8,        MTLPixelFormatDepth32Float_Stencil8)

TextureFormat MTLPixelFormatToTextureFormat(MTLPixelFormat format) {
#define M(TF, MTL) case MTL: return TF;
    switch(format) {
        MTL_FORMAT_MAPPING(M)
        default: return TextureFormat::kUnsupported;
    }
#undef M
}
MTLPixelFormat TextureFormatToMTLPixelFormat(TextureFormat format) {
    // Validate constants that can't be statically validated due to @available
    validate_mtl_pixelformats();

#define M(TF, MTL) case TF: return MTL;
    switch(format) {
        MTL_FORMAT_MAPPING(M)
        default: return MTLPixelFormatInvalid;
    }
#undef M
}

}  // namespace skgpu::graphite
