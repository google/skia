/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkYUVASizeInfo.h"
#include "SkTemplates.h"

void SkYUVASizeInfo::computePlanes(void* base, void* planes[4]) const {
    planes[0] = base;
    for (int i = 1; i < 4; ++i) {
        planes[i] = SkTAddOffset<void>(planes[i-1], fWidthBytes1[i-1] * fSizes[i-1].height());
    }
}
