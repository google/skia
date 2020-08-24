/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <memory>

#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkTypes.h"
#include "src/core/SkBigPicture.h"
#include "src/core/SkMiniRecorder.h"
#include "src/core/SkRecord.h"
#include "src/core/SkRecordDraw.h"
#include "src/core/SkRecordOpts.h"
#include "src/core/SkRecordedDrawable.h"
#include "src/core/SkRecorder.h"

SkPictureRecorder::SkPictureRecorder() {
    fActivelyRecording = false;
    fMiniRecorder = std::make_unique<SkMiniRecorder>();
    fRecorder = std::make_unique<SkRecorder>(nullptr, SkRect::MakeEmpty(), fMiniRecorder.get());
}

SkPictureRecorder::~SkPictureRecorder() {}

SkCanvas* SkPictureRecorder::beginRecording(const SkRect& userCullRect,
                                            sk_sp<SkBBoxHierarchy> bbh) {
    const SkRect cullRect = userCullRect.isEmpty() ? SkRect::MakeEmpty() : userCullRect;

    fCullRect = cullRect;
    fBBH = std::move(bbh);

    if (!fRecord) {
        fRecord.reset(new SkRecord);
    }
    fRecorder->reset(fRecord.get(), cullRect, fMiniRecorder.get());
    fActivelyRecording = true;
    return this->getRecordingCanvas();
}

SkCanvas* SkPictureRecorder::beginRecording(const SkRect& bounds, SkBBHFactory* factory) {
    return this->beginRecording(bounds, factory ? (*factory)() : nullptr);
}

SkCanvas* SkPictureRecorder::getRecordingCanvas() {
    return fActivelyRecording ? fRecorder.get() : nullptr;
}

sk_sp<SkPicture> SkPictureRecorder::finishRecordingAsPicture() {
    fActivelyRecording = false;
    fRecorder->restoreToCount(1);  // If we were missing any restores, add them now.

    if (fRecord->count() == 0) {
        auto pic = fMiniRecorder->detachAsPicture(fBBH ? nullptr : &fCullRect);
        if (fBBH) {
            SkRect bounds = pic->cullRect();  // actually the computed bounds, not fCullRect.
            SkBBoxHierarchy::Metadata meta;
            meta.isDraw = true;               // All mini-recorder pictures are single draws.
            fBBH->insert(&bounds, &meta, 1);
        }
        fBBH.reset(nullptr);
        return pic;
    }

    // TODO: delay as much of this work until just before first playback?
    SkRecordOptimize(fRecord.get());

    SkDrawableList* drawableList = fRecorder->getDrawableList();
    std::unique_ptr<SkBigPicture::SnapshotArray> pictList{
        drawableList ? drawableList->newDrawableSnapshot() : nullptr
    };

    if (fBBH) {
        SkAutoTMalloc<SkRect> bounds(fRecord->count());
        SkAutoTMalloc<SkBBoxHierarchy::Metadata> meta(fRecord->count());
        SkRecordFillBounds(fCullRect, *fRecord, bounds, meta);

        fBBH->insert(bounds, meta, fRecord->count());

        // Now that we've calculated content bounds, we can update fCullRect, often trimming it.
        SkRect bbhBound = SkRect::MakeEmpty();
        for (int i = 0; i < fRecord->count(); i++) {
            bbhBound.join(bounds[i]);
        }
        SkASSERT((bbhBound.isEmpty() || fCullRect.contains(bbhBound))
              || (bbhBound.isEmpty() && fCullRect.isEmpty()));
        fCullRect = bbhBound;
    }

    size_t subPictureBytes = fRecorder->approxBytesUsedBySubPictures();
    for (int i = 0; pictList && i < pictList->count(); i++) {
        subPictureBytes += pictList->begin()[i]->approximateBytesUsed();
    }
    return sk_make_sp<SkBigPicture>(fCullRect,
                                    std::move(fRecord),
                                    std::move(pictList),
                                    std::move(fBBH),
                                    subPictureBytes);
}

sk_sp<SkPicture> SkPictureRecorder::finishRecordingAsPictureWithCull(const SkRect& cullRect) {
    fCullRect = cullRect;
    return this->finishRecordingAsPicture();
}


void SkPictureRecorder::partialReplay(SkCanvas* canvas) const {
    if (nullptr == canvas) {
        return;
    }

    int drawableCount = 0;
    SkDrawable* const* drawables = nullptr;
    SkDrawableList* drawableList = fRecorder->getDrawableList();
    if (drawableList) {
        drawableCount = drawableList->count();
        drawables = drawableList->begin();
    }
    SkRecordDraw(*fRecord, canvas, nullptr, drawables, drawableCount, nullptr/*bbh*/, nullptr/*callback*/);
}

sk_sp<SkDrawable> SkPictureRecorder::finishRecordingAsDrawable() {
    fActivelyRecording = false;
    fRecorder->flushMiniRecorder();
    fRecorder->restoreToCount(1);  // If we were missing any restores, add them now.

    SkRecordOptimize(fRecord.get());

    if (fBBH) {
        SkAutoTMalloc<SkRect> bounds(fRecord->count());
        SkAutoTMalloc<SkBBoxHierarchy::Metadata> meta(fRecord->count());
        SkRecordFillBounds(fCullRect, *fRecord, bounds, meta);
        fBBH->insert(bounds, meta, fRecord->count());
    }

    sk_sp<SkDrawable> drawable =
         sk_make_sp<SkRecordedDrawable>(std::move(fRecord), std::move(fBBH),
                                        fRecorder->detachDrawableList(), fCullRect);

    return drawable;
}
