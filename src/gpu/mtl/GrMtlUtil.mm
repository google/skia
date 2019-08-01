/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlUtil.h"

#include "include/gpu/GrSurface.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlRenderTarget.h"
#include "src/gpu/mtl/GrMtlTexture.h"
#include "src/sksl/SkSLCompiler.h"

#import <Metal/Metal.h>

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

#define PRINT_MSL 0 // print out the MSL code generated

bool GrPixelConfigToMTLFormat(GrPixelConfig config, MTLPixelFormat* format) {
    MTLPixelFormat dontCare;
    if (!format) {
        format = &dontCare;
    }

    switch (config) {
        case kUnknown_GrPixelConfig:
            return false;
        case kRGBA_8888_GrPixelConfig:
            *format = MTLPixelFormatRGBA8Unorm;
            return true;
        case kRGB_888_GrPixelConfig:
            *format = MTLPixelFormatRGBA8Unorm;
            return true;
        case kRGB_888X_GrPixelConfig:
            *format = MTLPixelFormatRGBA8Unorm;
            return true;
        case kRG_88_GrPixelConfig:
            *format = MTLPixelFormatRG8Unorm;
            return true;
        case kBGRA_8888_GrPixelConfig:
            *format = MTLPixelFormatBGRA8Unorm;
            return true;
        case kSRGBA_8888_GrPixelConfig:
            *format = MTLPixelFormatRGBA8Unorm_sRGB;
            return true;
        case kRGBA_1010102_GrPixelConfig:
            *format = MTLPixelFormatRGB10A2Unorm;
            return true;
        case kRGB_565_GrPixelConfig:
#ifdef SK_BUILD_FOR_IOS
            *format = MTLPixelFormatB5G6R5Unorm;
            return true;
#else
            return false;
#endif
        case kRGBA_4444_GrPixelConfig:
#ifdef SK_BUILD_FOR_IOS
            *format = MTLPixelFormatABGR4Unorm;
            return true;
#else
            return false;
#endif
        case kAlpha_8_GrPixelConfig: // fall through
        case kAlpha_8_as_Red_GrPixelConfig:
            *format = MTLPixelFormatR8Unorm;
            return true;
        case kAlpha_8_as_Alpha_GrPixelConfig:
            *format = MTLPixelFormatA8Unorm;
            return true;
        case kGray_8_GrPixelConfig: // fall through
        case kGray_8_as_Red_GrPixelConfig:
            *format = MTLPixelFormatR8Unorm;
            return true;
        case kGray_8_as_Lum_GrPixelConfig:
            return false;
        case kRGBA_float_GrPixelConfig:
            *format = MTLPixelFormatRGBA32Float;
            return true;
        case kRGBA_half_GrPixelConfig:
            *format = MTLPixelFormatRGBA16Float;
            return true;
        case kRGBA_half_Clamped_GrPixelConfig:
            *format = MTLPixelFormatRGBA16Float;
            return true;
        case kAlpha_half_GrPixelConfig: // fall through
        case kAlpha_half_as_Red_GrPixelConfig:
            *format = MTLPixelFormatR16Float;
            return true;
        case kAlpha_half_as_Lum_GrPixelConfig:
            return false;
        case kRGB_ETC1_GrPixelConfig:
#ifdef SK_BUILD_FOR_IOS
            *format = MTLPixelFormatETC2_RGB8;
            return true;
#else
            return false;
#endif
        case kR_16_GrPixelConfig:
            *format = MTLPixelFormatR16Unorm;
            return true;
        case kRG_1616_GrPixelConfig:
            *format = MTLPixelFormatRG16Unorm;
            return true;

        // Experimental (for Y416 and mutant P016/P010)
        case kRGBA_16161616_GrPixelConfig:
            *format = MTLPixelFormatRGBA16Unorm;
            return true;
        case kRG_half_GrPixelConfig:
            *format = MTLPixelFormatRG16Float;
            return true;
    }
    SK_ABORT("Unexpected config");
    return false;
}

MTLTextureDescriptor* GrGetMTLTextureDescriptor(id<MTLTexture> mtlTexture) {
    MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
    texDesc.textureType = mtlTexture.textureType;
    texDesc.pixelFormat = mtlTexture.pixelFormat;
    texDesc.width = mtlTexture.width;
    texDesc.height = mtlTexture.height;
    texDesc.depth = mtlTexture.depth;
    texDesc.mipmapLevelCount = mtlTexture.mipmapLevelCount;
    texDesc.arrayLength = mtlTexture.arrayLength;
    texDesc.sampleCount = mtlTexture.sampleCount;
    texDesc.usage = mtlTexture.usage;
    return texDesc;
}

#if PRINT_MSL
void print_msl(const char* source) {
    SkTArray<SkString> lines;
    SkStrSplit(source, "\n", kStrict_SkStrSplitMode, &lines);
    for (int i = 0; i < lines.count(); i++) {
        SkString& line = lines[i];
        line.prependf("%4i\t", i + 1);
        SkDebugf("%s\n", line.c_str());
    }
}
#endif

id<MTLLibrary> GrCompileMtlShaderLibrary(const GrMtlGpu* gpu,
                                         const char* shaderString,
                                         SkSL::Program::Kind kind,
                                         const SkSL::Program::Settings& settings,
                                         SkSL::Program::Inputs* outInputs) {
    std::unique_ptr<SkSL::Program> program =
            gpu->shaderCompiler()->convertProgram(kind,
                                                  SkSL::String(shaderString),
                                                  settings);

    if (!program) {
        SkDebugf("SkSL error:\n%s\n", gpu->shaderCompiler()->errorText().c_str());
        SkASSERT(false);
        return nil;
    }

    *outInputs = program->fInputs;
    SkSL::String code;
    if (!gpu->shaderCompiler()->toMetal(*program, &code)) {
        SkDebugf("%s\n", gpu->shaderCompiler()->errorText().c_str());
        SkASSERT(false);
        return nil;
    }
    NSString* mtlCode = [[NSString alloc] initWithCString: code.c_str()
                                                 encoding: NSASCIIStringEncoding];
#if PRINT_MSL
    print_msl([mtlCode cStringUsingEncoding: NSASCIIStringEncoding]);
#endif

    MTLCompileOptions* defaultOptions = [[MTLCompileOptions alloc] init];
#if defined(SK_BUILD_FOR_MAC) && defined(GR_USE_COMPLETION_HANDLER)
    bool timedout;
    id<MTLLibrary> compiledLibrary = GrMtlNewLibraryWithSource(gpu->device(), mtlCode,
                                                               defaultOptions, &timedout);
    if (timedout) {
        // try again
        compiledLibrary = GrMtlNewLibraryWithSource(gpu->device(), mtlCode,
                                                    defaultOptions, &timedout);
    }
#else
    NSError* error = nil;
    id<MTLLibrary> compiledLibrary = [gpu->device() newLibraryWithSource: mtlCode
                                                                 options: defaultOptions
                                                                   error: &error];
    if (error) {
        SkDebugf("Error compiling MSL shader: %s\n%s\n",
                 code.c_str(),
                 [[error localizedDescription] cStringUsingEncoding: NSASCIIStringEncoding]);
        return nil;
    }
#endif
    return compiledLibrary;
}

id<MTLLibrary> GrMtlNewLibraryWithSource(id<MTLDevice> device, NSString* mslCode,
                                         MTLCompileOptions* options, bool* timedout) {
    dispatch_semaphore_t compilerSemaphore = dispatch_semaphore_create(0);

    __block dispatch_semaphore_t semaphore = compilerSemaphore;
    __block id<MTLLibrary> compiledLibrary;
    [device newLibraryWithSource: mslCode
                         options: options
               completionHandler:
        ^(id<MTLLibrary> library, NSError* error) {
            if (error) {
                SkDebugf("Error compiling MSL shader: %s\n%s\n",
                    mslCode,
                    [[error localizedDescription] cStringUsingEncoding: NSASCIIStringEncoding]);
            }
            compiledLibrary = library;
            dispatch_semaphore_signal(semaphore);
        }
    ];

    // Wait 100 ms for the compiler
    if (dispatch_semaphore_wait(compilerSemaphore, dispatch_time(DISPATCH_TIME_NOW, 100000))) {
        SkDebugf("Timeout compiling MSL shader\n");
        *timedout = true;
        return nil;
    }

    *timedout = false;
    return compiledLibrary;
}

id<MTLRenderPipelineState> GrMtlNewRenderPipelineStateWithDescriptor(
        id<MTLDevice> device, MTLRenderPipelineDescriptor* pipelineDescriptor, bool* timedout) {
    dispatch_semaphore_t pipelineSemaphore = dispatch_semaphore_create(0);

    __block dispatch_semaphore_t semaphore = pipelineSemaphore;
    __block id<MTLRenderPipelineState> pipelineState;
    [device newRenderPipelineStateWithDescriptor: pipelineDescriptor
                               completionHandler:
        ^(id<MTLRenderPipelineState> state, NSError* error) {
            if (error) {
                SkDebugf("Error creating pipeline: %s\n",
                    [[error localizedDescription] cStringUsingEncoding: NSASCIIStringEncoding]);
            }
            pipelineState = state;
            dispatch_semaphore_signal(semaphore);
        }
     ];

    // Wait 500 ms for pipeline creation
    if (dispatch_semaphore_wait(pipelineSemaphore, dispatch_time(DISPATCH_TIME_NOW, 500000))) {
        SkDebugf("Timeout creating pipeline.\n");
        *timedout = true;
        return nil;
    }

    *timedout = false;
    return pipelineState;
}

id<MTLTexture> GrGetMTLTextureFromSurface(GrSurface* surface) {
    id<MTLTexture> mtlTexture = nil;

    GrMtlRenderTarget* renderTarget = static_cast<GrMtlRenderTarget*>(surface->asRenderTarget());
    GrMtlTexture* texture;
    if (renderTarget) {
        // We should not be using this for multisampled rendertargets
        if (renderTarget->numSamples() > 1) {
            SkASSERT(false);
            return nil;
        }
        mtlTexture = renderTarget->mtlColorTexture();
    } else {
        texture = static_cast<GrMtlTexture*>(surface->asTexture());
        if (texture) {
            mtlTexture = texture->mtlTexture();
        }
    }
    return mtlTexture;
}


//////////////////////////////////////////////////////////////////////////////
// CPP Utils

GrMTLPixelFormat GrGetMTLPixelFormatFromMtlTextureInfo(const GrMtlTextureInfo& info) {
    id<MTLTexture> mtlTexture = GrGetMTLTexture(info.fTexture.get());
    return static_cast<GrMTLPixelFormat>(mtlTexture.pixelFormat);
}

size_t GrMtlBytesPerFormat(MTLPixelFormat format) {
    switch (format) {
        case MTLPixelFormatA8Unorm:
        case MTLPixelFormatR8Unorm:
            return 1;

#ifdef SK_BUILD_FOR_IOS
        case MTLPixelFormatB5G6R5Unorm:
        case MTLPixelFormatABGR4Unorm:
#endif
        case MTLPixelFormatRG8Unorm:
        case MTLPixelFormatR16Float:
        case MTLPixelFormatR16Unorm:
            return 2;

        case MTLPixelFormatRGBA8Unorm:
        case MTLPixelFormatBGRA8Unorm:
        case MTLPixelFormatRGBA8Unorm_sRGB:
        case MTLPixelFormatRGB10A2Unorm:
        case MTLPixelFormatRG16Unorm:
        case MTLPixelFormatRG16Float:
            return 4;

        case MTLPixelFormatRGBA16Float:
        case MTLPixelFormatRGBA16Unorm:
            return 8;

        case MTLPixelFormatRGBA32Float:
            return 16;

#ifdef SK_BUILD_FOR_IOS
        case  MTLPixelFormatETC2_RGB8:
            return 0;
#endif
        default:
            SK_ABORT("Invalid Mtl format");
            return 0;
    }

    SK_ABORT("Invalid Mtl format");
    return 0;
}

#if GR_TEST_UTILS
const char* GrMtlFormatToStr(GrMTLPixelFormat mtlFormat) {
    switch (mtlFormat) {
        case MTLPixelFormatInvalid:         return "Invalid";
        case MTLPixelFormatRGBA8Unorm:      return "RGBA8Unorm";
        case MTLPixelFormatR8Unorm:         return "R8Unorm";
        case MTLPixelFormatA8Unorm:         return "A8Unorm";
        case MTLPixelFormatBGRA8Unorm:      return "BGRA8Unorm";
#ifdef SK_BUILD_FOR_IOS
        case MTLPixelFormatB5G6R5Unorm:     return "B5G6R5Unorm";
#endif
        case MTLPixelFormatRGBA16Float:     return "RGBA16Float";
        case MTLPixelFormatR16Float:        return "R16Float";
        case MTLPixelFormatRG8Unorm:        return "RG8Unorm";
        case MTLPixelFormatRGB10A2Unorm:    return "RGB10A2Unorm";
#ifdef SK_BUILD_FOR_IOS
        case MTLPixelFormatABGR4Unorm:      return "ABGR4Unorm";
#endif
        case MTLPixelFormatRGBA32Float:     return "RGBA32Float";
        case MTLPixelFormatRGBA8Unorm_sRGB: return "RGBA8Unorm_sRGB";
        case MTLPixelFormatR16Unorm:        return "R16Unorm";
        case MTLPixelFormatRG16Unorm:       return "RG16Unorm";
#ifdef SK_BUILD_FOR_IOS
        case MTLPixelFormatETC2_RGB8:      return "ETC2_RGB8";
#endif
        case MTLPixelFormatRGBA16Unorm:    return "RGBA16Unorm";
        case MTLPixelFormatRG16Float:      return "RG16Float";

        default:                           return "Unknown";
    }
}

#endif



