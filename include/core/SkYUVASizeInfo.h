/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkYUVASizeInfo_DEFINED
#define SkYUVASizeInfo_DEFINED

#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSize.h"

struct SK_API SkYUVASpec {
    /**
     * Specifies How YUV (and optionally A) split across planes. Within each plane the
     * pixmap/texture channels are assigned to the YUVA values in the order specified, e.g.
     * for kY_UV U is in channel 0 of plane 1 and V is in channel 1 of plane 1. Channel ordering
     * within a pixmap/texture:
     * A:               0:A
     * Luminance/Gray:  0:Gray
     * RG               0:R,    1:G
     * RGB              0:R,    1:G, 2:B
     * RGBA             0:R,    1:G, 2:B, 3:A
     *
     * UV subsampling is also specified using J:a:b notation (e.g. 4:2:0 is 1/2 horizontal and
     * 1/2 vertical resolution for U and V). Alpha is always assumed to be full resolution.
     */
    enum class Planes {
//      kYUV_444,     ///< Plane 0: YUV
//      kYVU_444,     ///< Plane 0: YVU
//      kY_UV_420,    ///< Plane 0: Y,    Plane 1: UV
//      kY_VU_420,    ///< Plane 0: Y,    Plane 1: VU
        kY_U_V_444,   ///< Plane 0: Y,    Plane 1: U,  Plane 2: V
        kY_U_V_422,   ///< Plane 0: Y,    Plane 1: U,  Plane 2: V
        kY_U_V_420,   ///< Plane 0: Y,    Plane 1: U,  Plane 2: V
        kY_U_V_440,   ///< Plane 0: Y,    Plane 1: U,  Plane 2: V
        kY_U_V_411,   ///< Plane 0: Y,    Plane 1: U,  Plane 2: V
        kY_U_V_410,   ///< Plane 0: Y,    Plane 1: U,  Plane 2: V
//      kYUVA_4444,   ///< Plane 0: YUVA
//      kYUV_A_4444,  ///< Plane 0: YUV,  Plane 1: A
//      kYVU_A_4444,  ///< Plane 0: YVU,  Plane 1: A
//      kY_UV_A_4204, ///< Plane 0: Y,    Plane 1: UV, Plane 2: A
//      kY_VU_A_4204, ///< Plane 0: Y,    Plane 1: VU, Plane 2: A
//      kY_U_V_A_4204,///< Plane 0: Y,    Plane 1: U,  Plane 2: V, Plane 3: A
    };
    // static constexpr int kMaxPlanes = 4;

    Planes fPlanes = Planes::kY_U_V_444;
    SkYUVColorSpace fYUVColorSpace = SkYUVColorSpace::kIdentity_SkYUVColorSpace;

    static int ExpectedPlaneDims(Planes, SkEncodedOrigin, SkISize imageDims, SkISize planeDims[4]);
    static int NumPlanes(Planes planes);

    int expectedPlaneDims(SkISize imageDims, SkISize planeDims[4]) const {
        return ExpectedPlaneDims(fPlanes, fOrigin, imageDims, planeDims);
    }

    int numPlanes() const { return NumPlanes(fPlanes); }

    /**
     * YUVA data often comes from formats like JPEG that support EXIF orientation.
     * Code that operates on the raw YUV data often needs to know that orientation.
     */
    SkEncodedOrigin fOrigin;
};

struct SK_API SkYUVASizeInfo {
    static constexpr auto kMaxCount = 4;

    SkISize     fSizes[kMaxCount] = {};

    /**
     * While the widths of the Y, U, V and A planes are not restricted, the
     * implementation often requires that the width of the memory allocated
     * for each plane be a multiple of 8.
     *
     * This struct allows us to inform the client how many "widthBytes"
     * that we need.  Note that we use the new idea of "widthBytes"
     * because this idea is distinct from "rowBytes" (used elsewhere in
     * Skia).  "rowBytes" allow the last row of the allocation to not
     * include any extra padding, while, in this case, every single row of
     * the allocation must be at least "widthBytes".
     */
    size_t      fWidthBytes[kMaxCount] = {};

    /**
     * YUVA data often comes from formats like JPEG that support EXIF orientation.
     * Code that operates on the raw YUV data often needs to know that orientation.
     */
    SkEncodedOrigin fOrigin = kDefault_SkEncodedOrigin;

    bool operator==(const SkYUVASizeInfo& that) const {
        for (int i = 0; i < kMaxCount; ++i) {
            SkASSERT((!fSizes[i].isEmpty() && fWidthBytes[i]) ||
                     (fSizes[i].isEmpty() && !fWidthBytes[i]));
            if (fSizes[i] != that.fSizes[i] || fWidthBytes[i] != that.fWidthBytes[i]) {
                return false;
            }
        }

        return true;
    }

    size_t computeTotalBytes() const;

    void computePlanes(void* base, void* planes[kMaxCount]) const;

};

#endif // SkYUVASizeInfo_DEFINED
