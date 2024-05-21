/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/mtl/MtlGraphiteUtils.h"
#include "src/gpu/graphite/mtl/MtlGraphiteUtilsPriv.h"

#include "include/gpu/ShaderErrorHandler.h"
#include "include/gpu/graphite/Context.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/mtl/MtlQueueManager.h"
#include "src/gpu/graphite/mtl/MtlSharedContext.h"
#include "src/gpu/mtl/MtlUtilsPriv.h"

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

size_t MtlFormatBytesPerBlock(MtlPixelFormat format) {
    return skgpu::MtlFormatBytesPerBlock((MTLPixelFormat) format);
}

SkTextureCompressionType MtlFormatToCompressionType(MtlPixelFormat format) {
    return skgpu::MtlFormatToCompressionType((MTLPixelFormat) format);
}

} // namespace skgpu::graphite
