/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkCanvasDrawable.h"
#include "SkThread.h"

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

SkCanvasDrawable::SkCanvasDrawable() : fGenerationID(0) {}

static void draw_bbox(SkCanvas* canvas, const SkRect& r) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(0xFFFF7088);
    canvas->drawRect(r, paint);
    canvas->drawLine(r.left(), r.top(), r.right(), r.bottom(), paint);
    canvas->drawLine(r.left(), r.bottom(), r.right(), r.top(), paint);
}

void SkCanvasDrawable::draw(SkCanvas* canvas) {
    SkAutoCanvasRestore acr(canvas, true);
    this->onDraw(canvas);

    if (false) {
        draw_bbox(canvas, this->getBounds());
    }
}

SkPicture* SkCanvasDrawable::newPictureSnapshot() {
    return this->onNewPictureSnapshot();
}

uint32_t SkCanvasDrawable::getGenerationID() {
    if (0 == fGenerationID) {
        fGenerationID = next_generation_id();
    }
    return fGenerationID;
}

SkRect SkCanvasDrawable::getBounds() {
    return this->onGetBounds();
}

void SkCanvasDrawable::notifyDrawingChanged() {
    fGenerationID = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

#include "SkPictureRecorder.h"

SkPicture* SkCanvasDrawable::onNewPictureSnapshot() {
    SkPictureRecorder recorder;

    const SkRect bounds = this->getBounds();
    SkCanvas* canvas = recorder.beginRecording(bounds, NULL, 0);
    this->draw(canvas);
    if (false) {
        draw_bbox(canvas, bounds);
    }
    return recorder.endRecording();
}
