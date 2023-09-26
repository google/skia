/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrTypes.h"

class SkColorSpace;
class SkImage;
class SkPixmap;
struct SkISize;

#ifdef SK_GANESH
class GrDirectContext;
#endif

#ifdef SK_GRAPHITE
namespace skgpu::graphite {
    class Recorder;
}
using Origin = skgpu::Origin; // TODO: Can we migrate Ganesh to use this?
#include "include/gpu/graphite/BackendTexture.h"
#endif

namespace sk_gpu_test {

using Mipmapped = skgpu::Mipmapped;
using Protected = skgpu::Protected;
using Renderable = skgpu::Renderable;

#ifdef SK_GANESH
/**
 * Creates a backend texture with pixmap contents and wraps it in a SkImage that safely deletes
 * the texture when it goes away. Unlike using makeTextureImage() on a non-GPU image, this will
 * fail rather than fallback if the pixmaps's color type doesn't map to a supported texture format.
 * For testing purposes the texture can be made renderable to exercise different code paths for
 * renderable textures/formats.
 */
sk_sp<SkImage> MakeBackendTextureImage(GrDirectContext*,
                                       const SkPixmap&,
                                       Renderable,
                                       GrSurfaceOrigin,
                                       Protected = Protected::kNo);

/** Creates an image of with a solid color. */
sk_sp<SkImage> MakeBackendTextureImage(GrDirectContext*,
                                       const SkImageInfo& info,
                                       SkColor4f,
                                       Mipmapped = Mipmapped::kNo,
                                       Renderable = Renderable::kNo,
                                       GrSurfaceOrigin = GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
                                       Protected isProtected = Protected::kNo);
#endif  // SK_GANESH

#ifdef SK_GRAPHITE
/*
 * Graphite version of MakeBackendTextureImage
 */
sk_sp<SkImage> MakeBackendTextureImage(skgpu::graphite::Recorder*,
                                       const SkPixmap&,
                                       Mipmapped,
                                       Renderable,
                                       Origin,
                                       Protected = Protected::kNo);
#endif  // SK_GRAPHITE

}  // namespace sk_gpu_test
