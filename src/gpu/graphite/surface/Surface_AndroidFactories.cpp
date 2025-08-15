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

#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
/**
 * When building for the Android framework, there are formats defined outside of those publicly
 * available in android/hardware_buffer.h.
 */
#include <vndk/hardware_buffer.h>
#endif

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

    if (!SkToBool(bufferDesc.usage & AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT) ||
        !SkToBool(bufferDesc.usage & AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE)) {
        if (releaseP) {
            releaseP(releaseC);
        }
        return nullptr;
    }

    bool isProtectedContent = SkToBool(bufferDesc.usage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT);

    bool fromWindowLocal = false;
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    fromWindowLocal = fromWindow;
#endif

    SkISize dims = SkISize::Make(bufferDesc.width, bufferDesc.height);

    BackendTexture backendTexture = recorder->createBackendTexture(hardwareBuffer,
                                                                   /* isRenderable= */ true,
                                                                   isProtectedContent,
                                                                   dims,
                                                                   fromWindowLocal);
    if (!backendTexture.isValid()) {
        if (releaseP) {
            releaseP(releaseC);
        }
        return nullptr;
    }


    /**
     * Determine the color type for the hardware buffer based on its format. In most cases, simply
     * checking `AHardwareBufferUtils::GetSkColorTypeFromBufferFormat` is sufficient. However, it is
     * possible that Skia determines that the AHwBuf should be imported using an external format.
     * In this case - even though we can never write to a texture using an external format - Skia
     * validity checks will expect the usage of kExternalFormatColorType (see VulkanCaps's
     * fExternalFormatColorTypeInfo), so enforce that here.
     */
    SkColorType ct;
    const bool textureUsesExternalFormat =
            TextureInfoPriv::ViewFormat(backendTexture.info()) == TextureFormat::kExternal;

#if defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)

    // Check for a special case - the usage of AHARDWAREBUFFER_FORMAT_B8G8R8A8_UNORM - when
    // building for the Android framework.
    //
    // Skia internal piping and validity checks rely upon SkSurfaces having a defined SkColorType
    // that is compatible with a texture's color information. b/431290055 exposed a case where
    // we could end up using an SkColorType that is not compatible with the BackendTexture's
    // VkFormat.
    //
    // Essentially, the issue arose in a case where the driver may report that a certain VkFormat
    // can be used when importing the AHwBuf via VkAndroidHardwareBufferFormatPropertiesANDROID.
    // When *creating* an SkSurface from an AHwBuf-based BackendTexture, the surface's color type is
    // determined by the AHwBuf format (using `GetSkColorTypeFromBufferFormat`).
    // However, when *validating* the surface, Skia checks whether the color type is compatible with
    // the texture's VkFormat (which is a separate value from the AHwBuf format).
    //
    // Ideally, we would simply add the bug's AHwBuf format to `GetSkColorTypeFromBufferFormat` and
    // map to the expected SkColorType. However, this utility is shared b/w ganesh and graphite,
    // and adding this functionality introduced complexity within ganesh. One such obstacle is that
    // its SkImages::DeferredFromAHardwareBuffer API does not know the AHwBuf format at creation.
    // While this could be worked around, the change would be fairly invasive for minimal reward
    // given that no clients have requested that we support this format in ganesh.
    //
    // Therefore, simply manually check for this case within graphite.
    if (!textureUsesExternalFormat &&
        bufferDesc.format == AHARDWAREBUFFER_FORMAT_B8G8R8A8_UNORM) {
        ct = kBGRA_8888_SkColorType;
    } else
#endif
    {
        ct = textureUsesExternalFormat
                ? AHardwareBufferUtils::kExternalFormatColorType
                : AHardwareBufferUtils::GetSkColorTypeFromBufferFormat(bufferDesc.format);
    }

    /**
     * Ensure that we have not somehow ended up in a situation where the determined color type is
     * incompatible with the texture's format.
     */
    SkASSERT(textureUsesExternalFormat ||
             recorder->priv().caps()->areColorTypeAndTextureInfoCompatible(ct,
                                                                           backendTexture.info()));

    /* Will call 'releaseP' if SkSurface creation fails. */
    return SkSurfaces::WrapBackendTexture(recorder,
                                          backendTexture,
                                          ct,
                                          std::move(colorSpace),
                                          surfaceProps,
                                          releaseP,
                                          releaseC);
}

}  // namespace SkSurfaces

#endif // __ANDROID_API__ >= 26
