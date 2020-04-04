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
        case kAlpha_16_GrPixelConfig:
            *format = MTLPixelFormatR16Unorm;
            return true;
        case kRG_1616_GrPixelConfig:
            *format = MTLPixelFormatRG16Unorm;
            return true;
        case kRGBA_16161616_GrPixelConfig:
            *format = MTLPixelFormatRGBA16Unorm;
            return true;
        case kRG_half_GrPixelConfig:
            *format = MTLPixelFormatRG16Float;
            return true;
    }
    SK_ABORT("Unexpected config");
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
    if (@available(macOS 10.11, iOS 9.0, *)) {
        texDesc.usage = mtlTexture.usage;
    }
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

id<MTLLibrary> GrGenerateMtlShaderLibrary(const GrMtlGpu* gpu,
                                          const SkSL::String& shaderString,
                                          SkSL::Program::Kind kind,
                                          const SkSL::Program::Settings& settings,
                                          SkSL::String* mslShader,
                                          SkSL::Program::Inputs* outInputs) {
    std::unique_ptr<SkSL::Program> program =
            gpu->shaderCompiler()->convertProgram(kind,
                                                  shaderString,
                                                  settings);

    if (!program) {
        SkDebugf("SkSL error:\n%s\n", gpu->shaderCompiler()->errorText().c_str());
        SkASSERT(false);
        return nil;
    }

    *outInputs = program->fInputs;
    if (!gpu->shaderCompiler()->toMetal(*program, mslShader)) {
        SkDebugf("%s\n", gpu->shaderCompiler()->errorText().c_str());
        SkASSERT(false);
        return nil;
    }

    return GrCompileMtlShaderLibrary(gpu, *mslShader);
}

id<MTLLibrary> GrCompileMtlShaderLibrary(const GrMtlGpu* gpu,
                                         const SkSL::String& shaderString) {
    NSString* mtlCode = [[NSString alloc] initWithCString: shaderString.c_str()
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
                 shaderString.c_str(),
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

bool GrMtlFormatIsCompressed(MTLPixelFormat mtlFormat) {
    switch (mtlFormat) {
#ifdef SK_BUILD_FOR_IOS
        case MTLPixelFormatETC2_RGB8:
            return true;
#endif
        default:
            return false;
    }
}

bool GrMtlFormatToCompressionType(MTLPixelFormat mtlFormat,
                                  SkImage::CompressionType* compressionType) {
    switch (mtlFormat) {
#ifdef SK_BUILD_FOR_IOS
        case MTLPixelFormatETC2_RGB8:
            *compressionType = SkImage::kETC1_CompressionType;
            return true;
#endif
        default:
            return false;
    }
}

#if GR_TEST_UTILS
bool GrMtlFormatIsBGRA(GrMTLPixelFormat mtlFormat) {
    return mtlFormat == MTLPixelFormatBGRA8Unorm;
}

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



