/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_Base_DEFINED
#define SkImage_Base_DEFINED

#include "SkImage.h"
#include "SkSurface.h"

enum {
    kNeedNewImageUniqueID = 0
};

static SkSurfaceProps copy_or_safe_defaults(const SkSurfaceProps* props) {
    return props ? *props : SkSurfaceProps(0, kUnknown_SkPixelGeometry);
}

class SkImage_Base : public SkImage {
public:
    SkImage_Base(int width, int height, uint32_t uniqueID, const SkSurfaceProps* props)
        : INHERITED(width, height, uniqueID)
        , fProps(copy_or_safe_defaults(props))
    {}

    /**
     *  If the props weren't know at constructor time, call this but only before the image is
     *  ever released into the wild (since the props field must appear to be immutable).
     */
    void initWithProps(const SkSurfaceProps& props) {
        SkASSERT(this->unique());   // only viewed by one thread
        SkSurfaceProps* mutableProps = const_cast<SkSurfaceProps*>(&fProps);
        SkASSERT(mutableProps != &props);   // check for self-assignment
        mutableProps->~SkSurfaceProps();
        new (mutableProps) SkSurfaceProps(props);
    }

    const SkSurfaceProps& props() const { return fProps; }

    virtual SkSurface* onNewSurface(const SkImageInfo&, const SkSurfaceProps&) const = 0;

    virtual const void* onPeekPixels(SkImageInfo*, size_t* /*rowBytes*/) const {
        return NULL;
    }

    // Default impl calls onDraw
    virtual bool onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                              int srcX, int srcY) const;
    
    virtual GrTexture* getTexture() const { return NULL; }

    // return a read-only copy of the pixels. We promise to not modify them,
    // but only inspect them (or encode them).
    virtual bool getROPixels(SkBitmap*) const { return false; }

    virtual SkShader* onNewShader(SkShader::TileMode,
                                  SkShader::TileMode,
                                  const SkMatrix* localMatrix) const { return NULL; };

    // newWidth > 0, newHeight > 0, subset either NULL or a proper subset of this bounds
    virtual SkImage* onNewImage(int newWidth, int newHeight, const SkIRect* subset,
                                SkFilterQuality) const;
    virtual SkData* onRefEncoded() const { return NULL; }

    virtual bool onAsLegacyBitmap(SkBitmap*, LegacyBitmapMode) const;

private:
    const SkSurfaceProps fProps;

    typedef SkImage INHERITED;
};

static inline SkImage_Base* as_IB(SkImage* image) {
    return static_cast<SkImage_Base*>(image);
}

static inline const SkImage_Base* as_IB(const SkImage* image) {
    return static_cast<const SkImage_Base*>(image);
}

#endif
