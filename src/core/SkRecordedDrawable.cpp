/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMatrix.h"
#include "include/core/SkPictureRecorder.h"
#include "src/core/SkPictureData.h"
#include "src/core/SkPicturePlayback.h"
#include "src/core/SkPictureRecord.h"
#include "src/core/SkRecordDraw.h"
#include "src/core/SkRecordedDrawable.h"

#if defined(SK_GANESH)
#include "include/private/chromium/Slug.h"
#endif

size_t SkRecordedDrawable::onApproximateBytesUsed() {
    size_t drawablesSize = 0;
    if (fDrawableList) {
        for (auto&& drawable : *fDrawableList) {
            drawablesSize += drawable->approximateBytesUsed();
        }
    }
    return sizeof(*this) +
           (fRecord ? fRecord->bytesUsed() : 0) +
           (fBBH ? fBBH->bytesUsed() : 0) +
           drawablesSize;
}

void SkRecordedDrawable::onDraw(SkCanvas* canvas) {
    SkDrawable* const* drawables = nullptr;
    int drawableCount = 0;
    if (fDrawableList) {
        drawables = fDrawableList->begin();
        drawableCount = fDrawableList->count();
    }
    SkRecordDraw(*fRecord, canvas, nullptr, drawables, drawableCount, fBBH.get(), nullptr);
}

SkPicture* SkRecordedDrawable::onNewPictureSnapshot() {
    // TODO: should we plumb-down the BBHFactory and recordFlags from our host
    //       PictureRecorder?
    std::unique_ptr<SkBigPicture::SnapshotArray> pictList{
        fDrawableList ? fDrawableList->newDrawableSnapshot() : nullptr
    };

    size_t subPictureBytes = 0;
    for (int i = 0; pictList && i < pictList->count(); i++) {
        subPictureBytes += pictList->begin()[i]->approximateBytesUsed();
    }
    return new SkBigPicture(fBounds, fRecord, std::move(pictList), fBBH, subPictureBytes);
}

void SkRecordedDrawable::flatten(SkWriteBuffer& buffer) const {
    // Write the bounds.
    buffer.writeRect(fBounds);

    // Create an SkPictureRecord to record the draw commands.
    SkPictInfo info;
    SkPictureRecord pictureRecord(SkISize::Make(fBounds.width(), fBounds.height()), 0);

    // If the query contains the whole picture, don't bother with the bounding box hierarchy.
    SkBBoxHierarchy* bbh;
    if (pictureRecord.getLocalClipBounds().contains(fBounds)) {
        bbh = nullptr;
    } else {
        bbh = fBBH.get();
    }

    // Record the draw commands.
    SkDrawable* const* drawables = fDrawableList ? fDrawableList->begin() : nullptr;
    int drawableCount            = fDrawableList ? fDrawableList->count() : 0;
    pictureRecord.beginRecording();
    SkRecordDraw(*fRecord, &pictureRecord, nullptr, drawables, drawableCount, bbh, nullptr);
    pictureRecord.endRecording();

    // Flatten the recorded commands and drawables.
    SkPictureData pictureData(pictureRecord, info);
    pictureData.flatten(buffer);
}

sk_sp<SkFlattenable> SkRecordedDrawable::CreateProc(SkReadBuffer& buffer) {
    // Read the bounds.
    SkRect bounds;
    buffer.readRect(&bounds);

    // Unflatten into a SkPictureData.
    SkPictInfo info;
    info.setVersion(buffer.getVersion());
    info.fCullRect = bounds;
    std::unique_ptr<SkPictureData> pictureData(SkPictureData::CreateFromBuffer(buffer, info));
    if (!pictureData) {
        return nullptr;
    }

    // Create a drawable.
    SkPicturePlayback playback(pictureData.get());
    SkPictureRecorder recorder;
    playback.draw(recorder.beginRecording(bounds), nullptr, &buffer);
    return recorder.finishRecordingAsDrawable();
}
