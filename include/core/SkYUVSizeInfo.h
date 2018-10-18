/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkYUVSizeInfo_DEFINED
#define SkYUVSizeInfo_DEFINED

#include "SkImageInfo.h"
#include "SkSize.h"
#include "SkYUVAIndex.h"

struct SkYUVSizeInfo {
    static constexpr auto kMaxCount = 4;

    SkColorType fColorTypes[kMaxCount];
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

    bool operator==(const SkYUVSizeInfo& that) const {
        for (int i = 0; i < kMaxCount; ++i) {
            if (fColorTypes[i] != that.fColorTypes[i]) {
                return false;
            }

            if (kUnknown_SkColorType == fColorTypes[i]) {
                SkASSERT(!fSizes[i].fWidth && !fSizes[i].fHeight && !fWidthBytes[i]);
                SkASSERT(!that.fSizes[i].fWidth && !that.fSizes[i].fHeight && !that.fWidthBytes[i]);
                continue;
            }

            SkASSERT(fSizes[i].fWidth && fSizes[i].fHeight && fWidthBytes[i]);
            if (fSizes[i] != that.fSizes[i] || fWidthBytes[i] != that.fWidthBytes[i]) {
                return false;
            }
        }

        return true;
    }

    size_t computeTotalBytes() const {
        size_t totalBytes = 0;

        for (int i = 0; i < kMaxCount; ++i) {
            SkASSERT(kUnknown_SkColorType != fColorTypes[i] ||
                        (!fSizes[i].fWidth && !fSizes[i].fHeight && !fWidthBytes[i]));

            totalBytes += fWidthBytes[i] * fSizes[i].height();
        }

        return totalBytes;
    }

    void computePlanes(void* base, void* planes[kMaxCount]) const;

};

#endif // SkYUVSizeInfo_DEFINED
