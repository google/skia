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
                                                                unsigned int start,
                                                                const SkMatrix& ctm) {
    ReplacementInfo* replacement = SkNEW_ARGS(ReplacementInfo, (pictureID, start, ctm));
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

const GrReplacements::ReplacementInfo* GrReplacements::lookupByStart(uint32_t pictureID,
                                                                     size_t start,
                                                                     const SkMatrix& ctm) const {
    return fReplacementHash.find(ReplacementInfo::Key(pictureID, start, ctm));
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

// Used by GrRecordReplaceDraw. It intercepts nested drawPicture calls and
// also draws them with replaced layers.
class ReplaceDraw : public SkRecords::Draw {
public:
    ReplaceDraw(SkCanvas* canvas,
                const SkPicture* picture,
                const GrReplacements* replacements,
                const SkMatrix& initialMatrix,
                SkDrawPictureCallback* callback)
        : INHERITED(canvas)
        , fCanvas(canvas)
        , fPicture(picture)
        , fReplacements(replacements)
        , fInitialMatrix(initialMatrix)
        , fCallback(callback)
        , fIndex(0)
        , fNumReplaced(0) {
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
        SkAutoCanvasMatrixPaint acmp(fCanvas, dp.matrix, dp.paint, dp.picture->cullRect());

        // Draw sub-pictures with the same replacement list but a different picture
        ReplaceDraw draw(fCanvas, dp.picture, fReplacements, fInitialMatrix, fCallback);

        fNumReplaced += draw.draw();
    }
    void operator()(const SkRecords::SaveLayer& sl) {

        // For a saveLayer command, check if it can be replaced by a drawBitmap
        // call and, if so, draw it and then update the current op index accordingly.
        size_t startOffset;
        if (fOps.count()) {
            startOffset = fOps[fIndex];
        } else {
            startOffset = fIndex;
        }

        const SkMatrix& ctm = fCanvas->getTotalMatrix();
        const GrReplacements::ReplacementInfo* ri = fReplacements->lookupByStart(
                                                            fPicture->uniqueID(),
                                                            startOffset,
                                                            ctm);

        if (ri) {
            fNumReplaced++;
            draw_replacement_bitmap(ri, fCanvas, fInitialMatrix);

            if (fPicture->fBBH.get()) {
                while (fOps[fIndex] < ri->fStop) {
                    ++fIndex;
                }
                SkASSERT(fOps[fIndex] == ri->fStop);
            } else {
                fIndex = ri->fStop;
            }
            return;
        }

        // This is a fail for layer hoisting
        this->INHERITED::operator()(sl);
    }

private:
    SkCanvas*              fCanvas;
    const SkPicture*       fPicture;
    const GrReplacements*  fReplacements;
    const SkMatrix         fInitialMatrix;
    SkDrawPictureCallback* fCallback;

    SkTDArray<unsigned>    fOps;
    int                    fIndex;
    int                    fNumReplaced;

    typedef Draw INHERITED;
};

int GrRecordReplaceDraw(const SkPicture* picture,
                        SkCanvas* canvas,
                        const GrReplacements* replacements,
                        const SkMatrix& initialMatrix,
                        SkDrawPictureCallback* callback) {
    SkAutoCanvasRestore saveRestore(canvas, true /*save now, restore at exit*/);

    ReplaceDraw draw(canvas, picture, replacements, initialMatrix, callback);

    return draw.draw();
}
