/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBBHFactory.h"
#include "src/core/SkBigPicture.h"
#include "src/core/SkPictureCommon.h"
#include "src/core/SkRecord.h"
#include "src/core/SkRecordDraw.h"
#include "src/core/SkTraceEvent.h"

SkBigPicture::SkBigPicture(const SkRect& cull,
                           sk_sp<SkRecord> record,
                           std::unique_ptr<SnapshotArray> drawablePicts,
                           sk_sp<SkBBoxHierarchy> bbh)
    : fCullRect(cull)
    , fRecord(std::move(record))
    , fDrawablePicts(std::move(drawablePicts))
    , fBBH(std::move(bbh))
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
int    SkBigPicture::approximateOpCount()   const { return fRecord->count(); }

int SkBigPicture::drawableCount() const {
    return fDrawablePicts ? fDrawablePicts->count() : 0;
}

SkPicture const* const* SkBigPicture::drawablePicts() const {
    return fDrawablePicts ? fDrawablePicts->begin() : nullptr;
}

