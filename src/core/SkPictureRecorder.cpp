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
    SkSafeSetNull(fCanvas);
    fPicture.reset(SkNEW(SkPicture));

    fPicture->fWidth = width;
    fPicture->fHeight = height;

    const SkISize size = SkISize::Make(width, height);

    if (NULL != bbhFactory) {
        SkAutoTUnref<SkBBoxHierarchy> tree((*bbhFactory)(width, height));
        SkASSERT(NULL != tree);
        fCanvas = SkNEW_ARGS(SkBBoxHierarchyRecord, (fPicture, size, recordFlags, tree.get()));
    } else {
        fCanvas = SkNEW_ARGS(SkPictureRecord, (fPicture, size, recordFlags));
    }

    fCanvas->beginRecording();

    return fCanvas;
}

SkCanvas* SkPictureRecorder::getRecordingCanvas() {
    return fCanvas;
}

SkPicture* SkPictureRecorder::endRecording() {
    if (NULL == fPicture.get()) {
        return NULL;
    }

    SkASSERT(NULL == fPicture->fPlayback);
    SkASSERT(NULL != fCanvas);

    fCanvas->endRecording();

    SkPictInfo info;
    fPicture->createHeader(&info);
    fPicture->fPlayback = SkNEW_ARGS(SkPicturePlayback, (fPicture, *fCanvas, info));

    SkSafeSetNull(fCanvas);

    return fPicture.detach();
}

void SkPictureRecorder::internalOnly_EnableOpts(bool enableOpts) {
    if (NULL != fCanvas) {
        fCanvas->internalOnly_EnableOpts(enableOpts);
    }
}

void SkPictureRecorder::partialReplay(SkCanvas* canvas) const {
    if (NULL == fPicture.get() || NULL == canvas) {
        // Not recording or nothing to replay into
        return;
    }

    SkASSERT(NULL != fCanvas);

    SkAutoTDelete<SkPicturePlayback> playback(SkPicture::FakeEndRecording(fPicture.get(),
                                                                          *fCanvas,
                                                                          false));
    playback->draw(*canvas, NULL);
}
