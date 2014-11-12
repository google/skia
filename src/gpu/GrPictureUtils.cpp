/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPictureUtils.h"

#include "SkBBoxHierarchy.h"
#include "SkPaintPriv.h"
#include "SkPatchUtils.h"
#include "SkRecord.h"
#include "SkRecords.h"

SkPicture::AccelData::Key GrAccelData::ComputeAccelDataKey() {
    static const SkPicture::AccelData::Key gGPUID = SkPicture::AccelData::GenerateDomain();

    return gGPUID;
}

