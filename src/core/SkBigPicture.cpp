/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBBoxHierarchy.h"
#include "SkBigPicture.h"
#include "SkPathEffect.h"
#include "SkPictureCommon.h"
#include "SkRecord.h"
#include "SkRecordDraw.h"

SkBigPicture::SkBigPicture(const SkRect& cull,
                           SkRecord* record,
                           SnapshotArray* drawablePicts,
                           SkBBoxHierarchy* bbh,
                           AccelData* accelData,
                           size_t approxBytesUsedBySubPictures)
    : fCullRect(cull)
    , fAnalysis(*record)
    , fApproxBytesUsedBySubPictures(approxBytesUsedBySubPictures)
    , fRecord(record)               // Take ownership of caller's ref.
    , fDrawablePicts(drawablePicts) // Take ownership.
    , fBBH(bbh)                     // Take ownership of caller's ref.
    , fAccelData(accelData)         // Take ownership of caller's ref.
{}

void SkBigPicture::playback(SkCanvas* canvas, AbortCallback* callback) const {
    SkASSERT(canvas);

    // If the query contains the whole picture, don't bother with the BBH.
    SkRect clipBounds = { 0, 0, 0, 0 };
    (void)canvas->getClipBounds(&clipBounds);
    const bool useBBH = !clipBounds.contains(this->cullRect());

    SkRecordDraw(*fRecord,
                 canvas,
                 this->drawablePicts(),
                 nullptr,
                 this->drawableCount(),
                 useBBH ? fBBH.get() : nullptr,
                 callback);
}

void SkBigPicture::partialPlayback(SkCanvas* canvas,
                                   unsigned start,
                                   unsigned stop,
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

SkRect SkBigPicture::cullRect()             const { return fCullRect; }
bool   SkBigPicture::hasText()              const { return fAnalysis.fHasText; }
bool   SkBigPicture::willPlayBackBitmaps()  const { return fAnalysis.fWillPlaybackBitmaps; }
int    SkBigPicture::numSlowPaths()         const { return fAnalysis.fNumSlowPathsAndDashEffects; }
int    SkBigPicture::approximateOpCount()   const { return fRecord->count(); }
size_t SkBigPicture::approximateBytesUsed() const {
    size_t bytes = sizeof(*this) + fRecord->bytesUsed() + fApproxBytesUsedBySubPictures;
    if (fBBH) { bytes += fBBH->bytesUsed(); }
    return bytes;
}
bool SkBigPicture::suitableForGpuRasterization(GrContext*, const char** reason) const {
    return fAnalysis.suitableForGpuRasterization(reason);
}

int SkBigPicture::drawableCount() const {
    return fDrawablePicts ? fDrawablePicts->count() : 0;
}
SkPicture const* const* SkBigPicture::drawablePicts() const {
    return fDrawablePicts ? fDrawablePicts->begin() : nullptr;
}

// Some ops have a paint, some have an optional paint.  Either way, get back a pointer.
static const SkPaint* as_ptr(const SkPaint& p) { return &p; }
static const SkPaint* as_ptr(const SkRecords::Optional<SkPaint>& p) { return p; }

struct SkBigPicture::PathCounter {
    SK_CREATE_MEMBER_DETECTOR(paint);

    PathCounter() : fNumSlowPathsAndDashEffects(0) {}

    // Recurse into nested pictures.
    void operator()(const SkRecords::DrawPicture& op) {
        fNumSlowPathsAndDashEffects += op.picture->numSlowPaths();
    }

    void checkPaint(const SkPaint* paint) {
        if (paint && paint->getPathEffect()) {
            // Initially assume it's slow.
            fNumSlowPathsAndDashEffects++;
        }
    }

    void operator()(const SkRecords::DrawPoints& op) {
        this->checkPaint(&op.paint);
        const SkPathEffect* effect = op.paint.getPathEffect();
        if (effect) {
            SkPathEffect::DashInfo info;
            SkPathEffect::DashType dashType = effect->asADash(&info);
            if (2 == op.count && SkPaint::kRound_Cap != op.paint.getStrokeCap() &&
                SkPathEffect::kDash_DashType == dashType && 2 == info.fCount) {
                fNumSlowPathsAndDashEffects--;
            }
        }
    }

    void operator()(const SkRecords::DrawPath& op) {
        this->checkPaint(&op.paint);
        if (op.paint.isAntiAlias() && !op.path.isConvex()) {
            SkPaint::Style paintStyle = op.paint.getStyle();
            const SkRect& pathBounds = op.path.getBounds();
            if (SkPaint::kStroke_Style == paintStyle &&
                0 == op.paint.getStrokeWidth()) {
                // AA hairline concave path is not slow.
            } else if (SkPaint::kFill_Style == paintStyle && pathBounds.width() < 64.f &&
                       pathBounds.height() < 64.f && !op.path.isVolatile()) {
                // AADF eligible concave path is not slow.
            } else {
                fNumSlowPathsAndDashEffects++;
            }
        }
    }

    template <typename T>
    SK_WHEN(HasMember_paint<T>, void) operator()(const T& op) {
        this->checkPaint(as_ptr(op.paint));
    }

    template <typename T>
    SK_WHEN(!HasMember_paint<T>, void) operator()(const T& op) { /* do nothing */ }

    int fNumSlowPathsAndDashEffects;
};

SkBigPicture::Analysis::Analysis(const SkRecord& record) {
    SkTextHunter   text;
    SkBitmapHunter bitmap;
    PathCounter    path;

    bool hasText = false, hasBitmap = false;
    for (unsigned i = 0; i < record.count(); i++) {
        hasText   = hasText   || record.visit<bool>(i,   text);
        hasBitmap = hasBitmap || record.visit<bool>(i, bitmap);
        record.visit<void>(i, path);
    }

    fHasText                    = hasText;
    fWillPlaybackBitmaps        = hasBitmap;
    fNumSlowPathsAndDashEffects = SkTMin<int>(path.fNumSlowPathsAndDashEffects, 255);
}

bool SkBigPicture::Analysis::suitableForGpuRasterization(const char** reason) const {
    if (fNumSlowPathsAndDashEffects > 5) {
        if (reason) { *reason = "Too many slow paths (either concave or dashed)."; }
        return false;
    }
    return true;
}
