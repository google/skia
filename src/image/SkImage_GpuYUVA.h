/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_GpuYUVA_DEFINED
#define SkImage_GpuYUVA_DEFINED

#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "src/gpu/ganesh/GrYUVATextureProxies.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_GpuBase.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>

class GrDirectContext;
class GrFragmentProcessor;
class GrImageContext;
class GrRecordingContext;
class GrSurfaceProxyView;
class SkMatrix;
enum SkColorType : int;
enum class GrColorType;
enum class GrImageTexGenPolicy : int;
enum class GrSemaphoresSubmitted : bool;
enum class SkTileMode;
struct GrFlushInfo;
struct SkRect;

namespace skgpu {
enum class Mipmapped : bool;
}

// Wraps the 1 to 4 planes of a YUVA image for consumption by the GPU.
// Initially any direct rendering will be done by passing the individual planes to a shader.
// Once any method requests a flattened image (e.g., onReadPixels), the flattened RGB
// proxy will be stored and used for any future rendering.
class SkImage_GpuYUVA final : public SkImage_GpuBase {
public:
    SkImage_GpuYUVA(sk_sp<GrImageContext>,
                    uint32_t uniqueID,
                    GrYUVATextureProxies proxies,
                    sk_sp<SkColorSpace>);

    bool onHasMipmaps() const override;

    GrSemaphoresSubmitted onFlush(GrDirectContext*, const GrFlushInfo&) const override;

    SkImage_Base::Type type() const override { return SkImage_Base::Type::kGaneshYUVA; }

    size_t onTextureSize() const override;

    using SkImage_GpuBase::onMakeColorTypeAndColorSpace;
    sk_sp<SkImage> onMakeColorTypeAndColorSpace(SkColorType, sk_sp<SkColorSpace>,
                                                GrDirectContext*) const final;

    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const final;

    bool setupMipmapsForPlanes(GrRecordingContext*) const;

private:
    enum class ColorSpaceMode {
        kConvert,
        kReinterpret,
    };
    SkImage_GpuYUVA(sk_sp<GrImageContext>,
                    const SkImage_GpuYUVA* image,
                    sk_sp<SkColorSpace> targetCS,
                    ColorSpaceMode csMode);

    std::tuple<GrSurfaceProxyView, GrColorType> onAsView(GrRecordingContext*,
                                                         skgpu::Mipmapped,
                                                         GrImageTexGenPolicy) const override;

    std::unique_ptr<GrFragmentProcessor> onAsFragmentProcessor(GrRecordingContext*,
                                                               SkSamplingOptions,
                                                               const SkTileMode[2],
                                                               const SkMatrix&,
                                                               const SkRect*,
                                                               const SkRect*) const override;

    mutable GrYUVATextureProxies     fYUVAProxies;

    // If this is non-null then the planar data should be converted from fFromColorSpace to
    // this->colorSpace(). Otherwise we assume the planar data (post YUV->RGB conversion) is already
    // in this->colorSpace().
    const sk_sp<SkColorSpace>        fFromColorSpace;

    // Repeated calls to onMakeColorSpace will result in a proliferation of unique IDs and
    // SkImage_GpuYUVA instances. Cache the result of the last successful onMakeColorSpace call.
    mutable sk_sp<SkColorSpace>      fOnMakeColorSpaceTarget;
    mutable sk_sp<SkImage>           fOnMakeColorSpaceResult;

    using INHERITED = SkImage_GpuBase;
};

#endif
