/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlUtil.h"

#include "include/gpu/GrBackendSurface.h"
#include "include/private/GrTypesPriv.h"
#include "include/private/SkMutex.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GrSurface.h"
#include "src/gpu/mtl/GrMtlGpu.h"
#include "src/gpu/mtl/GrMtlRenderTarget.h"
#include "src/gpu/mtl/GrMtlTexture.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/utils/SkShaderUtils.h"

#import <Metal/Metal.h>
#ifdef SK_BUILD_FOR_IOS
#import <UIKit/UIApplication.h>
#endif

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
    if (@available(macOS 10.11, iOS 9.0, *)) {
        texDesc.usage = mtlTexture.usage;
    }
    return texDesc;
}

// Print the source code for all shaders generated.
static const bool gPrintSKSL = false;
static const bool gPrintMSL = false;

bool GrSkSLToMSL(const GrMtlGpu* gpu,
                 const SkSL::String& sksl,
                 SkSL::ProgramKind programKind,
                 const SkSL::Program::Settings& settings,
                 SkSL::String* msl,
                 SkSL::Program::Inputs* outInputs,
                 GrContextOptions::ShaderErrorHandler* errorHandler) {
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

id<MTLLibrary> GrCompileMtlShaderLibrary(const GrMtlGpu* gpu,
                                         const SkSL::String& msl,
                                         GrContextOptions::ShaderErrorHandler* errorHandler) {
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
    if (gpu->caps()->shaderCaps()->canUseFastMath()) {
        options.fastMathEnabled = YES;
    }

    NSError* error = nil;
#if defined(SK_BUILD_FOR_MAC)
    id<MTLLibrary> compiledLibrary = GrMtlNewLibraryWithSource(gpu->device(), nsSource,
                                                               options, &error);
#else
    id<MTLLibrary> compiledLibrary = [gpu->device() newLibraryWithSource:nsSource
                                                                 options:options
                                                                   error:&error];
#endif
    if (!compiledLibrary) {
        errorHandler->compileError(msl.c_str(), error.debugDescription.UTF8String);
        return nil;
    }

    return compiledLibrary;
}

void GrPrecompileMtlShaderLibrary(const GrMtlGpu* gpu,
                                  const SkSL::String& msl) {
    auto nsSource = [[NSString alloc] initWithBytesNoCopy:const_cast<char*>(msl.c_str())
                                                   length:msl.size()
                                                 encoding:NSUTF8StringEncoding
                                             freeWhenDone:NO];
    // Do nothing after completion for now.
    // TODO: cache the result somewhere so we can use it later.
    MTLNewLibraryCompletionHandler completionHandler =
            ^(id<MTLLibrary> library, NSError* error) {};
    [gpu->device() newLibraryWithSource:nsSource
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

uint32_t GrMtlFormatChannels(GrMTLPixelFormat mtlFormat) {
    switch (mtlFormat) {
        case MTLPixelFormatRGBA8Unorm:      return kRGBA_SkColorChannelFlags;
        case MTLPixelFormatR8Unorm:         return kRed_SkColorChannelFlag;
        case MTLPixelFormatA8Unorm:         return kAlpha_SkColorChannelFlag;
        case MTLPixelFormatBGRA8Unorm:      return kRGBA_SkColorChannelFlags;
#if defined(SK_BUILD_FOR_IOS) && !TARGET_OS_SIMULATOR
        case MTLPixelFormatB5G6R5Unorm:     return kRGB_SkColorChannelFlags;
#endif
        case MTLPixelFormatRGBA16Float:     return kRGBA_SkColorChannelFlags;
        case MTLPixelFormatR16Float:        return kRed_SkColorChannelFlag;
        case MTLPixelFormatRG8Unorm:        return kRG_SkColorChannelFlags;
        case MTLPixelFormatRGB10A2Unorm:    return kRGBA_SkColorChannelFlags;
#ifdef SK_BUILD_FOR_MAC
        case MTLPixelFormatBGR10A2Unorm:    return kRGBA_SkColorChannelFlags;
#endif
#if defined(SK_BUILD_FOR_IOS) && !TARGET_OS_SIMULATOR
        case MTLPixelFormatABGR4Unorm:      return kRGBA_SkColorChannelFlags;
#endif
        case MTLPixelFormatRGBA8Unorm_sRGB: return kRGBA_SkColorChannelFlags;
        case MTLPixelFormatR16Unorm:        return kRed_SkColorChannelFlag;
        case MTLPixelFormatRG16Unorm:       return kRG_SkColorChannelFlags;
#ifdef SK_BUILD_FOR_IOS
        case MTLPixelFormatETC2_RGB8:       return kRGB_SkColorChannelFlags;
#else
        case MTLPixelFormatBC1_RGBA:        return kRGBA_SkColorChannelFlags;
#endif
        case MTLPixelFormatRGBA16Unorm:     return kRGBA_SkColorChannelFlags;
        case MTLPixelFormatRG16Float:       return kRG_SkColorChannelFlags;
        case MTLPixelFormatStencil8:        return 0;

        default:                            return 0;
    }
}

GrColorFormatDesc GrMtlFormatDesc(GrMTLPixelFormat mtlFormat)  {
    switch (mtlFormat) {
        case MTLPixelFormatRGBA8Unorm:
            return GrColorFormatDesc::MakeRGBA(8, GrColorTypeEncoding::kUnorm);
        case MTLPixelFormatR8Unorm:
            return GrColorFormatDesc::MakeR(8, GrColorTypeEncoding::kUnorm);
        case MTLPixelFormatA8Unorm:
            return GrColorFormatDesc::MakeAlpha(8, GrColorTypeEncoding::kUnorm);
        case MTLPixelFormatBGRA8Unorm:
            return GrColorFormatDesc::MakeRGBA(8, GrColorTypeEncoding::kUnorm);
#if defined(SK_BUILD_FOR_IOS) && !TARGET_OS_SIMULATOR
        case MTLPixelFormatB5G6R5Unorm:
            return GrColorFormatDesc::MakeRGB(5, 6, 5, GrColorTypeEncoding::kUnorm);
#endif
        case MTLPixelFormatRGBA16Float:
            return GrColorFormatDesc::MakeRGBA(16, GrColorTypeEncoding::kFloat);
        case MTLPixelFormatR16Float:
            return GrColorFormatDesc::MakeR(16, GrColorTypeEncoding::kFloat);
        case MTLPixelFormatRG8Unorm:
            return GrColorFormatDesc::MakeRG(8, GrColorTypeEncoding::kUnorm);
        case MTLPixelFormatRGB10A2Unorm:
            return GrColorFormatDesc::MakeRGBA(10, 2, GrColorTypeEncoding::kUnorm);
#ifdef SK_BUILD_FOR_MAC
        case MTLPixelFormatBGR10A2Unorm:
            return GrColorFormatDesc::MakeRGBA(10, 2, GrColorTypeEncoding::kUnorm);
#endif
#if defined(SK_BUILD_FOR_IOS) && !TARGET_OS_SIMULATOR
        case MTLPixelFormatABGR4Unorm:
            return GrColorFormatDesc::MakeRGBA(4, GrColorTypeEncoding::kUnorm);
#endif
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
#ifdef SK_BUILD_FOR_IOS
        case MTLPixelFormatETC2_RGB8: return GrColorFormatDesc::MakeInvalid();
#else
        case MTLPixelFormatBC1_RGBA:  return GrColorFormatDesc::MakeInvalid();
#endif

        // This type only describes color channels.
        case MTLPixelFormatStencil8: return GrColorFormatDesc::MakeInvalid();

        default:
            return GrColorFormatDesc::MakeInvalid();
    }
}

SkImage::CompressionType GrMtlBackendFormatToCompressionType(const GrBackendFormat& format) {
    MTLPixelFormat mtlFormat = GrBackendFormatAsMTLPixelFormat(format);
    return GrMtlFormatToCompressionType(mtlFormat);
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

int GrMtlTextureInfoSampleCount(const GrMtlTextureInfo& info) {
    id<MTLTexture> texture = GrGetMTLTexture(info.fTexture.get());
    if (!texture) {
        return 0;
    }
    return texture.sampleCount;
}

size_t GrMtlBackendFormatBytesPerBlock(const GrBackendFormat& format) {
    MTLPixelFormat mtlFormat = GrBackendFormatAsMTLPixelFormat(format);
    return GrMtlFormatBytesPerBlock(mtlFormat);
}

size_t GrMtlFormatBytesPerBlock(MTLPixelFormat mtlFormat) {
    switch (mtlFormat) {
        case MTLPixelFormatInvalid:         return 0;
        case MTLPixelFormatRGBA8Unorm:      return 4;
        case MTLPixelFormatR8Unorm:         return 1;
        case MTLPixelFormatA8Unorm:         return 1;
        case MTLPixelFormatBGRA8Unorm:      return 4;
#ifdef SK_BUILD_FOR_IOS
        case MTLPixelFormatB5G6R5Unorm:     return 2;
#endif
        case MTLPixelFormatRGBA16Float:     return 8;
        case MTLPixelFormatR16Float:        return 2;
        case MTLPixelFormatRG8Unorm:        return 2;
        case MTLPixelFormatRGB10A2Unorm:    return 4;
#ifdef SK_BUILD_FOR_MAC
        case MTLPixelFormatBGR10A2Unorm:    return 4;
#endif
#ifdef SK_BUILD_FOR_IOS
        case MTLPixelFormatABGR4Unorm:      return 2;
#endif
        case MTLPixelFormatRGBA8Unorm_sRGB: return 4;
        case MTLPixelFormatR16Unorm:        return 2;
        case MTLPixelFormatRG16Unorm:       return 4;
#ifdef SK_BUILD_FOR_IOS
        case MTLPixelFormatETC2_RGB8:       return 8;
#else
        case MTLPixelFormatBC1_RGBA:        return 8;
#endif
        case MTLPixelFormatRGBA16Unorm:     return 8;
        case MTLPixelFormatRG16Float:       return 4;
        case MTLPixelFormatStencil8:        return 1;

        default:                            return 0;
    }
}

int GrMtlBackendFormatStencilBits(const GrBackendFormat& format) {
    MTLPixelFormat mtlFormat = GrBackendFormatAsMTLPixelFormat(format);
    return GrMtlFormatStencilBits(mtlFormat);
}

int GrMtlFormatStencilBits(MTLPixelFormat mtlFormat) {
    switch(mtlFormat) {
     case MTLPixelFormatStencil8:
         return 8;
     default:
         return 0;
    }
}

#ifdef SK_BUILD_FOR_IOS
bool GrMtlIsAppInBackground() {
    return [NSThread isMainThread] &&
           ([UIApplication sharedApplication].applicationState == UIApplicationStateBackground);
}
#endif

#if defined(SK_DEBUG) || GR_TEST_UTILS
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
        case MTLPixelFormatStencil8:        return "Stencil8";

        default:                            return "Unknown";
    }
}

#endif

GR_NORETAIN_END
