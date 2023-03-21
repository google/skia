/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26

#include "include/android/SkImageAndroid.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextThreadSafeProxy.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrImageContext.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrAHardwareBufferImageGenerator.h"
#include "src/gpu/ganesh/GrAHardwareBufferUtils_impl.h"
#include "src/gpu/ganesh/GrBackendTextureImageGenerator.h"
#include "src/gpu/ganesh/GrBackendUtils.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrGpuResourcePriv.h"
#include "src/gpu/ganesh/GrImageContextPriv.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrRenderTask.h"
#include "src/gpu/ganesh/GrSemaphore.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/SurfaceContext.h"
#include "src/gpu/ganesh/SurfaceFillContext.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Gpu.h"

#include <algorithm>
#include <cstddef>
#include <utility>

namespace sk_image_factory {

sk_sp<SkImage> MakeFromAHardwareBuffer(AHardwareBuffer* graphicBuffer, SkAlphaType at) {
    auto gen = GrAHardwareBufferImageGenerator::Make(graphicBuffer, at, nullptr,
                                                     kTopLeft_GrSurfaceOrigin);
    return SkImage::MakeFromGenerator(std::move(gen));
}

sk_sp<SkImage> MakeFromAHardwareBuffer(AHardwareBuffer* graphicBuffer, SkAlphaType at,
                                                sk_sp<SkColorSpace> cs,
                                                GrSurfaceOrigin surfaceOrigin) {
    auto gen = GrAHardwareBufferImageGenerator::Make(graphicBuffer, at, cs, surfaceOrigin);
    return SkImage::MakeFromGenerator(std::move(gen));
}

sk_sp<SkImage> MakeFromAHardwareBufferWithData(GrDirectContext* dContext,
                                                        const SkPixmap& pixmap,
                                                        AHardwareBuffer* hardwareBuffer,
                                                        GrSurfaceOrigin surfaceOrigin) {
    AHardwareBuffer_Desc bufferDesc;
    AHardwareBuffer_describe(hardwareBuffer, &bufferDesc);

    if (!SkToBool(bufferDesc.usage & AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE)) {
        return nullptr;
    }


    GrBackendFormat backendFormat = GrAHardwareBufferUtils::GetBackendFormat(dContext,
                                                                             hardwareBuffer,
                                                                             bufferDesc.format,
                                                                             true);

    if (!backendFormat.isValid()) {
        return nullptr;
    }

    GrAHardwareBufferUtils::DeleteImageProc deleteImageProc = nullptr;
    GrAHardwareBufferUtils::UpdateImageProc updateImageProc = nullptr;
    GrAHardwareBufferUtils::TexImageCtx deleteImageCtx = nullptr;

    const bool isRenderable = SkToBool(bufferDesc.usage & AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER);

    GrBackendTexture backendTexture =
            GrAHardwareBufferUtils::MakeBackendTexture(dContext, hardwareBuffer,
                                                       bufferDesc.width, bufferDesc.height,
                                                       &deleteImageProc, &updateImageProc,
                                                       &deleteImageCtx, false, backendFormat,
                                                       isRenderable);
    if (!backendTexture.isValid()) {
        return nullptr;
    }
    SkASSERT(deleteImageProc);

    auto releaseHelper = skgpu::RefCntedCallback::Make(deleteImageProc, deleteImageCtx);

    SkColorType colorType =
            GrAHardwareBufferUtils::GetSkColorTypeFromBufferFormat(bufferDesc.format);

    GrColorType grColorType = SkColorTypeToGrColorType(colorType);

    GrProxyProvider* proxyProvider = dContext->priv().proxyProvider();
    if (!proxyProvider) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> proxy = proxyProvider->wrapBackendTexture(
            backendTexture, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo, kRW_GrIOType,
            std::move(releaseHelper));
    if (!proxy) {
        return nullptr;
    }

    skgpu::Swizzle swizzle = dContext->priv().caps()->getReadSwizzle(backendFormat, grColorType);
    GrSurfaceProxyView framebufferView(std::move(proxy), surfaceOrigin, swizzle);
    SkColorInfo colorInfo = pixmap.info().colorInfo().makeColorType(colorType);
    sk_sp<SkImage> image = sk_make_sp<SkImage_Gpu>(sk_ref_sp(dContext),
                                                   kNeedNewImageUniqueID,
                                                   framebufferView,
                                                   std::move(colorInfo));
    if (!image) {
        return nullptr;
    }

    GrDrawingManager* drawingManager = dContext->priv().drawingManager();
    if (!drawingManager) {
        return nullptr;
    }

    skgpu::ganesh::SurfaceContext surfaceContext(
            dContext, std::move(framebufferView), image->imageInfo().colorInfo());

    surfaceContext.writePixels(dContext, pixmap, {0, 0});

    GrSurfaceProxy* p[1] = {surfaceContext.asSurfaceProxy()};
    drawingManager->flush(p, SkSurface::BackendSurfaceAccess::kNoAccess, {}, nullptr);

    return image;
}

} // namespace sk_image_factory

#if !defined(SK_DISABLE_LEGACY_IMAGE_FACTORIES)

sk_sp<SkImage> SkImage::MakeFromAHardwareBuffer(
        AHardwareBuffer* hardwareBuffer,
        SkAlphaType alphaType) {
    return sk_image_factory::MakeFromAHardwareBuffer(hardwareBuffer, alphaType);
}

sk_sp<SkImage> SkImage::MakeFromAHardwareBuffer(
        AHardwareBuffer* hardwareBuffer,
        SkAlphaType alphaType,
        sk_sp<SkColorSpace> colorSpace,
        GrSurfaceOrigin surfaceOrigin) {
    return sk_image_factory::MakeFromAHardwareBuffer(hardwareBuffer, alphaType,
                                                     colorSpace, surfaceOrigin);
}

sk_sp<SkImage> SkImage::MakeFromAHardwareBufferWithData(
        GrDirectContext* context,
        const SkPixmap& pixmap,
        AHardwareBuffer* hardwareBuffer,
        GrSurfaceOrigin surfaceOrigin) {
    return sk_image_factory::MakeFromAHardwareBufferWithData(context,
                                                             pixmap,
                                                             hardwareBuffer,
                                                             surfaceOrigin);
}

#endif // SK_DISABLE_LEGACY_IMAGE_FACTORIES

#endif // defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26
