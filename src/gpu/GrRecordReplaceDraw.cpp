/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRecordReplaceDraw.h"
#include "SkCanvasPriv.h"
#include "SkImage.h"
#include "SkRecordDraw.h"
#include "SkRecords.h"

GrReplacements::ReplacementInfo* GrReplacements::newReplacement(uint32_t pictureID,
                                                                const SkMatrix& initialMat,
                                                                const int* key, int keySize) {
    ReplacementInfo* replacement = SkNEW_ARGS(ReplacementInfo, (pictureID, initialMat, 
                                                                key, keySize));
    fReplacementHash.add(replacement);
    return replacement;
}

void GrReplacements::freeAll() {
    SkTDynamicHash<ReplacementInfo, ReplacementInfo::Key>::Iter iter(&fReplacementHash);

    for (; !iter.done(); ++iter) {
        ReplacementInfo* replacement = &(*iter);
        SkDELETE(replacement);
    }

    fReplacementHash.reset();
}

const GrReplacements::ReplacementInfo* GrReplacements::lookup(uint32_t pictureID,
                                                              const SkMatrix& initialMat,
                                                              const int* key,
                                                              int keySize) const {
    return fReplacementHash.find(ReplacementInfo::Key(pictureID, initialMat, key, keySize));
}

static inline void draw_replacement_bitmap(const GrReplacements::ReplacementInfo* ri,
                                           SkCanvas* canvas) {
    SkRect src = SkRect::Make(ri->fSrcRect);
    SkRect dst = SkRect::MakeXYWH(SkIntToScalar(ri->fPos.fX),
                                  SkIntToScalar(ri->fPos.fY),
                                  SkIntToScalar(ri->fSrcRect.width()),
                                  SkIntToScalar(ri->fSrcRect.height()));

    canvas->save();
    canvas->setMatrix(SkMatrix::I());
    canvas->drawImageRect(ri->fImage, &src, dst, ri->fPaint);
    canvas->restore();
}

// Used by GrRecordReplaceDraw. It intercepts nested drawPicture calls and
// also draws them with replaced layers.
class ReplaceDraw : public SkRecords::Draw {
public:
    ReplaceDraw(SkCanvas* canvas,
                SkPicture const* const drawablePicts[], int drawableCount,
                const SkPicture* topLevelPicture,
                const SkPicture* picture,
                const GrReplacements* replacements,
                const SkMatrix& initialMatrix,
                SkDrawPictureCallback* callback,
                const int* opIndices, int numIndices)
        : INHERITED(canvas, drawablePicts, NULL, drawableCount)
        , fCanvas(canvas)
        , fTopLevelPicture(topLevelPicture)
        , fPicture(picture)
        , fReplacements(replacements)
        , fInitialMatrix(initialMatrix)
        , fCallback(callback)
        , fIndex(0)
        , fNumReplaced(0) {
        fOpIndexStack.append(numIndices, opIndices);
    }

    int draw() {
        const SkBBoxHierarchy* bbh = fPicture->fBBH.get();
        const SkRecord* record = fPicture->fRecord.get();
        if (NULL == record) {
            return 0;
        }

        fNumReplaced = 0;

        fOps.rewind();

        if (bbh) {
            // Draw only ops that affect pixels in the canvas's current clip.
            // The SkRecord and BBH were recorded in identity space.  This canvas
            // is not necessarily in that same space.  getClipBounds() returns us
            // this canvas' clip bounds transformed back into identity space, which
            // lets us query the BBH.
            SkRect query = { 0, 0, 0, 0 };
            (void)fCanvas->getClipBounds(&query);

            bbh->search(query, &fOps);

            for (fIndex = 0; fIndex < fOps.count(); ++fIndex) {
                if (fCallback && fCallback->abortDrawing()) {
                    return fNumReplaced;
                }

                record->visit<void>(fOps[fIndex], *this);
            }

        } else {
            for (fIndex = 0; fIndex < (int) record->count(); ++fIndex) {
                if (fCallback && fCallback->abortDrawing()) {
                    return fNumReplaced;
                }

                record->visit<void>(fIndex, *this);
            }
        }

        return fNumReplaced;
    }

    // Same as Draw for all ops except DrawPicture and SaveLayer.
    template <typename T> void operator()(const T& r) {
        this->INHERITED::operator()(r);
    }
    void operator()(const SkRecords::DrawPicture& dp) {

        int drawPictureOffset;
        if (fOps.count()) {
            drawPictureOffset = fOps[fIndex];
        } else {
            drawPictureOffset = fIndex;
        }

        fOpIndexStack.push(drawPictureOffset);

        SkAutoCanvasMatrixPaint acmp(fCanvas, dp.matrix, dp.paint, dp.picture->cullRect());

        // Draw sub-pictures with the same replacement list but a different picture
        ReplaceDraw draw(fCanvas, this->drawablePicts(), this->drawableCount(),
                         fTopLevelPicture, dp.picture, fReplacements, fInitialMatrix, fCallback,
                         fOpIndexStack.begin(), fOpIndexStack.count());

        fNumReplaced += draw.draw();

        fOpIndexStack.pop();
    }
    void operator()(const SkRecords::SaveLayer& sl) {

        // For a saveLayer command, check if it can be replaced by a drawBitmap
        // call and, if so, draw it and then update the current op index accordingly.
        unsigned startOffset;
        if (fOps.count()) {
            startOffset = fOps[fIndex];
        } else {
            startOffset = fIndex;
        }

        fOpIndexStack.push(startOffset);

        const GrReplacements::ReplacementInfo* ri = fReplacements->lookup(
                                                                    fTopLevelPicture->uniqueID(),
                                                                    fInitialMatrix,
                                                                    fOpIndexStack.begin(),
                                                                    fOpIndexStack.count());

        if (ri) {
            fNumReplaced++;
            draw_replacement_bitmap(ri, fCanvas);

            if (fPicture->fBBH.get()) {
                while (fOps[fIndex] < ri->fStop) {
                    ++fIndex;
                }
                SkASSERT(fOps[fIndex] == ri->fStop);
            } else {
                fIndex = ri->fStop;
            }
            fOpIndexStack.pop();
            return;
        }

        // This is a fail for layer hoisting
        this->INHERITED::operator()(sl);

        fOpIndexStack.pop();
    }

private:
    SkCanvas*              fCanvas;
    const SkPicture*       fTopLevelPicture;
    const SkPicture*       fPicture;
    const GrReplacements*  fReplacements;
    const SkMatrix         fInitialMatrix;
    SkDrawPictureCallback* fCallback;

    SkTDArray<unsigned>    fOps;
    int                    fIndex;
    int                    fNumReplaced;

    // The op code indices of all the enclosing drawPicture and saveLayer calls
    SkTDArray<int>         fOpIndexStack;

    typedef Draw INHERITED;
};

int GrRecordReplaceDraw(const SkPicture* picture,
                        SkCanvas* canvas,
                        const GrReplacements* replacements,
                        const SkMatrix& initialMatrix,
                        SkDrawPictureCallback* callback) {
    SkAutoCanvasRestore saveRestore(canvas, true /*save now, restore at exit*/);

    // TODO: drawablePicts?
    ReplaceDraw draw(canvas, NULL, 0, 
                     picture, picture, 
                     replacements, initialMatrix, callback, NULL, 0);

    return draw.draw();
}
