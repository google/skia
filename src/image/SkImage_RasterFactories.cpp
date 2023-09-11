/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkMath.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkImagePriv.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Raster.h"

#include <cstddef>
#include <cstdint>
#include <utility>

class SkImageFilter;
struct SkIPoint;
struct SkIRect;
enum class SkTextureCompressionType;

namespace skif {
Functors MakeRasterFunctors();
} // namespace skif

static bool valid_args(const SkImageInfo& info, size_t rowBytes, size_t* minSize) {
    const int maxDimension = SK_MaxS32 >> 2;

    // TODO(mtklein): eliminate anything here that setInfo() has already checked.
    SkBitmap b;
    if (!b.setInfo(info, rowBytes)) {
        return false;
    }

    if (info.width() <= 0 || info.height() <= 0) {
        return false;
    }
    if (info.width() > maxDimension || info.height() > maxDimension) {
        return false;
    }
    if ((unsigned)info.colorType() > (unsigned)kLastEnum_SkColorType) {
        return false;
    }
    if ((unsigned)info.alphaType() > (unsigned)kLastEnum_SkAlphaType) {
        return false;
    }

    if (kUnknown_SkColorType == info.colorType()) {
        return false;
    }
    if (!info.validRowBytes(rowBytes)) {
        return false;
    }

    size_t size = info.computeByteSize(rowBytes);
    if (SkImageInfo::ByteSizeOverflowed(size)) {
        return false;
    }

    if (minSize) {
        *minSize = size;
    }
    return true;
}

namespace SkImages {
sk_sp<SkImage> RasterFromBitmap(const SkBitmap& bm) {
    if (!bm.pixelRef()) {
        return nullptr;
    }

    return SkMakeImageFromRasterBitmap(bm, kIfMutable_SkCopyPixelsMode);
}

sk_sp<SkImage> RasterFromPixmapCopy(const SkPixmap& pmap) {
    return MakeRasterCopyPriv(pmap, kNeedNewImageUniqueID);
}

sk_sp<SkImage> RasterFromData(const SkImageInfo& info, sk_sp<SkData> data, size_t rowBytes) {
    size_t size;
    if (!valid_args(info, rowBytes, &size) || !data) {
        return nullptr;
    }

    // did they give us enough data?
    if (data->size() < size) {
        return nullptr;
    }

    return sk_make_sp<SkImage_Raster>(info, std::move(data), rowBytes);
}

sk_sp<SkImage> MakeWithFilter(sk_sp<SkImage> src,
                              const SkImageFilter* filter,
                              const SkIRect& subset,
                              const SkIRect& clipBounds,
                              SkIRect* outSubset,
                              SkIPoint* offset) {
    if (!src || !filter) {
        return nullptr;
    }

    return as_IFB(filter)->makeImageWithFilter(skif::MakeRasterFunctors(),
                                               std::move(src),
                                               subset,
                                               clipBounds,
                                               outSubset,
                                               offset);
}

// TODO: this could be improved to decode and make use of the mipmap
// levels potentially present in the compressed data. For now, any
// mipmap levels are discarded.
sk_sp<SkImage> RasterFromCompressedTextureData(sk_sp<SkData> data,
                                               int width,
                                               int height,
                                               SkTextureCompressionType type) {
    size_t expectedSize = SkCompressedFormatDataSize(type, {width, height}, false);
    if (!data || data->size() < expectedSize) {
        return nullptr;
    }

    SkAlphaType at =
            SkTextureCompressionTypeIsOpaque(type) ? kOpaque_SkAlphaType : kPremul_SkAlphaType;

    SkImageInfo ii = SkImageInfo::MakeN32(width, height, at);

    if (!valid_args(ii, ii.minRowBytes(), nullptr)) {
        return nullptr;
    }

    SkBitmap bitmap;
    if (!bitmap.tryAllocPixels(ii)) {
        return nullptr;
    }

    if (!SkDecompress(std::move(data), {width, height}, type, &bitmap)) {
        return nullptr;
    }

    bitmap.setImmutable();
    return RasterFromBitmap(bitmap);
}

sk_sp<SkImage> RasterFromPixmap(const SkPixmap& pmap, RasterReleaseProc proc, ReleaseContext ctx) {
    size_t size;
    if (!valid_args(pmap.info(), pmap.rowBytes(), &size) || !pmap.addr()) {
        return nullptr;
    }

    sk_sp<SkData> data(SkData::MakeWithProc(pmap.addr(), size, proc, ctx));
    return sk_make_sp<SkImage_Raster>(pmap.info(), std::move(data), pmap.rowBytes());
}

}  // namespace SkImages

sk_sp<SkImage> MakeRasterCopyPriv(const SkPixmap& pmap, uint32_t id) {
    size_t size;
    if (!valid_args(pmap.info(), pmap.rowBytes(), &size) || !pmap.addr()) {
        return nullptr;
    }

    // Here we actually make a copy of the caller's pixel data
    sk_sp<SkData> data(SkData::MakeWithCopy(pmap.addr(), size));
    return sk_make_sp<SkImage_Raster>(pmap.info(), std::move(data), pmap.rowBytes(), id);
}
