/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if SK_SUPPORT_GPU
#include "GrLayerHoister.h"
#include "GrRecordReplaceDraw.h"
#endif

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

#undef SK_IGNORE_GPU_LAYER_HOISTING
#define SK_IGNORE_GPU_LAYER_HOISTING 1

void SkMultiPictureDraw::draw() {

#ifndef SK_IGNORE_GPU_LAYER_HOISTING
    GrContext* context = NULL;

    SkTDArray<GrHoistedLayer> atlased, nonAtlased, recycled;

    for (int i = 0; i < fDrawData.count(); ++i) {
        if (fDrawData[i].canvas->getGrContext() &&
            !fDrawData[i].paint && fDrawData[i].matrix.isIdentity()) {
            SkASSERT(NULL == context || context == fDrawData[i].canvas->getGrContext());
            context = fDrawData[i].canvas->getGrContext();

            // TODO: this path always tries to optimize pictures. Should we
            // switch to this API approach (vs. SkCanvas::EXPERIMENTAL_optimize)?
            fDrawData[i].canvas->EXPERIMENTAL_optimize(fDrawData[i].picture);

            SkRect clipBounds;
            if (!fDrawData[i].canvas->getClipBounds(&clipBounds)) {
                continue;
            }

            GrLayerHoister::FindLayersToHoist(context, fDrawData[i].picture,
                                              clipBounds, &atlased, &nonAtlased, &recycled);
        }
    }

    GrReplacements replacements;

    if (NULL != context) {
        GrLayerHoister::DrawLayers(atlased, nonAtlased, recycled, &replacements);
    }
#endif

    for (int i = 0; i < fDrawData.count(); ++i) {
#ifndef SK_IGNORE_GPU_LAYER_HOISTING
        if (fDrawData[i].canvas->getGrContext() && 
            !fDrawData[i].paint && fDrawData[i].matrix.isIdentity()) {
            // Render the entire picture using new layers
            const SkMatrix initialMatrix = fDrawData[i].canvas->getTotalMatrix();

            GrRecordReplaceDraw(fDrawData[i].picture, fDrawData[i].canvas, 
                                &replacements, initialMatrix, NULL);
        } else 
#endif
        {
            fDrawData[i].canvas->drawPicture(fDrawData[i].picture,
                                             &fDrawData[i].matrix,
                                             fDrawData[i].paint);
        }
    }

#ifndef SK_IGNORE_GPU_LAYER_HOISTING
    if (NULL != context) {
        GrLayerHoister::UnlockLayers(context, atlased, nonAtlased, recycled);
    }
#endif

    this->reset();
}

