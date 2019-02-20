/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */

#ifndef SkSpecialSurface_DEFINED
#define SkSpecialSurface_DEFINED

#include "SkImageInfo.h"
#include "SkRefCnt.h"
#include "SkSurfaceProps.h"

#if SK_SUPPORT_GPU
#include "GrTypesPriv.h"
#endif

class GrBackendFormat;
class GrContext;
class SkBitmap;
class SkCanvas;
class SkSpecialImage;

/**
 * SkSpecialSurface is a restricted form of SkSurface solely for internal use. It differs
 * from SkSurface in that:
 *     - it can be backed by GrTextures larger than [ fWidth, fHeight ]
 *     - it can't be used for tiling
 *     - it becomes inactive once a snapshot of it is taken (i.e., no copy-on-write)
 *     - it has no generation ID
 */
class SkSpecialSurface : public SkRefCnt {
public:
    const SkSurfaceProps& props() const { return fProps; }

    int width() const { return fSubset.width(); }
    int height() const { return fSubset.height(); }

    /**
    *  Return a canvas that will draw into this surface. This will always
    *  return the same canvas for a given surface, and is managed/owned by the
    *  surface.
    *
    *  The canvas will be invalid after 'newImageSnapshot' is called.
    */
    SkCanvas* getCanvas();

    /**
    *  Returns an image of the current state of the surface pixels up to this
    *  point. The canvas returned by 'getCanvas' becomes invalidated by this
    *  call and no more drawing to this surface is allowed.
    *
    *  Note: the caller inherits a ref from this call that must be balanced
    */
    sk_sp<SkSpecialImage> makeImageSnapshot();

#if SK_SUPPORT_GPU
    /**
     *  Allocate a new GPU-backed SkSpecialSurface. If the requested surface cannot
     *  be created, nullptr will be returned.
     */
    static sk_sp<SkSpecialSurface> MakeRenderTarget(GrRecordingContext*,
                                                    const GrBackendFormat& format,
                                                    int width, int height,
                                                    GrPixelConfig config,
                                                    sk_sp<SkColorSpace> colorSpace,
                                                    const SkSurfaceProps* = nullptr);
#endif

    /**
     * Use and existing SkBitmap as the backing store.
     */
    static sk_sp<SkSpecialSurface> MakeFromBitmap(const SkIRect& subset, SkBitmap& bm,
                                                  const SkSurfaceProps* = nullptr);

    /**
     *  Return a new CPU-backed surface, with the memory for the pixels automatically
     *  allocated.
     *
     *  If the requested surface cannot be created, or the request is not a
     *  supported configuration, nullptr will be returned.
     */
    static sk_sp<SkSpecialSurface> MakeRaster(const SkImageInfo&,
                                              const SkSurfaceProps* = nullptr);

protected:
    SkSpecialSurface(const SkIRect& subset, const SkSurfaceProps*);

    // For testing only
    friend class TestingSpecialSurfaceAccess;
    const SkIRect& subset() const { return fSubset; }

private:
    const SkSurfaceProps fProps;
    const SkIRect        fSubset;

    typedef SkRefCnt INHERITED;
};

#endif
