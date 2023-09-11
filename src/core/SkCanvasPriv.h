/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCanvasPriv_DEFINED
#define SkCanvasPriv_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkScalar.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkNoncopyable.h"

#include <cstddef>

class SkDevice;
class SkImageFilter;
class SkMatrix;
class SkReadBuffer;
struct SkRect;
class SkWriteBuffer;

#if defined(GRAPHITE_TEST_UTILS)
namespace skgpu::graphite {
    class TextureProxy;
}
#endif

class SkAutoCanvasMatrixPaint : SkNoncopyable {
public:
    SkAutoCanvasMatrixPaint(SkCanvas*, const SkMatrix*, const SkPaint*, const SkRect& bounds);
    ~SkAutoCanvasMatrixPaint();

private:
    SkCanvas*   fCanvas;
    int         fSaveCount;
};

class SkCanvasPriv {
public:
    // The lattice has pointers directly into the readbuffer
    static bool ReadLattice(SkReadBuffer&, SkCanvas::Lattice*);

    static void WriteLattice(SkWriteBuffer&, const SkCanvas::Lattice&);

    // return the byte-size of the lattice, even if the buffer is null
    // storage must be 4-byte aligned
    static size_t WriteLattice(void* storage, const SkCanvas::Lattice&);

    static int SaveBehind(SkCanvas* canvas, const SkRect* subset) {
        return canvas->only_axis_aligned_saveBehind(subset);
    }
    static void DrawBehind(SkCanvas* canvas, const SkPaint& paint) {
        canvas->drawClippedToSaveBehind(paint);
    }

    // Exposed for testing on non-Android framework builds
    static void ResetClip(SkCanvas* canvas) {
        canvas->internal_private_resetClip();
    }

    static SkDevice* TopDevice(const SkCanvas* canvas) {
        return canvas->topDevice();
    }

#if defined(GRAPHITE_TEST_UTILS)
    static skgpu::graphite::TextureProxy* TopDeviceGraphiteTargetProxy(SkCanvas*);
#endif

    // The experimental_DrawEdgeAAImageSet API accepts separate dstClips and preViewMatrices arrays,
    // where entries refer into them, but no explicit size is provided. Given a set of entries,
    // computes the minimum length for these arrays that would provide index access errors.
    static void GetDstClipAndMatrixCounts(const SkCanvas::ImageSetEntry set[], int count,
                                          int* totalDstClipCount, int* totalMatrixCount);

    static SkCanvas::SaveLayerRec ScaledBackdropLayer(const SkRect* bounds,
                                                      const SkPaint* paint,
                                                      const SkImageFilter* backdrop,
                                                      SkScalar backdropScale,
                                                      SkCanvas::SaveLayerFlags saveLayerFlags) {
        return SkCanvas::SaveLayerRec(bounds, paint, backdrop, backdropScale, saveLayerFlags);
    }

    static SkScalar GetBackdropScaleFactor(const SkCanvas::SaveLayerRec& rec) {
        return rec.fExperimentalBackdropScale;
    }

    static void SetBackdropScaleFactor(SkCanvas::SaveLayerRec* rec, SkScalar scale) {
        rec->fExperimentalBackdropScale = scale;
    }

    // Attempts to convert an image filter to its equivalent color filter, which if possible,
    // modifies the paint to compose the image filter's color filter into the paint's color filter
    // slot.
    // Returns true if the paint has been modified.
    // Requires the paint to have an image filter and the copy-on-write be initialized.
    static bool ImageToColorFilter(SkPaint*);
};

/**
 *  This constant is trying to balance the speed of ref'ing a subpicture into a parent picture,
 *  against the playback cost of recursing into the subpicture to get at its actual ops.
 *
 *  For now we pick a conservatively small value, though measurement (and other heuristics like
 *  the type of ops contained) may justify changing this value.
 */
constexpr int kMaxPictureOpsToUnrollInsteadOfRef = 1;

/**
 *  We implement ImageFilters for a given draw by creating a layer, then applying the
 *  imagefilter to the pixels of that layer (its backing surface/image), and then
 *  we call restore() to xfer that layer to the main canvas.
 *
 *  1. SaveLayer (with a paint containing the current imagefilter and xfermode)
 *  2. Generate the src pixels:
 *      Remove the imagefilter and the xfermode from the paint that we (AutoDrawLooper)
 *      return (fPaint). We then draw the primitive (using srcover) into a cleared
 *      buffer/surface.
 *  3. Restore the layer created in #1
 *      The imagefilter is passed the buffer/surface from the layer (now filled with the
 *      src pixels of the primitive). It returns a new "filtered" buffer, which we
 *      draw onto the previous layer using the xfermode from the original paint.
 */
class AutoLayerForImageFilter {
public:
    // "rawBounds" is the original bounds of the primitive about to be drawn, unmodified by the
    // paint. It's used to determine the size of the offscreen layer for filters.
    // If null, the clip will be used instead.
    //
    // Draw functions should use layer->paint() instead of the passed-in paint.
    AutoLayerForImageFilter(SkCanvas* canvas,
                            const SkPaint& paint,
                            const SkRect* rawBounds = nullptr);

    AutoLayerForImageFilter(const AutoLayerForImageFilter&) = delete;
    AutoLayerForImageFilter& operator=(const AutoLayerForImageFilter&) = delete;
    AutoLayerForImageFilter(AutoLayerForImageFilter&&) = default;
    AutoLayerForImageFilter& operator=(AutoLayerForImageFilter&&) = default;

    ~AutoLayerForImageFilter();

    const SkPaint& paint() const { return fPaint; }

private:
    SkPaint         fPaint;
    SkCanvas*       fCanvas;
    bool            fTempLayerForImageFilter;

    SkDEBUGCODE(int fSaveCount;)
};

#endif
