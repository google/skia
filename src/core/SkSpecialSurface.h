/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */

#ifndef SkSpecialSurface_DEFINED
#define SkSpecialSurface_DEFINED

#include "SkRefCnt.h"
#include "SkSurfaceProps.h"

class GrContext;
struct GrSurfaceDesc;
class SkCanvas;
struct SkImageInfo;
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

    /**
     *  Use an existing (renderTarget-capable) GrTexture as the backing store.
     */
    static sk_sp<SkSpecialSurface> MakeFromTexture(SkImageFilter::Proxy* proxy,
                                                   const SkIRect& subset, GrTexture*,
                                                   const SkSurfaceProps* = nullptr);

    /**
     *  Allocate a new GPU-backed SkSpecialSurface. If the requested surface cannot
     *  be created, nullptr will be returned.
     */
    static sk_sp<SkSpecialSurface> MakeRenderTarget(SkImageFilter::Proxy* proxy,
                                                    GrContext*, const GrSurfaceDesc&,
                                                    const SkSurfaceProps* = nullptr);

    /**
     * Use and existing SkBitmap as the backing store.
     */
    static sk_sp<SkSpecialSurface> MakeFromBitmap(SkImageFilter::Proxy* proxy,
                                                  const SkIRect& subset, SkBitmap& bm,
                                                  const SkSurfaceProps* = nullptr);

    /**
     *  Return a new CPU-backed surface, with the memory for the pixels automatically
     *  allocated.
     *
     *  If the requested surface cannot be created, or the request is not a
     *  supported configuration, nullptr will be returned.
     */
    static sk_sp<SkSpecialSurface> MakeRaster(SkImageFilter::Proxy* proxy,
                                              const SkImageInfo&,
                                              const SkSurfaceProps* = nullptr);

protected:
    SkSpecialSurface(SkImageFilter::Proxy*, const SkIRect& subset, const SkSurfaceProps*);

    // For testing only
    friend class TestingSpecialSurfaceAccess;
    const SkIRect& subset() const { return fSubset; }

    // TODO: remove this ASAP (see skbug.com/4965)
    SkImageFilter::Proxy* proxy() const { return fProxy; }

private:
    const SkSurfaceProps fProps;
    const SkIRect        fSubset;

    // TODO: remove this ASAP (see skbug.com/4965)
    SkImageFilter::Proxy* fProxy;

    typedef SkRefCnt INHERITED;
};

#endif
