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
#include "include/gpu/GrTypes.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"  // IWYU pragma: keep
#include "src/gpu/ganesh/SkGr.h"

#include <memory>
#include <string_view>
#include <tuple>

class GrFragmentProcessor;
class GrRecordingContext;
class SkImage;
class SkImage_Raster;
class SkMatrix;
enum class GrColorType;
enum class SkTileMode;
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
}  // namespace skgpu::ganesh
#endif
