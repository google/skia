/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlUtils.h"

#include "experimental/graphite/src/mtl/MtlGpu.h"
#include "include/private/SkSLString.h"
#include "src/core/SkTraceEvent.h"

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

MTLPixelFormat SkColorTypeToFormat(SkColorType colorType) {
    switch (colorType) {
        case kRGBA_8888_SkColorType:
            return MTLPixelFormatRGBA8Unorm;
        case kAlpha_8_SkColorType:
            return MTLPixelFormatR8Unorm;
        default:
            // TODO: fill in the rest of the formats
            SkUNREACHABLE;
    }
}

MTLPixelFormat DepthStencilTypeToFormat(DepthStencilType type) {
    // TODO: Decide if we want to change this to always return a combined depth and stencil format
    // to allow more sharing of depth stencil allocations.
    switch (type) {
        case DepthStencilType::kDepthOnly:
            // MTLPixelFormatDepth16Unorm is also a universally supported option here
            return MTLPixelFormatDepth32Float;
        case DepthStencilType::kStencilOnly:
            return MTLPixelFormatStencil8;
        case DepthStencilType::kDepthStencil:
            // MTLPixelFormatDepth24Unorm_Stencil8 is supported on Mac family GPUs.
            return MTLPixelFormatDepth32Float_Stencil8;
    }
}

sk_cfp<id<MTLLibrary>> CompileShaderLibrary(const Gpu* gpu,
                                            const SkSL::String& msl) {
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
        SkDebugf("Shader compilation error\n"
                 "------------------------\n");
        SkDebugf("%s", msl.c_str());
        SkDebugf("Errors:\n%s", error.debugDescription.UTF8String);

        return nil;
    }

    return compiledLibrary;
}

} // namespace skgpu::mtl

