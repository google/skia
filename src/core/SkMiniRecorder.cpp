/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkTLazy.h"
#include "SkMiniRecorder.h"
#include "SkOnce.h"
#include "SkPicture.h"
#include "SkPictureCommon.h"
#include "SkRecordDraw.h"
#include "SkTextBlob.h"

using namespace SkRecords;

class SkEmptyPicture final : public SkPicture {
public:
    void playback(SkCanvas*, AbortCallback*) const override { }

    size_t approximateBytesUsed() const override { return sizeof(*this); }
    int    approximateOpCount()   const override { return 0; }
    SkRect cullRect()             const override { return SkRect::MakeEmpty(); }
    int    numSlowPaths()         const override { return 0; }
    bool   willPlayBackBitmaps()  const override { return false; }
};

// Calculate conservative bounds for each type of draw op that can be its own mini picture.
// These are fairly easy because we know they can't be affected by any matrix or saveLayers.
static SkRect adjust_for_paint(SkRect bounds, const SkPaint& paint) {
    return paint.canComputeFastBounds() ? paint.computeFastBounds(bounds, &bounds)
                                        : SkRect::MakeLargest();
}
static SkRect bounds(const DrawRect& op) {
    return adjust_for_paint(op.rect, op.paint);
}
static SkRect bounds(const DrawPath& op) {
    return op.path.isInverseFillType() ? SkRect::MakeLargest()
                                       : adjust_for_paint(op.path.getBounds(), op.paint);
}
static SkRect bounds(const DrawTextBlob& op) {
    return adjust_for_paint(op.blob->bounds().makeOffset(op.x, op.y), op.paint);
}

template <typename T>
class SkMiniPicture final : public SkPicture {
public:
    SkMiniPicture(const SkRect* cull, T* op) : fCull(cull ? *cull : bounds(*op)) {
        memcpy(&fOp, op, sizeof(fOp));  // We take ownership of op's guts.
    }

    void playback(SkCanvas* c, AbortCallback*) const override {
        SkRecords::Draw(c, nullptr, nullptr, 0, nullptr)(fOp);
    }

    size_t approximateBytesUsed() const override { return sizeof(*this); }
    int    approximateOpCount()   const override { return 1; }
    SkRect cullRect()             const override { return fCull; }
    bool   willPlayBackBitmaps()  const override { return SkBitmapHunter()(fOp); }
    int    numSlowPaths()         const override {
        SkPathCounter counter;
        counter(fOp);
        return counter.fNumSlowPathsAndDashEffects;
    }

private:
    SkRect fCull;
    T      fOp;
};


SkMiniRecorder::SkMiniRecorder() : fState(State::kEmpty) {}
SkMiniRecorder::~SkMiniRecorder() {
    if (fState != State::kEmpty) {
        // We have internal state pending.
        // Detaching then deleting a picture is an easy way to clean up.
        (void)this->detachAsPicture(nullptr);
    }
    SkASSERT(fState == State::kEmpty);
}

#define TRY_TO_STORE(Type, ...)                    \
    if (fState != State::kEmpty) { return false; } \
    fState = State::k##Type;                       \
    new (fBuffer.get()) Type{__VA_ARGS__};         \
    return true

bool SkMiniRecorder::drawRect(const SkRect& rect, const SkPaint& paint) {
    TRY_TO_STORE(DrawRect, paint, rect);
}

bool SkMiniRecorder::drawPath(const SkPath& path, const SkPaint& paint) {
    TRY_TO_STORE(DrawPath, paint, path);
}

bool SkMiniRecorder::drawTextBlob(const SkTextBlob* b, SkScalar x, SkScalar y, const SkPaint& p) {
    TRY_TO_STORE(DrawTextBlob, p, sk_ref_sp(b), x, y);
}
#undef TRY_TO_STORE


sk_sp<SkPicture> SkMiniRecorder::detachAsPicture(const SkRect* cull) {
#define CASE(Type)              \
    case State::k##Type:        \
        fState = State::kEmpty; \
        return sk_make_sp<SkMiniPicture<Type>>(cull, reinterpret_cast<Type*>(fBuffer.get()))

    static SkOnce once;
    static SkPicture* empty;

    switch (fState) {
        case State::kEmpty:
            once([]{ empty = new SkEmptyPicture; });
            return sk_ref_sp(empty);
        CASE(DrawPath);
        CASE(DrawRect);
        CASE(DrawTextBlob);
    }
    SkASSERT(false);
    return nullptr;
#undef CASE
}

void SkMiniRecorder::flushAndReset(SkCanvas* canvas) {
#define CASE(Type)                                                  \
    case State::k##Type: {                                          \
        fState = State::kEmpty;                                     \
        Type* op = reinterpret_cast<Type*>(fBuffer.get());          \
        SkRecords::Draw(canvas, nullptr, nullptr, 0, nullptr)(*op); \
        op->~Type();                                                \
    } return

    switch (fState) {
        case State::kEmpty: return;
        CASE(DrawPath);
        CASE(DrawRect);
        CASE(DrawTextBlob);
    }
    SkASSERT(false);
#undef CASE
}
