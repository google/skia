/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TestSurface_DEFINED
#define TestSurface_DEFINED

#include "include/core/SkColorSpace.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/gpu/GpuTypes.h"
#include "src/core/SkColorData.h"

#if defined(SK_GANESH)
#include "include/gpu/ganesh/GrTypes.h"
#endif

class SkSurface;
class SkSurfaceProps;
enum SkColorType : int;
struct SkImageInfo;

#if defined(SK_GANESH)
class GrDirectContext;
#endif

#if defined(SK_GRAPHITE)
namespace skgpu::graphite {
    class Recorder;
}
#endif

namespace sk_gpu_test {
#if defined(SK_GANESH)
sk_sp<SkSurface> MakeBackendTextureSurface(GrDirectContext*,
                                           const SkImageInfo&,
                                           GrSurfaceOrigin,
                                           int sampleCnt,
                                           skgpu::Mipmapped = skgpu::Mipmapped::kNo,
                                           GrProtected = GrProtected::kNo,
                                           const SkSurfaceProps* = nullptr);

sk_sp<SkSurface> MakeBackendTextureSurface(GrDirectContext*,
                                           SkISize,
                                           GrSurfaceOrigin,
                                           int sampleCnt,
                                           SkColorType,
                                           sk_sp<SkColorSpace> = nullptr,
                                           skgpu::Mipmapped = skgpu::Mipmapped::kNo,
                                           GrProtected = GrProtected::kNo,
                                           const SkSurfaceProps* = nullptr);

/** Creates an SkSurface backed by a non-textureable render target. */
sk_sp<SkSurface> MakeBackendRenderTargetSurface(GrDirectContext*,
                                                const SkImageInfo&,
                                                GrSurfaceOrigin,
                                                int sampleCnt,
                                                GrProtected = GrProtected::kNo,
                                                const SkSurfaceProps* = nullptr);

sk_sp<SkSurface> MakeBackendRenderTargetSurface(GrDirectContext*,
                                                SkISize,
                                                GrSurfaceOrigin,
                                                int sampleCnt,
                                                SkColorType,
                                                sk_sp<SkColorSpace> = nullptr,
                                                GrProtected = GrProtected::kNo,
                                                const SkSurfaceProps* = nullptr);
#endif  // SK_GANESH

#ifdef SK_GRAPHITE
/*
 * Graphite version of MakeBackendTextureSurface
 */
sk_sp<SkSurface> MakeBackendTextureSurface(skgpu::graphite::Recorder*,
                                           const SkImageInfo&,
                                           skgpu::Mipmapped = skgpu::Mipmapped::kNo,
                                           skgpu::Protected = skgpu::Protected::kNo,
                                           const SkSurfaceProps* = nullptr);

#if defined(SK_DAWN)
/*
 * Variation that wraps a WGPUTextureView. Only supported on Dawn backend.
 */
sk_sp<SkSurface> MakeBackendTextureViewSurface(skgpu::graphite::Recorder*,
                                               const SkImageInfo&,
                                               skgpu::Mipmapped = skgpu::Mipmapped::kNo,
                                               skgpu::Protected = skgpu::Protected::kNo,
                                               const SkSurfaceProps* = nullptr);
#endif // SK_DAWN

#endif  // SK_GRAPHITE

}  // namespace sk_gpu_test

#endif
