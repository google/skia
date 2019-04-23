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

struct SK_API SkYUVASizeInfo {
    static constexpr auto kMaxCount = 4;

    SkISize     fSizes[kMaxCount];

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
    size_t      fWidthBytes[kMaxCount];

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

    size_t computeTotalBytes() const {
        size_t totalBytes = 0;

        for (int i = 0; i < kMaxCount; ++i) {
            SkASSERT((!fSizes[i].isEmpty() && fWidthBytes[i]) ||
                     (fSizes[i].isEmpty() && !fWidthBytes[i]));
            totalBytes += fWidthBytes[i] * fSizes[i].height();
        }

        return totalBytes;
    }

    void computePlanes(void* base, void* planes[kMaxCount]) const;

};

#endif // SkYUVASizeInfo_DEFINED
