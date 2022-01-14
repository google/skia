/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlUtils.h"

#include "experimental/graphite/src/mtl/MtlGpu.h"
#include "include/gpu/ShaderErrorHandler.h"
#include "include/private/SkSLString.h"
#include "src/core/SkTraceEvent.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/utils/SkShaderUtils.h"

#ifdef SK_BUILD_FOR_IOS
#import <UIKit/UIApplication.h>
#endif

namespace skgpu::mtl {

bool FormatIsDepthOrStencil(MTLPixelFormat format) {
    switch (format) {
        case MTLPixelFormatStencil8: // fallthrough
        case MTLPixelFormatDepth32Float_Stencil8:
            return true;
        default:
            return false;
    }
}

bool FormatIsDepth(MTLPixelFormat format) {
    switch (format) {
        case MTLPixelFormatDepth32Float_Stencil8:
            return true;
        default:
            return false;
    }
}

bool FormatIsStencil(MTLPixelFormat format) {
    switch (format) {
        case MTLPixelFormatStencil8: // fallthrough
        case MTLPixelFormatDepth32Float_Stencil8:
            return true;
        default:
            return false;
    }
}

MTLPixelFormat SkColorTypeToFormat(SkColorType colorType) {
    switch (colorType) {
        case kRGBA_8888_SkColorType:
            return MTLPixelFormatRGBA8Unorm;
        case kBGRA_8888_SkColorType:
            return MTLPixelFormatBGRA8Unorm;
        case kAlpha_8_SkColorType:
            return MTLPixelFormatR8Unorm;
        case kRGBA_F16_SkColorType:
            return MTLPixelFormatRGBA16Float;
        default:
            // TODO: fill in the rest of the formats
            SkUNREACHABLE;
    }
}

MTLPixelFormat DepthStencilFlagsToFormat(Mask<DepthStencilFlags> mask) {
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
static const bool gPrintSKSL = false;
static const bool gPrintMSL = false;

bool SkSLToMSL(const Gpu* gpu,
               const SkSL::String& sksl,
               SkSL::ProgramKind programKind,
               const SkSL::Program::Settings& settings,
               SkSL::String* msl,
               SkSL::Program::Inputs* outInputs,
               ShaderErrorHandler* errorHandler) {
#ifdef SK_DEBUG
    SkSL::String src = SkShaderUtils::PrettyPrint(sksl);
#else
    const SkSL::String& src = sksl;
#endif
    SkSL::Compiler* compiler = gpu->shaderCompiler();
    std::unique_ptr<SkSL::Program> program =
            gpu->shaderCompiler()->convertProgram(programKind,
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

sk_cfp<id<MTLLibrary>> CompileShaderLibrary(const Gpu* gpu,
                                            const SkSL::String& msl,
                                            ShaderErrorHandler* errorHandler) {
    TRACE_EVENT0("skia.shaders", "driver_compile_shader");
    auto nsSource = [[NSString alloc] initWithBytesNoCopy:const_cast<char*>(msl.c_str())
                                                   length:msl.size()
                                                 encoding:NSUTF8StringEncoding
                                             freeWhenDone:NO];
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
    sk_cfp<id<MTLLibrary>> compiledLibrary([gpu->device() newLibraryWithSource:nsSource
                                                                       options:options
                                                                         error:&error]);
    if (!compiledLibrary) {
        errorHandler->compileError(msl.c_str(), error.debugDescription.UTF8String);
        return nil;
    }

    return compiledLibrary;
}

#ifdef SK_BUILD_FOR_IOS
bool IsAppInBackground() {
    return [NSThread isMainThread] &&
           ([UIApplication sharedApplication].applicationState == UIApplicationStateBackground);
}
#endif
} // namespace skgpu::mtl
