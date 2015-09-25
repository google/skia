/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_Base_DEFINED
#define SkImage_Base_DEFINED

#include "SkAtomics.h"
#include "SkImage.h"
#include "SkSurface.h"

#include <new>

enum {
    kNeedNewImageUniqueID = 0
};

class SkImage_Base : public SkImage {
public:
    SkImage_Base(int width, int height, uint32_t uniqueID, const SkSurfaceProps* props);
    virtual ~SkImage_Base();

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

    virtual const void* onPeekPixels(SkImageInfo*, size_t* /*rowBytes*/) const {
        return nullptr;
    }

    // Default impl calls onDraw
    virtual bool onReadPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                              int srcX, int srcY) const;

    virtual GrTexture* peekTexture() const { return nullptr; }

    // return a read-only copy of the pixels. We promise to not modify them,
    // but only inspect them (or encode them).
    virtual bool getROPixels(SkBitmap*) const = 0;

    // Caller must call unref when they are done.
    virtual GrTexture* asTextureRef(GrContext*, SkImageUsageType) const = 0;

    virtual SkImage* onNewSubset(const SkIRect&) const = 0;

    virtual SkData* onRefEncoded() const { return nullptr; }

    virtual bool onAsLegacyBitmap(SkBitmap*, LegacyBitmapMode) const;

    virtual bool onIsLazyGenerated() const { return false; }

    // Call when this image is part of the key to a resourcecache entry. This allows the cache
    // to know automatically those entries can be purged when this SkImage deleted.
    void notifyAddedToCache() const {
        fAddedToCache.store(true);
    }

private:
    const SkSurfaceProps fProps;

    // Set true by caches when they cache content that's derived from the current pixels.
    mutable SkAtomic<bool> fAddedToCache;

    typedef SkImage INHERITED;
};

static inline SkImage_Base* as_IB(SkImage* image) {
    return static_cast<SkImage_Base*>(image);
}

static inline const SkImage_Base* as_IB(const SkImage* image) {
    return static_cast<const SkImage_Base*>(image);
}

#endif
