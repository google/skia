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
class SkSurface : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkSurface)

    /**
     *  Create a new surface, using the specified pixels/rowbytes as its
     *  backend.
     *
     *  If the requested surface cannot be created, or the request is not a
     *  supported configuration, NULL will be returned.
     */
    static SkSurface* NewRasterDirect(const SkImage::Info&, void* pixels, size_t rowBytes);

    /**
     *  Return a new surface, with the memory for the pixels automatically
     *  allocated.
     *
     *  If the requested surface cannot be created, or the request is not a
     *  supported configuration, NULL will be returned.
     */
    static SkSurface* NewRaster(const SkImage::Info&);

    /**
     *  Return a new surface whose contents will be recorded into a picture.
     *  When this surface is drawn into another canvas, its contents will be
     *  "replayed" into that canvas.
     */
    static SkSurface* NewPicture(int width, int height);

    /**
     *  Return a new surface using the specified render target.
     */
    static SkSurface* NewRenderTargetDirect(GrContext*, GrRenderTarget*);

    /**
     *  Return a new surface whose contents will be drawn to an offscreen
     *  render target, allocated by the surface.
     */
    static SkSurface* NewRenderTarget(GrContext*, const SkImage::Info&, int sampleCount = 0);

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
     *  Call this if the contents have changed. This will (lazily) force a new
     *  value to be returned from generationID() when it is called next.
     */
    void notifyContentChanged();

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
    SkSurface* newSurface(const SkImage::Info&);

    /**
     *  Returns an image of the current state of the surface pixels up to this
     *  point. Subsequent changes to the surface (by drawing into its canvas)
     *  will not be reflected in this image.
     */
    SkImage* newImageShapshot();

    /**
     *  Thought the caller could get a snapshot image explicitly, and draw that,
     *  it seems that directly drawing a surface into another canvas might be
     *  a common pattern, and that we could possibly be more efficient, since
     *  we'd know that the "snapshot" need only live until we've handed it off
     *  to the canvas.
     */
    void draw(SkCanvas*, SkScalar x, SkScalar y, const SkPaint*);

protected:
    SkSurface(int width, int height);

    // called by subclass if their contents have changed
    void dirtyGenerationID() {
        fGenerationID = 0;
    }

private:
    const int   fWidth;
    const int   fHeight;
    uint32_t    fGenerationID;

    typedef SkRefCnt INHERITED;
};

#endif
