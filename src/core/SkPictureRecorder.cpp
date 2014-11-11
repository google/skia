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
#include "SkRecordOpts.h"
#include "SkTypes.h"

// Must place SK_SUPPORT_GPU after other includes so it is defined in the
// Android framework build.
#if SK_SUPPORT_GPU
#include "GrPictureUtils.h"
#endif

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

#if SK_SUPPORT_GPU
    SkAutoTUnref<GrAccelData> saveLayerData;

    if (fBBH && (fFlags & kComputeSaveLayerInfo_RecordFlag)) {
        SkPicture::AccelData::Key key = GrAccelData::ComputeAccelDataKey();

        saveLayerData.reset(SkNEW_ARGS(GrAccelData, (key)));
    }
#endif

    if (fBBH.get()) {
        SkRect cullRect = SkRect::MakeWH(fCullWidth, fCullHeight);

#if SK_SUPPORT_GPU
        if (saveLayerData) {
            SkRecordComputeLayers(cullRect, *fRecord, fBBH.get(), saveLayerData);
        } else {
#endif
            SkRecordFillBounds(cullRect, *fRecord, fBBH.get());
#if SK_SUPPORT_GPU
        }
#endif
    }

    SkPicture* pict = SkNEW_ARGS(SkPicture, (fCullWidth, fCullHeight, fRecord.detach(), fBBH.get()));

#if SK_SUPPORT_GPU
    if (saveLayerData) {
        pict->EXPERIMENTAL_addAccelData(saveLayerData);
    }
#endif

    return pict;
}

void SkPictureRecorder::partialReplay(SkCanvas* canvas) const {
    if (NULL == canvas) {
        return;
    }
    SkRecordDraw(*fRecord, canvas, NULL/*bbh*/, NULL/*callback*/);
}
