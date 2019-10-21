/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkYUVASizeInfo.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkSafeMath.h"

size_t SkYUVASizeInfo::computeTotalBytes() const {
    SkSafeMath safe;
    size_t totalBytes = 0;

    for (int i = 0; i < kMaxCount; ++i) {
        SkASSERT((!fSizes[i].isEmpty() && fWidthBytes[i]) ||
                 (fSizes[i].isEmpty() && !fWidthBytes[i]));
        totalBytes = safe.add(totalBytes, safe.mul(fWidthBytes[i], fSizes[i].height()));
    }

    return safe.ok() ? totalBytes : SIZE_MAX;
}

void SkYUVASizeInfo::computePlanes(void* base, void* planes[SkYUVASizeInfo::kMaxCount]) const {
    planes[0] = base;
    int i = 1;
    for (; i < SkYUVASizeInfo::kMaxCount; ++i) {
        if (fSizes[i].isEmpty()) {
            break;
        }
        planes[i] = SkTAddOffset<void>(planes[i - 1], fWidthBytes[i - 1] * fSizes[i - 1].height());
    }
    for (; i < SkYUVASizeInfo::kMaxCount; ++i) {
        planes[i] = nullptr;
    }
}
