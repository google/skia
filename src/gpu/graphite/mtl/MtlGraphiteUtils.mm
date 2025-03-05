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

TextureFormat MTLPixelFormatToTextureFormat(MTLPixelFormat format) {
    switch (format) {
        case MTLPixelFormatR8Unorm:               return TextureFormat::kR8;
        case MTLPixelFormatR16Unorm:              return TextureFormat::kR16;
        case MTLPixelFormatR16Float:              return TextureFormat::kR16F;
        case MTLPixelFormatR32Float:              return TextureFormat::kR32F;
        case MTLPixelFormatA8Unorm:               return TextureFormat::kA8;
        case MTLPixelFormatRG8Unorm:              return TextureFormat::kRG8;
        case MTLPixelFormatRG16Unorm:             return TextureFormat::kRG16;
        case MTLPixelFormatRG16Float:             return TextureFormat::kRG16F;
        case MTLPixelFormatRG32Float:             return TextureFormat::kRG32F;
        case MTLPixelFormatB5G6R5Unorm:           return TextureFormat::kB5_G6_R5;
        case MTLPixelFormatBGR10_XR:              return TextureFormat::kBGR10_XR;
        case MTLPixelFormatRGBA8Unorm:            return TextureFormat::kRGBA8;
        case MTLPixelFormatRGBA16Unorm:           return TextureFormat::kRGBA16;
        case MTLPixelFormatRGBA32Float:           return TextureFormat::kRGBA32F;
        case MTLPixelFormatRGB10A2Unorm:          return TextureFormat::kRGB10_A2;
        case MTLPixelFormatRGBA8Unorm_sRGB:       return TextureFormat::kRGBA8_sRGB;
        case MTLPixelFormatBGRA8Unorm:            return TextureFormat::kBGRA8;
        case MTLPixelFormatBGR10A2Unorm:          return TextureFormat::kBGR10_A2;
        case MTLPixelFormatBGRA8Unorm_sRGB:       return TextureFormat::kBGRA8_sRGB;
        case MTLPixelFormatABGR4Unorm:            return TextureFormat::kABGR4;
        case MTLPixelFormatBGRA10_XR:             return TextureFormat::kBGRA10x6_XR;
        case MTLPixelFormatETC2_RGB8:             return TextureFormat::kRGB8_ETC2;
        case MTLPixelFormatETC2_RGB8_sRGB:        return TextureFormat::kRGB8_ETC2_sRGB;
        case MTLPixelFormatStencil8:              return TextureFormat::kS8;
        case MTLPixelFormatDepth16Unorm:          return TextureFormat::kD16;
        case MTLPixelFormatDepth32Float:          return TextureFormat::kD32F;
        case MTLPixelFormatDepth32Float_Stencil8: return TextureFormat::kD32F_S8;
#if defined(SK_BUILD_FOR_MAC)
        case MTLPixelFormatDepth24Unorm_Stencil8: return TextureFormat::kD24_S8;
        case MTLPixelFormatBC1_RGBA:              return TextureFormat::kRGBA8_BC1;
        case MTLPixelFormatBC1_RGBA_sRGB:         return TextureFormat::kRGBA8_BC1_sRGB;
#endif
        default:                                  return TextureFormat::kUnsupported;
    }
}

}  // namespace skgpu::graphite
