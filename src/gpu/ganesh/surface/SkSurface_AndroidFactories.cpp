/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 26

#include "include/android/AHardwareBufferUtils.h"
#include "include/android/GrAHardwareBufferUtils.h"
#include "include/android/SkSurfaceAndroid.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrContextThreadSafeProxy.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/base/SkTo.h"
#include "include/private/chromium/GrSurfaceCharacterization.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkDevice.h"
#include "src/core/SkSurfacePriv.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/Device.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpuResourcePriv.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyPriv.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/image/SkImage_Ganesh.h"
#include "src/gpu/ganesh/surface/SkSurface_Ganesh.h"
#include "src/image/SkImage_Base.h"

#include <algorithm>
#include <cstddef>
#include <utility>

#include <android/hardware_buffer.h>
#include <inttypes.h>

namespace SkSurfaces {

sk_sp<SkSurface> WrapAndroidHardwareBuffer(GrDirectContext* dContext,
                                           AHardwareBuffer* hardwareBuffer,
                                           GrSurfaceOrigin origin,
                                           sk_sp<SkColorSpace> colorSpace,
                                           const SkSurfaceProps* surfaceProps,
                                           bool fromWindow) {
    AHardwareBuffer_Desc bufferDesc;
    AHardwareBuffer_describe(hardwareBuffer, &bufferDesc);

    if (!SkToBool(bufferDesc.usage & AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT)) {
        SkDebugf("%s failed due buffer lacking AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT usage bit",
                 __func__);
        return nullptr;
    }

    bool isTextureable = SkToBool(bufferDesc.usage & AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE);
    if (!isTextureable) {
        SkDebugf("%s failed due buffer lacking AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE usage bit",
                 __func__);
        return nullptr;
    }

    GrBackendFormat backendFormat = GrAHardwareBufferUtils::GetBackendFormat(
            dContext, hardwareBuffer, bufferDesc.format, true);
    if (!backendFormat.isValid()) {
        SkDebugf("%s requires a known format, but failed due not finding a valid backend format for"
                 " %" PRIu32 ".", __func__, bufferDesc.format);
        return nullptr;
    }

    GrAHardwareBufferUtils::DeleteImageProc deleteImageProc = nullptr;
    GrAHardwareBufferUtils::UpdateImageProc updateImageProc = nullptr;
    GrAHardwareBufferUtils::TexImageCtx deleteImageCtx = nullptr;

    bool isProtectedContent =
            SkToBool(bufferDesc.usage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT);

    bool fromWindowLocal = false;
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    fromWindowLocal = fromWindow;
#endif

    GrBackendTexture backendTexture =
            GrAHardwareBufferUtils::MakeBackendTexture(dContext,
                                                       hardwareBuffer,
                                                       bufferDesc.width,
                                                       bufferDesc.height,
                                                       &deleteImageProc,
                                                       &updateImageProc,
                                                       &deleteImageCtx,
                                                       isProtectedContent,
                                                       backendFormat,
                                                       true,
                                                       fromWindowLocal);
    if (!backendTexture.isValid()) {
        SkDebugf("%s failed due to backend texture creation failing", __func__);
        return nullptr;
    }

    SkColorType colorType =
            AHardwareBufferUtils::GetSkColorTypeFromBufferFormat(bufferDesc.format);

    // Will call deleteImageProc if SkSurface creation fails.
    sk_sp<SkSurface> surface = SkSurfaces::WrapBackendTexture(dContext,
                                                              backendTexture,
                                                              origin,
                                                              0,
                                                              colorType,
                                                              std::move(colorSpace),
                                                              surfaceProps,
                                                              deleteImageProc,
                                                              deleteImageCtx);

    return surface;
}

}  // namespace SkSurfaces

#endif
