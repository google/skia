/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */

#ifndef SkSpecialSurface_DEFINED
#define SkSpecialSurface_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurfaceProps.h"

#if SK_SUPPORT_GPU
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#endif

#if SK_GRAPHITE_ENABLED
namespace skgpu::graphite {
    class Recorder;
}
#endif

class GrBackendFormat;
class GrRecordingContext;
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

    const SkIRect& subset() const { return fSubset; }
    int width() const { return fSubset.width(); }
    int height() const { return fSubset.height(); }

    /**
    *  Return a canvas that will draw into this special surface. This will always
    *  return the same canvas for a given special surface, and is managed/owned by the
    *  special surface.
    *
    *  The canvas will be invalid after 'makeImageSnapshot' is called.
    */
    SkCanvas* getCanvas() { return fCanvas.get(); }

    /**
    *  Returns an image of the current state of the surface pixels up to this
    *  point. The canvas returned by 'getCanvas' becomes invalidated by this
    *  call and no more drawing to this surface is allowed.
    */
    sk_sp<SkSpecialImage> makeImageSnapshot();

#if SK_SUPPORT_GPU
    /**
     *  Allocate a new GPU-backed SkSpecialSurface. If the requested surface cannot
     *  be created, nullptr will be returned.
     */
    static sk_sp<SkSpecialSurface> MakeRenderTarget(GrRecordingContext*,
                                                    const SkImageInfo&,
                                                    const SkSurfaceProps&,
                                                    GrSurfaceOrigin);
#endif

#if SK_GRAPHITE_ENABLED
    static sk_sp<SkSpecialSurface> MakeGraphite(skgpu::graphite::Recorder*,
                                                const SkImageInfo&,
                                                const SkSurfaceProps&);
#endif

    /**
     * Use and existing SkBitmap as the backing store.
     */
    static sk_sp<SkSpecialSurface> MakeFromBitmap(const SkIRect& subset,
                                                  SkBitmap& bm,
                                                  const SkSurfaceProps&);

    /**
     *  Return a new CPU-backed surface, with the memory for the pixels automatically
     *  allocated.
     *
     *  If the requested surface cannot be created, or the request is not a
     *  supported configuration, nullptr will be returned.
     */
    static sk_sp<SkSpecialSurface> MakeRaster(const SkImageInfo&,
                                              const SkSurfaceProps&);

protected:
    SkSpecialSurface(const SkIRect& subset, const SkSurfaceProps&);

    virtual sk_sp<SkSpecialImage> onMakeImageSnapshot() = 0;

    std::unique_ptr<SkCanvas> fCanvas;   // initialized by derived classes in ctors

private:
    const SkSurfaceProps fProps;
    const SkIRect        fSubset;
};

#endif
