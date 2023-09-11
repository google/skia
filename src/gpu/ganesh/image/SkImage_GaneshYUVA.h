/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_GaneshYUVA_DEFINED
#define SkImage_GaneshYUVA_DEFINED

#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "src/gpu/ganesh/GrYUVATextureProxies.h"
#include "src/gpu/ganesh/image/SkImage_GaneshBase.h"
#include "src/image/SkImage_Base.h"

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
enum GrSurfaceOrigin : int;
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
class SkImage_GaneshYUVA final : public SkImage_GaneshBase {
public:
    SkImage_GaneshYUVA(sk_sp<GrImageContext>,
                       uint32_t uniqueID,
                       GrYUVATextureProxies proxies,
                       sk_sp<SkColorSpace>);

    // From SkImage.h
    size_t textureSize() const override;

    // From SkImage_Base.h
    SkImage_Base::Type type() const override { return SkImage_Base::Type::kGaneshYUVA; }
    bool onHasMipmaps() const override;
    bool onIsProtected() const override;

    using SkImage_GaneshBase::onMakeColorTypeAndColorSpace;
    sk_sp<SkImage> onMakeColorTypeAndColorSpace(SkColorType,
                                                sk_sp<SkColorSpace>,
                                                GrDirectContext*) const final;

    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const final;

    // From SkImage_GaneshBase.h
    GrSemaphoresSubmitted flush(GrDirectContext*, const GrFlushInfo&) const override;

   std::tuple<GrSurfaceProxyView, GrColorType> asView(GrRecordingContext*,
                                                       skgpu::Mipmapped,
                                                       GrImageTexGenPolicy) const override;

    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(GrRecordingContext*,
                                                             SkSamplingOptions,
                                                             const SkTileMode[2],
                                                             const SkMatrix&,
                                                             const SkRect*,
                                                             const SkRect*) const override;

    bool setupMipmapsForPlanes(GrRecordingContext*) const;

    GrSurfaceOrigin origin() const override { return fYUVAProxies.textureOrigin(); }

private:
    enum class ColorSpaceMode {
        kConvert,
        kReinterpret,
    };
    SkImage_GaneshYUVA(sk_sp<GrImageContext>,
                       const SkImage_GaneshYUVA* image,
                       sk_sp<SkColorSpace> targetCS,
                       ColorSpaceMode csMode);

    mutable GrYUVATextureProxies fYUVAProxies;

    // If this is non-null then the planar data should be converted from fFromColorSpace to
    // this->colorSpace(). Otherwise we assume the planar data (post YUV->RGB conversion) is already
    // in this->colorSpace().
    const sk_sp<SkColorSpace> fFromColorSpace;

    // Repeated calls to onMakeColorSpace will result in a proliferation of unique IDs and
    // SkImage_GaneshYUVA instances. Cache the result of the last successful onMakeColorSpace call.
    mutable sk_sp<SkColorSpace> fOnMakeColorSpaceTarget;
    mutable sk_sp<SkImage> fOnMakeColorSpaceResult;

    using INHERITED = SkImage_GaneshBase;
};

#endif
