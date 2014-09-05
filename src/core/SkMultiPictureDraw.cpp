/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkMultiPictureDraw.h"
#include "SkPicture.h"

SkMultiPictureDraw::SkMultiPictureDraw(int reserve) {
    if (reserve > 0) {
        fDrawData.setReserve(reserve);
    }
}

void SkMultiPictureDraw::reset() {
    for (int i = 0; i < fDrawData.count(); ++i) {
        fDrawData[i].picture->unref();
        fDrawData[i].canvas->unref();
        SkDELETE(fDrawData[i].paint);
    }

    fDrawData.rewind();
}

void SkMultiPictureDraw::add(SkCanvas* canvas, 
                             const SkPicture* picture,
                             const SkMatrix* matrix, 
                             const SkPaint* paint) {
    if (NULL == canvas || NULL == picture) {
        SkDEBUGFAIL("parameters to SkMultiPictureDraw::add should be non-NULL");
        return;
    }

    DrawData* data = fDrawData.append();

    data->picture = SkRef(picture);
    data->canvas = SkRef(canvas);
    if (matrix) {
        data->matrix = *matrix;
    } else {
        data->matrix.setIdentity();
    }
    if (paint) {
        data->paint = SkNEW_ARGS(SkPaint, (*paint));
    } else {
        data->paint = NULL;
    }
}

void SkMultiPictureDraw::draw() {
    for (int i = 0; i < fDrawData.count(); ++i) {
        fDrawData[i].canvas->drawPicture(fDrawData[i].picture, 
                                         &fDrawData[i].matrix, 
                                         fDrawData[i].paint);
    }

    this->reset();
}

