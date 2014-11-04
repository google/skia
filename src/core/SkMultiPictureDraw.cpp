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
#include "SkMultiPictureDraw.h"
#include "SkPicture.h"
#include "SkTaskGroup.h"

#if SK_SUPPORT_GPU
#include "GrLayerHoister.h"
#include "GrRecordReplaceDraw.h"
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
        fPaint = SkNEW_ARGS(SkPaint, (*paint));
    } else {
        fPaint = NULL;
    }
}

void SkMultiPictureDraw::DrawData::Reset(SkTDArray<DrawData>& data) {
    for (int i = 0; i < data.count(); ++i) {
        data[i].fPicture->unref();
        data[i].fCanvas->unref();
        SkDELETE(data[i].fPaint);
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
    if (NULL == canvas || NULL == picture) {
        SkDEBUGFAIL("parameters to SkMultiPictureDraw::add should be non-NULL");
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

void SkMultiPictureDraw::draw() {
    AutoMPDReset mpdreset(this);
    // we place the taskgroup after the MPDReset, to ensure that we don't delete the DrawData
    // objects until after we're finished the tasks (which have pointers to the data).

    SkTaskGroup group;
    group.batch(DrawData::Draw, fThreadSafeDrawData.begin(), fThreadSafeDrawData.count());
    // we deliberately don't call wait() here, since the destructor will do that, this allows us
    // to continue processing gpu-data without having to wait on the cpu tasks.

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

    for (int i = 0; i < count; ++i) {
        const DrawData& data = fGPUDrawData[i];
        // we only expect 1 context for all the canvases
        SkASSERT(data.fCanvas->getGrContext() == context);

        if (!data.fPaint && data.fMatrix.isIdentity()) {
            // TODO: this path always tries to optimize pictures. Should we
            // switch to this API approach (vs. SkCanvas::EXPERIMENTAL_optimize)?
            data.fCanvas->EXPERIMENTAL_optimize(data.fPicture);

            SkRect clipBounds;
            if (!data.fCanvas->getClipBounds(&clipBounds)) {
                continue;
            }

            // TODO: sorting the cacheable layers from smallest to largest
            // would improve the packing and reduce the number of swaps
            // TODO: another optimization would be to make a first pass to
            // lock any required layer that is already in the atlas
            GrLayerHoister::FindLayersToAtlas(context, data.fPicture,
                                              clipBounds,
                                              &atlasedNeedRendering, &atlasedRecycled);
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
        if (!data.fPaint && data.fMatrix.isIdentity()) {

            SkRect clipBounds;
            if (!canvas->getClipBounds(&clipBounds)) {
                continue;
            }

            // Find the layers required by this canvas. It will return atlased
            // layers in the 'recycled' list since they have already been drawn.
            GrLayerHoister::FindLayersToHoist(context, picture,
                                              clipBounds, &needRendering, &recycled);

            GrLayerHoister::DrawLayers(context, needRendering);

            GrReplacements replacements;

            GrLayerHoister::ConvertLayersToReplacements(needRendering, &replacements);
            GrLayerHoister::ConvertLayersToReplacements(recycled, &replacements);

            const SkMatrix initialMatrix = canvas->getTotalMatrix();

            // Render the entire picture using new layers
            GrRecordReplaceDraw(picture, canvas, &replacements, initialMatrix, NULL);

            GrLayerHoister::UnlockLayers(context, needRendering);
            GrLayerHoister::UnlockLayers(context, recycled);

            needRendering.rewind();
            recycled.rewind();
        } else
#endif
        {
            canvas->drawPicture(picture, &data.fMatrix, data.fPaint);
        }
    }

#if !defined(SK_IGNORE_GPU_LAYER_HOISTING) && SK_SUPPORT_GPU
    GrLayerHoister::UnlockLayers(context, atlasedNeedRendering);
    GrLayerHoister::UnlockLayers(context, atlasedRecycled);
#if !GR_CACHE_HOISTED_LAYERS
    GrLayerHoister::PurgeCache(context);
#endif
#endif
}

