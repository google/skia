/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef YUVUtils_DEFINED
#define YUVUtils_DEFINED

#include "include/core/SkImage.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/gpu/GrBackendSurface.h"
#include "src/core/SkAutoMalloc.h"

#include <tuple>

class SkData;

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
                                              GrMipmapped = GrMipmapped::kNo,
                                              sk_sp<SkColorSpace> = nullptr);
    static std::unique_ptr<LazyYUVImage> Make(SkYUVAPixmaps,
                                              GrMipmapped = GrMipmapped::kNo,
                                              sk_sp<SkColorSpace> = nullptr);

    enum class Type { kFromPixmaps, kFromGenerator, kFromTextures };

    SkISize dimensions() const { return fPixmaps.yuvaInfo().dimensions(); }

    sk_sp<SkImage> refImage(GrRecordingContext* rContext, Type);

private:
    // Decoded YUV data
    SkYUVAPixmaps fPixmaps;

    GrMipmapped fMipmapped;

    sk_sp<SkColorSpace> fColorSpace;

    // Memoized SkImages formed with planes, one for each Type.
    sk_sp<SkImage> fYUVImage[4];

    LazyYUVImage() = default;

    bool reset(sk_sp<SkData> data, GrMipmapped, sk_sp<SkColorSpace>);
    bool reset(SkYUVAPixmaps pixmaps, GrMipmapped, sk_sp<SkColorSpace>);

    bool ensureYUVImage(GrRecordingContext* rContext, Type type);
};

} // namespace sk_gpu_test

#endif // YUVUtils_DEFINED
