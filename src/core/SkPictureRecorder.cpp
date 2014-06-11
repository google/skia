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
#include "SkTypes.h" 

SkPictureRecorder::~SkPictureRecorder() {
    SkSafeSetNull(fCanvas);
}

SkCanvas* SkPictureRecorder::beginRecording(int width, int height,
                                            SkBBHFactory* bbhFactory /* = NULL */,
                                            uint32_t recordFlags /* = 0 */) {
    SkSafeSetNull(fCanvas); // terminate any prior recording(s)

    fWidth = width;
    fHeight = height;

    const SkISize size = SkISize::Make(width, height);

    if (NULL != bbhFactory) {
        SkAutoTUnref<SkBBoxHierarchy> tree((*bbhFactory)(width, height));
        SkASSERT(NULL != tree);
        fCanvas = SkNEW_ARGS(SkBBoxHierarchyRecord, (size, recordFlags, tree.get()));
    } else {
        fCanvas = SkNEW_ARGS(SkPictureRecord, (size, recordFlags));
    }

    fCanvas->beginRecording();

    return fCanvas;
}

SkCanvas* SkPictureRecorder::getRecordingCanvas() {
    return fCanvas;
}

SkPicture* SkPictureRecorder::endRecording() {
    if (NULL == fCanvas) {
        return NULL;
    }

    fCanvas->endRecording();

    const bool deepCopyOps = false;
    SkAutoTUnref<SkPicture> picture(SkNEW_ARGS(SkPicture, (fWidth, fHeight, 
                                                           *fCanvas, deepCopyOps)));
    SkSafeSetNull(fCanvas);

    return picture.detach();
}

void SkPictureRecorder::internalOnly_EnableOpts(bool enableOpts) {
    if (NULL != fCanvas) {
        fCanvas->internalOnly_EnableOpts(enableOpts);
    }
}

void SkPictureRecorder::partialReplay(SkCanvas* canvas) const {
    if (NULL == fCanvas || NULL == canvas) {
        // Not recording or nothing to replay into
        return;
    }

    const bool deepCopyOps = true;
    SkAutoTUnref<SkPicture> picture(SkNEW_ARGS(SkPicture, (fWidth, fHeight, 
                                                           *fCanvas, deepCopyOps)));
    picture->draw(canvas);
}
