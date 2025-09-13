/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/mtl/GrMtlUtil.h"

#import <Metal/Metal.h>

#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/private/base/SkMutex.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/ganesh/GrSurface.h"
#include "src/gpu/ganesh/mtl/GrMtlGpu.h"
#include "src/gpu/ganesh/mtl/GrMtlRenderTarget.h"
#include "src/gpu/ganesh/mtl/GrMtlTexture.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GR_NORETAIN_BEGIN

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
    if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
        texDesc.usage = mtlTexture.usage;
    }
    return texDesc;
}

id<MTLLibrary> GrCompileMtlShaderLibrary(const GrMtlGpu* gpu,
                                         const SkSL::NativeShader& msl,
                                         GrContextOptions::ShaderErrorHandler* errorHandler) {
    TRACE_EVENT0("skia.shaders", "driver_compile_shader");
    NSString* nsSource = [[NSString alloc] initWithBytesNoCopy:const_cast<char*>(msl.fText.c_str())
                                                        length:msl.fText.size()
                                                      encoding:NSUTF8StringEncoding
                                                  freeWhenDone:NO];
    if (!nsSource) {
        return nil;
    }
    MTLCompileOptions* options = [[MTLCompileOptions alloc] init];
    // array<> is supported in MSL 2.0 on MacOS 10.13+ and iOS 11+,
    // and in MSL 1.2 on iOS 10+ (but not MacOS).
    options.languageVersion = MTLLanguageVersion2_0;

#if (TARGET_OS_OSX && __MAC_OS_X_VERSION_MIN_REQUIRED >= 150000) || (TARGET_OS_IOS && __IPHONE_OS_VERSION_MIN_REQUIRED >= 180000)
        options.mathMode = MTLMathModeFast;
#else
        options.fastMathEnabled = YES;
#endif

    NSError* error = nil;
    id<MTLLibrary> compiledLibrary;
    if (@available(macOS 10.15, *)) {
        compiledLibrary = [gpu->device() newLibraryWithSource:(NSString* _Nonnull)nsSource
                                                      options:options
                                                        error:&error];
    } else {
        compiledLibrary = GrMtlNewLibraryWithSource(gpu->device(), (NSString* _Nonnull)nsSource,
                                                    options, &error);
    }
    if (!compiledLibrary) {
        errorHandler->compileError(
                msl.fText.c_str(), error.debugDescription.UTF8String, /*shaderWasCached=*/false);
        return nil;
    }

    return compiledLibrary;
}

void GrPrecompileMtlShaderLibrary(const GrMtlGpu* gpu, const SkSL::NativeShader& msl) {
    NSString* nsSource = [[NSString alloc] initWithBytesNoCopy:const_cast<char*>(msl.fText.c_str())
                                                        length:msl.fText.size()
                                                      encoding:NSUTF8StringEncoding
                                                  freeWhenDone:NO];
    if (!nsSource) {
        return;
    }
    // Do nothing after completion for now.
    // TODO: cache the result somewhere so we can use it later.
    MTLNewLibraryCompletionHandler completionHandler = ^(id<MTLLibrary> library, NSError* error) {};
    [gpu->device() newLibraryWithSource:(NSString* _Nonnull)nsSource
                                options:nil
                      completionHandler:completionHandler];
}

// Wrapper to get atomic assignment for compiles and pipeline creation
class MtlCompileResult : public SkRefCnt {
public:
    MtlCompileResult() : fCompiledObject(nil), fError(nil) {}
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
            ^(id<MTLLibrary> library, NSError* compileError) {
                compileResult->set(library, compileError);
                dispatch_semaphore_signal(semaphore);
                compileResult->unref();
            };

    [device newLibraryWithSource: mslCode
                         options: options
               completionHandler: completionHandler];

    // Wait 1 second for the compiler
    constexpr auto kTimeoutNS = 1000000000UL;
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
            ^(id<MTLRenderPipelineState> state, NSError* compileError) {
                compileResult->set(state, compileError);
                dispatch_semaphore_signal(semaphore);
                compileResult->unref();
            };

    [device newRenderPipelineStateWithDescriptor: pipelineDescriptor
                               completionHandler: completionHandler];

    // Wait 1 second for pipeline creation
    constexpr auto kTimeoutNS = 1000000000UL;
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
        // We should not be using this for multisampled rendertargets with a separate resolve
        // texture.
        if (renderTarget->resolveAttachment()) {
            SkASSERT(renderTarget->numSamples() > 1);
            SkASSERT(false);
            return nil;
        }
        mtlTexture = renderTarget->colorMTLTexture();
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
    id<MTLTexture> GR_NORETAIN mtlTexture = GrGetMTLTexture(info.fTexture.get());
    return static_cast<GrMTLPixelFormat>(mtlTexture.pixelFormat);
}

GrColorFormatDesc GrMtlFormatDesc(MTLPixelFormat mtlFormat) {
    switch (mtlFormat) {
        case MTLPixelFormatRGBA8Unorm:
            return GrColorFormatDesc::MakeRGBA(8, GrColorTypeEncoding::kUnorm);
        case MTLPixelFormatR8Unorm:
            return GrColorFormatDesc::MakeR(8, GrColorTypeEncoding::kUnorm);
        case MTLPixelFormatA8Unorm:
            return GrColorFormatDesc::MakeAlpha(8, GrColorTypeEncoding::kUnorm);
        case MTLPixelFormatBGRA8Unorm:
            return GrColorFormatDesc::MakeRGBA(8, GrColorTypeEncoding::kUnorm);
        case MTLPixelFormatB5G6R5Unorm:
            return GrColorFormatDesc::MakeRGB(5, 6, 5, GrColorTypeEncoding::kUnorm);
        case MTLPixelFormatRGBA16Float:
            return GrColorFormatDesc::MakeRGBA(16, GrColorTypeEncoding::kFloat);
        case MTLPixelFormatR16Float:
            return GrColorFormatDesc::MakeR(16, GrColorTypeEncoding::kFloat);
        case MTLPixelFormatRG8Unorm:
            return GrColorFormatDesc::MakeRG(8, GrColorTypeEncoding::kUnorm);
        case MTLPixelFormatRGB10A2Unorm:
            return GrColorFormatDesc::MakeRGBA(10, 2, GrColorTypeEncoding::kUnorm);
        case MTLPixelFormatBGR10A2Unorm:
            return GrColorFormatDesc::MakeRGBA(10, 2, GrColorTypeEncoding::kUnorm);
        case MTLPixelFormatABGR4Unorm:
            return GrColorFormatDesc::MakeRGBA(4, GrColorTypeEncoding::kUnorm);
        case MTLPixelFormatRGBA8Unorm_sRGB:
            return GrColorFormatDesc::MakeRGBA(8, GrColorTypeEncoding::kSRGBUnorm);
        case MTLPixelFormatR16Unorm:
            return GrColorFormatDesc::MakeR(16, GrColorTypeEncoding::kUnorm);
        case MTLPixelFormatRG16Unorm:
            return GrColorFormatDesc::MakeRG(16, GrColorTypeEncoding::kUnorm);
        case MTLPixelFormatRGBA16Unorm:
            return GrColorFormatDesc::MakeRGBA(16, GrColorTypeEncoding::kUnorm);
        case MTLPixelFormatRG16Float:
            return GrColorFormatDesc::MakeRG(16, GrColorTypeEncoding::kFloat);

        // Compressed texture formats are not expected to have a description.
        case MTLPixelFormatETC2_RGB8: return GrColorFormatDesc::MakeInvalid();
#ifdef SK_BUILD_FOR_MAC
        case MTLPixelFormatBC1_RGBA:  return GrColorFormatDesc::MakeInvalid();
#endif

        // This type only describes color channels.
        case MTLPixelFormatStencil8: return GrColorFormatDesc::MakeInvalid();

        default:
            return GrColorFormatDesc::MakeInvalid();
    }
}

SkTextureCompressionType GrMtlFormatToCompressionType(MTLPixelFormat format) {
    switch (format) {
        case MTLPixelFormatETC2_RGB8: return SkTextureCompressionType::kETC2_RGB8_UNORM;
#ifdef SK_BUILD_FOR_MAC
        case MTLPixelFormatBC1_RGBA:  return SkTextureCompressionType::kBC1_RGBA8_UNORM;
#endif
        default:                      return SkTextureCompressionType::kNone;
    }

    SkUNREACHABLE;
}

int GrMtlTextureInfoSampleCount(const GrMtlTextureInfo& info) {
    id<MTLTexture> texture = GrGetMTLTexture(info.fTexture.get());
    if (!texture) {
        return 0;
    }
    return texture.sampleCount;
}

int GrMtlFormatStencilBits(MTLPixelFormat format) {
    switch (format) {
        case MTLPixelFormatStencil8:
            return 8;
        default:
            return 0;
    }
}

#if defined(SK_DEBUG) || defined(GPU_TEST_UTILS)
bool GrMtlFormatIsBGRA8(GrMTLPixelFormat mtlFormat) {
    return mtlFormat == MTLPixelFormatBGRA8Unorm;
}
#endif

GR_NORETAIN_END
