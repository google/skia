/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkYUVAPixmaps_DEFINED
#define SkYUVAPixmaps_DEFINED

#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkYUVAInfo.h"
#include "include/private/SkTo.h"

#include <array>

struct SkYUVASizeInfo;
struct SkYUVAIndex;

/**
 * SkYUVAInfo combined with per-plane SkColorTypes and row bytes. Fully specifies the SkPixmaps
 * for a YUVA image without the actual pixel memory and data.
 */
class SK_API SkYUVAPixmapInfo {
public:
    static constexpr auto kMaxPlanes = SkYUVAInfo::kMaxPlanes;

    SkYUVAPixmapInfo() = default;
    SkYUVAPixmapInfo(const SkYUVAInfo& yuvaInfo,
                     const SkColorType colorTypes[kMaxPlanes],
                     const size_t rowBytes[kMaxPlanes]);
    SkYUVAPixmapInfo(const SkYUVAPixmapInfo&) = default;

    SkYUVAPixmapInfo& operator=(const SkYUVAPixmapInfo&) = default;

    const SkYUVAInfo& yuvaInfo() const { return fYUVAInfo; }

    SkYUVColorSpace yuvColorSpace() const { return fYUVAInfo.yuvColorSpace(); }

    int numPlanes() const { return fYUVAInfo.numPlanes(); }

    size_t rowBytes(int i) const { return fRowBytes[i]; }

    const SkImageInfo& planeInfo(int i) const { return fPlaneInfos[i]; }

    size_t computeTotalBytes(size_t planeSizes[kMaxPlanes] = nullptr) const;

    bool initPixmapsFromSingleAllocation(void* memory, SkPixmap pixmaps[kMaxPlanes]) const;

    bool isValid() const { return fPlaneInfos[0].colorType() != kUnknown_SkColorType; }

private:
    SkYUVAInfo fYUVAInfo;
    SkImageInfo fPlaneInfos[kMaxPlanes] = {};
    size_t fRowBytes[kMaxPlanes] = {};
    static_assert(kUnknown_SkColorType == 0, "default init isn't kUnknown");
};

/**
 * Helper to store SkPixmap planes as described by a SkYUVAPixmapInfo. Can be responsible for
 * allocating/freeing memory for pixmaps or use external memory.
 */
class SK_API SkYUVAPixmaps {
public:
    static constexpr auto kMaxPlanes = SkYUVAPixmapInfo::kMaxPlanes;

    /** Allocate space for pixmaps' pixels in the SkYUVAPixmaps. */
    static SkYUVAPixmaps Allocate(const SkYUVAPixmapInfo& yuvaPixmapInfo);
    /**
     * Use storage in SkData as backing store for pixmaps' pixels. SkData is retained by the
     * SkYUVAPixmapes.
     */
    static SkYUVAPixmaps FromData(const SkYUVAPixmapInfo&, sk_sp<SkData>);

    /**
     * Use passed in memory as backing store for pixmaps' pixels. Caller must ensure memory remains
     * allocated while pixmaps are in use. There must be at least
     * SkYUVAPixmapInfo::computeTotalBytes() allocated starting at memory.
     */
    static SkYUVAPixmaps FromExternalMemory(const SkYUVAPixmapInfo&, void* memory);

    /**
     * Wraps existing SkPixmaps. The SkYUVAPixmaps will have no ownership of the pixel memory so the
     * caller must ensure it remains valid. Will return an invalid SkYUVAPixmaps if the SkYUVAInfo
     * isn't compatible with the SkPixmap array (number of planes, plane dimensions,
     * sufficient color channels in planes, ...).
     */
    static SkYUVAPixmaps FromExternalPixmaps(const SkYUVAInfo&, const SkPixmap[kMaxPlanes]);

    SkYUVAPixmaps() = default;
    ~SkYUVAPixmaps() = default;

    SkYUVAPixmaps(SkYUVAPixmaps&& that) = default;
    SkYUVAPixmaps& operator=(SkYUVAPixmaps&& that) = default;
    SkYUVAPixmaps(const SkYUVAPixmaps&) = default;
    SkYUVAPixmaps& operator=(const SkYUVAPixmaps& that) = default;

    bool isValid() const { return !fYUVAInfo.dimensions().isEmpty(); }
    const SkYUVAInfo& yuvaInfo() const { return fYUVAInfo; }
    int numPlanes() const { return fYUVAInfo.numPlanes(); }
    const std::array<SkPixmap, kMaxPlanes>& planes() const { return fPlanes; }
    const SkPixmap& plane(int i) const { return fPlanes[SkToSizeT(i)]; }

    bool toLegacy(SkYUVASizeInfo*, SkYUVAIndex[4]);

private:
    SkYUVAPixmaps(const SkYUVAPixmapInfo&, sk_sp<SkData>);
    SkYUVAPixmaps(const SkYUVAInfo&, const SkPixmap[kMaxPlanes]);

    SkYUVAInfo fYUVAInfo;
    std::array<SkPixmap, kMaxPlanes> fPlanes;
    sk_sp<SkData> fData;
};

#endif
