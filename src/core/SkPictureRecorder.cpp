/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPictureRecorder.h"
#include "SkRecord.h"
#include "SkRecordDraw.h"
#include "SkRecorder.h"
#include "SkTypes.h"

SkPictureRecorder::SkPictureRecorder() {}

SkPictureRecorder::~SkPictureRecorder() {}

SkCanvas* SkPictureRecorder::beginRecording(SkScalar width, SkScalar height,
                                            SkBBHFactory* bbhFactory /* = NULL */,
                                            uint32_t recordFlags /* = 0 */) {
    fCullWidth = width;
    fCullHeight = height;

    if (bbhFactory) {
        fBBH.reset((*bbhFactory)(width, height));
        SkASSERT(fBBH.get());
    }

    fRecord.reset(SkNEW(SkRecord));
    fRecorder.reset(SkNEW_ARGS(SkRecorder, (fRecord.get(), width, height)));
    return this->getRecordingCanvas();
}

SkCanvas* SkPictureRecorder::getRecordingCanvas() {
    return fRecorder.get();
}

SkPicture* SkPictureRecorder::endRecording() {
    return SkNEW_ARGS(SkPicture, (fCullWidth, fCullHeight, fRecord.detach(), fBBH.get()));
}

void SkPictureRecorder::partialReplay(SkCanvas* canvas) const {
    if (NULL == canvas) {
        return;
    }
    SkRecordDraw(*fRecord, canvas, NULL/*bbh*/, NULL/*callback*/);
}
