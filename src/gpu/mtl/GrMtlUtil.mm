/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlUtil.h"

#include "include/gpu/GrSurface.h"
#include "include/private/GrTypesPriv.h"
#include "include/private/SkMutex.h"
#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlRenderTarget.h"
#include "src/gpu/mtl/GrMtlTexture.h"
#include "src/sksl/SkSLCompiler.h"

#import <Metal/Metal.h>

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

#define PRINT_MSL 0 // print out the MSL code generated

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
#if defined(SK_BUILD_FOR_MAC)
    id<MTLLibrary> compiledLibrary = GrMtlNewLibraryWithSource(gpu->device(), mtlCode,
                                                               defaultOptions);
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

@interface AtomicID : NSObject
- (instancetype) initWithValue: (id)value;
- (void)set: (id)value;
- (id)get;
@end
@implementation AtomicID {
    SkMutex fMutex;
    id fValue;
}
- (instancetype)initWithValue: (id)value {
    self = [super init];
    if (self) {
        fValue = value;
    }

    return self;
}
- (void)set: (id)value {
    fMutex.acquire();
    fValue = value;
    fMutex.release();
}
- (id)get {
    id returnVal;
    fMutex.acquire();
    returnVal = fValue;
    fMutex.release();
    return returnVal;
}
@end

id<MTLLibrary> GrMtlNewLibraryWithSource(id<MTLDevice> device, NSString* mslCode,
                                         MTLCompileOptions* options) {
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);

    __block AtomicID* atomicID = [[AtomicID alloc] initWithValue:nil];
    [device newLibraryWithSource: mslCode
                         options: options
               completionHandler:
        ^(id<MTLLibrary> library, NSError* error) {
            [atomicID set:library];
            dispatch_semaphore_signal(semaphore);
            if (error) {
                SkDebugf("Error compiling MSL shader: %s\n%s\n",
                    mslCode,
                    [[error localizedDescription] cStringUsingEncoding: NSASCIIStringEncoding]);
            }
        }
    ];

    // Wait 1000 ms for the compiler
    if (dispatch_semaphore_wait(semaphore, dispatch_time(DISPATCH_TIME_NOW, 1000000UL))) {
        SkDebugf("Timeout compiling MSL shader\n");
        return nil;
    }

    id<MTLLibrary> compiledLibrary = (id<MTLLibrary>)[atomicID get];
    return compiledLibrary;
}

id<MTLRenderPipelineState> GrMtlNewRenderPipelineStateWithDescriptor(
        id<MTLDevice> device, MTLRenderPipelineDescriptor* pipelineDescriptor) {
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);

    __block AtomicID* atomicID = [[AtomicID alloc] initWithValue:nil];
    [device newRenderPipelineStateWithDescriptor: pipelineDescriptor
                               completionHandler:
        ^(id<MTLRenderPipelineState> state, NSError* error) {
            [atomicID set:state];
            dispatch_semaphore_signal(semaphore);
            if (error) {
                SkDebugf("Error creating pipeline: %s\n",
                    [[error localizedDescription] cStringUsingEncoding: NSASCIIStringEncoding]);
            }
        }
     ];

    // Wait 1000 ms for pipeline creation
    if (dispatch_semaphore_wait(semaphore, dispatch_time(DISPATCH_TIME_NOW, 1000000UL))) {
        SkDebugf("Timeout creating pipeline.\n");
        return nil;
    }

    id<MTLRenderPipelineState> pipelineState = (id<MTLRenderPipelineState>)[atomicID get];
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
#else
        case MTLPixelFormatBC1_RGBA:
            return true;
#endif
        default:
            return false;
    }
}

SkImage::CompressionType GrMtlFormatToCompressionType(MTLPixelFormat mtlFormat) {
    switch (mtlFormat) {
#ifdef SK_BUILD_FOR_IOS
        case MTLPixelFormatETC2_RGB8: return SkImage::CompressionType::kETC2_RGB8_UNORM;
#else
        case MTLPixelFormatBC1_RGBA:  return SkImage::CompressionType::kBC1_RGBA8_UNORM;
#endif
        default:                      return SkImage::CompressionType::kNone;
    }

    SkUNREACHABLE;
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
        case MTLPixelFormatETC2_RGB8:       return "ETC2_RGB8";
#else
        case MTLPixelFormatBC1_RGBA:        return "BC1_RGBA";
#endif
        case MTLPixelFormatRGBA16Unorm:     return "RGBA16Unorm";
        case MTLPixelFormatRG16Float:       return "RG16Float";

        default:                            return "Unknown";
    }
}

#endif



