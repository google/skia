/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkYUVASizeInfo.h"
#include "SkTemplates.h"

void SkYUVASizeInfo::computePlanes(void* base, void* planes[SkYUVASizeInfo::kMaxCount]) const {
    planes[0] = base;
    for (int i = 0; i <= SkYUVASizeInfo::kMaxCount; ++i) {
        planes[i] = SkTAddOffset<void>(planes[i-1], fWidthBytes[i-1] * fSizes[i-1].height());
    }
}
