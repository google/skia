/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPictureRecorder.h"

#include "include/core/SkBBHFactory.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkPicture.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTemplates.h"
#include "src/core/SkBigPicture.h"
#include "src/core/SkRecord.h"
#include "src/core/SkRecordCanvas.h"
#include "src/core/SkRecordDraw.h"
#include "src/core/SkRecordOpts.h"
#include "src/core/SkRecordedDrawable.h"

#include <cstddef>
#include <memory>
#include <utility>

using namespace skia_private;

SkPictureRecorder::SkPictureRecorder() {
    fActivelyRecording = false;
    fRecorder = std::make_unique<SkRecordCanvas>(nullptr, SkRect::MakeEmpty());
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
    fRecorder->reset(fRecord.get(), cullRect);
    fActivelyRecording = true;
    return this->getRecordingCanvas();
}

SkCanvas* SkPictureRecorder::beginRecording(const SkRect& bounds, SkBBHFactory* factory) {
    return this->beginRecording(bounds, factory ? (*factory)() : nullptr);
}

SkCanvas* SkPictureRecorder::getRecordingCanvas() {
    return fActivelyRecording ? fRecorder.get() : nullptr;
}

class SkEmptyPicture final : public SkPicture {
public:
    void playback(SkCanvas*, AbortCallback*) const override { }

    size_t approximateBytesUsed() const override { return sizeof(*this); }
    int    approximateOpCount(bool nested)   const override { return 0; }
    SkRect cullRect()             const override { return SkRect::MakeEmpty(); }
};

sk_sp<SkPicture> SkPictureRecorder::finishRecordingAsPicture() {
    fActivelyRecording = false;
    fRecorder->restoreToCount(1);  // If we were missing any restores, add them now.

    if (fRecord->count() == 0) {
        return sk_make_sp<SkEmptyPicture>();
    }

    // TODO: delay as much of this work until just before first playback?
    SkRecordOptimize(fRecord.get());

    SkDrawableList* drawableList = fRecorder->getDrawableList();
    std::unique_ptr<SkBigPicture::SnapshotArray> pictList{
        drawableList ? drawableList->newDrawableSnapshot() : nullptr
    };

    if (fBBH) {
        AutoTArray<SkRect> bounds(fRecord->count());
        AutoTMalloc<SkBBoxHierarchy::Metadata> meta(fRecord->count());
        SkRecordFillBounds(fCullRect, *fRecord, bounds.data(), meta);

        fBBH->insert(bounds.data(), meta, fRecord->count());

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
    fRecorder->restoreToCount(1);  // If we were missing any restores, add them now.

    SkRecordOptimize(fRecord.get());

    if (fBBH) {
        AutoTArray<SkRect> bounds(fRecord->count());
        AutoTMalloc<SkBBoxHierarchy::Metadata> meta(fRecord->count());
        SkRecordFillBounds(fCullRect, *fRecord, bounds.data(), meta);
        fBBH->insert(bounds.data(), meta, fRecord->count());
    }

    sk_sp<SkDrawable> drawable =
         sk_make_sp<SkRecordedDrawable>(std::move(fRecord), std::move(fBBH),
                                        fRecorder->detachDrawableList(), fCullRect);

    return drawable;
}
