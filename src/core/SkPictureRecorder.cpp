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
    fWidth = width;
    fHeight = height;

    const SkISize size = SkISize::Make(width, height);

    if (NULL != bbhFactory) {
        SkAutoTUnref<SkBBoxHierarchy> tree((*bbhFactory)(width, height));
        SkASSERT(NULL != tree);
        fPictureRecord.reset(SkNEW_ARGS(SkBBoxHierarchyRecord, (size, recordFlags, tree.get())));
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

    // TODO: plumb bbhFactory through
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
        picture = SkNEW_ARGS(SkPicture, (fWidth, fHeight, fRecord.detach()));
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
        SkRecordDraw(*fRecord, canvas);
    }

    if (NULL != fPictureRecord.get()) {
        const bool deepCopyOps = true;
        SkPicture picture(fWidth, fHeight, *fPictureRecord.get(), deepCopyOps);
        picture.draw(canvas);
    }
}
