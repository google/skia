/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBigPicture.h"
#include "SkData.h"
#include "SkDrawable.h"
#include "SkImage.h"
#include "SkPictureRecorder.h"
#include "SkPictureUtils.h"
#include "SkRecord.h"
#include "SkRecordDraw.h"
#include "SkRecordOpts.h"
#include "SkRecordedDrawable.h"
#include "SkRecorder.h"
#include "SkTypes.h"
#include "SkTLogic.h"

namespace SkRecords {

    struct OptimizeFor {
        GrContext* fCtx;

        // A few ops have a top-level SkImage:
        void operator()(DrawAtlas*     op) { this->make_texture(&op->atlas); }
        void operator()(DrawImage*     op) { this->make_texture(&op->image); }
        void operator()(DrawImageNine* op) { this->make_texture(&op->image); }
        void operator()(DrawImageRect* op) { this->make_texture(&op->image); }
        void make_texture(sk_sp<const SkImage>* img) const {
            *img = (*img)->makeTextureImage(fCtx);
        }

        // Some ops have a paint, some have an optional paint.
        // Either way, get back a pointer.
        static SkPaint* AsPtr(SkPaint& p) { return &p; }
        static SkPaint* AsPtr(SkRecords::Optional<SkPaint>& p) { return p; }

        // For all other types of ops, look for images inside the paint.
        template <typename T>
        SK_WHEN(T::kTags & kHasPaint_Tag, void) operator()(T* op) {
            SkMatrix matrix;
            SkShader::TileMode xy[2];

            if (auto paint  = AsPtr(op->paint))
            if (auto shader = paint->getShader())
            if (auto image  = shader->isAImage(&matrix, xy)) {
                paint->setShader(image->makeTextureImage(fCtx)->makeShader(xy[0], xy[1], &matrix));
            }

            // TODO: re-build compose shaders
        }

        // Control ops, etc.  Nothing to do for these.
        template <typename T>
        SK_WHEN(!(T::kTags & kHasPaint_Tag), void) operator()(T*) {}
    };

} // namespace SkRecords

static void optimize_for(GrContext* ctx, SkRecord* record) {
    for (int i = 0; ctx && i < record->count(); i++) {
        record->mutate(i, SkRecords::OptimizeFor{ctx});
    }
}

SkPictureRecorder::SkPictureRecorder() {
    fActivelyRecording = false;
    fRecorder.reset(new SkRecorder(nullptr, SkRect::MakeWH(0, 0), &fMiniRecorder));
}

SkPictureRecorder::~SkPictureRecorder() {}

SkCanvas* SkPictureRecorder::beginRecording(const SkRect& cullRect,
                                            SkBBHFactory* bbhFactory /* = nullptr */,
                                            uint32_t recordFlags /* = 0 */) {
    fCullRect = cullRect;
    fFlags = recordFlags;

    if (bbhFactory) {
        fBBH.reset((*bbhFactory)(cullRect));
        SkASSERT(fBBH.get());
    }

    if (!fRecord) {
        fRecord.reset(new SkRecord);
    }
    SkRecorder::DrawPictureMode dpm = (recordFlags & kPlaybackDrawPicture_RecordFlag)
        ? SkRecorder::Playback_DrawPictureMode
        : SkRecorder::Record_DrawPictureMode;
    fRecorder->reset(fRecord.get(), cullRect, dpm, &fMiniRecorder);
    fActivelyRecording = true;
    return this->getRecordingCanvas();
}

SkCanvas* SkPictureRecorder::getRecordingCanvas() {
    return fActivelyRecording ? fRecorder.get() : nullptr;
}

sk_sp<SkPicture> SkPictureRecorder::finishRecordingAsPicture(uint32_t finishFlags) {
    fActivelyRecording = false;
    fRecorder->restoreToCount(1);  // If we were missing any restores, add them now.

    if (fRecord->count() == 0) {
        if (finishFlags & kReturnNullForEmpty_FinishFlag) {
            return nullptr;
        }
        return fMiniRecorder.detachAsPicture(fCullRect);
    }

    // TODO: delay as much of this work until just before first playback?
    SkRecordOptimize(fRecord);
    optimize_for(fGrContextToOptimizeFor, fRecord);

    if (fRecord->count() == 0) {
        if (finishFlags & kReturnNullForEmpty_FinishFlag) {
            return nullptr;
        }
    }

    SkDrawableList* drawableList = fRecorder->getDrawableList();
    SkBigPicture::SnapshotArray* pictList =
        drawableList ? drawableList->newDrawableSnapshot() : nullptr;

    if (fBBH.get()) {
        SkAutoTMalloc<SkRect> bounds(fRecord->count());
        SkRecordFillBounds(fCullRect, *fRecord, bounds);
        fBBH->insert(bounds, fRecord->count());

        // Now that we've calculated content bounds, we can update fCullRect, often trimming it.
        // TODO: get updated fCullRect from bounds instead of forcing the BBH to return it?
        SkRect bbhBound = fBBH->getRootBound();
        SkASSERT((bbhBound.isEmpty() || fCullRect.contains(bbhBound))
            || (bbhBound.isEmpty() && fCullRect.isEmpty()));
        fCullRect = bbhBound;
    }

    size_t subPictureBytes = fRecorder->approxBytesUsedBySubPictures();
    for (int i = 0; pictList && i < pictList->count(); i++) {
        subPictureBytes += SkPictureUtils::ApproximateBytesUsed(pictList->begin()[i]);
    }
    return sk_make_sp<SkBigPicture>(fCullRect, fRecord.release(), pictList, fBBH.release(),
                                    subPictureBytes);
}

sk_sp<SkPicture> SkPictureRecorder::finishRecordingAsPictureWithCull(const SkRect& cullRect,
                                                                     uint32_t finishFlags) {
    fCullRect = cullRect;
    return this->finishRecordingAsPicture(finishFlags);
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

sk_sp<SkDrawable> SkPictureRecorder::finishRecordingAsDrawable(uint32_t finishFlags) {
    fActivelyRecording = false;
    fRecorder->flushMiniRecorder();
    fRecorder->restoreToCount(1);  // If we were missing any restores, add them now.

    SkRecordOptimize(fRecord);
    optimize_for(fGrContextToOptimizeFor, fRecord);

    if (fRecord->count() == 0) {
        if (finishFlags & kReturnNullForEmpty_FinishFlag) {
            return nullptr;
        }
    }

    if (fBBH.get()) {
        SkAutoTMalloc<SkRect> bounds(fRecord->count());
        SkRecordFillBounds(fCullRect, *fRecord, bounds);
        fBBH->insert(bounds, fRecord->count());
    }

    sk_sp<SkDrawable> drawable =
         sk_make_sp<SkRecordedDrawable>(fRecord, fBBH, fRecorder->detachDrawableList(), fCullRect);

    // release our refs now, so only the drawable will be the owner.
    fRecord.reset(nullptr);
    fBBH.reset(nullptr);

    return drawable;
}
