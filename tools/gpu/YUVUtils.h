/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef YUVUtils_DEFINED
#define YUVUtils_DEFINED

#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/gpu/GrBackendSurface.h"
#include "src/base/SkAutoMalloc.h"

#include <tuple>

class SkData;
#if defined(SK_GRAPHITE)
namespace skgpu::graphite { class Recorder; }
#endif

namespace sk_gpu_test {

// Splits an input image into A8 YUV[A] planes using the passed subsampling and YUV color space. If
// the src image is opaque there will be three planes (Y, U, and V) and if not there will be a
// fourth A plane. The planes are returned along with a SkYUVAInfo describing the resulting planar
// image. Images are made as textures if GrRecordingContext is not null, otherwise as cpu images.
std::tuple<std::array<sk_sp<SkImage>, SkYUVAInfo::kMaxPlanes>, SkYUVAInfo>
MakeYUVAPlanesAsA8(SkImage*,
                   SkYUVColorSpace,
                   SkYUVAInfo::Subsampling,
                   GrRecordingContext*);

// Utility that decodes a JPEG but preserves the YUVA8 planes in the image, and uses
// MakeFromYUVAPixmaps to create a GPU multiplane YUVA image for a context. It extracts the planar
// data once, and lazily creates the actual SkImage when the GrContext is provided (and refreshes
// the image if the context has changed, as in Viewer)
class LazyYUVImage {
public:
    // Returns null if the data could not be extracted into YUVA planes
    static std::unique_ptr<LazyYUVImage> Make(sk_sp<SkData> data,
                                              skgpu::Mipmapped = skgpu::Mipmapped::kNo,
                                              sk_sp<SkColorSpace> = nullptr);
    static std::unique_ptr<LazyYUVImage> Make(SkYUVAPixmaps,
                                              skgpu::Mipmapped = skgpu::Mipmapped::kNo,
                                              sk_sp<SkColorSpace> = nullptr);

    enum class Type { kFromPixmaps, kFromGenerator, kFromTextures, kFromImages };

    SkISize dimensions() const { return fPixmaps.yuvaInfo().dimensions(); }

    sk_sp<SkImage> refImage(GrRecordingContext* rContext, Type);
#if defined(SK_GRAPHITE)
    sk_sp<SkImage> refImage(skgpu::graphite::Recorder* recorder, Type);
#endif

private:
    // Decoded YUV data
    SkYUVAPixmaps fPixmaps;

    skgpu::Mipmapped fMipmapped;

    sk_sp<SkColorSpace> fColorSpace;

    // Memoized SkImages formed with planes, one for each Type.
    sk_sp<SkImage> fYUVImage[4];

    LazyYUVImage() = default;

    bool reset(sk_sp<SkData> data, skgpu::Mipmapped, sk_sp<SkColorSpace>);
    bool reset(SkYUVAPixmaps pixmaps, skgpu::Mipmapped, sk_sp<SkColorSpace>);

    bool ensureYUVImage(GrRecordingContext* rContext, Type type);
#if defined(SK_GRAPHITE)
    bool ensureYUVImage(skgpu::graphite::Recorder* recorder, Type type);
#endif
};

} // namespace sk_gpu_test

#endif // YUVUtils_DEFINED
