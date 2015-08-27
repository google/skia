/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAtomics.h"
#include "SkCanvas.h"
#include "SkDrawable.h"

static int32_t next_generation_id() {
    static int32_t gCanvasDrawableGenerationID;

    // do a loop in case our global wraps around, as we never want to
    // return a 0
    int32_t genID;
    do {
        genID = sk_atomic_inc(&gCanvasDrawableGenerationID) + 1;
    } while (0 == genID);
    return genID;
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

    if (false) {
        draw_bbox(canvas, this->getBounds());
    }
}

void SkDrawable::draw(SkCanvas* canvas, SkScalar x, SkScalar y) {
    SkMatrix matrix = SkMatrix::MakeTrans(x, y);
    this->draw(canvas, &matrix);
}

SkPicture* SkDrawable::newPictureSnapshot() {
    return this->onNewPictureSnapshot();
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

void SkDrawable::notifyDrawingChanged() {
    fGenerationID = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

#include "SkPictureRecorder.h"

SkPicture* SkDrawable::onNewPictureSnapshot() {
    SkPictureRecorder recorder;

    const SkRect bounds = this->getBounds();
    SkCanvas* canvas = recorder.beginRecording(bounds, nullptr, 0);
    this->draw(canvas);
    if (false) {
        draw_bbox(canvas, bounds);
    }
    return recorder.endRecording();
}
