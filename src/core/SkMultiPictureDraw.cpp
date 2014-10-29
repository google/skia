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

    // Start by collecting all the layers that are going to be atlased and render 
    // them (if necessary). Hoisting the free floating layers is deferred until
    // drawing the canvas that requires them.
    SkTDArray<GrHoistedLayer> atlasedNeedRendering, atlasedRecycled;

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

            // TODO: sorting the cacheable layers from smallest to largest
            // would improve the packing and reduce the number of swaps
            // TODO: another optimization would be to make a first pass to
            // lock any required layer that is already in the atlas
            GrLayerHoister::FindLayersToAtlas(context, fDrawData[i].picture,
                                              clipBounds, 
                                              &atlasedNeedRendering, &atlasedRecycled);
        }
    }

    if (NULL != context) {
        GrLayerHoister::DrawLayersToAtlas(context, atlasedNeedRendering);
    }

    SkTDArray<GrHoistedLayer> needRendering, recycled;
#endif

    for (int i = 0; i < fDrawData.count(); ++i) {
#ifndef SK_IGNORE_GPU_LAYER_HOISTING
        if (fDrawData[i].canvas->getGrContext() && 
            !fDrawData[i].paint && fDrawData[i].matrix.isIdentity()) {

            SkRect clipBounds;
            if (!fDrawData[i].canvas->getClipBounds(&clipBounds)) {
                continue;
            }

            // Find the layers required by this canvas. It will return atlased
            // layers in the 'recycled' list since they have already been drawn.
            GrLayerHoister::FindLayersToHoist(context, fDrawData[i].picture,
                                              clipBounds, &needRendering, &recycled);

            GrLayerHoister::DrawLayers(context, needRendering);

            GrReplacements replacements;

            GrLayerHoister::ConvertLayersToReplacements(needRendering, &replacements);
            GrLayerHoister::ConvertLayersToReplacements(recycled, &replacements);

            const SkMatrix initialMatrix = fDrawData[i].canvas->getTotalMatrix();

            // Render the entire picture using new layers
            GrRecordReplaceDraw(fDrawData[i].picture, fDrawData[i].canvas, 
                                &replacements, initialMatrix, NULL);

            GrLayerHoister::UnlockLayers(context, needRendering);
            GrLayerHoister::UnlockLayers(context, recycled);

            needRendering.rewind();
            recycled.rewind();
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
        GrLayerHoister::UnlockLayers(context, atlasedNeedRendering);
        GrLayerHoister::UnlockLayers(context, atlasedRecycled);
#if !GR_CACHE_HOISTED_LAYERS
        GrLayerHoister::PurgeCache(context);
#endif
    }
#endif

    this->reset();
}

