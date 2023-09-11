/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkDrawable.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"  // IWYU pragma: keep
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"

#include <atomic>
#include <cstddef>
#include <cstdint>

static int32_t next_generation_id() {
    static std::atomic<int32_t> nextID{1};

    int32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == 0);
    return id;
}

SkDrawable::SkDrawable() : fGenerationID(0) {}

static void draw_bbox(SkCanvas* canvas, const SkRect& r) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(0xFFFF7088);
    canvas->drawRect(r, paint);
    canvas->drawLine(r.left(), r.top(), r.right(), r.bottom(), paint);
    canvas->drawLine(r.left(), r.bottom(), r.right(), r.top(), paint);
}

void SkDrawable::draw(SkCanvas* canvas, const SkMatrix* matrix) {
    SkAutoCanvasRestore acr(canvas, true);
    if (matrix) {
        canvas->concat(*matrix);
    }
    this->onDraw(canvas);

    if ((false)) {
        draw_bbox(canvas, this->getBounds());
    }
}

void SkDrawable::draw(SkCanvas* canvas, SkScalar x, SkScalar y) {
    SkMatrix matrix = SkMatrix::Translate(x, y);
    this->draw(canvas, &matrix);
}

sk_sp<SkPicture> SkDrawable::makePictureSnapshot() {
    return this->onMakePictureSnapshot();
}

uint32_t SkDrawable::getGenerationID() {
    if (0 == fGenerationID) {
        fGenerationID = next_generation_id();
    }
    return fGenerationID;
}

SkRect SkDrawable::getBounds() {
    return this->onGetBounds();
}

size_t SkDrawable::approximateBytesUsed() {
    return this->onApproximateBytesUsed();
}
size_t SkDrawable::onApproximateBytesUsed() {
    return 0;
}

void SkDrawable::notifyDrawingChanged() {
    fGenerationID = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkPicture> SkDrawable::onMakePictureSnapshot() {
    SkPictureRecorder recorder;

    const SkRect bounds = this->getBounds();
    SkCanvas* canvas = recorder.beginRecording(bounds);
    this->draw(canvas);
    if ((false)) {
        draw_bbox(canvas, bounds);
    }
    return recorder.finishRecordingAsPicture();
}
