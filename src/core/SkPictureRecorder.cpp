/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBBoxHierarchyRecord.h"
#include "SkPictureRecord.h"
#include "SkPictureRecorder.h"
#include "SkRecord.h"
#include "SkRecordDraw.h"
#include "SkRecorder.h"
#include "SkTypes.h"

SkPictureRecorder::SkPictureRecorder() {}

SkPictureRecorder::~SkPictureRecorder() {}

SkCanvas* SkPictureRecorder::beginRecording(int width, int height,
                                            SkBBHFactory* bbhFactory /* = NULL */,
                                            uint32_t recordFlags /* = 0 */) {
#ifdef SK_PICTURE_USE_SK_RECORD
    return EXPERIMENTAL_beginRecording(width, height, bbhFactory);
#else
    return DEPRECATED_beginRecording(width, height, bbhFactory, recordFlags);
#endif
}

SkCanvas* SkPictureRecorder::DEPRECATED_beginRecording(int width, int height,
                                                       SkBBHFactory* bbhFactory /* = NULL */,
                                                       uint32_t recordFlags /* = 0 */) {
    fWidth = width;
    fHeight = height;

    const SkISize size = SkISize::Make(width, height);

    if (NULL != bbhFactory) {
        // We don't need to hold a ref on the BBH ourselves, but might as well for
        // consistency with EXPERIMENTAL_beginRecording(), which does need to.
        fBBH.reset((*bbhFactory)(width, height));
        SkASSERT(NULL != fBBH.get());
        fPictureRecord.reset(SkNEW_ARGS(SkBBoxHierarchyRecord, (size, recordFlags, fBBH.get())));
    } else {
        fPictureRecord.reset(SkNEW_ARGS(SkPictureRecord, (size, recordFlags)));
    }

    fPictureRecord->beginRecording();
    return this->getRecordingCanvas();
}

SkCanvas* SkPictureRecorder::EXPERIMENTAL_beginRecording(int width, int height,
                                                         SkBBHFactory* bbhFactory /* = NULL */) {
    fWidth = width;
    fHeight = height;

    if (NULL != bbhFactory) {
        fBBH.reset((*bbhFactory)(width, height));
        SkASSERT(NULL != fBBH.get());
    }

    fRecord.reset(SkNEW(SkRecord));
    fRecorder.reset(SkNEW_ARGS(SkRecorder, (fRecord.get(), width, height)));
    return this->getRecordingCanvas();
}

SkCanvas* SkPictureRecorder::getRecordingCanvas() {
    if (NULL != fRecorder.get()) {
        return fRecorder.get();
    }
    return fPictureRecord.get();
}

SkPicture* SkPictureRecorder::endRecording() {
    SkPicture* picture = NULL;

    if (NULL != fRecord.get()) {
        picture = SkNEW_ARGS(SkPicture, (fWidth, fHeight, fRecord.detach(), fBBH.get()));
    }

    if (NULL != fPictureRecord.get()) {
        fPictureRecord->endRecording();
        const bool deepCopyOps = false;
        picture = SkNEW_ARGS(SkPicture, (fWidth, fHeight, *fPictureRecord.get(), deepCopyOps));
    }

    return picture;
}

void SkPictureRecorder::internalOnly_EnableOpts(bool enableOpts) {
    if (NULL != fPictureRecord.get()) {
        fPictureRecord->internalOnly_EnableOpts(enableOpts);
    }
}

void SkPictureRecorder::partialReplay(SkCanvas* canvas) const {
    if (NULL == canvas) {
        return;
    }

    if (NULL != fRecord.get()) {
        SkRecordDraw(*fRecord, canvas, NULL/*bbh*/, NULL/*callback*/);
    }

    if (NULL != fPictureRecord.get()) {
        const bool deepCopyOps = true;
        SkPicture picture(fWidth, fHeight, *fPictureRecord.get(), deepCopyOps);
        picture.draw(canvas);
    }
}
