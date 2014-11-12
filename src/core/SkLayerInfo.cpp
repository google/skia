/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLayerInfo.h"

SkPicture::AccelData::Key SkLayerInfo::ComputeKey() {
    static const SkPicture::AccelData::Key gGPUID = SkPicture::AccelData::GenerateDomain();

    return gGPUID;
}

