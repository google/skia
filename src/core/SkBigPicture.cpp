/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBBoxHierarchy.h"
#include "SkBigPicture.h"
#include "SkPictureCommon.h"
#include "SkRecord.h"
#include "SkRecordDraw.h"
#include "SkTraceEvent.h"

SkBigPicture::SkBigPicture(const SkRect& cull,
                           SkRecord* record,
                           SnapshotArray* drawablePicts,
                           SkBBoxHierarchy* bbh,
                           size_t approxBytesUsedBySubPictures)
    : fCullRect(cull)
    , fApproxBytesUsedBySubPictures(approxBytesUsedBySubPictures)
    , fRecord(record)               // Take ownership of caller's ref.
    , fDrawablePicts(drawablePicts) // Take ownership.
    , fBBH(bbh)                     // Take ownership of caller's ref.
{}

void SkBigPicture::playback(SkCanvas* canvas, AbortCallback* callback) const {
    SkASSERT(canvas);

    // If the query contains the whole picture, don't bother with the BBH.
    const bool useBBH = !canvas->getLocalClipBounds().contains(this->cullRect());

    SkRecordDraw(*fRecord,
                 canvas,
                 this->drawablePicts(),
                 nullptr,
                 this->drawableCount(),
                 useBBH ? fBBH.get() : nullptr,
                 callback);
}

void SkBigPicture::partialPlayback(SkCanvas* canvas,
                                   int start,
                                   int stop,
                                   const SkMatrix& initialCTM) const {
    SkASSERT(canvas);
    SkRecordPartialDraw(*fRecord,
                        canvas,
                        this->drawablePicts(),
                        this->drawableCount(),
                        start,
                        stop,
                        initialCTM);
}

const SkBigPicture::Analysis& SkBigPicture::analysis() const {
    fAnalysisOnce([this] { fAnalysis.init(*fRecord); });
    return fAnalysis;
}

SkRect SkBigPicture::cullRect()            const { return fCullRect; }
bool   SkBigPicture::willPlayBackBitmaps() const { return this->analysis().fWillPlaybackBitmaps; }
int    SkBigPicture::numSlowPaths() const { return this->analysis().fNumSlowPathsAndDashEffects; }
int    SkBigPicture::approximateOpCount()   const { return fRecord->count(); }
size_t SkBigPicture::approximateBytesUsed() const {
    size_t bytes = sizeof(*this) + fRecord->bytesUsed() + fApproxBytesUsedBySubPictures;
    if (fBBH) { bytes += fBBH->bytesUsed(); }
    return bytes;
}

int SkBigPicture::drawableCount() const {
    return fDrawablePicts ? fDrawablePicts->count() : 0;
}

SkPicture const* const* SkBigPicture::drawablePicts() const {
    return fDrawablePicts ? fDrawablePicts->begin() : nullptr;
}

void SkBigPicture::Analysis::init(const SkRecord& record) {
    TRACE_EVENT0("disabled-by-default-skia", "SkBigPicture::Analysis::init()");
    SkBitmapHunter bitmap;
    SkPathCounter  path;

    bool hasBitmap = false;
    for (int i = 0; i < record.count(); i++) {
        hasBitmap = hasBitmap || record.visit(i, bitmap);
        record.visit(i, path);
    }

    fWillPlaybackBitmaps        = hasBitmap;
    fNumSlowPathsAndDashEffects = SkTMin<int>(path.fNumSlowPathsAndDashEffects, 255);
}
