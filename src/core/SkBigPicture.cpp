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
                           SkBBoxHierarchy* bbh)
    : fCullRect(cull)
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

SkRect SkBigPicture::cullRect()            const { return fCullRect; }
int    SkBigPicture::approximateOpCount()  const { return fRecord->count(); }

int SkBigPicture::drawableCount() const {
    return fDrawablePicts ? fDrawablePicts->count() : 0;
}

SkPicture const* const* SkBigPicture::drawablePicts() const {
    return fDrawablePicts ? fDrawablePicts->begin() : nullptr;
}

