/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDiscardablePixelRef_DEFINED
#define SkDiscardablePixelRef_DEFINED

#include "SkPixelRef.h"
#include "SkImageGenerator.h"
#include "SkImageInfo.h"

class SkDiscardableMemory;

/**
 * An interface that allows a purgable PixelRef to re-decode an image.
 */

class SkDiscardablePixelRef : public SkPixelRef {
public:
    /**
     *  Takes ownership of SkImageGenerator.  If this method fails for
     *  whatever reason, it will return false and immediatetely delete
     *  the generator.  If it succeeds, it will modify destination
     *  bitmap.
     *
     *  If Install fails or when the SkDiscardablePixelRef that is
     *  installed into destination is destroyed, it will call
     *  SkDELETE() on the generator.  Therefore, generator should be
     *  allocated with SkNEW() or SkNEW_ARGS().
     */
    static bool Install(SkImageGenerator* generator, SkBitmap* destination);

    SK_DECLARE_UNFLATTENABLE_OBJECT()

protected:
    ~SkDiscardablePixelRef();
    virtual void* onLockPixels(SkColorTable**) SK_OVERRIDE;
    virtual void onUnlockPixels() SK_OVERRIDE;
    virtual bool onLockPixelsAreWritable() const SK_OVERRIDE { return false; }

    virtual SkData* onRefEncodedData() SK_OVERRIDE {
        return fGenerator->refEncodedData();
    }

private:
    SkImageGenerator* const fGenerator;
    const SkImageInfo fInfo;
    const size_t fSize;  // size of memory to be allocated
    const size_t fRowBytes;
    // These const members should not change over the life of the
    // PixelRef, since the SkBitmap doesn't expect them to change.

    SkDiscardableMemory* fDiscardableMemory;

    /* Takes ownership of SkImageGenerator. */
    SkDiscardablePixelRef(SkImageGenerator* generator,
                          const SkImageInfo& info,
                          size_t size,
                          size_t rowBytes);
};
#endif  // SkDiscardablePixelRef_DEFINED
