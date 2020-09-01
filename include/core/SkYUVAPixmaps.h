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
#include <bitset>

class GrImageContext;
struct SkYUVASizeInfo;
struct SkYUVAIndex;

/**
 * SkYUVAInfo combined with per-plane SkColorTypes and row bytes. Fully specifies the SkPixmaps
 * for a YUVA image without the actual pixel memory and data.
 */
class SK_API SkYUVAPixmapInfo {
public:
    static constexpr auto kMaxPlanes = SkYUVAInfo::kMaxPlanes;

    using PlanarConfig = SkYUVAInfo::PlanarConfig;

    /**
     * Data type for Y, U, V, and possibly A channels independent of how values are packed into
     * planes.
     **/
    enum class DataType {
        kUnorm8,   ///< 8 bit unsigned normalized
        kUnorm16,  ///< 16 bit unsigned normalized
        kFloat16,  ///< 16 bit (half) floating point

        kLast = kFloat16
    };
    static constexpr int kDataTypeCnt = static_cast<int>(DataType::kLast) + 1;

    class SupportedDataTypes {
    public:
        /** Defaults to nothing supported. */
        constexpr SupportedDataTypes() = default;

        /** Init based on texture formats supported by the context. */
        SupportedDataTypes(const GrImageContext&);

        /** All combinations of PlanarConfig and DataType are supported. */
        static constexpr SupportedDataTypes All() {
            SupportedDataTypes combinations;
            combinations.fDataTypeSupport = std::bitset<kDataTypeCnt>(~(0ULL));
            return combinations;
        }

        constexpr bool supported(PlanarConfig, DataType type) const {
            return fDataTypeSupport[static_cast<size_t>(type)];
        }

        /**
         * Update to add support for pixmaps with numChannel channels where each channel is
         * represented as DataType.
         */
        void enableDataType(DataType, int numChannels);

    private:
        // Because all of our PlanarConfigs are currently single channel per-plane, we just need to
        // keep track of whether each data type is supported (implicitly as a single channel plane).
        // As we add multi-channel per-plane PlanarConfigs this will have to change.
        std::bitset<kDataTypeCnt> fDataTypeSupport = {};
    };

    /**
     * Gets the default SkColorType to use with numChannels channels, each represented as DataType.
     * Returns kUnknown_SkColorType if no such color type.
     */
    static SkColorType DefaultColorTypeForDataType(DataType dataType, int numChannels);

    /**
     * If the SkColorType is supported for YUVA pixmaps this will return the number of YUVA channels
     * that can be stored in a plane of this color type and what the DataType is of those channels.
     * If the SkColorType is not supported as a YUVA plane the number of channels is reported as 0
     * and the DataType returned should be ignored.
     */
    static std::tuple<int, DataType> NumChannelsAndDataType(SkColorType);

    /** Default SkYUVAPixmapInfo is invalid. */
    SkYUVAPixmapInfo() = default;

    /**
     * Initializes the SkYUVAPixmapInfo from a SkYUVAInfo with per-plane color types and row bytes.
     * This will be invalid if the colorTypes aren't compatible with the SkYUVAInfo or if a
     * rowBytes entry is not valid for the plane dimensions and color type. Color type and
     * row byte values beyond the number of planes in SkYUVAInfo are ignored. All SkColorTypes
     * must have the same DataType or this will be invalid.
     *
     * If rowBytes is nullptr then bpp*width is assumed for each plane.
     */
    SkYUVAPixmapInfo(const SkYUVAInfo&,
                     const SkColorType[kMaxPlanes],
                     const size_t rowBytes[kMaxPlanes]);
    /**
     * Like above but uses DefaultColorTypeForDataType to determine each plane's SkColorType. If
     * rowBytes is nullptr then bpp*width is assumed for each plane.
     */
    SkYUVAPixmapInfo(const SkYUVAInfo&, DataType, const size_t rowBytes[kMaxPlanes]);

    SkYUVAPixmapInfo(const SkYUVAPixmapInfo&) = default;

    SkYUVAPixmapInfo& operator=(const SkYUVAPixmapInfo&) = default;

    bool operator==(const SkYUVAPixmapInfo&) const;
    bool operator!=(const SkYUVAPixmapInfo& that) const { return !(*this == that); }

    const SkYUVAInfo& yuvaInfo() const { return fYUVAInfo; }

    SkYUVColorSpace yuvColorSpace() const { return fYUVAInfo.yuvColorSpace(); }

    /** The number of SkPixmap planes, 0 if this SkYUVAPixmapInfo is invalid. */
    int numPlanes() const { return this->isValid() ? fYUVAInfo.numPlanes() : 0; }

    /** The per-YUV[A] channel data type. */
    DataType dataType() const { return fDataType; }

    /**
     * Row bytes for the ith plane. Returns zero if i >= numPlanes() or this SkYUVAPixmapInfo is
     * invalid.
     */
    size_t rowBytes(int i) const { return fRowBytes[static_cast<size_t>(i)]; }

    /** Image info for the ith plane, or default SkImageInfo if i >= numPlanes() */
    const SkImageInfo& planeInfo(int i) const { return fPlaneInfos[static_cast<size_t>(i)]; }

    /**
     * Determine size to allocate for all planes. Optionally retrieves the per-plane sizes in
     * planeSizes if not null. If total size overflows will return SIZE_MAX and set all planeSizes
     * to SIZE_MAX. Returns 0 and fills planesSizes with 0 if this SkYUVAPixmapInfo is not valid.
     */
    size_t computeTotalBytes(size_t planeSizes[kMaxPlanes] = nullptr) const;

    /**
     * Takes an allocation that is assumed to be at least computeTotalBytes() in size and configures
     * the first numPlanes() entries in pixmaps array to point into that memory. The remaining
     * entries of pixmaps are default initialized. Fails if this SkYUVAPixmapInfo not valid.
     */
    bool initPixmapsFromSingleAllocation(void* memory, SkPixmap pixmaps[kMaxPlanes]) const;

    /**
     * Returns true if this has been configured with a non-empty dimensioned SkYUVAInfo with
     * compatible color types and row bytes.
     */
    bool isValid() const { return fPlaneInfos[0].colorType() != kUnknown_SkColorType; }

    /** Is this valid and does it use color types allowed by the passed SupportedDataTypes? */
    bool isSupported(const SupportedDataTypes&) const;

private:
    SkYUVAInfo fYUVAInfo;
    std::array<SkImageInfo, kMaxPlanes> fPlaneInfos = {};
    std::array<size_t, kMaxPlanes> fRowBytes = {};
    DataType fDataType = DataType::kUnorm8;
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
     * SkYUVAPixmaps.
     */
    static SkYUVAPixmaps FromData(const SkYUVAPixmapInfo&, sk_sp<SkData>);

    /**
     * Use passed in memory as backing store for pixmaps' pixels. Caller must ensure memory remains
     * allocated while pixmaps are in use. There must be at least
     * SkYUVAPixmapInfo::computeTotalBytes() allocated starting at memory.
     */
    static SkYUVAPixmaps FromExternalMemory(const SkYUVAPixmapInfo&, void* memory);

    /**
     * Wraps existing SkPixmaps. The SkYUVAPixmaps will have no ownership of the SkPixmaps' pixel
     * memory so the caller must ensure it remains valid. Will return an invalid SkYUVAPixmaps if
     * the SkYUVAInfo isn't compatible with the SkPixmap array (number of planes, plane dimensions,
     * sufficient color channels in planes, ...).
     */
    static SkYUVAPixmaps FromExternalPixmaps(const SkYUVAInfo&, const SkPixmap[kMaxPlanes]);

    /** Default SkYUVAPixmaps is invalid. */
    SkYUVAPixmaps() = default;
    ~SkYUVAPixmaps() = default;

    SkYUVAPixmaps(SkYUVAPixmaps&& that) = default;
    SkYUVAPixmaps& operator=(SkYUVAPixmaps&& that) = default;
    SkYUVAPixmaps(const SkYUVAPixmaps&) = default;
    SkYUVAPixmaps& operator=(const SkYUVAPixmaps& that) = default;

    /** Does have initialized pixmaps compatible with its SkYUVAInfo. */
    bool isValid() const { return !fYUVAInfo.dimensions().isEmpty(); }

    const SkYUVAInfo& yuvaInfo() const { return fYUVAInfo; }

    /** Number of pixmap planes or 0 if this SkYUVAPixmaps is invalid. */
    int numPlanes() const { return this->isValid() ? fYUVAInfo.numPlanes() : 0; }

    /**
     * Access the SkPixmap planes. They are default initialized if this is not a valid
     * SkYUVAPixmaps.
     */
    const std::array<SkPixmap, kMaxPlanes>& planes() const { return fPlanes; }

    /**
     * Get the ith SkPixmap plane. SkPixmap will be default initialized if i >= numPlanes or this
     * SkYUVAPixmaps is invalid.
     */
    const SkPixmap& plane(int i) const { return fPlanes[SkToSizeT(i)]; }

    /**
     * Conversion to legacy SkYUVA data structures.
     */
    bool toLegacy(SkYUVASizeInfo*, SkYUVAIndex[4]);

private:
    SkYUVAPixmaps(const SkYUVAPixmapInfo&, sk_sp<SkData>);
    SkYUVAPixmaps(const SkYUVAInfo&, const SkPixmap[kMaxPlanes]);

    SkYUVAInfo fYUVAInfo;
    std::array<SkPixmap, kMaxPlanes> fPlanes = {};
    sk_sp<SkData> fData;
};

#endif
