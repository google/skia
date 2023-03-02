/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/mtl/MtlUtils.h"
#include "src/gpu/graphite/mtl/MtlUtilsPriv.h"

#include "include/gpu/ShaderErrorHandler.h"
#include "include/gpu/graphite/Context.h"
#include "include/private/SkSLString.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/mtl/MtlQueueManager.h"
#include "src/gpu/graphite/mtl/MtlSharedContext.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/utils/SkShaderUtils.h"

#ifdef SK_BUILD_FOR_IOS
#import <UIKit/UIApplication.h>
#endif

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

bool MtlFormatIsDepthOrStencil(MTLPixelFormat format) {
    switch (format) {
        case MTLPixelFormatStencil8: // fallthrough
        case MTLPixelFormatDepth32Float:
        case MTLPixelFormatDepth32Float_Stencil8:
            return true;
        default:
            return false;
    }
}

bool MtlFormatIsDepth(MTLPixelFormat format) {
    switch (format) {
        case MTLPixelFormatDepth32Float:
        case MTLPixelFormatDepth32Float_Stencil8:
            return true;
        default:
            return false;
    }
}

bool MtlFormatIsStencil(MTLPixelFormat format) {
    switch (format) {
        case MTLPixelFormatStencil8: // fallthrough
        case MTLPixelFormatDepth32Float_Stencil8:
            return true;
        default:
            return false;
    }
}

MTLPixelFormat MtlDepthStencilFlagsToFormat(SkEnumBitMask<DepthStencilFlags> mask) {
    // TODO: Decide if we want to change this to always return a combined depth and stencil format
    // to allow more sharing of depth stencil allocations.
    if (mask == DepthStencilFlags::kDepth) {
        // MTLPixelFormatDepth16Unorm is also a universally supported option here
        return MTLPixelFormatDepth32Float;
    } else if (mask == DepthStencilFlags::kStencil) {
        return MTLPixelFormatStencil8;
    } else if (mask == DepthStencilFlags::kDepthStencil) {
        // MTLPixelFormatDepth24Unorm_Stencil8 is supported on Mac family GPUs.
        return MTLPixelFormatDepth32Float_Stencil8;
    }
    SkASSERT(false);
    return MTLPixelFormatInvalid;
}

// Print the source code for all shaders generated.
#ifdef SK_PRINT_SKSL_SHADERS
static const bool gPrintSKSL = true;
#else
static const bool gPrintSKSL = false;
#endif

#ifdef SK_PRINT_NATIVE_SHADERS
static const bool gPrintMSL = true;
#else
static const bool gPrintMSL = false;
#endif

bool SkSLToMSL(SkSL::Compiler* compiler,
               const std::string& sksl,
               SkSL::ProgramKind programKind,
               const SkSL::ProgramSettings& settings,
               std::string* msl,
               SkSL::Program::Inputs* outInputs,
               ShaderErrorHandler* errorHandler) {
#ifdef SK_DEBUG
    std::string src = SkShaderUtils::PrettyPrint(sksl);
#else
    const std::string& src = sksl;
#endif
    std::unique_ptr<SkSL::Program> program = compiler->convertProgram(programKind,
                                                                      src,
                                                                      settings);
    if (!program || !compiler->toMetal(*program, msl)) {
        errorHandler->compileError(src.c_str(), compiler->errorText().c_str());
        return false;
    }

    if (gPrintSKSL || gPrintMSL) {
        SkShaderUtils::PrintShaderBanner(programKind);
        if (gPrintSKSL) {
            SkDebugf("SKSL:\n");
            SkShaderUtils::PrintLineByLine(SkShaderUtils::PrettyPrint(sksl));
        }
        if (gPrintMSL) {
            SkDebugf("MSL:\n");
            SkShaderUtils::PrintLineByLine(SkShaderUtils::PrettyPrint(*msl));
        }
    }

    *outInputs = program->fInputs;
    return true;
}

sk_cfp<id<MTLLibrary>> MtlCompileShaderLibrary(const MtlSharedContext* sharedContext,
                                               const std::string& msl,
                                               ShaderErrorHandler* errorHandler) {
    TRACE_EVENT0("skia.shaders", "driver_compile_shader");
    NSString* nsSource = [[NSString alloc] initWithBytesNoCopy:const_cast<char*>(msl.c_str())
                                                        length:msl.size()
                                                      encoding:NSUTF8StringEncoding
                                                  freeWhenDone:NO];
    if (!nsSource) {
        return nil;
    }
    MTLCompileOptions* options = [[MTLCompileOptions alloc] init];
    // array<> is supported in MSL 2.0 on MacOS 10.13+ and iOS 11+,
    // and in MSL 1.2 on iOS 10+ (but not MacOS).
    if (@available(macOS 10.13, iOS 11.0, *)) {
        options.languageVersion = MTLLanguageVersion2_0;
#if defined(SK_BUILD_FOR_IOS)
    } else if (@available(macOS 10.12, iOS 10.0, *)) {
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
        errorHandler->compileError(msl.c_str(), error.debugDescription.UTF8String);
        return nil;
    }

    return compiledLibrary;
}

bool MtlFormatIsCompressed(MTLPixelFormat mtlFormat) {
    switch (mtlFormat) {
        case MTLPixelFormatETC2_RGB8:
            return true;
#ifdef SK_BUILD_FOR_MAC
        case MTLPixelFormatBC1_RGBA:
            return true;
#endif
        default:
            return false;
    }
}

#ifdef SK_BUILD_FOR_IOS
bool MtlIsAppInBackground() {
    return [NSThread isMainThread] &&
           ([UIApplication sharedApplication].applicationState == UIApplicationStateBackground);
}
#endif
} // namespace skgpu::graphite
