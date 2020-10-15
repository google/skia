/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkYUVAInfo_DEFINED
#define SkYUVAInfo_DEFINED

#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSize.h"
#include "include/core/SkYUVAIndex.h"

/**
 * Specifies the structure of planes for a YUV image with optional alpha. The actual planar data
 * is not part of this structure and depending on usage is in external textures or pixmaps.
 */
class SK_API SkYUVAInfo {
public:
    /**
     * Specifies how YUV (and optionally A) are divided among planes. Planes are separated by
     * underscores in the enum value names. Within each plane the pixmap/texture channels are
     * mapped to the YUVA channels in the order specified, e.g. for kY_UV Y is in channel 0 of plane
     * 0, U is in channel 0 of plane 1, and V is in channel 1 of plane 1. Channel ordering
     * within a pixmap/texture given the channels it contains:
     * A:               0:A
     * Luminance/Gray:  0:Gray
     * RG               0:R,    1:G
     * RGB              0:R,    1:G, 2:B
     * RGBA             0:R,    1:G, 2:B, 3:A
     *
     * UV subsampling is also specified in the enum value names using J:a:b notation (e.g. 4:2:0 is
     * 1/2 horizontal and 1/2 vertical resolution for U and V). A fourth number is added if alpha
     * is present (always 4 as only full resolution alpha is supported).
     *
     * Currently this only has three-plane formats but more will be added as usage and testing of
     * this expands.
     */
    enum class PlanarConfig {
        kY_U_V_444,    ///< Plane 0: Y, Plane 1: U,  Plane 2: V
        kY_U_V_422,    ///< Plane 0: Y, Plane 1: U,  Plane 2: V
        kY_U_V_420,    ///< Plane 0: Y, Plane 1: U,  Plane 2: V
        kY_V_U_420,    ///< Plane 0: Y, Plane 1: V,  Plane 2: U
        kY_U_V_440,    ///< Plane 0: Y, Plane 1: U,  Plane 2: V
        kY_U_V_411,    ///< Plane 0: Y, Plane 1: U,  Plane 2: V
        kY_U_V_410,    ///< Plane 0: Y, Plane 1: U,  Plane 2: V

        kY_U_V_A_4204, ///< Plane 0: Y, Plane 1: U,  Plane 2: V, Plane 3: A
        kY_V_U_A_4204, ///< Plane 0: Y, Plane 1: V,  Plane 2: U, Plane 3: A

        kY_UV_420,     ///< Plane 0: Y, Plane 1: UV
        kY_VU_420,     ///< Plane 0: Y, Plane 1: VU

        kY_UV_A_4204,  ///< Plane 0: Y, Plane 1: UV, Plane 2: A
        kY_VU_A_4204,  ///< Plane 0: Y, Plane 1: VU, Plane 2: A

        kYUV_444,      ///< Plane 0: YUV
        kUYV_444,      ///< Plane 0: UYV

        kYUVA_4444,    ///< Plane 0: YUVA
        kUYVA_4444,    ///< Plane 0: UYVA
    };

    /**
     * Describes how subsampled chroma values are sited relative to luma values.
     *
     * Currently only centered siting is supported but will expand to support additional sitings.
     */
    enum class Siting {
        /**
         * Subsampled chroma value is sited at the center of the block of corresponding luma values.
         */
        kCentered,
    };

    static constexpr int kMaxPlanes = 4;

    /**
     * Given image dimensions, a planar configuration, and origin, determine the expected size of
     * each plane. Returns the number of expected planes. planeDimensions[0] through
     * planeDimensons[<ret>] are written. The input image dimensions are as displayed (after the
     * planes have been transformed to the intended display orientation). The plane dimensions
     * are output as stored in memory.
     */
    static int PlaneDimensions(SkISize imageDimensions,
                               PlanarConfig,
                               SkEncodedOrigin,
                               SkISize planeDimensions[kMaxPlanes]);

    /** Number of planes for a given PlanarConfig. */
    static constexpr int NumPlanes(PlanarConfig);

    /**
     * Number of Y, U, V, A channels in the ith plane for a given PlanarConfig (or 0 if i is
     * invalid).
     */
    static constexpr int NumChannelsInPlane(PlanarConfig, int i);

    /**
     * Given a PlanarConfig and a set of channel flags for each plane, convert to SkYUVAIndex
     * representation. Fails if channel flags aren't valid for the PlanarConfig (i.e. don't have
     * enough channels in a plane).
     */
    static bool GetYUVAIndices(PlanarConfig,
                               const uint32_t planeChannelFlags[kMaxPlanes],
                               SkYUVAIndex indices[SkYUVAIndex::kIndexCount]);

    /** Does the PlanarConfig have alpha values? */
    static bool HasAlpha(PlanarConfig);

    SkYUVAInfo() = default;
    SkYUVAInfo(const SkYUVAInfo&) = default;

    /**
     * 'dimensions' should specify the size of the full resolution image (after planes have been
     * oriented to how the image is displayed as indicated by 'origin').
     */
    SkYUVAInfo(SkISize dimensions,
               PlanarConfig,
               SkYUVColorSpace,
               SkEncodedOrigin origin = kTopLeft_SkEncodedOrigin,
               Siting sitingX = Siting::kCentered,
               Siting sitingY = Siting::kCentered);

    SkYUVAInfo& operator=(const SkYUVAInfo& that) = default;

    PlanarConfig planarConfig() const { return fPlanarConfig; }

    /**
     * Dimensions of the full resolution image (after planes have been oriented to how the image
     * is displayed as indicated by fOrigin).
     */
    SkISize dimensions() const { return fDimensions; }
    int width() const { return fDimensions.width(); }
    int height() const { return fDimensions.height(); }

    SkYUVColorSpace yuvColorSpace() const { return fYUVColorSpace; }
    Siting sitingX() const { return fSitingX; }
    Siting sitingY() const { return fSitingY; }

    SkEncodedOrigin origin() const { return fOrigin; }

    bool hasAlpha() const { return HasAlpha(fPlanarConfig); }

    /**
     * Returns the number of planes and initializes planeDimensions[0]..planeDimensions[<ret>] to
     * the expected dimensions for each plane. Dimensions are as stored in memory, before
     * transformation to image display space as indicated by origin().
     */
    int planeDimensions(SkISize planeDimensions[kMaxPlanes]) const {
        return PlaneDimensions(fDimensions, fPlanarConfig, fOrigin, planeDimensions);
    }

    /**
     * Given a per-plane row bytes, determine size to allocate for all planes. Optionally retrieves
     * the per-plane byte sizes in planeSizes if not null. If total size overflows will return
     * SIZE_MAX and set all planeSizes to SIZE_MAX.
     */
    size_t computeTotalBytes(const size_t rowBytes[kMaxPlanes],
                             size_t planeSizes[kMaxPlanes] = nullptr) const;

    int numPlanes() const { return NumPlanes(fPlanarConfig); }

    int numChannelsInPlane(int i) const { return NumChannelsInPlane(fPlanarConfig, i); }

    /**
     * Given a set of channel flags for each plane, converts this->planarConfig() to SkYUVAIndex
     * representation. Fails if the channel flags aren't valid for the PlanarConfig (i.e. don't have
     * enough channels in a plane).
     */
    bool toYUVAIndices(const uint32_t channelFlags[4], SkYUVAIndex indices[4]) const {
        return GetYUVAIndices(fPlanarConfig, channelFlags, indices);
    }

    bool operator==(const SkYUVAInfo& that) const;
    bool operator!=(const SkYUVAInfo& that) const { return !(*this == that); }

private:
    SkISize fDimensions = {0, 0};

    PlanarConfig fPlanarConfig = PlanarConfig::kY_U_V_444;

    SkYUVColorSpace fYUVColorSpace = SkYUVColorSpace::kIdentity_SkYUVColorSpace;

    /**
     * YUVA data often comes from formats like JPEG that support EXIF orientation.
     * Code that operates on the raw YUV data often needs to know that orientation.
     */
    SkEncodedOrigin fOrigin = kTopLeft_SkEncodedOrigin;

    Siting fSitingX = Siting::kCentered;
    Siting fSitingY = Siting::kCentered;
};

constexpr int SkYUVAInfo::NumPlanes(PlanarConfig planarConfig) {
    switch (planarConfig) {
        case PlanarConfig::kY_U_V_444:    return 3;
        case PlanarConfig::kY_U_V_422:    return 3;
        case PlanarConfig::kY_U_V_420:    return 3;
        case PlanarConfig::kY_V_U_420:    return 3;
        case PlanarConfig::kY_U_V_440:    return 3;
        case PlanarConfig::kY_U_V_411:    return 3;
        case PlanarConfig::kY_U_V_410:    return 3;

        case PlanarConfig::kY_U_V_A_4204: return 4;
        case PlanarConfig::kY_V_U_A_4204: return 4;

        case PlanarConfig::kY_UV_420:     return 2;
        case PlanarConfig::kY_VU_420:     return 2;

        case PlanarConfig::kY_UV_A_4204:  return 3;
        case PlanarConfig::kY_VU_A_4204:  return 3;

        case PlanarConfig::kYUV_444:      return 1;
        case PlanarConfig::kUYV_444:      return 1;
        case PlanarConfig::kYUVA_4444:    return 1;
        case PlanarConfig::kUYVA_4444:    return 1;
    }
    SkUNREACHABLE;
}

constexpr int SkYUVAInfo::NumChannelsInPlane(PlanarConfig config, int i) {
    switch (config) {
        case SkYUVAInfo::PlanarConfig::kY_U_V_444:
        case SkYUVAInfo::PlanarConfig::kY_U_V_422:
        case SkYUVAInfo::PlanarConfig::kY_U_V_420:
        case SkYUVAInfo::PlanarConfig::kY_V_U_420:
        case SkYUVAInfo::PlanarConfig::kY_U_V_440:
        case SkYUVAInfo::PlanarConfig::kY_U_V_411:
        case SkYUVAInfo::PlanarConfig::kY_U_V_410:
            return i >= 0 && i < 3 ? 1 : 0;
        case SkYUVAInfo::PlanarConfig::kY_U_V_A_4204:
        case SkYUVAInfo::PlanarConfig::kY_V_U_A_4204:
            return i >= 0 && i < 4 ? 1 : 0;
        case SkYUVAInfo::PlanarConfig::kY_UV_420:
        case SkYUVAInfo::PlanarConfig::kY_VU_420:
            switch (i) {
                case 0:  return 1;
                case 1:  return 2;
                default: return 0;
            }
        case SkYUVAInfo::PlanarConfig::kY_UV_A_4204:
        case SkYUVAInfo::PlanarConfig::kY_VU_A_4204:
            switch (i) {
                case 0:  return 1;
                case 1:  return 2;
                case 2:  return 1;
                default: return 0;
            }
        case SkYUVAInfo::PlanarConfig::kYUV_444:
        case SkYUVAInfo::PlanarConfig::kUYV_444:
            return i == 0 ? 3 : 0;
        case SkYUVAInfo::PlanarConfig::kYUVA_4444:
        case SkYUVAInfo::PlanarConfig::kUYVA_4444:
            return i == 0 ? 4 : 0;
    }
    return 0;
}

#endif
