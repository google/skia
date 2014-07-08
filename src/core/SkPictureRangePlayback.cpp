/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkPictureData.h"
#include "SkPictureRangePlayback.h"

void SkPictureRangePlayback::draw(SkCanvas* canvas, SkDrawPictureCallback* callback) {
    AutoResetOpID aroi(this);
    SkASSERT(0 == fCurOffset);

    SkReader32 reader(fPictureData->opData()->bytes(), fPictureData->opData()->size());

    if (0 != fStart || 0 != fStop) {
        reader.setOffset(fStart);
        uint32_t size;
        SkDEBUGCODE(DrawType op = ) ReadOpAndSize(&reader, &size);
        SkASSERT(SAVE_LAYER == op);
        reader.setOffset(fStart + size);
    }

    // Record this, so we can concat w/ it if we encounter a setMatrix()
    SkMatrix initialMatrix = canvas->getTotalMatrix();

    SkAutoCanvasRestore acr(canvas, false);

    while (!reader.eof()) {
        if (NULL != callback && callback->abortDrawing()) {
            return;
        }

        if (0 != fStart || 0 != fStop) {
            size_t offset = reader.offset();
            if (offset >= fStop) {
                SkDEBUGCODE(uint32_t size;)
                SkDEBUGCODE(DrawType op = ReadOpAndSize(&reader, &size);)
                SkASSERT(RESTORE == op);
                return;
            }
        }

        fCurOffset = reader.offset();
        uint32_t size;
        DrawType op = ReadOpAndSize(&reader, &size);
        if (NOOP == op) {
            // NOOPs are to be ignored - do not propagate them any further
            reader.setOffset(fCurOffset + size);
            continue;
        }

        this->handleOp(&reader, op, size, canvas, initialMatrix);
    }
}
