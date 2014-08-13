/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSWMaskHelper_DEFINED
#define GrSWMaskHelper_DEFINED

#include "GrColor.h"
#include "GrDrawState.h"
#include "SkBitmap.h"
#include "SkDraw.h"
#include "SkMatrix.h"
#include "SkRasterClip.h"
#include "SkRegion.h"
#include "SkTextureCompressor.h"
#include "SkTypes.h"

class GrAutoScratchTexture;
class GrContext;
class GrTexture;
class SkPath;
class SkStrokeRec;
class GrDrawTarget;

/**
 * The GrSWMaskHelper helps generate clip masks using the software rendering
 * path. It is intended to be used as:
 *
 *   GrSWMaskHelper helper(context);
 *   helper.init(...);
 *
 *      draw one or more paths/rects specifying the required boolean ops
 *
 *   toTexture();   // to get it from the internal bitmap to the GPU
 *
 * The result of this process will be the final mask (on the GPU) in the
 * upper left hand corner of the texture.
 */
class GrSWMaskHelper : SkNoncopyable {
public:
    GrSWMaskHelper(GrContext* context)
    : fContext(context)
    , fCompressionMode(kNone_CompressionMode) {
    }

    // set up the internal state in preparation for draws. Since many masks
    // may be accumulated in the helper during creation, "resultBounds"
    // allows the caller to specify the region of interest - to limit the
    // amount of work. allowCompression should be set to false if you plan on using
    // your own texture to draw into, and not a scratch texture via getTexture().
    bool init(const SkIRect& resultBounds, const SkMatrix* matrix, bool allowCompression = true);

    // Draw a single rect into the accumulation bitmap using the specified op
    void draw(const SkRect& rect, SkRegion::Op op,
              bool antiAlias, uint8_t alpha);

    // Draw a single path into the accumuation bitmap using the specified op
    void draw(const SkPath& path, const SkStrokeRec& stroke, SkRegion::Op op,
              bool antiAlias, uint8_t alpha);

    // Helper function to get a scratch texture suitable for capturing the
    // result (i.e., right size & format)
    bool getTexture(GrAutoScratchTexture* texture);

    // Move the mask generation results from the internal bitmap to the gpu.
    void toTexture(GrTexture* texture);

    // Reset the internal bitmap
    void clear(uint8_t alpha) {
        fBM.eraseColor(SkColorSetARGB(alpha, alpha, alpha, alpha));
    }

    // Canonical usage utility that draws a single path and uploads it
    // to the GPU. The result is returned in "result".
    static GrTexture* DrawPathMaskToTexture(GrContext* context,
                                            const SkPath& path,
                                            const SkStrokeRec& stroke,
                                            const SkIRect& resultBounds,
                                            bool antiAlias,
                                            SkMatrix* matrix);

    // This utility routine is used to add a path's mask to some other draw.
    // The ClipMaskManager uses it to accumulate clip masks while the
    // GrSoftwarePathRenderer uses it to fulfill a drawPath call.
    // It draws with "texture" as a path mask into "target" using "rect" as
    // geometry and the current drawState. The current drawState is altered to
    // accommodate the mask.
    // Note that this method assumes that the GrPaint::kTotalStages slot in
    // the draw state can be used to hold the mask texture stage.
    // This method is really only intended to be used with the
    // output of DrawPathMaskToTexture.
    static void DrawToTargetWithPathMask(GrTexture* texture,
                                         GrDrawTarget* target,
                                         const SkIRect& rect);

private:
    GrContext*      fContext;
    SkMatrix        fMatrix;
    SkBitmap        fBM;
    SkDraw          fDraw;
    SkRasterClip    fRasterClip;

    // This enum says whether or not we should compress the mask:
    // kNone_CompressionMode: compression is not supported on this device.
    // kCompress_CompressionMode: compress the bitmap before it gets sent to the gpu
    // kBlitter_CompressionMode: write to the bitmap using a special compressed blitter.
    enum CompressionMode {
        kNone_CompressionMode,
        kCompress_CompressionMode,
        kBlitter_CompressionMode,
    } fCompressionMode;

    // This is the buffer into which we store our compressed data. This buffer is
    // only allocated (non-null) if fCompressionMode is kBlitter_CompressionMode
    SkAutoMalloc fCompressedBuffer;

    // This is the desired format within which to compress the
    // texture. This value is only valid if fCompressionMode is not kNone_CompressionMode.
    SkTextureCompressor::Format fCompressedFormat;

    // Actually sends the texture data to the GPU. This is called from
    // toTexture with the data filled in depending on the texture config.
    void sendTextureData(GrTexture *texture, const GrTextureDesc& desc,
                         const void *data, int rowbytes);

    // Compresses the bitmap stored in fBM and sends the compressed data
    // to the GPU to be stored in 'texture' using sendTextureData.
    void compressTextureData(GrTexture *texture, const GrTextureDesc& desc);

    typedef SkNoncopyable INHERITED;
};

#endif // GrSWMaskHelper_DEFINED
