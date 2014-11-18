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

SkCanvas* SkPictureRecorder::beginRecording(SkScalar width, SkScalar height,
                                            SkBBHFactory* bbhFactory /* = NULL */,
                                            uint32_t recordFlags /* = 0 */) {
    fFlags = recordFlags;
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
    // TODO: delay as much of this work until just before first playback?
    SkRecordOptimize(fRecord);

    SkAutoTUnref<SkLayerInfo> saveLayerData;

    if (fBBH && (fFlags & kComputeSaveLayerInfo_RecordFlag)) {
        SkPicture::AccelData::Key key = SkLayerInfo::ComputeKey();

        saveLayerData.reset(SkNEW_ARGS(SkLayerInfo, (key)));
    }

    if (fBBH.get()) {
        SkRect cullRect = SkRect::MakeWH(fCullWidth, fCullHeight);

        if (saveLayerData) {
            SkRecordComputeLayers(cullRect, *fRecord, fBBH.get(), saveLayerData);
        } else {
            SkRecordFillBounds(cullRect, *fRecord, fBBH.get());
        }
    }

    // TODO: we should remember these from our caller
    SkBBHFactory* factory = NULL;
    uint32_t recordFlags = 0;
    SkAutoDataUnref drawablePicts(fRecorder->newDrawableSnapshot(factory, recordFlags));
    SkPicture* pict = SkNEW_ARGS(SkPicture, (fCullWidth, fCullHeight, fRecord.detach(),
                                             drawablePicts, fBBH.get()));

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
