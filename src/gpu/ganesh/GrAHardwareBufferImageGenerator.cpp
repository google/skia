/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26

#include "src/gpu/ganesh/GrAHardwareBufferImageGenerator.h"

#include "include/android/GrAHardwareBufferUtils.h"
#include "include/core/SkColorSpace.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "src/core/SkMessageBus.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrResourceCache.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrResourceProviderPriv.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/SkGr.h"

#include <android/hardware_buffer.h>

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
    : GrTextureGenerator(info)
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

GrSurfaceProxyView GrAHardwareBufferImageGenerator::makeView(GrRecordingContext* context) {
    if (context->abandoned()) {
        return {};
    }

    auto direct = context->asDirectContext();
    if (!direct) {
        return {};
    }

    GrBackendFormat backendFormat = GrAHardwareBufferUtils::GetBackendFormat(direct,
                                                                             fHardwareBuffer,
                                                                             fBufferFormat,
                                                                             false);

    GrColorType grColorType = SkColorTypeToGrColorType(this->getInfo().colorType());

    int width = this->getInfo().width();
    int height = this->getInfo().height();

    auto proxyProvider = context->priv().proxyProvider();

    AHardwareBuffer* hardwareBuffer = fHardwareBuffer;
    AHardwareBuffer_acquire(hardwareBuffer);

    class AutoAHBRelease {
    public:
        AutoAHBRelease(AHardwareBuffer* ahb) : fAhb(ahb) {}
        // std::function() must be CopyConstructible, but ours should never actually be copied.
        AutoAHBRelease(const AutoAHBRelease&) { SkASSERT(0); }
        AutoAHBRelease(AutoAHBRelease&& that) : fAhb(that.fAhb) { that.fAhb = nullptr; }
        ~AutoAHBRelease() { fAhb ? AHardwareBuffer_release(fAhb) : void(); }

        AutoAHBRelease& operator=(AutoAHBRelease&& that) {
            fAhb = std::exchange(that.fAhb, nullptr);
            return *this;
        }
        AutoAHBRelease& operator=(const AutoAHBRelease&) = delete;

        AHardwareBuffer* get() const { return fAhb; }

    private:
        AHardwareBuffer* fAhb;
    };

    sk_sp<GrTextureProxy> texProxy = proxyProvider->createLazyProxy(
            [direct, buffer = AutoAHBRelease(hardwareBuffer)](
                    GrResourceProvider* resourceProvider,
                    const GrSurfaceProxy::LazySurfaceDesc& desc)
                    -> GrSurfaceProxy::LazyCallbackResult {
                GrAHardwareBufferUtils::DeleteImageProc deleteImageProc = nullptr;
                GrAHardwareBufferUtils::UpdateImageProc updateImageProc = nullptr;
                GrAHardwareBufferUtils::TexImageCtx texImageCtx = nullptr;

                bool isProtected = desc.fProtected == GrProtected::kYes;
                GrBackendTexture backendTex =
                        GrAHardwareBufferUtils::MakeBackendTexture(direct,
                                                                   buffer.get(),
                                                                   desc.fDimensions.width(),
                                                                   desc.fDimensions.height(),
                                                                   &deleteImageProc,
                                                                   &updateImageProc,
                                                                   &texImageCtx,
                                                                   isProtected,
                                                                   desc.fFormat,
                                                                   false);
                if (!backendTex.isValid()) {
                    return {};
                }
                SkASSERT(deleteImageProc && texImageCtx);

                // We make this texture cacheable to avoid recreating a GrTexture every time this
                // is invoked. We know the owning SkImage will send an invalidation message when the
                // image is destroyed, so the texture will be removed at that time. Note that the
                // proxy will be keyed in GrProxyProvider but that cache just allows extant proxies
                // to be reused. It does not retain them. After a flush the proxy will be deleted
                // and a subsequent use of the image will recreate a new proxy around the GrTexture
                // found in the GrResourceCache.
                // This is the last use of GrWrapCacheable::kYes so if we actually cached the proxy
                // we could remove wrapped GrGpuResource caching.
                sk_sp<GrTexture> tex = resourceProvider->wrapBackendTexture(
                        backendTex, kBorrow_GrWrapOwnership, GrWrapCacheable::kYes, kRead_GrIOType);
                if (!tex) {
                    deleteImageProc(texImageCtx);
                    return {};
                }

                if (deleteImageProc) {
                    tex->setRelease(deleteImageProc, texImageCtx);
                }

                return tex;
            },
            backendFormat,
            {width, height},
            skgpu::Mipmapped::kNo,
            GrMipmapStatus::kNotAllocated,
            GrInternalSurfaceFlags::kReadOnly,
            SkBackingFit::kExact,
            skgpu::Budgeted::kNo,
            GrProtected(fIsProtectedContent),
            GrSurfaceProxy::UseAllocator::kYes,
            "AHardwareBufferImageGenerator_MakeView");

    skgpu::Swizzle readSwizzle = context->priv().caps()->getReadSwizzle(backendFormat, grColorType);

    return GrSurfaceProxyView(std::move(texProxy), fSurfaceOrigin, readSwizzle);
}

GrSurfaceProxyView GrAHardwareBufferImageGenerator::onGenerateTexture(
        GrRecordingContext* context,
        const SkImageInfo& info,
        skgpu::Mipmapped mipmapped,
        GrImageTexGenPolicy texGenPolicy) {
    GrSurfaceProxyView texProxyView = this->makeView(context);
    if (!texProxyView.proxy()) {
        return {};
    }
    SkASSERT(texProxyView.asTextureProxy());

    if (texGenPolicy == GrImageTexGenPolicy::kDraw && mipmapped == skgpu::Mipmapped::kNo) {
        // If we have the correct mip support, we're done
        return texProxyView;
    }

    // Otherwise, make a copy for the requested MIP map setting.
    skgpu::Budgeted budgeted = texGenPolicy == GrImageTexGenPolicy::kNew_Uncached_Unbudgeted
                                       ? skgpu::Budgeted::kNo
                                       : skgpu::Budgeted::kYes;

    return GrSurfaceProxyView::Copy(context,
                                    std::move(texProxyView),
                                    mipmapped,
                                    SkIRect::MakeWH(info.width(), info.height()),
                                    SkBackingFit::kExact,
                                    budgeted,
                                    /*label=*/"AHardwareBufferImageGenerator_GenerateTexture");
}

bool GrAHardwareBufferImageGenerator::onIsValid(GrRecordingContext* context) const {
    if (nullptr == context) {
        return false; //CPU backend is not supported, because hardware buffer can be swizzled
    }
    return GrBackendApi::kOpenGL == context->backend() ||
           GrBackendApi::kVulkan == context->backend();
}

#endif //SK_BUILD_FOR_ANDROID_FRAMEWORK
