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
                           sk_sp<SkBBoxHierarchy> bbh,
                           size_t approxBytesUsedBySubPictures)
    : fCullRect(cull)
    , fApproxBytesUsedBySubPictures(approxBytesUsedBySubPictures)
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
                                   const SkM44& initialCTM) const {
    SkASSERT(canvas);
    SkRecordPartialDraw(*fRecord,
                        canvas,
                        this->drawablePicts(),
                        this->drawableCount(),
                        start,
                        stop,
                        initialCTM);
}

struct NestedApproxOpCounter {
    int fCount = 0;

    template <typename T> void operator()(const T& op) {
        fCount += 1;
    }
    void operator()(const SkRecords::DrawPicture& op) {
        fCount += op.picture->approximateOpCount(true);
    }
};

SkRect SkBigPicture::cullRect()            const { return fCullRect; }
int SkBigPicture::approximateOpCount(bool nested) const {
    if (nested) {
        NestedApproxOpCounter visitor;
        for (int i = 0; i < fRecord->count(); i++) {
            fRecord->visit(i, visitor);
        }
        return visitor.fCount;
    } else {
        return fRecord->count();
    }
}
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

