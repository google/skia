/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkTypes.h"

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK

#include "GrAndroidBufferImageGenerator.h"

#include "GrBackendSurface.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrResourceCache.h"
#include "GrResourceProvider.h"

#include "SkGr.h"
#include "gl/GrGLGpu.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "gl/GrGLDefines.h"
#include "gl/GrGLUtil.h"

#include "gl/GrGLInterface.h"
#include "gl/GrGLAssembleInterface.h"

class BufferCleanupHelper : public SkNVRefCnt<BufferCleanupHelper> {
public:
    BufferCleanupHelper(GrContext* context, GrEGLImage image, GrEGLDisplay display)
        : fContext(context)
        , fImage(image)
        , fDisplay(display) { }
    ~BufferCleanupHelper() {
        auto glInterface = ((GrGLGpu*)fContext->getGpu())->glContext().interface();
        if (glInterface) {
            GR_GL_CALL(glInterface, EGLDestroyImage(fDisplay, fImage));
        }
    }
private:
    GrContext* fContext; // should we call ref() to keep the context?
    GrEGLImage fImage;
    GrEGLDisplay fDisplay;
};

std::unique_ptr<SkImageGenerator> GrAndroidBufferImageGenerator::Make(
        AHardwareBuffer* graphicBuffer, SkAlphaType alphaType, sk_sp<SkColorSpace> colorSpace) {
    AHardwareBuffer_Desc bufferDesc;
    AHardwareBuffer_describe(graphicBuffer, &bufferDesc);
    SkColorType colorType;
    switch (bufferDesc.format) {
    case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
        colorType = kRGBA_8888_SkColorType;
        break;
    case AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT:
        colorType = kRGBA_F16_SkColorType;
        break;
    case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
        colorType = kRGB_565_SkColorType;
        break;
    default:
        return nullptr;
    }
    SkImageInfo info = SkImageInfo::Make(bufferDesc.width, bufferDesc.height, colorType,
                                         alphaType, std::move(colorSpace));
    return std::unique_ptr<SkImageGenerator>(new GrAndroidBufferImageGenerator(info, graphicBuffer,
            alphaType));
}

GrAndroidBufferImageGenerator::GrAndroidBufferImageGenerator(const SkImageInfo& info,
        AHardwareBuffer* graphicBuffer, SkAlphaType alphaType)
    : INHERITED(info)
    , fGraphicBuffer(graphicBuffer)
    , fAlphaType(alphaType) {
    AHardwareBuffer_acquire(fGraphicBuffer);
}

GrAndroidBufferImageGenerator::~GrAndroidBufferImageGenerator() {
    AHardwareBuffer_release(fGraphicBuffer);
}

void GrAndroidBufferImageGenerator::deleteImageTexture(void* context) {
     BufferCleanupHelper* cleanupHelper = static_cast<BufferCleanupHelper*>(context);
     cleanupHelper->unref();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU


sk_sp<GrTextureProxy> GrAndroidBufferImageGenerator::onGenerateTexture(
        GrContext* context, const SkImageInfo& info, const SkIPoint& origin) {
    // TODO: return a cached GrTextureProxy if invoked with the same context
    // TODO: if we cache GrTextureProxy, then deleteImageTexture may be invoked on the wrong thread

    auto glInterface = ((GrGLGpu*)context->getGpu())->glContext().interface();
    GrGLClearErr(glInterface);

    typedef GrEGLClientBuffer (EGLAPIENTRY *GetNativeClientBufferANDROIDProc)(const AHardwareBuffer*);
    GetNativeClientBufferANDROIDProc eglGetNativeClientBufferANDROID =
        (GetNativeClientBufferANDROIDProc)eglGetProcAddress("eglGetNativeClientBufferANDROID");
    if (!eglGetNativeClientBufferANDROID) {
        return nullptr;
    }
    GrEGLClientBuffer clientBuffer = eglGetNativeClientBufferANDROID(fGraphicBuffer);
    GrEGLint attribs[] = { GR_EGL_IMAGE_PRESERVED, GR_EGL_TRUE,
                           GR_EGL_NONE };
    GrEGLDisplay display = GR_EGL_NO_DISPLAY;
    GrEGLGetCurrentDisplayProc getCurrentDisplay =
            (GrEGLGetCurrentDisplayProc) eglGetCurrentDisplay;
    if (getCurrentDisplay) {
        display = getCurrentDisplay();
    }
    GrEGLImage image;
    GR_GL_CALL_RET(glInterface, image,
                   EGLCreateImage(display, EGL_NO_CONTEXT, GR_EGL_NATIVE_BUFFER_ANDROID, clientBuffer,
                                  attribs));
    if (GR_EGL_NO_IMAGE == image) {
        return nullptr;
    }
    typedef GrGLvoid (EGLAPIENTRY *EGLImageTargetTexture2DProc)(GrGLenum, GrGLeglImage);
    EGLImageTargetTexture2DProc glEGLImageTargetTexture2D =
        (EGLImageTargetTexture2DProc)eglGetProcAddress("glEGLImageTargetTexture2DOES");
    if (!glEGLImageTargetTexture2D) {
        GR_GL_CALL(glInterface, EGLDestroyImage(display, image));
        return nullptr;
    }
    GrGLuint texID;
    GR_GL_CALL(glInterface, GenTextures(1, &texID));
    if (!texID) {
        GR_GL_CALL(glInterface, EGLDestroyImage(display, image));
        return nullptr;
    }
    GR_GL_CALL(glInterface, BindTexture(GR_GL_TEXTURE_EXTERNAL, texID));
    if (GR_GL_GET_ERROR(glInterface) != GR_GL_NO_ERROR) {
        GR_GL_CALL(glInterface, DeleteTextures(1, &texID));
        GR_GL_CALL(glInterface, EGLDestroyImage(display, image));
        return nullptr;
    }
    glEGLImageTargetTexture2D(GR_GL_TEXTURE_EXTERNAL, image);
    if (GR_GL_GET_ERROR(glInterface) != GR_GL_NO_ERROR) {
        GR_GL_CALL(glInterface, DeleteTextures(1, &texID));
        GR_GL_CALL(glInterface, EGLDestroyImage(display, image));
        return nullptr;
    }
    context->resetContext();  //TODO: can we get rid of this?

    GrGLTextureInfo textureInfo;
    textureInfo.fTarget = GR_GL_TEXTURE_EXTERNAL;
    textureInfo.fID = texID;

    GrPixelConfig pixelConfig;
    switch (getInfo().colorType()) {
    case kRGBA_8888_SkColorType:
        pixelConfig = kRGBA_8888_GrPixelConfig;
        break;
    case kRGBA_F16_SkColorType:
        pixelConfig = kRGBA_half_GrPixelConfig;
        break;
    case kRGB_565_SkColorType:
        pixelConfig = kRGB_565_GrPixelConfig;
        break;
    default:
        GR_GL_CALL(glInterface, DeleteTextures(1, &texID));
        GR_GL_CALL(glInterface, EGLDestroyImage(display, image));
        return nullptr;
    }

    GrBackendTexture backendTex(getInfo().width(), getInfo().height(), pixelConfig, textureInfo);
    if (backendTex.width() <= 0 || backendTex.height() <= 0) {
        GR_GL_CALL(glInterface, DeleteTextures(1, &texID));
        GR_GL_CALL(glInterface, EGLDestroyImage(display, image));
        return nullptr;
    }
    GrBackendTextureFlags flags = kNone_GrBackendTextureFlag;
    sk_sp<GrTexture> tex = context->resourceProvider()->wrapBackendTexture(backendTex,
                                                                       kTopLeft_GrSurfaceOrigin,
                                                                       flags,
                                                                       0,
                                                                       kAdopt_GrWrapOwnership);
    if (!tex) {
        GR_GL_CALL(glInterface, DeleteTextures(1, &texID));
        GR_GL_CALL(glInterface, EGLDestroyImage(display, image));
        return nullptr;
    }
    tex->setRelease(deleteImageTexture, new BufferCleanupHelper(context, image, display));
    sk_sp<GrTextureProxy> proxy(GrSurfaceProxy::MakeWrapped(std::move(tex)));

    if (0 == origin.fX && 0 == origin.fY &&
            info.width() == backendTex.width() && info.height() == backendTex.height()) {
        // If the caller wants the entire texture, we're done
        return proxy;
    } else {
        // Otherwise, make a copy of the requested subset. Make sure our temporary is renderable,
        // because Vulkan will want to do the copy as a draw.
        GrSurfaceDesc desc;
        desc.fConfig = proxy->config();
        desc.fWidth = info.width();
        desc.fHeight = info.height();
        desc.fOrigin = proxy->origin();
        desc.fIsMipMapped = proxy->isMipMapped();
        desc.fFlags = kRenderTarget_GrSurfaceFlag;

        sk_sp<GrSurfaceContext> sContext(context->contextPriv().makeDeferredSurfaceContext(
            desc, SkBackingFit::kExact, SkBudgeted::kYes));
        if (!sContext) {
            return nullptr;
        }

        SkIRect subset = SkIRect::MakeXYWH(origin.fX, origin.fY, info.width(), info.height());
        if (!sContext->copy(proxy.get(), subset, SkIPoint::Make(0, 0))) {
            return nullptr;
        }

        return sContext->asTextureProxyRef();
    }
}
#endif

bool GrAndroidBufferImageGenerator::onIsValid(GrContext* context) const {
    if (nullptr == context) {
        return false; //CPU backend is not supported, because hardware buffer can be swizzled
    }
    // TODO: add Vulkan support
    return kOpenGL_GrBackend == context->contextPriv().getBackend();
}

#endif //SK_BUILD_FOR_ANDROID_FRAMEWORK
