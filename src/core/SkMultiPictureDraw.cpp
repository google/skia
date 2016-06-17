/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Need to include something before #if SK_SUPPORT_GPU so that the Android
// framework build, which gets its defines from SkTypes rather than a makefile,
// has the definition before checking it.
#include "SkCanvas.h"
#include "SkCanvasPriv.h"
#include "SkMultiPictureDraw.h"
#include "SkPicture.h"
#include "SkTaskGroup.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrDrawContext.h"
#include "GrLayerHoister.h"
#include "GrRecordReplaceDraw.h"
#include "GrRenderTarget.h"
#endif

void SkMultiPictureDraw::DrawData::draw() {
    fCanvas->drawPicture(fPicture, &fMatrix, fPaint);
}

void SkMultiPictureDraw::DrawData::init(SkCanvas* canvas, const SkPicture* picture,
                                        const SkMatrix* matrix, const SkPaint* paint) {
    fPicture = SkRef(picture);
    fCanvas = SkRef(canvas);
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
        data[i].fCanvas->unref();
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

#if !defined(SK_IGNORE_GPU_LAYER_HOISTING) && SK_SUPPORT_GPU
    GrContext* context = fGPUDrawData[0].fCanvas->getGrContext();
    SkASSERT(context);

    // Start by collecting all the layers that are going to be atlased and render
    // them (if necessary). Hoisting the free floating layers is deferred until
    // drawing the canvas that requires them.
    SkTDArray<GrHoistedLayer> atlasedNeedRendering, atlasedRecycled;

    GrLayerHoister::Begin(context);

    for (int i = 0; i < count; ++i) {
        const DrawData& data = fGPUDrawData[i];
        // we only expect 1 context for all the canvases
        SkASSERT(data.fCanvas->getGrContext() == context);

        if (!data.fPaint && 
            (kRGBA_8888_SkColorType == data.fCanvas->imageInfo().colorType() ||
             kBGRA_8888_SkColorType == data.fCanvas->imageInfo().colorType())) {
            SkRect clipBounds;
            if (!data.fCanvas->getClipBounds(&clipBounds)) {
                continue;
            }

            SkMatrix initialMatrix = data.fCanvas->getTotalMatrix();
            initialMatrix.preConcat(data.fMatrix);

            GrDrawContext* dc = data.fCanvas->internal_private_accessTopLayerDrawContext();
            SkASSERT(dc);

            // TODO: sorting the cacheable layers from smallest to largest
            // would improve the packing and reduce the number of swaps
            // TODO: another optimization would be to make a first pass to
            // lock any required layer that is already in the atlas
            GrLayerHoister::FindLayersToAtlas(context, data.fPicture, initialMatrix,
                                              clipBounds,
                                              &atlasedNeedRendering, &atlasedRecycled,
                                              dc->numColorSamples());
        }
    }

    GrLayerHoister::DrawLayersToAtlas(context, atlasedNeedRendering);

    SkTDArray<GrHoistedLayer> needRendering, recycled;
#endif

    for (int i = 0; i < count; ++i) {
        const DrawData& data = fGPUDrawData[i];
        SkCanvas* canvas = data.fCanvas;
        const SkPicture* picture = data.fPicture;

#if !defined(SK_IGNORE_GPU_LAYER_HOISTING) && SK_SUPPORT_GPU
        if (!data.fPaint) {

            SkRect clipBounds;
            if (!canvas->getClipBounds(&clipBounds)) {
                continue;
            }

            SkAutoCanvasMatrixPaint acmp(canvas, &data.fMatrix, data.fPaint, picture->cullRect());

            const SkMatrix initialMatrix = canvas->getTotalMatrix();

            GrDrawContext* dc = data.fCanvas->internal_private_accessTopLayerDrawContext();
            SkASSERT(dc);

            // Find the layers required by this canvas. It will return atlased
            // layers in the 'recycled' list since they have already been drawn.
            GrLayerHoister::FindLayersToHoist(context, picture, initialMatrix,
                                              clipBounds, &needRendering, &recycled,
                                              dc->numColorSamples());

            GrLayerHoister::DrawLayers(context, needRendering);

            // Render the entire picture using new layers
            GrRecordReplaceDraw(picture, canvas, context->getLayerCache(),
                                initialMatrix, nullptr);

            GrLayerHoister::UnlockLayers(context, needRendering);
            GrLayerHoister::UnlockLayers(context, recycled);

            needRendering.rewind();
            recycled.rewind();
        } else
#endif
        {
            canvas->drawPicture(picture, &data.fMatrix, data.fPaint);
        }
        if (flush) {
            canvas->flush();
        }
    }

#if !defined(SK_IGNORE_GPU_LAYER_HOISTING) && SK_SUPPORT_GPU
    GrLayerHoister::UnlockLayers(context, atlasedNeedRendering);
    GrLayerHoister::UnlockLayers(context, atlasedRecycled);
    GrLayerHoister::End(context);
#endif
}
