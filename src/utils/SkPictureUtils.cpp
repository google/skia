/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBBoxHierarchy.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkPictureUtils.h"
#include "SkRecord.h"
#include "SkShader.h"

size_t SkPictureUtils::ApproximateBytesUsed(const SkPicture* pict) {
    size_t byteCount = sizeof(*pict);

    byteCount += pict->fRecord->bytesUsed();
    if (pict->fBBH.get()) {
        byteCount += pict->fBBH->bytesUsed();
    }
    byteCount += pict->fApproxBytesUsedBySubPictures;

    return byteCount;
}
