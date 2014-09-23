/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRecordReplaceDraw.h"
#include "SkImage.h"
#include "SkRecordDraw.h"

GrReplacements::ReplacementInfo* GrReplacements::push() {
    SkDEBUGCODE(this->validate());
    return fReplacements.push();
}

void GrReplacements::freeAll() {
    for (int i = 0; i < fReplacements.count(); ++i) {
        fReplacements[i].fImage->unref();
        SkDELETE(fReplacements[i].fPaint);
    }
    fReplacements.reset();
}

#ifdef SK_DEBUG
void GrReplacements::validate() const {
    // Check that the ranges are monotonically increasing and non-overlapping
    if (fReplacements.count() > 0) {
        SkASSERT(fReplacements[0].fStart < fReplacements[0].fStop);

        for (int i = 1; i < fReplacements.count(); ++i) {
            SkASSERT(fReplacements[i].fStart < fReplacements[i].fStop);
            SkASSERT(fReplacements[i - 1].fStop < fReplacements[i].fStart);
        }
    }
}
#endif

const GrReplacements::ReplacementInfo*
GrReplacements::lookupByStart(size_t start, int* searchStart) const {
    SkDEBUGCODE(this->validate());
    for (int i = *searchStart; i < fReplacements.count(); ++i) {
        if (start == fReplacements[i].fStart) {
            *searchStart = i + 1;
            return &fReplacements[i];
        } else if (start < fReplacements[i].fStart) {
            return NULL;  // the ranges are monotonically increasing and non-overlapping
        }
    }

    return NULL;
}

static inline void draw_replacement_bitmap(const GrReplacements::ReplacementInfo* ri,
                                           SkCanvas* canvas,
                                           const SkMatrix& initialMatrix) {
    SkRect src = SkRect::Make(ri->fSrcRect);
    SkRect dst = SkRect::MakeXYWH(SkIntToScalar(ri->fPos.fX),
                                  SkIntToScalar(ri->fPos.fY),
                                  SkIntToScalar(ri->fSrcRect.width()),
                                  SkIntToScalar(ri->fSrcRect.height()));

    canvas->save();
    canvas->setMatrix(initialMatrix);
    canvas->drawImageRect(ri->fImage, &src, dst, ri->fPaint);
    canvas->restore();
}

void GrRecordReplaceDraw(const SkRecord& record,
                         SkCanvas* canvas,
                         const SkBBoxHierarchy* bbh,
                         const GrReplacements* replacements,
                         SkDrawPictureCallback* callback) {
    SkAutoCanvasRestore saveRestore(canvas, true /*save now, restore at exit*/);

    SkRecords::Draw draw(canvas);
    const GrReplacements::ReplacementInfo* ri = NULL;
    int searchStart = 0;

    const SkMatrix initialMatrix = canvas->getTotalMatrix();

    if (bbh) {
        // Draw only ops that affect pixels in the canvas's current clip.
        // The SkRecord and BBH were recorded in identity space.  This canvas
        // is not necessarily in that same space.  getClipBounds() returns us
        // this canvas' clip bounds transformed back into identity space, which
        // lets us query the BBH.
        SkRect query = { 0, 0, 0, 0 };
        (void)canvas->getClipBounds(&query);

        SkTDArray<void*> ops;
        bbh->search(query, &ops);

        for (int i = 0; i < ops.count(); i++) {
            if (callback && callback->abortDrawing()) {
                return;
            }
            ri = replacements->lookupByStart((uintptr_t)ops[i], &searchStart);
            if (ri) {
                draw_replacement_bitmap(ri, canvas, initialMatrix);

                while ((uintptr_t)ops[i] < ri->fStop) {
                    ++i;
                }
                SkASSERT((uintptr_t)ops[i] == ri->fStop);
                continue;
            }

            record.visit<void>((uintptr_t)ops[i], draw);
        }
    } else {
        for (unsigned int i = 0; i < record.count(); ++i) {
            if (callback && callback->abortDrawing()) {
                return;
            }
            ri = replacements->lookupByStart(i, &searchStart);
            if (ri) {
                draw_replacement_bitmap(ri, canvas, initialMatrix);
                i = ri->fStop;
                continue;
            }

            record.visit<void>(i, draw);
        }
    }
}
