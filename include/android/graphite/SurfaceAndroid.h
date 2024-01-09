/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SurfaceAndroid_DEFINED
#define SurfaceAndroid_DEFINED

#include "include/core/SkRefCnt.h"

struct AHardwareBuffer;
class SkColorSpace;
class SkSurface;
class SkSurfaceProps;

namespace skgpu::graphite {
    class Recorder;
}

namespace SkSurfaces {

using ReleaseContext = void*;
using BufferReleaseProc = void (*)(ReleaseContext);

/** Private; only to be used by Android Framework.
    Creates an SkSurface from an Android hardware buffer.

    Upon success bufferReleaseProc is called when it is safe to delete the buffer in the
    backend API (accounting only for use of the buffer by this surface). If SkSurface creation
    fails bufferReleaseProc is called before this function returns.

    Currently this is only supported for buffers that can be textured as well as rendered to.
    In other words the buffer must have both AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT and
    AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE usage bits.

    @param recorder          the applicable Graphite recorder
    @param hardwareBuffer    an Android hardware buffer
    @param colorSpace        range of colors; may be nullptr
    @param surfaceProps      LCD striping orientation and setting for device independent
                             fonts; may be nullptr
    @param bufferReleaseProc function called when the buffer can be released
    @param ReleaseContext    state passed to bufferReleaseProc
    @param fromWindow        Whether or not the AHardwareBuffer is part of an Android Window.
                             Currently only used with Vulkan backend.
    @return                  created SkSurface, or nullptr
*/
SK_API sk_sp<SkSurface> WrapAndroidHardwareBuffer(skgpu::graphite::Recorder* recorder,
                                                  AHardwareBuffer* hardwareBuffer,
                                                  sk_sp<SkColorSpace> colorSpace,
                                                  const SkSurfaceProps* surfaceProps,
                                                  BufferReleaseProc = nullptr,
                                                  ReleaseContext = nullptr,
                                                  bool fromWindow = false);

}  // namespace SkSurfaces

#endif // SurfaceAndroid_DEFINED
