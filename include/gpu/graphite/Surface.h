/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Surface_DEFINED
#define skgpu_graphite_Surface_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GpuTypes.h"

struct SkImageInfo;

namespace skgpu::graphite {
class BackendTexture;
class Recorder;
}  // namespace skgpu::graphite

namespace SkSurfaces {
/**
 * In Graphite, while clients hold a ref on an SkSurface, the backing gpu object does _not_
 * count against the budget. Once an SkSurface is freed, the backing gpu object may or may
 * not become a scratch (i.e., reusable) resource but, if it does, it will be counted against
 * the budget.
 */
SK_API sk_sp<SkSurface> RenderTarget(skgpu::graphite::Recorder*,
                                     const SkImageInfo& imageInfo,
                                     skgpu::Mipmapped = skgpu::Mipmapped::kNo,
                                     const SkSurfaceProps* surfaceProps = nullptr);

/**
 * Wraps a GPU-backed texture in an SkSurface. Depending on the backend gpu API, the caller may
 * be required to ensure the texture is valid for the lifetime of the returned SkSurface. The
 * required lifetimes for the specific apis are:
 *     Metal: Skia will call retain on the underlying MTLTexture so the caller can drop it once
 *            this call returns.
 *
 * SkSurface is returned if all the parameters are valid. The backendTexture is valid if its
 * format agrees with colorSpace and recorder; for instance, if backendTexture has an sRGB
 * configuration, then the recorder must support sRGB, and colorSpace must be present. Further,
 * backendTexture's width and height must not exceed the recorder's capabilities, and the
 * recorder must be able to support the back-end texture.
 */
SK_API sk_sp<SkSurface> WrapBackendTexture(skgpu::graphite::Recorder*,
                                           const skgpu::graphite::BackendTexture&,
                                           SkColorType colorType,
                                           sk_sp<SkColorSpace> colorSpace,
                                           const SkSurfaceProps* props);
}  // namespace SkSurfaces

#endif  // skgpu_graphite_Surface_DEFINED
