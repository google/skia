/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_RasterPinnable_DEFINED
#define SkImage_RasterPinnable_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Raster.h"

#include <cstdint>
#include <memory>
#include <tuple>

class GrRecordingContext;
class SkBitmap;
enum class GrImageTexGenPolicy : int;

namespace skgpu {
enum class Mipmapped : bool;
}

struct PinnedData {
    GrSurfaceProxyView fPinnedView;
    int32_t fPinnedCount = 0;
    uint32_t fPinnedUniqueID = SK_InvalidUniqueID;
    uint32_t fPinnedContextID = SK_InvalidUniqueID;
    GrColorType fPinnedColorType = GrColorType::kUnknown;
};

class SkImage_RasterPinnable final : public SkImage_Raster {
public:
    SkImage_RasterPinnable(const SkBitmap& bm)
            : SkImage_Raster(bm, /*bitmapMayBeMutable = */ true) {}

    std::tuple<GrSurfaceProxyView, GrColorType> asView(GrRecordingContext*,
                                                       skgpu::Mipmapped,
                                                       GrImageTexGenPolicy) const;

    SkImage_Base::Type type() const override { return SkImage_Base::Type::kRasterPinnable; }

    std::unique_ptr<PinnedData> fPinnedData;
};

#endif
