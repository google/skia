/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkLayerInfo.h"
#include "SkPictureRecorder.h"
#include "SkRecord.h"
#include "SkRecordDraw.h"
#include "SkRecorder.h"
#include "SkRecordOpts.h"
#include "SkTypes.h"

SkPictureRecorder::SkPictureRecorder() {}

SkPictureRecorder::~SkPictureRecorder() {}

SkCanvas* SkPictureRecorder::beginRecording(const SkRect& cullRect,
                                            SkBBHFactory* bbhFactory /* = NULL */,
                                            uint32_t recordFlags /* = 0 */) {
    fCullRect = cullRect;
    fFlags = recordFlags;

    if (bbhFactory) {
        fBBH.reset((*bbhFactory)(cullRect));
        SkASSERT(fBBH.get());
    }

    fRecord.reset(SkNEW(SkRecord));
    fRecorder.reset(SkNEW_ARGS(SkRecorder, (fRecord.get(), cullRect)));
    return this->getRecordingCanvas();
}

SkCanvas* SkPictureRecorder::getRecordingCanvas() {
    return fRecorder.get();
}

SkPicture* SkPictureRecorder::endRecording() {
    // TODO: delay as much of this work until just before first playback?
    SkRecordOptimize(fRecord);

    SkAutoTUnref<SkLayerInfo> saveLayerData;

    if (fBBH && (fFlags & kComputeSaveLayerInfo_RecordFlag)) {
        SkPicture::AccelData::Key key = SkLayerInfo::ComputeKey();

        saveLayerData.reset(SkNEW_ARGS(SkLayerInfo, (key)));
    }

    if (fBBH.get()) {
        if (saveLayerData) {
            SkRecordComputeLayers(fCullRect, *fRecord, fBBH.get(), saveLayerData);
        } else {
            SkRecordFillBounds(fCullRect, *fRecord, fBBH.get());
        }
    }

    // TODO: we should remember these from our caller
    SkBBHFactory* factory = NULL;
    uint32_t recordFlags = 0;
    SkAutoTDelete<SkPicture::SnapshotArray> drawablePicts(
            fRecorder->newDrawableSnapshot(factory, recordFlags));
    SkPicture* pict = SkNEW_ARGS(SkPicture, (fCullRect, fRecord.detach(),
                                             drawablePicts.detach(), fBBH.get()));

    if (saveLayerData) {
        pict->EXPERIMENTAL_addAccelData(saveLayerData);
    }

    return pict;
}

void SkPictureRecorder::partialReplay(SkCanvas* canvas) const {
    if (NULL == canvas) {
        return;
    }

    int drawableCount = 0;
    SkRecordDraw(*fRecord, canvas, NULL, drawableCount, NULL/*bbh*/, NULL/*callback*/);
}
