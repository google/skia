/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBBoxHierarchyRecord.h"
#include "SkPicturePlayback.h"
#include "SkPictureRecord.h"
#include "SkPictureRecorder.h"
#include "SkRecord.h"
#include "SkRecordDraw.h"
#include "SkRecorder.h"
#include "SkTypes.h"

SkPictureRecorder::~SkPictureRecorder() {
    this->reset();
}

void SkPictureRecorder::reset() {
    SkSafeSetNull(fPictureRecord);
    SkSafeSetNull(fRecorder);
    SkDELETE(fRecord);
    fRecord = NULL;
}

SkCanvas* SkPictureRecorder::beginRecording(int width, int height,
                                            SkBBHFactory* bbhFactory /* = NULL */,
                                            uint32_t recordFlags /* = 0 */) {
    this->reset();  // terminate any prior recording(s)
    fWidth = width;
    fHeight = height;

    const SkISize size = SkISize::Make(width, height);

    if (NULL != bbhFactory) {
        SkAutoTUnref<SkBBoxHierarchy> tree((*bbhFactory)(width, height));
        SkASSERT(NULL != tree);
        fPictureRecord = SkNEW_ARGS(SkBBoxHierarchyRecord, (size, recordFlags, tree.get()));
    } else {
        fPictureRecord = SkNEW_ARGS(SkPictureRecord, (size, recordFlags));
    }

    fPictureRecord->beginRecording();
    return this->getRecordingCanvas();
}

SkCanvas* SkPictureRecorder::EXPERIMENTAL_beginRecording(int width, int height,
                                                         SkBBHFactory* bbhFactory /* = NULL */) {
    this->reset();
    fWidth = width;
    fHeight = height;

    // TODO: plumb bbhFactory through
    fRecord   = SkNEW(SkRecord);
    fRecorder = SkNEW_ARGS(SkRecorder, (fRecord, width, height));
    return this->getRecordingCanvas();
}

SkCanvas* SkPictureRecorder::getRecordingCanvas() {
    if (NULL != fRecorder) {
        return fRecorder;
    }
    return fPictureRecord;
}

SkPicture* SkPictureRecorder::endRecording() {
    SkPicture* picture = NULL;

    if (NULL != fRecorder) {
        // TODO: picture = SkNEW_ARGS(SkPicture, (fWidth, fHeight, fRecord));
        //       fRecord = NULL;
    }

    if (NULL != fPictureRecord) {
        fPictureRecord->endRecording();
        const bool deepCopyOps = false;
        picture = SkNEW_ARGS(SkPicture, (fWidth, fHeight, *fPictureRecord, deepCopyOps));
    }

    this->reset();
    return picture;
}

void SkPictureRecorder::internalOnly_EnableOpts(bool enableOpts) {
    if (NULL != fPictureRecord) {
        fPictureRecord->internalOnly_EnableOpts(enableOpts);
    }
}

void SkPictureRecorder::partialReplay(SkCanvas* canvas) const {
    if (NULL == canvas) {
        return;
    }

    if (NULL != fRecorder) {
        SkRecordDraw(*fRecord, canvas);
    }

    if (NULL != fPictureRecord) {
        const bool deepCopyOps = true;
        SkPicture picture(fWidth, fHeight, *fPictureRecord, deepCopyOps);
        picture.draw(canvas);
    }
}
