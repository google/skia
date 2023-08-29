/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrImageUtils_DEFINED
#define GrImageUtils_DEFINED

#include "include/core/SkRefCnt.h"

#include "include/core/SkSamplingOptions.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/gpu/GrTypes.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"  // IWYU pragma: keep
#include "src/gpu/ganesh/SkGr.h"

#include <cstdint>
#include <memory>
#include <string_view>
#include <tuple>

class GrCaps;
class GrFragmentProcessor;
class GrImageContext;
class GrRecordingContext;
class SkImage;
class SkImage_Lazy;
class SkImage_Raster;
class SkMatrix;
enum SkAlphaType : int;
enum SkColorType : int;
enum class GrColorType;
enum class SkTileMode;
namespace skgpu { enum class Mipmapped : bool; }
struct SkRect;

namespace skgpu::ganesh {
// Returns a GrSurfaceProxyView representation of the image, if possible. This also returns
// a color type. This may be different than the image's color type when the image is not
// texture-backed and the capabilities of the GPU require a data type conversion to put
// the data in a texture.
std::tuple<GrSurfaceProxyView, GrColorType> AsView(
        GrRecordingContext*,
        const SkImage*,
        GrMipmapped,
        GrImageTexGenPolicy = GrImageTexGenPolicy::kDraw);

inline std::tuple<GrSurfaceProxyView, GrColorType> AsView(
        GrRecordingContext* ctx,
        sk_sp<const SkImage> img,
        GrMipmapped mm,
        GrImageTexGenPolicy policy = GrImageTexGenPolicy::kDraw) {
    return AsView(ctx, img.get(), mm, policy);
}

std::tuple<GrSurfaceProxyView, GrColorType> RasterAsView(
        GrRecordingContext*,
        const SkImage_Raster*,
        GrMipmapped,
        GrImageTexGenPolicy = GrImageTexGenPolicy::kDraw);

// Utility for making a copy of an existing view when the GrImageTexGenPolicy is not kDraw.
GrSurfaceProxyView CopyView(GrRecordingContext*,
                            GrSurfaceProxyView src,
                            GrMipmapped,
                            GrImageTexGenPolicy,
                            std::string_view label);

// Returns the texture proxy. CachingHint refers to whether the generator's output should be
// cached in CPU memory. We will always cache the generated texture on success.
GrSurfaceProxyView LockTextureProxyView(GrRecordingContext*,
                                        const SkImage_Lazy*,
                                        GrImageTexGenPolicy,
                                        skgpu::Mipmapped);

// Returns the GrColorType to use with the GrTextureProxy returned from lockTextureProxy. This
// may be different from the color type on the image in the case where we need up upload CPU
// data to a texture but the GPU doesn't support the format of CPU data. In this case we convert
// the data to RGBA_8888 unorm on the CPU then upload that.
GrColorType ColorTypeOfLockTextureProxy(const GrCaps*, SkColorType);

/**
 * Returns a GrFragmentProcessor that can be used with the passed GrRecordingContext to
 * draw the image. SkSamplingOptions indicates the filter and SkTileMode[] indicates the x and
 * y tile modes. The passed matrix is applied to the coordinates before sampling the image.
 * Optional 'subset' indicates whether the tile modes should be applied to a subset of the image
 * Optional 'domain' is a bound on the coordinates of the image that will be required and can be
 * used to optimize the shader if 'subset' is also specified.
 */
std::unique_ptr<GrFragmentProcessor> AsFragmentProcessor(GrRecordingContext*,
                                                         const SkImage*,
                                                         SkSamplingOptions,
                                                         const SkTileMode[2],
                                                         const SkMatrix&,
                                                         const SkRect* subset = nullptr,
                                                         const SkRect* domain = nullptr);

inline std::unique_ptr<GrFragmentProcessor> AsFragmentProcessor(GrRecordingContext* ctx,
                                                                sk_sp<const SkImage> img,
                                                                SkSamplingOptions opt,
                                                                const SkTileMode tm[2],
                                                                const SkMatrix& m,
                                                                const SkRect* subset = nullptr,
                                                                const SkRect* domain = nullptr) {
    return AsFragmentProcessor(ctx, img.get(), opt, tm, m, subset, domain);
}

std::unique_ptr<GrFragmentProcessor> MakeFragmentProcessorFromView(GrRecordingContext*,
                                                                   GrSurfaceProxyView,
                                                                   SkAlphaType,
                                                                   SkSamplingOptions,
                                                                   const SkTileMode[2],
                                                                   const SkMatrix&,
                                                                   const SkRect* subset,
                                                                   const SkRect* domain);

/**
 * Returns input view if it is already mipmapped. Otherwise, attempts to make a mipmapped view
 * with the same contents. If the mipmapped copy is successfully created it will be cached
 * using the image unique ID. A subsequent call with the same unique ID will return the cached
 * view if it has not been purged. The view is cached with a key domain specific to this
 * function.
 */
GrSurfaceProxyView FindOrMakeCachedMipmappedView(GrRecordingContext*,
                                                 GrSurfaceProxyView,
                                                 uint32_t imageUniqueID);

/** Init based on texture formats supported by the context. */
SkYUVAPixmapInfo::SupportedDataTypes SupportedTextureFormats(const GrImageContext&);

}  // namespace skgpu::ganesh

namespace skif {
class Context;
struct ContextInfo;
struct Functors;
Functors MakeGaneshFunctors(GrRecordingContext* context, GrSurfaceOrigin origin);
Context MakeGaneshContext(GrRecordingContext* context,
                          GrSurfaceOrigin origin,
                          const ContextInfo& info);
}  // namespace skif

#endif
