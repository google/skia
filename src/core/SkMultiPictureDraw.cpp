/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkCanvasPriv.h"
#include "SkMultiPictureDraw.h"
#include "SkPicture.h"
#include "SkTaskGroup.h"

void SkMultiPictureDraw::DrawData::draw() {
    fCanvas->drawPicture(fPicture, &fMatrix, fPaint);
}

void SkMultiPictureDraw::DrawData::init(SkCanvas* canvas, const SkPicture* picture,
                                        const SkMatrix* matrix, const SkPaint* paint) {
    fPicture = SkRef(picture);
    fCanvas = canvas;
    if (matrix) {
        fMatrix = *matrix;
    } else {
        fMatrix.setIdentity();
    }
    if (paint) {
        fPaint = new SkPaint(*paint);
    } else {
        fPaint = nullptr;
    }
}

void SkMultiPictureDraw::DrawData::Reset(SkTDArray<DrawData>& data) {
    for (int i = 0; i < data.count(); ++i) {
        data[i].fPicture->unref();
        delete data[i].fPaint;
    }
    data.rewind();
}

//////////////////////////////////////////////////////////////////////////////////////

SkMultiPictureDraw::SkMultiPictureDraw(int reserve) {
    if (reserve > 0) {
        fGPUDrawData.setReserve(reserve);
        fThreadSafeDrawData.setReserve(reserve);
    }
}

void SkMultiPictureDraw::reset() {
    DrawData::Reset(fGPUDrawData);
    DrawData::Reset(fThreadSafeDrawData);
}

void SkMultiPictureDraw::add(SkCanvas* canvas,
                             const SkPicture* picture,
                             const SkMatrix* matrix,
                             const SkPaint* paint) {
    if (nullptr == canvas || nullptr == picture) {
        SkDEBUGFAIL("parameters to SkMultiPictureDraw::add should be non-nullptr");
        return;
    }

    SkTDArray<DrawData>& array = canvas->getGrContext() ? fGPUDrawData : fThreadSafeDrawData;
    array.append()->init(canvas, picture, matrix, paint);
}

class AutoMPDReset : SkNoncopyable {
    SkMultiPictureDraw* fMPD;
public:
    AutoMPDReset(SkMultiPictureDraw* mpd) : fMPD(mpd) {}
    ~AutoMPDReset() { fMPD->reset(); }
};

//#define FORCE_SINGLE_THREAD_DRAWING_FOR_TESTING

void SkMultiPictureDraw::draw(bool flush) {
    AutoMPDReset mpdreset(this);

#ifdef FORCE_SINGLE_THREAD_DRAWING_FOR_TESTING
    for (int i = 0; i < fThreadSafeDrawData.count(); ++i) {
        fThreadSafeDrawData[i].draw();
    }
#else
    SkTaskGroup().batch(fThreadSafeDrawData.count(), [&](int i) {
        fThreadSafeDrawData[i].draw();
    });
#endif

    // N.B. we could get going on any GPU work from this main thread while the CPU work runs.
    // But in practice, we've either got GPU work or CPU work, not both.

    const int count = fGPUDrawData.count();
    if (0 == count) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        const DrawData& data = fGPUDrawData[i];
        SkCanvas* canvas = data.fCanvas;
        const SkPicture* picture = data.fPicture;

        canvas->drawPicture(picture, &data.fMatrix, data.fPaint);
        if (flush) {
            canvas->flush();
        }
    }
}
