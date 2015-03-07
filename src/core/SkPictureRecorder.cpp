/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkDrawable.h"
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

SkPicture* SkPictureRecorder::endRecordingAsPicture() {
    // TODO: delay as much of this work until just before first playback?
    SkRecordOptimize(fRecord);

    SkAutoTUnref<SkLayerInfo> saveLayerData;

    if (fBBH && (fFlags & kComputeSaveLayerInfo_RecordFlag)) {
        SkPicture::AccelData::Key key = SkLayerInfo::ComputeKey();

        saveLayerData.reset(SkNEW_ARGS(SkLayerInfo, (key)));
    }

    SkDrawableList* drawableList = fRecorder->getDrawableList();
    SkPicture::SnapshotArray* pictList = drawableList ? drawableList->newDrawableSnapshot() : NULL;

    if (fBBH.get()) {
        if (saveLayerData) {
            SkRecordComputeLayers(fCullRect, *fRecord, pictList, fBBH.get(), saveLayerData);
        } else {
            SkRecordFillBounds(fCullRect, *fRecord, fBBH.get());
        }
        SkRect bbhBound = fBBH->getRootBound();
        SkASSERT((bbhBound.isEmpty() || fCullRect.contains(bbhBound))
            || (bbhBound.isEmpty() && fCullRect.isEmpty()));
        fCullRect = bbhBound;
    }

    SkPicture* pict = SkNEW_ARGS(SkPicture, (fCullRect, fRecord, pictList, fBBH));

    if (saveLayerData) {
        pict->EXPERIMENTAL_addAccelData(saveLayerData);
    }

    // release our refs now, so only the picture will be the owner.
    fRecorder.reset(NULL);
    fRecord.reset(NULL);
    fBBH.reset(NULL);

    return pict;
}

void SkPictureRecorder::partialReplay(SkCanvas* canvas) const {
    if (NULL == canvas) {
        return;
    }

    int drawableCount = 0;
    SkDrawable* const* drawables = NULL;
    SkDrawableList* drawableList = fRecorder->getDrawableList();
    if (drawableList) {
        drawableCount = drawableList->count();
        drawables = drawableList->begin();
    }
    SkRecordDraw(*fRecord, canvas, NULL, drawables, drawableCount, NULL/*bbh*/, NULL/*callback*/);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkRecordedDrawable : public SkDrawable {
    SkAutoTUnref<SkRecord>          fRecord;
    SkAutoTUnref<SkBBoxHierarchy>   fBBH;
    SkAutoTDelete<SkDrawableList>   fDrawableList;
    const SkRect                    fBounds;
    const bool                      fDoSaveLayerInfo;

public:
    SkRecordedDrawable(SkRecord* record, SkBBoxHierarchy* bbh, SkDrawableList* drawableList,
                       const SkRect& bounds, bool doSaveLayerInfo)
        : fRecord(SkRef(record))
        , fBBH(SkSafeRef(bbh))
        , fDrawableList(drawableList)   // we take ownership
        , fBounds(bounds)
        , fDoSaveLayerInfo(doSaveLayerInfo)
    {}

protected:
    SkRect onGetBounds() SK_OVERRIDE { return fBounds; }

    void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkDrawable* const* drawables = NULL;
        int drawableCount = 0;
        if (fDrawableList) {
            drawables = fDrawableList->begin();
            drawableCount = fDrawableList->count();
        }
        SkRecordDraw(*fRecord, canvas, NULL, drawables, drawableCount, fBBH, NULL/*callback*/);
    }

    SkPicture* onNewPictureSnapshot() SK_OVERRIDE {
        SkPicture::SnapshotArray* pictList = NULL;
        if (fDrawableList) {
            // TODO: should we plumb-down the BBHFactory and recordFlags from our host
            //       PictureRecorder?
            pictList = fDrawableList->newDrawableSnapshot();
        }

        SkAutoTUnref<SkLayerInfo> saveLayerData;

        if (fBBH && fDoSaveLayerInfo) {
            SkPicture::AccelData::Key key = SkLayerInfo::ComputeKey();

            saveLayerData.reset(SkNEW_ARGS(SkLayerInfo, (key)));

            SkBBoxHierarchy* bbh = NULL;    // we've already computed fBBH (received in constructor)
            // TODO: update saveLayer info computation to reuse the already computed
            // bounds in 'fBBH'
            SkRecordComputeLayers(fBounds, *fRecord, pictList, bbh, saveLayerData);
        }

        SkPicture* pict = SkNEW_ARGS(SkPicture, (fBounds, fRecord, pictList, fBBH));

        if (saveLayerData) {
            pict->EXPERIMENTAL_addAccelData(saveLayerData);
        }
        return pict;
    }
};

SkDrawable* SkPictureRecorder::endRecordingAsDrawable() {
    // TODO: delay as much of this work until just before first playback?
    SkRecordOptimize(fRecord);

    if (fBBH.get()) {
        SkRecordFillBounds(fCullRect, *fRecord, fBBH.get());
    }

    SkDrawable* drawable = SkNEW_ARGS(SkRecordedDrawable,
                                        (fRecord, fBBH, fRecorder->detachDrawableList(),
                                         fCullRect,
                                         SkToBool(fFlags & kComputeSaveLayerInfo_RecordFlag)));

    // release our refs now, so only the drawable will be the owner.
    fRecorder.reset(NULL);
    fRecord.reset(NULL);
    fBBH.reset(NULL);

    return drawable;
}
