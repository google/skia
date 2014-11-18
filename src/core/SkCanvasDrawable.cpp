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

void SkCanvasDrawable::draw(SkCanvas* canvas) {
    SkAutoCanvasRestore acr(canvas, true);
    this->onDraw(canvas);
}

SkPicture* SkCanvasDrawable::newPictureSnapshot(SkBBHFactory* bbhFactory, uint32_t recordFlags) {
    return this->onNewPictureSnapshot(bbhFactory, recordFlags);
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

SkPicture* SkCanvasDrawable::onNewPictureSnapshot(SkBBHFactory* bbhFactory, uint32_t recordFlags) {
    const SkRect bounds = this->getBounds();
    SkPictureRecorder recorder;
    this->draw(recorder.beginRecording(bounds.width(), bounds.height(), bbhFactory, recordFlags));
    return recorder.endRecording();
}
