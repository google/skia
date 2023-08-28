/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkCanvasPriv.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAlign.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkAutoMalloc.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/core/SkWriter32.h"

#include <utility>
#include <cstdint>

SkAutoCanvasMatrixPaint::SkAutoCanvasMatrixPaint(SkCanvas* canvas, const SkMatrix* matrix,
                                                 const SkPaint* paint, const SkRect& bounds)
        : fCanvas(canvas)
        , fSaveCount(canvas->getSaveCount()) {
    if (paint) {
        SkRect newBounds = bounds;
        if (matrix) {
            matrix->mapRect(&newBounds);
        }
        canvas->saveLayer(&newBounds, paint);
    } else if (matrix) {
        canvas->save();
    }

    if (matrix) {
        canvas->concat(*matrix);
    }
}

SkAutoCanvasMatrixPaint::~SkAutoCanvasMatrixPaint() {
    fCanvas->restoreToCount(fSaveCount);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool SkCanvasPriv::ReadLattice(SkReadBuffer& buffer, SkCanvas::Lattice* lattice) {
    lattice->fXCount = buffer.readInt();
    lattice->fXDivs = buffer.skipT<int32_t>(lattice->fXCount);
    lattice->fYCount = buffer.readInt();
    lattice->fYDivs = buffer.skipT<int32_t>(lattice->fYCount);
    int flagCount = buffer.readInt();
    lattice->fRectTypes = nullptr;
    lattice->fColors = nullptr;
    if (flagCount) {
        lattice->fRectTypes = buffer.skipT<SkCanvas::Lattice::RectType>(flagCount);
        lattice->fColors = buffer.skipT<SkColor>(flagCount);
    }
    lattice->fBounds = buffer.skipT<SkIRect>();
    return buffer.isValid();
}

size_t SkCanvasPriv::WriteLattice(void* buffer, const SkCanvas::Lattice& lattice) {
    int flagCount = lattice.fRectTypes ? (lattice.fXCount + 1) * (lattice.fYCount + 1) : 0;

    const size_t size = (1 + lattice.fXCount + 1 + lattice.fYCount + 1) * sizeof(int32_t) +
                        SkAlign4(flagCount * sizeof(SkCanvas::Lattice::RectType)) +
                        SkAlign4(flagCount * sizeof(SkColor)) +
                        sizeof(SkIRect);

    if (buffer) {
        SkWriter32 writer(buffer, size);
        writer.write32(lattice.fXCount);
        writer.write(lattice.fXDivs, lattice.fXCount * sizeof(uint32_t));
        writer.write32(lattice.fYCount);
        writer.write(lattice.fYDivs, lattice.fYCount * sizeof(uint32_t));
        writer.write32(flagCount);
        writer.writePad(lattice.fRectTypes, flagCount * sizeof(uint8_t));
        writer.write(lattice.fColors, flagCount * sizeof(SkColor));
        SkASSERT(lattice.fBounds);
        writer.write(lattice.fBounds, sizeof(SkIRect));
        SkASSERT(writer.bytesWritten() == size);
    }
    return size;
}

void SkCanvasPriv::WriteLattice(SkWriteBuffer& buffer, const SkCanvas::Lattice& lattice) {
    const size_t size = WriteLattice(nullptr, lattice);
    SkAutoSMalloc<1024> storage(size);
    WriteLattice(storage.get(), lattice);
    buffer.writePad32(storage.get(), size);
}

void SkCanvasPriv::GetDstClipAndMatrixCounts(const SkCanvas::ImageSetEntry set[], int count,
                                             int* totalDstClipCount, int* totalMatrixCount) {
    int dstClipCount = 0;
    int maxMatrixIndex = -1;
    for (int i = 0; i < count; ++i) {
        dstClipCount += 4 * set[i].fHasClip;
        if (set[i].fMatrixIndex > maxMatrixIndex) {
            maxMatrixIndex = set[i].fMatrixIndex;
        }
    }

    *totalDstClipCount = dstClipCount;
    *totalMatrixCount = maxMatrixIndex + 1;
}

// Attempts to convert an image filter to its equivalent color filter, which if possible, modifies
// the paint to compose the image filter's color filter into the paint's color filter slot.
// Returns true if the paint has been modified.
// Requires the paint to have an image filter and the copy-on-write be initialized.
bool SkCanvasPriv::ImageToColorFilter(SkPaint* paint) {
    SkASSERT(SkToBool(paint) && paint->getImageFilter());

    SkColorFilter* imgCFPtr;
    if (!paint->getImageFilter()->asAColorFilter(&imgCFPtr)) {
        return false;
    }
    sk_sp<SkColorFilter> imgCF(imgCFPtr);

    SkColorFilter* paintCF = paint->getColorFilter();
    if (paintCF) {
        // The paint has both a colorfilter(paintCF) and an imagefilter-that-is-a-colorfilter(imgCF)
        // and we need to combine them into a single colorfilter.
        imgCF = imgCF->makeComposed(sk_ref_sp(paintCF));
    }

    paint->setColorFilter(std::move(imgCF));
    paint->setImageFilter(nullptr);
    return true;
}

#if defined(GRAPHITE_TEST_UTILS)
#include "src/gpu/graphite/Device.h"

skgpu::graphite::TextureProxy* SkCanvasPriv::TopDeviceGraphiteTargetProxy(SkCanvas* canvas) {
    if (auto gpuDevice = canvas->topDevice()->asGraphiteDevice()) {
        return gpuDevice->target();
    }
    return nullptr;
}

#endif // defined(GRAPHITE_TEST_UTILS)


AutoLayerForImageFilter::AutoLayerForImageFilter(SkCanvas* canvas,
                                                 const SkPaint& paint,
                                                 const SkRect* rawBounds)
            : fPaint(paint)
            , fCanvas(canvas)
            , fTempLayerForImageFilter(false) {
    SkDEBUGCODE(fSaveCount = canvas->getSaveCount();)

    if (fPaint.getImageFilter() && !SkCanvasPriv::ImageToColorFilter(&fPaint)) {
        // The draw paint has an image filter that couldn't be simplified to an equivalent
        // color filter, so we have to inject an automatic saveLayer().
        SkPaint restorePaint;
        restorePaint.setImageFilter(fPaint.refImageFilter());
        restorePaint.setBlender(fPaint.refBlender());

        // Remove the restorePaint fields from our "working" paint
        fPaint.setImageFilter(nullptr);
        fPaint.setBlendMode(SkBlendMode::kSrcOver);

        SkRect storage;
        if (rawBounds && fPaint.canComputeFastBounds()) {
            // Make rawBounds include all paint outsets except for those due to image filters.
            // At this point, fPaint's image filter has been moved to 'restorePaint'.
            SkASSERT(!fPaint.getImageFilter());
            rawBounds = &fPaint.computeFastBounds(*rawBounds, &storage);
        }

        canvas->fSaveCount += 1;
        (void)canvas->internalSaveLayer(SkCanvas::SaveLayerRec(rawBounds, &restorePaint),
                                        SkCanvas::kFullLayer_SaveLayerStrategy);
        fTempLayerForImageFilter = true;
    }
}

AutoLayerForImageFilter::~AutoLayerForImageFilter() {
    if (fTempLayerForImageFilter) {
        fCanvas->fSaveCount -= 1;
        fCanvas->internalRestore();
    }
    SkASSERT(fCanvas->getSaveCount() == fSaveCount);
}
