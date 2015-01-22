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

struct MeasureRecords {
    template <typename T> size_t operator()(const T& op) { return 0; }
    size_t operator()(const SkRecords::DrawPicture& op) {
        return SkPictureUtils::ApproximateBytesUsed(op.picture);
    }
};

size_t SkPictureUtils::ApproximateBytesUsed(const SkPicture* pict) {
    size_t byteCount = sizeof(*pict);

    byteCount += pict->fRecord->bytesUsed();
    if (pict->fBBH.get()) {
        byteCount += pict->fBBH->bytesUsed();
    }
    MeasureRecords visitor;
    for (unsigned curOp = 0; curOp < pict->fRecord->count(); curOp++) {
        byteCount += pict->fRecord->visit<size_t>(curOp, visitor);
    }

    return byteCount;
}
