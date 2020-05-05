/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlUtil.h"

#include "include/private/GrTypesPriv.h"
#include "include/private/SkMutex.h"
#include "src/gpu/GrSurface.h"
#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlRenderTarget.h"
#include "src/gpu/mtl/GrMtlTexture.h"
#include "src/sksl/SkSLCompiler.h"

#import <Metal/Metal.h>

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

#define PRINT_MSL 0 // print out the MSL code generated

NSError* GrCreateMtlError(NSString* description, GrMtlErrorCode errorCode) {
    NSDictionary* userInfo = [NSDictionary dictionaryWithObject:description
                                                         forKey:NSLocalizedDescriptionKey];
    return [NSError errorWithDomain:@"org.skia.ganesh"
                               code:(NSInteger)errorCode
                           userInfo:userInfo];
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
    NSError* error = nil;
#if defined(SK_BUILD_FOR_MAC)
    id<MTLLibrary> compiledLibrary = GrMtlNewLibraryWithSource(gpu->device(), mtlCode,
                                                               defaultOptions, &error);
#else
    id<MTLLibrary> compiledLibrary = [gpu->device() newLibraryWithSource: mtlCode
                                                                 options: defaultOptions
                                                                   error: &error];
#endif
    if (!compiledLibrary) {
        SkDebugf("Error compiling MSL shader: %s\n%s\n",
                 shaderString.c_str(),
                 [[error localizedDescription] cStringUsingEncoding: NSASCIIStringEncoding]);
        return nil;
    }

    return compiledLibrary;
}

// Wrapper to get atomic assignment for compiles and pipeline creation
class MtlCompileResult : public SkRefCnt {
public:
    MtlCompileResult() : fCompiledObject(nil), fError(nil) {}
    ~MtlCompileResult() = default;
    void set(id compiledObject, NSError* error) {
        SkAutoMutexExclusive automutex(fMutex);
        fCompiledObject = compiledObject;
        fError = error;
    }
    std::pair<id, NSError*> get() {
        SkAutoMutexExclusive automutex(fMutex);
        return std::make_pair(fCompiledObject, fError);
    }
private:
    SkMutex fMutex;
    id fCompiledObject SK_GUARDED_BY(fMutex);
    NSError* fError SK_GUARDED_BY(fMutex);
};

id<MTLLibrary> GrMtlNewLibraryWithSource(id<MTLDevice> device, NSString* mslCode,
                                         MTLCompileOptions* options, NSError** error) {
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    sk_sp<MtlCompileResult> compileResult(new MtlCompileResult);
    // We have to increment the ref for the Obj-C block manually because it won't do it for us
    compileResult->ref();
    MTLNewLibraryCompletionHandler completionHandler =
            ^(id<MTLLibrary> library, NSError* error) {
                compileResult->set(library, error);
                dispatch_semaphore_signal(semaphore);
                compileResult->unref();
            };

    [device newLibraryWithSource: mslCode
                         options: options
               completionHandler: completionHandler];

    // Wait 100 ms for the compiler
    constexpr auto kTimeoutNS = 100000000UL;
    if (dispatch_semaphore_wait(semaphore, dispatch_time(DISPATCH_TIME_NOW, kTimeoutNS))) {
        if (error) {
            constexpr auto kTimeoutMS = kTimeoutNS/1000000UL;
            NSString* description =
                    [NSString stringWithFormat:@"Compilation took longer than %lu ms",
                                               kTimeoutMS];
            *error = GrCreateMtlError(description, GrMtlErrorCode::kTimeout);
        }
        return nil;
    }

    id<MTLLibrary> compiledLibrary;
    std::tie(compiledLibrary, *error) = compileResult->get();

    return compiledLibrary;
}

id<MTLRenderPipelineState> GrMtlNewRenderPipelineStateWithDescriptor(
        id<MTLDevice> device, MTLRenderPipelineDescriptor* pipelineDescriptor, NSError** error) {
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    sk_sp<MtlCompileResult> compileResult(new MtlCompileResult);
    // We have to increment the ref for the Obj-C block manually because it won't do it for us
    compileResult->ref();
    MTLNewRenderPipelineStateCompletionHandler completionHandler =
            ^(id<MTLRenderPipelineState> state, NSError* error) {
                compileResult->set(state, error);
                dispatch_semaphore_signal(semaphore);
                compileResult->unref();
            };

    [device newRenderPipelineStateWithDescriptor: pipelineDescriptor
                               completionHandler: completionHandler];

    // Wait 100 ms for pipeline creation
    constexpr auto kTimeoutNS = 100000000UL;
    if (dispatch_semaphore_wait(semaphore, dispatch_time(DISPATCH_TIME_NOW, kTimeoutNS))) {
        if (error) {
            constexpr auto kTimeoutMS = kTimeoutNS/1000000UL;
            NSString* description =
                    [NSString stringWithFormat:@"Pipeline creation took longer than %lu ms",
                                               kTimeoutMS];
            *error = GrCreateMtlError(description, GrMtlErrorCode::kTimeout);
        }
        return nil;
    }

    id<MTLRenderPipelineState> pipelineState;
    std::tie(pipelineState, *error) = compileResult->get();

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
bool GrMtlFormatIsBGRA8(GrMTLPixelFormat mtlFormat) {
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
#ifdef SK_BUILD_FOR_MAC
        case MTLPixelFormatBGR10A2Unorm:    return "BGR10A2Unorm";
#endif
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



