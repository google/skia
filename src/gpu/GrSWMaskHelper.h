/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSWMaskHelper_DEFINED
#define GrSWMaskHelper_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkRegion.h"
#include "include/core/SkTypes.h"
#include "include/private/GrTypesPriv.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkDraw.h"
#include "src/core/SkRasterClip.h"
#include "src/gpu/GrSurfaceProxyView.h"

class GrShape;
class GrStyledShape;
class GrRecordingContext;
class GrTextureProxy;
class SkTaskGroup;

/**
 * The GrSWMaskHelper helps generate clip masks using the software rendering
 * path. It is intended to be used as:
 *
 *   auto data = copy_required_drawing_data();
 *   GrSurfaceProxyView proxy
 *           = GrSWMaskHelper::MakeTexture(bounds,
 *                                         context,
 *                                         SkBackingFit::kApprox/kExact,
 *                                         [data{std::move(data)}](GrSWMaskHelper* helper) {
 *       // draw one or more paths/rects specifying the required boolean ops
 *       helper->drawRect(data.rect(), ...);
 *   });
 *
 * The result of this process will be the final mask (on the GPU) in the
 * upper left hand corner of the texture.
 */
class GrSWMaskHelper : public SkNVRefCnt<GrSWMaskHelper> {
public:
    using DrawFunc = std::function<void(GrSWMaskHelper*)>;

    // Make a texture by drawing a software mask. If the context has a task group, the draw will be
    // done async in that task group, and a lazy proxy will be returned to wait & upload it.
    // Otherwise the drawing is done immediately.
    // NOTE: The draw fn is async – it should not capture by reference.
    static GrSurfaceProxyView MakeTexture(SkIRect bounds,
                                          GrRecordingContext*,
                                          SkBackingFit,
                                          DrawFunc&&);

    // Draw a single rect into the accumulation bitmap using the specified op
    void drawRect(const SkRect& rect, const SkMatrix& matrix, SkRegion::Op op, GrAA, uint8_t alpha);

    // Draw a single rrect into the accumulation bitmap using the specified op
    void drawRRect(const SkRRect& rrect, const SkMatrix& matrix, SkRegion::Op op, GrAA,
                   uint8_t alpha);

    // Draw a single path into the accumuation bitmap using the specified op
    void drawShape(const GrStyledShape&, const SkMatrix& matrix, SkRegion::Op op, GrAA,
                   uint8_t alpha);

    // Like the GrStyledShape variant, but assumes a simple fill style
    void drawShape(const GrShape&, const SkMatrix& matrix, SkRegion::Op op, GrAA, uint8_t alpha);

    // Reset the internal bitmap
    void clear(uint8_t alpha) {
        fBitmap.pixmap().erase(SkColorSetARGB(alpha, 0xFF, 0xFF, 0xFF));
    }

private:
    GrSWMaskHelper(const SkIRect& resultBounds);

    GrSWMaskHelper(const GrSWMaskHelper&) = delete;
    GrSWMaskHelper& operator=(const GrSWMaskHelper&) = delete;

    GrSurfaceProxyView nonThreadedExecute(GrRecordingContext*,
                                          SkBackingFit,
                                          const DrawFunc&);

    // `this` must be heap-allocated. Task & proxy take on ownership; do not unref.
    GrSurfaceProxyView threadedExecute(SkTaskGroup*,
                                       GrRecordingContext*,
                                       SkBackingFit,
                                       DrawFunc&&);

    bool allocate();

    SkVector     fTranslate;
    SkBitmap     fBitmap;
    SkDraw       fDraw;
    SkRasterClip fRasterClip;
    SkSemaphore  fSemaphore;
};

#endif // GrSWMaskHelper_DEFINED
