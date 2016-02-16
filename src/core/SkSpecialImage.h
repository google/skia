/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */

#ifndef SkSpecialImage_DEFINED
#define SkSpecialImage_DEFINED

#include "SkRefCnt.h"

class GrTexture;
class SkBitmap;
class SkCanvas;
class SkImage;
struct SkImageInfo;
class SkPaint;
class SkSpecialSurface;

/**
 * This is a restricted form of SkImage solely intended for internal use. It
 * differs from SkImage in that:
 *      - it can only be backed by raster or gpu (no generators)
 *      - it can be backed by a GrTexture larger than its nominal bounds
 *      - it can't be drawn tiled
 *      - it can't be drawn with MIPMAPs
 * It is similar to SkImage in that it abstracts how the pixels are stored/represented.
 *
 * Note: the contents of the backing storage outside of the subset rect are undefined.
 */
class SkSpecialImage : public SkRefCnt {
public:
    int width() const { return fSubset.width(); }
    int height() const { return fSubset.height(); }

    /**
     *  Draw this SpecialImage into the canvas.
     */
    void draw(SkCanvas*, SkScalar x, SkScalar y, const SkPaint*) const;

    static SkSpecialImage* NewFromImage(const SkIRect& subset, const SkImage*);
    static SkSpecialImage* NewFromRaster(const SkIRect& subset, const SkBitmap&);
    static SkSpecialImage* NewFromGpu(const SkIRect& subset, GrTexture*);

    /**
     *  Create a new surface with a backend that is compatible with this image.
     */
    SkSpecialSurface* newSurface(const SkImageInfo&) const;

protected:
    SkSpecialImage(const SkIRect& subset) : fSubset(subset) { }

    // The following 3 are for testing and shouldn't be used.
    friend class TestingSpecialImageAccess;
    friend class TestingSpecialSurfaceAccess;
    const SkIRect& subset() const { return fSubset; }

    /**
     *  If the SpecialImage is backed by cpu pixels, return the const address
     *  of those pixels and, if not null, return the ImageInfo and rowBytes.
     *  The returned address is only valid while the image object is in scope.
     *
     *  The returned ImageInfo represents the backing memory. Use 'subset'
     *  to get the active portion's dimensions.
     *
     *  On failure, return false and ignore the pixmap parameter.
     */
    bool peekPixels(SkPixmap*) const;

    /**
     *  If the SpecialImage is backed by a gpu texture, return that texture.
     *  The active portion of the texture can be retrieved via 'subset'.
     */
    GrTexture* peekTexture() const;

private:
    const SkIRect fSubset;

    typedef SkRefCnt INHERITED;
};

#endif

