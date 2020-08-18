/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkYUVASpec_DEFINED
#define SkYUVASpec_DEFINED

#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSize.h"

/**
 * Specifies the structure of planes for a YUV image with optional alpha. The actual planar data
 * is not part of this structure and depending on usage is in external textures or pixmaps.
 */
struct SK_API SkYUVASpec {
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
    enum class Planes {
        kY_U_V_444,  ///< Plane 0: Y, Plane 1: U, Plane 2: V
        kY_U_V_422,  ///< Plane 0: Y, Plane 1: U, Plane 2: V
        kY_U_V_420,  ///< Plane 0: Y, Plane 1: U, Plane 2: V
        kY_U_V_440,  ///< Plane 0: Y, Plane 1: U, Plane 2: V
        kY_U_V_411,  ///< Plane 0: Y, Plane 1: U, Plane 2: V
        kY_U_V_410,  ///< Plane 0: Y, Plane 1: U, Plane 2: V
    };

    /**
     * Describes how subsampled chroma values are sited relative to luma values.
     *
     * Currently only centered siting is supported but will expand to support additional sitings.
     */
    enum class Siting {
        /**
         * Subsampled chroma value is sited at the center of the block of corresponding luma
         */
        kCentered,
    };

    static constexpr int kMaxPlanes = 4;

    /**
     * Given a plane configuration, origin, and image dimensions, determine the expected size of
     * each plane. Returns the number of expected planes. planeDims[0] through planeDims[<ret>] are
     * written. The input image dimensions are as displayed (after the planes have been rotated to
     * the intended display orientation).
     */
    static int ExpectedPlaneDims(Planes planes,
                                 SkEncodedOrigin origin,
                                 SkISize imageDims,
                                 SkISize planeDims[kMaxPlanes]);
    static int NumPlanes(Planes planes);

    /**
     * Returns the number of planes and inits planeDims[0]..planeDims[<ret>] to the expected
     * dimensions for each plane.
     */
    int expectedPlaneDims(SkISize planeDims[kMaxPlanes]) const {
        return ExpectedPlaneDims(fPlanes, fOrigin, fDimensions, planeDims);
    }

    int numPlanes() const { return NumPlanes(fPlanes); }

    Planes fPlanes = Planes::kY_U_V_444;
    SkYUVColorSpace fYUVColorSpace = SkYUVColorSpace::kIdentity_SkYUVColorSpace;

    /**
     * Dimensions of the full resolution image (after planes have been oriented to how the image
     * is displayed as indicated by fOrigin).
     */
    SkISize fDimensions = {0, 0};

    Siting fSitingX = Siting::kCentered;
    Siting fSitingY = Siting::kCentered;

    /**
     * YUVA data often comes from formats like JPEG that support EXIF orientation.
     * Code that operates on the raw YUV data often needs to know that orientation.
     */
    SkEncodedOrigin fOrigin = kTopLeft_SkEncodedOrigin;
};

#endif
