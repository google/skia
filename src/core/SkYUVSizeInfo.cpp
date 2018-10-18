/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkYUVSizeInfo.h"
#include "SkTemplates.h"

void SkYUVSizeInfo::computePlanes(void* base, void* planes[SkYUVSizeInfo::kMaxCount]) const {
    planes[0] = base;
    int i = 1;
    for (; i < SkYUVSizeInfo::kMaxCount; ++i) {
        if (fSizes[i].isEmpty()) {
            break;
        }
        planes[i] = SkTAddOffset<void>(planes[i - 1], fWidthBytes[i - 1] * fSizes[i - 1].height());
    }
    for (; i < SkYUVSizeInfo::kMaxCount; ++i) {
        planes[i] = nullptr;
    }
}
