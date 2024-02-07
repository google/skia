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
#include "include/core/SkMaskFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/private/base/SkAlign.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkAutoMalloc.h"
#include "src/core/SkMaskFilterBase.h"
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
// the paint to compose the image filter's color filter into the paint's color filter slot. Returns
// true if the paint has been modified. Requires the paint to have an image filter.
bool SkCanvasPriv::ImageToColorFilter(SkPaint* paint) {
    SkASSERT(SkToBool(paint) && paint->getImageFilter());

    // An image filter logically runs after any mask filter and the src-over blending against the
    // layer's transparent black initial content. Moving the image filter (as a color filter) into
    // the color filter slot causes it to run before the mask filter or blending.
    //
    // Src-over blending against transparent black is a no-op, so skipping the layer and drawing the
    // output of the color filter-image filter with the original blender is valid.
    //
    // If there's also a mask filter on the paint, it will operate on an alpha-only layer that's
    // then shaded with the paint's effects. Moving the CF-IF into the paint's color filter slot
    // will mean that the CF-IF operates on the output of the original CF *before* it's combined
    // with the coverage value. Under normal circumstances the CF-IF evaluates the color after
    // coverage has been multiplied into the alpha channel.
    //
    // Some color filters may behave the same, e.g. cf(color)*coverage == cf(color*coverage), but
    // that's hard to detect so we disable the optimization when both image filters and mask filters
    // are present.
    if (paint->getMaskFilter()) {
        return false;
    }

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

AutoLayerForImageFilter::AutoLayerForImageFilter(SkCanvas* canvas,
                                                 const SkPaint& paint,
                                                 const SkRect* rawBounds,
                                                 bool skipMaskFilterLayer)
            : fPaint(paint)
            , fCanvas(canvas)
            , fTempLayersForFilters(0) {
    SkDEBUGCODE(fSaveCount = canvas->getSaveCount();)

    // Depending on the original paint, this will add 0, 1, or 2 layers that apply the
    // filter effects to a temporary layer that rasterized the remaining effects. Image filters
    // are applied to the result of any mask filter, so its layer is added first in the stack.
    //
    // If present on the original paint, the image filter layer's restore paint steals the blender
    // and the image filter so that the draw's paint will never have an image filter.
    if (fPaint.getImageFilter() && !SkCanvasPriv::ImageToColorFilter(&fPaint)) {
        this->addImageFilterLayer(rawBounds);
    }

    // If present on the original paint, the mask filter layer's restore paint steals all shading
    // effects and the draw's paint shading is updated to draw a solid opaque color (thus encoding
    // coverage into the alpha channel). The draw's paint preserves all geometric effects that have
    // to be applied before the mask filter. The layer's restore paint adds an image filter
    // representing the mask filter.
    if (fPaint.getMaskFilter() && !skipMaskFilterLayer) {
        this->addMaskFilterLayer(rawBounds);
    }

   // When the original paint has both an image filter and a mask filter, this will create two
   // internal layers and perform two restores when finished. This actually creates one fewer
   // offscreen passes compared to directly composing the mask filter's output with an
   // SkImageFilters::Shader node and passing that into the rest of the image filter.
}

AutoLayerForImageFilter::~AutoLayerForImageFilter() {
    for (int i = 0; i < fTempLayersForFilters; ++i) {
        fCanvas->fSaveCount -= 1;
        fCanvas->internalRestore();
    }
    SkASSERT(fCanvas->getSaveCount() == fSaveCount);
}

void AutoLayerForImageFilter::addImageFilterLayer(const SkRect* drawBounds) {
    // Shouldn't be adding a layer if there was no image filter to begin with.
    SkASSERT(fPaint.getImageFilter());

    // The restore paint for an image filter layer simply takes the image filter and blending off
    // the original paint. The blending is applied post image filter because otherwise it'd be
    // applied with the new layer's transparent dst and not be very interesting.
    SkPaint restorePaint;
    restorePaint.setImageFilter(fPaint.refImageFilter());
    restorePaint.setBlender(fPaint.refBlender());

    // Remove the restorePaint fields from our "working" paint, leaving all other shading and
    // geometry effects to be rendered into the layer. If there happens to be a mask filter, this
    // paint will still trigger a second layer for that filter.
    fPaint.setImageFilter(nullptr);
    fPaint.setBlendMode(SkBlendMode::kSrcOver);

    this->addLayer(restorePaint, drawBounds, /*coverageOnly=*/false);
}

void AutoLayerForImageFilter::addMaskFilterLayer(const SkRect* drawBounds) {
    // Shouldn't be adding a layer if there was no mask filter to begin with.
    SkASSERT(fPaint.getMaskFilter());

    // Image filters are evaluated after mask filters so any filter should have been converted to
    // a layer and removed from fPaint already.
    SkASSERT(!fPaint.getImageFilter());

    // TODO: Eventually all SkMaskFilters will implement this method so this can switch to an assert
    sk_sp<SkImageFilter> maskFilterAsImageFilter =
            as_MFB(fPaint.getMaskFilter())->asImageFilter(fCanvas->getTotalMatrix());
    if (!maskFilterAsImageFilter) {
        // This is a legacy mask filter that can be handled by raster and Ganesh directly, but will
        // be ignored by Graphite. Return now, leaving the paint with the mask filter so that the
        // underlying SkDevice can handle it if it will.
        return;
    }

    // The restore paint for the coverage layer takes over all shading effects that had been on the
    // original paint, which will be applied to the alpha-only output image from the mask filter
    // converted to an image filter.
    SkPaint restorePaint;
    restorePaint.setColor4f(fPaint.getColor4f());
    restorePaint.setShader(fPaint.refShader());
    restorePaint.setColorFilter(fPaint.refColorFilter());
    restorePaint.setBlender(fPaint.refBlender());
    restorePaint.setDither(fPaint.isDither());
    restorePaint.setImageFilter(maskFilterAsImageFilter);

    // Remove all shading effects from the "working" paint so that the layer's alpha channel
    // will correspond to the coverage. This leaves the original style and AA settings that
    // contribute to coverage (including any path effect).
    fPaint.setColor4f(SkColors::kWhite);
    fPaint.setShader(nullptr);
    fPaint.setColorFilter(nullptr);
    fPaint.setMaskFilter(nullptr);
    fPaint.setDither(false);
    fPaint.setBlendMode(SkBlendMode::kSrcOver);

    this->addLayer(restorePaint, drawBounds, /*coverageOnly=*/true);
}

void AutoLayerForImageFilter::addLayer(const SkPaint& restorePaint,
                                       const SkRect* drawBounds,
                                       bool coverageOnly) {
    SkRect storage;
    const SkRect* contentBounds = nullptr;
    if (drawBounds && fPaint.canComputeFastBounds()) {
        // The content bounds will include all paint outsets except for those that have been
        // extracted into 'restorePaint' or a previously added layer.
        contentBounds = &fPaint.computeFastBounds(*drawBounds, &storage);
    }

    fCanvas->fSaveCount += 1;
    fCanvas->internalSaveLayer(SkCanvas::SaveLayerRec(contentBounds, &restorePaint),
                               SkCanvas::kFullLayer_SaveLayerStrategy,
                               coverageOnly);
    fTempLayersForFilters += 1;
}
