/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCanvasPriv_DEFINED
#define SkCanvasPriv_DEFINED

#include "include/core/SkCanvas.h"
#include "include/private/SkNoncopyable.h"

class SkReadBuffer;
class SkWriteBuffer;

#if GR_TEST_UTILS
namespace skgpu {
    class SurfaceFillContext;
#if SK_GPU_V1
    namespace v1 { class SurfaceDrawContext; }
#endif // SK_GPU_V1
}
#endif // GR_TEST_UTILS

// This declaration must match the one in SkDeferredDisplayList.h
#if SK_SUPPORT_GPU
class GrRenderTargetProxy;
#else
using GrRenderTargetProxy = SkRefCnt;
#endif // SK_SUPPORT_GPU

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

#if GR_TEST_UTILS
#if SK_GPU_V1
    static skgpu::v1::SurfaceDrawContext* TopDeviceSurfaceDrawContext(SkCanvas*);
#endif
    static skgpu::SurfaceFillContext* TopDeviceSurfaceFillContext(SkCanvas*);
#endif // GR_TEST_UTILS
    static GrRenderTargetProxy* TopDeviceTargetProxy(SkCanvas*);

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

    static void DrawCustomMesh(SkCanvas*,
                               SkCustomMesh cm,
                               sk_sp<SkBlender> blender,
                               const SkPaint& paint);
};

/**
 *  This constant is trying to balance the speed of ref'ing a subpicture into a parent picture,
 *  against the playback cost of recursing into the subpicture to get at its actual ops.
 *
 *  For now we pick a conservatively small value, though measurement (and other heuristics like
 *  the type of ops contained) may justify changing this value.
 */
constexpr int kMaxPictureOpsToUnrollInsteadOfRef = 1;

#endif
