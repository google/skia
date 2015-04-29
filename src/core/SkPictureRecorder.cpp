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
#include "SkPictureUtils.h"
#include "SkRecord.h"
#include "SkRecordDraw.h"
#include "SkRecordOpts.h"
#include "SkRecorder.h"
#include "SkTypes.h"

SkPictureRecorder::SkPictureRecorder() {
    fActivelyRecording = false;
    fRecorder.reset(SkNEW_ARGS(SkRecorder, (nullptr, SkRect::MakeWH(0,0))));
}

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
    fRecorder->reset(fRecord.get(), cullRect);
    fActivelyRecording = true;
    return this->getRecordingCanvas();
}

SkCanvas* SkPictureRecorder::getRecordingCanvas() {
    return fActivelyRecording ? fRecorder.get() : nullptr;
}

SkPicture* SkPictureRecorder::endRecordingAsPicture() {
    fActivelyRecording = false;
    fRecorder->restoreToCount(1);  // If we were missing any restores, add them now.
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

    size_t subPictureBytes = fRecorder->approxBytesUsedBySubPictures();
    for (int i = 0; pictList && i < pictList->count(); i++) {
        subPictureBytes += SkPictureUtils::ApproximateBytesUsed(pictList->begin()[i]);
    }
    return SkNEW_ARGS(SkPicture, (fCullRect,
                                  fRecord.detach(),
                                  pictList,
                                  fBBH.detach(),
                                  saveLayerData.detach(),
                                  subPictureBytes));
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
    SkRect onGetBounds() override { return fBounds; }

    void onDraw(SkCanvas* canvas) override {
        SkDrawable* const* drawables = NULL;
        int drawableCount = 0;
        if (fDrawableList) {
            drawables = fDrawableList->begin();
            drawableCount = fDrawableList->count();
        }
        SkRecordDraw(*fRecord, canvas, NULL, drawables, drawableCount, fBBH, NULL/*callback*/);
    }

    SkPicture* onNewPictureSnapshot() override {
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

        size_t subPictureBytes = 0;
        for (int i = 0; pictList && i < pictList->count(); i++) {
            subPictureBytes += SkPictureUtils::ApproximateBytesUsed(pictList->begin()[i]);
        }
        // SkPicture will take ownership of a ref on both fRecord and fBBH.
        // We're not willing to give up our ownership, so we must ref them for SkPicture.
        return SkNEW_ARGS(SkPicture, (fBounds,
                                      SkRef(fRecord.get()),
                                      pictList,
                                      SkSafeRef(fBBH.get()),
                                      saveLayerData.detach(),
                                      subPictureBytes));
    }
};

SkDrawable* SkPictureRecorder::endRecordingAsDrawable() {
    fActivelyRecording = false;
    fRecorder->restoreToCount(1);  // If we were missing any restores, add them now.
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
    fRecord.reset(NULL);
    fBBH.reset(NULL);

    return drawable;
}
