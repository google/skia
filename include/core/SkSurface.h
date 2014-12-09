/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurface_DEFINED
#define SkSurface_DEFINED

#include "SkRefCnt.h"
#include "SkImage.h"
#include "SkSurfaceProps.h"

class SkCanvas;
class SkPaint;
class GrContext;
class GrRenderTarget;

/**
 *  SkSurface represents the backend/results of drawing to a canvas. For raster
 *  drawing, the surface will be pixels, but (for example) when drawing into
 *  a PDF or Picture canvas, the surface stores the recorded commands.
 *
 *  To draw into a canvas, first create the appropriate type of Surface, and
 *  then request the canvas from the surface.
 */
class SK_API SkSurface : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkSurface)

    /**
     *  Create a new surface, using the specified pixels/rowbytes as its
     *  backend.
     *
     *  If the requested surface cannot be created, or the request is not a
     *  supported configuration, NULL will be returned.
     */
    static SkSurface* NewRasterDirect(const SkImageInfo&, void* pixels, size_t rowBytes,
                                      const SkSurfaceProps* = NULL);

    /**
     *  The same as NewRasterDirect, but also accepts a call-back routine, which is invoked
     *  when the surface is deleted, and is passed the pixel memory and the specified context.
     */
    static SkSurface* NewRasterDirectReleaseProc(const SkImageInfo&, void* pixels, size_t rowBytes,
                                                 void (*releaseProc)(void* pixels, void* context),
                                                 void* context, const SkSurfaceProps* = NULL);

    /**
     *  Return a new surface, with the memory for the pixels automatically
     *  allocated.
     *
     *  If the requested surface cannot be created, or the request is not a
     *  supported configuration, NULL will be returned.
     */
    static SkSurface* NewRaster(const SkImageInfo&, const SkSurfaceProps* = NULL);

    /**
     *  Helper version of NewRaster. It creates a SkImageInfo with the
     *  specified width and height, and populates the rest of info to match
     *  pixels in SkPMColor format.
     */
    static SkSurface* NewRasterPMColor(int width, int height, const SkSurfaceProps* props = NULL) {
        return NewRaster(SkImageInfo::MakeN32Premul(width, height), props);
    }

    /**
     *  Return a new surface using the specified render target.
     */
    static SkSurface* NewRenderTargetDirect(GrRenderTarget*, const SkSurfaceProps*);
    
    static SkSurface* NewRenderTargetDirect(GrRenderTarget* target) {
        return NewRenderTargetDirect(target, NULL);
    }
    
    /**
     *  Return a new surface whose contents will be drawn to an offscreen
     *  render target, allocated by the surface.
     */
    static SkSurface* NewRenderTarget(GrContext*, const SkImageInfo&, int sampleCount,
                                      const SkSurfaceProps* = NULL);

    static SkSurface* NewRenderTarget(GrContext* gr, const SkImageInfo& info) {
        return NewRenderTarget(gr, info, 0, NULL);
    }

    /**
     *  Return a new surface whose contents will be drawn to an offscreen
     *  render target, allocated by the surface from the scratch texture pool
     *  managed by the GrContext. The scratch texture pool serves the purpose
     *  of retaining textures after they are no longer in use in order to
     *  re-use them later without having to re-allocate.  Scratch textures
     *  should be used in cases where high turnover is expected. This allows,
     *  for example, the copy on write to recycle a texture from a recently
     *  released SkImage snapshot of the surface.
     *  Note: Scratch textures count against the GrContext's cached resource
     *  budget.
     */
    static SkSurface* NewScratchRenderTarget(GrContext*, const SkImageInfo&, int sampleCount,
                                             const SkSurfaceProps* = NULL);

    static SkSurface* NewScratchRenderTarget(GrContext* gr, const SkImageInfo& info) {
        return NewScratchRenderTarget(gr, info, 0, NULL);
    }

#ifdef SK_SUPPORT_LEGACY_TEXTRENDERMODE
    /**
     *  Text rendering modes that can be passed to NewRenderTarget*
     */
    enum TextRenderMode {
        /**
         *  This will use the standard text rendering method
         */
        kStandard_TextRenderMode,
        /**
         *  This will use signed distance fields for text rendering when possible
         */
        kDistanceField_TextRenderMode,
    };
    static SkSurface* NewRenderTargetDirect(GrRenderTarget*, TextRenderMode);
    static SkSurface* NewRenderTarget(GrContext*, const SkImageInfo&, int sampleCount,
                                      TextRenderMode);
    static SkSurface* NewScratchRenderTarget(GrContext*, const SkImageInfo&, int sampleCount,
                                             TextRenderMode);
#endif

    int width() const { return fWidth; }
    int height() const { return fHeight; }

    /**
     *  Returns a unique non-zero, unique value identifying the content of this
     *  surface. Each time the content is changed changed, either by drawing
     *  into this surface, or explicitly calling notifyContentChanged()) this
     *  method will return a new value.
     *
     *  If this surface is empty (i.e. has a zero-dimention), this will return
     *  0.
     */
    uint32_t generationID();

    /**
     *  Modes that can be passed to notifyContentWillChange
     */
    enum ContentChangeMode {
        /**
         *  Use this mode if it is known that the upcoming content changes will
         *  clear or overwrite prior contents, thus making them discardable.
         */
        kDiscard_ContentChangeMode,
        /**
         *  Use this mode if prior surface contents need to be preserved or
         *  if in doubt.
         */
        kRetain_ContentChangeMode,
    };

    /**
     *  Call this if the contents are about to change. This will (lazily) force a new
     *  value to be returned from generationID() when it is called next.
     */
    void notifyContentWillChange(ContentChangeMode mode);

    /**
     *  Return a canvas that will draw into this surface. This will always
     *  return the same canvas for a given surface, and is manged/owned by the
     *  surface. It should not be used when its parent surface has gone out of
     *  scope.
     */
    SkCanvas* getCanvas();

    /**
     *  Return a new surface that is "compatible" with this one, in that it will
     *  efficiently be able to be drawn into this surface. Typical calling
     *  pattern:
     *
     *  SkSurface* A = SkSurface::New...();
     *  SkCanvas* canvasA = surfaceA->newCanvas();
     *  ...
     *  SkSurface* surfaceB = surfaceA->newSurface(...);
     *  SkCanvas* canvasB = surfaceB->newCanvas();
     *  ... // draw using canvasB
     *  canvasA->drawSurface(surfaceB); // <--- this will always be optimal!
     */
    SkSurface* newSurface(const SkImageInfo&);

    /**
     *  Returns an image of the current state of the surface pixels up to this
     *  point. Subsequent changes to the surface (by drawing into its canvas)
     *  will not be reflected in this image.
     */
    SkImage* newImageSnapshot();

    /**
     *  Thought the caller could get a snapshot image explicitly, and draw that,
     *  it seems that directly drawing a surface into another canvas might be
     *  a common pattern, and that we could possibly be more efficient, since
     *  we'd know that the "snapshot" need only live until we've handed it off
     *  to the canvas.
     */
    void draw(SkCanvas*, SkScalar x, SkScalar y, const SkPaint*);

    /**
     *  If the surface has direct access to its pixels (i.e. they are in local
     *  RAM) return the const-address of those pixels, and if not null, return
     *  the ImageInfo and rowBytes. The returned address is only valid while
     *  the surface object is in scope, and no API call is made on the surface
     *  or its canvas.
     *
     *  On failure, returns NULL and the info and rowBytes parameters are
     *  ignored.
     */
    const void* peekPixels(SkImageInfo* info, size_t* rowBytes);

    const SkSurfaceProps& props() const { return fProps; }

protected:
    SkSurface(int width, int height, const SkSurfaceProps*);
    SkSurface(const SkImageInfo&, const SkSurfaceProps*);

    // called by subclass if their contents have changed
    void dirtyGenerationID() {
        fGenerationID = 0;
    }

private:
    const SkSurfaceProps fProps;
    const int            fWidth;
    const int            fHeight;
    uint32_t             fGenerationID;

    typedef SkRefCnt INHERITED;
};

#endif
