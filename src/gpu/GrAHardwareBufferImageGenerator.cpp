/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26
#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES


#include "GrAHardwareBufferImageGenerator.h"

#include <android/hardware_buffer.h>

#include "GrAHardwareBufferUtils.h"
#include "GrBackendSurface.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrProxyProvider.h"
#include "GrResourceCache.h"
#include "GrResourceProvider.h"
#include "GrResourceProviderPriv.h"
#include "GrTexture.h"
#include "GrTextureProxy.h"
#include "SkMessageBus.h"
#include "gl/GrGLDefines.h"
#include "gl/GrGLTypes.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>

#ifdef SK_VULKAN
#include "vk/GrVkExtensions.h"
#include "vk/GrVkGpu.h"
#endif

#define PROT_CONTENT_EXT_STR "EGL_EXT_protected_content"
#define EGL_PROTECTED_CONTENT_EXT 0x32C0

std::unique_ptr<SkImageGenerator> GrAHardwareBufferImageGenerator::Make(
        AHardwareBuffer* graphicBuffer, SkAlphaType alphaType, sk_sp<SkColorSpace> colorSpace,
        GrSurfaceOrigin surfaceOrigin) {
    AHardwareBuffer_Desc bufferDesc;
    AHardwareBuffer_describe(graphicBuffer, &bufferDesc);

    SkColorType colorType =
            GrAHardwareBufferUtils::GetSkColorTypeFromBufferFormat(bufferDesc.format);
    SkImageInfo info = SkImageInfo::Make(bufferDesc.width, bufferDesc.height, colorType,
                                         alphaType, std::move(colorSpace));

    bool createProtectedImage = 0 != (bufferDesc.usage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT);
    return std::unique_ptr<SkImageGenerator>(new GrAHardwareBufferImageGenerator(
            info, graphicBuffer, alphaType, createProtectedImage,
            bufferDesc.format, surfaceOrigin));
}

GrAHardwareBufferImageGenerator::GrAHardwareBufferImageGenerator(const SkImageInfo& info,
        AHardwareBuffer* hardwareBuffer, SkAlphaType alphaType, bool isProtectedContent,
        uint32_t bufferFormat, GrSurfaceOrigin surfaceOrigin)
    : INHERITED(info)
    , fHardwareBuffer(hardwareBuffer)
    , fBufferFormat(bufferFormat)
    , fIsProtectedContent(isProtectedContent)
    , fSurfaceOrigin(surfaceOrigin) {
    AHardwareBuffer_acquire(fHardwareBuffer);
}

GrAHardwareBufferImageGenerator::~GrAHardwareBufferImageGenerator() {
    AHardwareBuffer_release(fHardwareBuffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<GrTextureProxy> GrAHardwareBufferImageGenerator::makeProxy(GrContext* context) {
    if (context->abandoned()) {
        return nullptr;
    }

    GrBackendFormat backendFormat = GrAHardwareBufferUtils::GetBackendFormat(context,
                                                                             fHardwareBuffer,
                                                                             fBufferFormat,
                                                                             false);

    GrPixelConfig pixelConfig = context->priv().caps()->getConfigFromBackendFormat(
            backendFormat, this->getInfo().colorType());

    if (pixelConfig == kUnknown_GrPixelConfig) {
        return nullptr;
    }

    int width = this->getInfo().width();
    int height = this->getInfo().height();

    GrSurfaceDesc desc;
    desc.fWidth = width;
    desc.fHeight = height;
    desc.fConfig = pixelConfig;

    GrTextureType textureType = GrTextureType::k2D;
    if (context->backend() == GrBackendApi::kOpenGL) {
        textureType = GrTextureType::kExternal;
    } else if (context->backend() == GrBackendApi::kVulkan) {
        const VkFormat* format = backendFormat.getVkFormat();
        SkASSERT(format);
        if (*format == VK_FORMAT_UNDEFINED) {
            textureType = GrTextureType::kExternal;
        }
    }

    auto proxyProvider = context->priv().proxyProvider();

    AHardwareBuffer* hardwareBuffer = fHardwareBuffer;
    AHardwareBuffer_acquire(hardwareBuffer);

    const bool isProtectedContent = fIsProtectedContent;

    sk_sp<GrTextureProxy> texProxy = proxyProvider->createLazyProxy(
            [context, hardwareBuffer, width, height, pixelConfig, isProtectedContent,
             backendFormat](GrResourceProvider* resourceProvider) {
                if (!resourceProvider) {
                    AHardwareBuffer_release(hardwareBuffer);
                    return sk_sp<GrTexture>();
                }

                GrAHardwareBufferUtils::DeleteImageProc deleteImageProc = nullptr;
                GrAHardwareBufferUtils::DeleteImageCtx deleteImageCtx = nullptr;

                GrBackendTexture backendTex =
                        GrAHardwareBufferUtils::MakeBackendTexture(context, hardwareBuffer,
                                                                   width, height,
                                                                   &deleteImageProc,
                                                                   &deleteImageCtx,
                                                                   isProtectedContent,
                                                                   backendFormat,
                                                                   false);
                if (!backendTex.isValid()) {
                    return sk_sp<GrTexture>();
                }
                SkASSERT(deleteImageProc && deleteImageCtx);

                backendTex.fConfig = pixelConfig;
                // We make this texture cacheable to avoid recreating a GrTexture every time this
                // is invoked. We know the owning SkIamge will send an invalidation message when the
                // image is destroyed, so the texture will be removed at that time.
                sk_sp<GrTexture> tex = resourceProvider->wrapBackendTexture(
                        backendTex, kBorrow_GrWrapOwnership, GrWrapCacheable::kYes, kRead_GrIOType);
                if (!tex) {
                    deleteImageProc(deleteImageCtx);
                    return sk_sp<GrTexture>();
                }

                if (deleteImageProc) {
                    sk_sp<GrReleaseProcHelper> releaseProcHelper(
                            new GrReleaseProcHelper(deleteImageProc, deleteImageCtx));
                    tex->setRelease(releaseProcHelper);
                }

                return tex;
            },
            backendFormat, desc, fSurfaceOrigin, GrMipMapped::kNo,
            GrInternalSurfaceFlags::kReadOnly, SkBackingFit::kExact, SkBudgeted::kNo);

    if (!texProxy) {
        AHardwareBuffer_release(hardwareBuffer);
    }
    return texProxy;
}

sk_sp<GrTextureProxy> GrAHardwareBufferImageGenerator::onGenerateTexture(
        GrContext* context, const SkImageInfo& info, const SkIPoint& origin, bool willNeedMipMaps) {
    sk_sp<GrTextureProxy> texProxy = this->makeProxy(context);
    if (!texProxy) {
        return nullptr;
    }

    if (0 == origin.fX && 0 == origin.fY &&
        info.width() == this->getInfo().width() && info.height() == this->getInfo().height()) {
        // If the caller wants the full texture we're done. The caller will handle making a copy for
        // mip maps if that is required.
        return texProxy;
    }
    // Otherwise, make a copy for the requested subset.
    SkIRect subset = SkIRect::MakeXYWH(origin.fX, origin.fY, info.width(), info.height());

    GrMipMapped mipMapped = willNeedMipMaps ? GrMipMapped::kYes : GrMipMapped::kNo;

    return GrSurfaceProxy::Copy(context, texProxy.get(), mipMapped, subset, SkBackingFit::kExact,
                                SkBudgeted::kYes);
}

bool GrAHardwareBufferImageGenerator::onIsValid(GrContext* context) const {
    if (nullptr == context) {
        return false; //CPU backend is not supported, because hardware buffer can be swizzled
    }
    return GrBackendApi::kOpenGL == context->backend() ||
           GrBackendApi::kVulkan == context->backend();
}

#endif //SK_BUILD_FOR_ANDROID_FRAMEWORK
