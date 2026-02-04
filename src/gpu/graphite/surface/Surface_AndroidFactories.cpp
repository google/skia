/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if __ANDROID_API__ >= 26

#include "include/android/AHardwareBufferUtils.h"
#include "include/android/graphite/SurfaceAndroid.h"
#include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/graphite/TextureInfoPriv.h"

#include <android/hardware_buffer.h>

using namespace skgpu::graphite;

namespace SkSurfaces {

sk_sp<SkSurface> WrapAndroidHardwareBuffer(Recorder* recorder,
                                           AHardwareBuffer* hardwareBuffer,
                                           sk_sp<SkColorSpace> colorSpace,
                                           const SkSurfaceProps* surfaceProps,
                                           BufferReleaseProc releaseP,
                                           ReleaseContext releaseC,
                                           bool fromWindow) {
    if (!recorder || !hardwareBuffer) {
        return nullptr;
    }

    AHardwareBuffer_Desc bufferDesc;
    AHardwareBuffer_describe(hardwareBuffer, &bufferDesc);

    // AHB formats that mask out the alpha channel do not make sense to render into. This is
    // consistent with ::RenderTarget(SkColorType) not allowing types like kRGB_888x to be an
    // SkSurface. However, these AHB formats may get imported into Vulkan/GL as a backend format
    // that does not mask the alpha channel, e.g. AHB_FORMAT_R8G8B8X8_UNORM maps to
    // VK_FORMAT_R8G8AB8A8_UNORM (https://developer.android.com/ndk/reference/group/a-hardware-buffer#group___a_hardware_buffer_1gga94397844440c1c27b5b454d56ba5ff38af4f001b0b3e8aea568b533f682418e7f)
    //
    // We also require them to be usable as color output and sampled images.
    if (!SkToBool(bufferDesc.usage & AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT) ||
        !SkToBool(bufferDesc.usage & AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE) ||
        bufferDesc.format == AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM) {
        if (releaseP) {
            releaseP(releaseC);
        }
        return nullptr;
    }

    const bool isProtectedContent =
            SkToBool(bufferDesc.usage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT);

#if !defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
    fromWindow = false; // ignored for non-framework builds
#endif

    const SkISize dims = SkISize::Make(bufferDesc.width, bufferDesc.height);
    BackendTexture backendTexture = recorder->createBackendTexture(hardwareBuffer,
                                                                   /* isRenderable= */ true,
                                                                   isProtectedContent,
                                                                   dims,
                                                                   fromWindow);
    if (!backendTexture.isValid()) {
        if (releaseP) {
            releaseP(releaseC);
        }
        return nullptr;
    }

    // Will call 'releaseP' if SkSurface creation fails.
    return SkSurfaces::WrapBackendTexture(recorder,
                                          backendTexture,
                                          std::move(colorSpace),
                                          surfaceProps,
                                          releaseP,
                                          releaseC);
}

}  // namespace SkSurfaces

#endif // __ANDROID_API__ >= 26
