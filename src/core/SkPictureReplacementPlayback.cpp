
/*
 * Copyright 2014 Google Inc.
 * 
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkPicture.h"
#include "SkPictureData.h"
#include "SkPictureReplacementPlayback.h"


SkPictureReplacementPlayback::PlaybackReplacements::ReplacementInfo*
SkPictureReplacementPlayback::PlaybackReplacements::push() {
    SkDEBUGCODE(this->validate());
    return fReplacements.push();
}

void SkPictureReplacementPlayback::PlaybackReplacements::freeAll() {
    for (int i = 0; i < fReplacements.count(); ++i) {
        SkDELETE(fReplacements[i].fBM);
    }
    fReplacements.reset();
}

#ifdef SK_DEBUG
void SkPictureReplacementPlayback::PlaybackReplacements::validate() const {
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

// TODO: Replace with hash or pass in "lastLookedUp" hint
SkPictureReplacementPlayback::PlaybackReplacements::ReplacementInfo*
SkPictureReplacementPlayback::PlaybackReplacements::lookupByStart(size_t start) {
    SkDEBUGCODE(this->validate());
    for (int i = 0; i < fReplacements.count(); ++i) {
        if (start == fReplacements[i].fStart) {
            return &fReplacements[i];
        } else if (start < fReplacements[i].fStart) {
            return NULL;  // the ranges are monotonically increasing and non-overlapping
        }
    }

    return NULL;
}

bool SkPictureReplacementPlayback::replaceOps(SkPictureStateTree::Iterator* iter,
                                              SkReader32* reader,
                                              SkCanvas* canvas,
                                              const SkMatrix& initialMatrix) {
    if (NULL != fReplacements) {
        // Potentially replace a block of operations with a single drawBitmap call
        PlaybackReplacements::ReplacementInfo* temp =
                                  fReplacements->lookupByStart(reader->offset());
        if (NULL != temp) {
            SkASSERT(NULL != temp->fBM);
            SkASSERT(NULL != temp->fPaint);
            canvas->save();
            canvas->setMatrix(initialMatrix);
            SkRect src = SkRect::Make(temp->fSrcRect);
            SkRect dst = SkRect::MakeXYWH(temp->fPos.fX, temp->fPos.fY,
                                          temp->fSrcRect.width(),
                                          temp->fSrcRect.height());
            canvas->drawBitmapRectToRect(*temp->fBM, &src, dst, temp->fPaint);
            canvas->restore();

            if (iter->isValid()) {
                // This save is needed since the BBH will automatically issue
                // a restore to balanced the saveLayer we're skipping
                canvas->save();

                // At this point we know that the PictureStateTree was aiming
                // for some draw op within temp's saveLayer (although potentially
                // in a separate saveLayer nested inside it).
                // We need to skip all the operations inside temp's range
                // along with all the associated state changes but update
                // the state tree to the first operation outside temp's range.

                uint32_t skipTo;
                do {
                    skipTo = iter->nextDraw();
                    if (SkPictureStateTree::Iterator::kDrawComplete == skipTo) {
                        break;
                    }

                    if (skipTo <= temp->fStop) {
                        reader->setOffset(skipTo);
                        uint32_t size;
                        DrawType op = ReadOpAndSize(reader, &size);
                        // Since we are relying on the normal SkPictureStateTree
                        // playback we need to convert any nested saveLayer calls
                        // it may issue into saves (so that all its internal
                        // restores will be balanced).
                        if (SAVE_LAYER == op) {
                            canvas->save();
                        }
                    }
                } while (skipTo <= temp->fStop);

                if (SkPictureStateTree::Iterator::kDrawComplete == skipTo) {
                    reader->setOffset(reader->size());      // skip to end
                    return true;
                }

                reader->setOffset(skipTo);
            } else {
                reader->setOffset(temp->fStop);
                uint32_t size;
                SkDEBUGCODE(DrawType op = ) ReadOpAndSize(reader, &size);
                SkASSERT(RESTORE == op);
            }

            return true;
        }
    }

    return false;
}

void SkPictureReplacementPlayback::draw(SkCanvas* canvas, SkDrawPictureCallback* callback) {
    AutoResetOpID aroi(this);
    SkASSERT(0 == fCurOffset);

    SkPictureStateTree::Iterator it;

    if (!this->initIterator(&it, canvas, fActiveOpsList)) {
        return;  // nothing to draw
    }

    SkReader32 reader(fPictureData->opData()->bytes(), fPictureData->opData()->size());

    StepIterator(&it, &reader);

    // Record this, so we can concat w/ it if we encounter a setMatrix()
    SkMatrix initialMatrix = canvas->getTotalMatrix();

    SkAutoCanvasRestore acr(canvas, false);

    while (!reader.eof()) {
        if (NULL != callback && callback->abortDrawing()) {
            return;
        }

        if (this->replaceOps(&it, &reader, canvas, initialMatrix)) {
            continue;
        }

        fCurOffset = reader.offset();
        uint32_t size;
        DrawType op = ReadOpAndSize(&reader, &size);
        if (NOOP == op) {
            // NOOPs are to be ignored - do not propagate them any further
            SkipIterTo(&it, &reader, fCurOffset + size);
            continue;
        }

        this->handleOp(&reader, op, size, canvas, initialMatrix);

        StepIterator(&it, &reader);
    }
}
