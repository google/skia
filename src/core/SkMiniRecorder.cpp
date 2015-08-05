/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkTLazy.h"
#include "SkLazyPtr.h"
#include "SkMiniRecorder.h"
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
    bool   hasText()              const override { return false; }
    int    numSlowPaths()         const override { return 0; }
    bool   willPlayBackBitmaps()  const override { return false; }
};
SK_DECLARE_STATIC_LAZY_PTR(SkEmptyPicture, gEmptyPicture);

template <typename T>
class SkMiniPicture final : public SkPicture {
public:
    SkMiniPicture(SkRect cull, T* op) : fCull(cull) {
        memcpy(&fOp, op, sizeof(fOp));  // We take ownership of op's guts.
    }

    void playback(SkCanvas* c, AbortCallback*) const override {
        SkRecords::Draw(c, nullptr, nullptr, 0, nullptr)(fOp);
    }

    size_t approximateBytesUsed() const override { return sizeof(*this); }
    int    approximateOpCount()   const override { return 1; }
    SkRect cullRect()             const override { return fCull; }
    bool   hasText()              const override { return SkTextHunter()(fOp); }
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
        SkDELETE(this->detachAsPicture(SkRect::MakeEmpty()));
    }
    SkASSERT(fState == State::kEmpty);
}

#define TRY_TO_STORE(Type, ...)                    \
    if (fState != State::kEmpty) { return false; } \
    fState = State::k##Type;                       \
    new (fBuffer.get()) Type(__VA_ARGS__);         \
    return true

bool SkMiniRecorder::drawBitmapRect(const SkBitmap& bm, const SkRect* src, const SkRect& dst,
                                    const SkPaint* p, SkCanvas::SrcRectConstraint constraint) {
    SkRect bounds;
    if (!src) {
        bm.getBounds(&bounds);
        src = &bounds;
    }
    SkTLazy<SkPaint> defaultPaint;
    if (!p) {
        p = defaultPaint.init();
    }
    TRY_TO_STORE(DrawBitmapRectFixedSize, *p, bm, *src, dst, constraint);
}

bool SkMiniRecorder::drawRect(const SkRect& rect, const SkPaint& paint) {
    TRY_TO_STORE(DrawRect, paint, rect);
}

bool SkMiniRecorder::drawPath(const SkPath& path, const SkPaint& paint) {
    TRY_TO_STORE(DrawPath, paint, path);
}

bool SkMiniRecorder::drawTextBlob(const SkTextBlob* b, SkScalar x, SkScalar y, const SkPaint& p) {
    TRY_TO_STORE(DrawTextBlob, p, b, x, y);
}
#undef TRY_TO_STORE


SkPicture* SkMiniRecorder::detachAsPicture(const SkRect& cull) {
#define CASE(Type)               \
    case State::k##Type:         \
        fState = State::kEmpty;  \
        return SkNEW_ARGS(SkMiniPicture<Type>, (cull, reinterpret_cast<Type*>(fBuffer.get())))

    switch (fState) {
        case State::kEmpty: return SkRef(gEmptyPicture.get());
        CASE(DrawBitmapRectFixedSize);
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
        CASE(DrawBitmapRectFixedSize);
        CASE(DrawPath);
        CASE(DrawRect);
        CASE(DrawTextBlob);
    }
    SkASSERT(false);
#undef CASE
}
