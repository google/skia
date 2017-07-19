/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPixelRef_DEFINED
#define SkPixelRef_DEFINED

#include "../private/SkAtomics.h"
#include "../private/SkMutex.h"
#include "../private/SkTDArray.h"
#include "SkBitmap.h"
#include "SkFilterQuality.h"
#include "SkImageInfo.h"
#include "SkPixmap.h"
#include "SkRefCnt.h"
#include "SkSize.h"
#include "SkString.h"

struct SkIRect;

class GrTexture;
class SkDiscardableMemory;

/** \class SkPixelRef

    This class is the smart container for pixel memory, and is used with SkBitmap.
    This class can be shared/accessed between multiple threads.
*/
class SK_API SkPixelRef : public SkRefCnt {
public:
    SkPixelRef(int width, int height, void* addr, size_t rowBytes);
    ~SkPixelRef() override;

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    void* pixels() const { return fPixels; }
    size_t rowBytes() const { return fRowBytes; }

    /** Returns a non-zero, unique value corresponding to the pixels in this
        pixelref. Each time the pixels are changed (and notifyPixelsChanged is
        called), a different generation ID will be returned.
    */
    uint32_t getGenerationID() const;

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    /** Returns a non-zero, unique value corresponding to this SkPixelRef.
        Unlike the generation ID, this ID remains the same even when the pixels
        are changed. IDs are not reused (until uint32_t wraps), so it is safe
        to consider this ID unique even after this SkPixelRef is deleted.

        Can be used as a key which uniquely identifies this SkPixelRef
        regardless of changes to its pixels or deletion of this object.
     */
    uint32_t getStableID() const { return fStableID; }
#endif

    /**
     *  Call this if you have changed the contents of the pixels. This will in-
     *  turn cause a different generation ID value to be returned from
     *  getGenerationID().
     */
    void notifyPixelsChanged();

    /** Returns true if this pixelref is marked as immutable, meaning that the
        contents of its pixels will not change for the lifetime of the pixelref.
    */
    bool isImmutable() const { return fMutability != kMutable; }

    /** Marks this pixelref is immutable, meaning that the contents of its
        pixels will not change for the lifetime of the pixelref. This state can
        be set on a pixelref, but it cannot be cleared once it is set.
    */
    void setImmutable();

    // Register a listener that may be called the next time our generation ID changes.
    //
    // We'll only call the listener if we're confident that we are the only SkPixelRef with this
    // generation ID.  If our generation ID changes and we decide not to call the listener, we'll
    // never call it: you must add a new listener for each generation ID change.  We also won't call
    // the listener when we're certain no one knows what our generation ID is.
    //
    // This can be used to invalidate caches keyed by SkPixelRef generation ID.
    struct GenIDChangeListener {
        virtual ~GenIDChangeListener() {}
        virtual void onChange() = 0;
    };

    // Takes ownership of listener.
    void addGenIDChangeListener(GenIDChangeListener* listener);

    // Call when this pixelref is part of the key to a resourcecache entry. This allows the cache
    // to know automatically those entries can be purged when this pixelref is changed or deleted.
    void notifyAddedToCache() {
        fAddedToCache.store(true);
    }

    virtual SkDiscardableMemory* diagnostic_only_getDiscardable() const { return NULL; }

protected:
    // default impl does nothing.
    virtual void onNotifyPixelsChanged();

    void android_only_reset(int width, int height, size_t rowBytes);

private:
    int                 fWidth;
    int                 fHeight;
    void*               fPixels;
    size_t              fRowBytes;

    // Bottom bit indicates the Gen ID is unique.
    bool genIDIsUnique() const { return SkToBool(fTaggedGenID.load() & 1); }
    mutable SkAtomic<uint32_t> fTaggedGenID;

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    const uint32_t fStableID;
#endif

    SkTDArray<GenIDChangeListener*> fGenIDChangeListeners;  // pointers are owned

    // Set true by caches when they cache content that's derived from the current pixels.
    SkAtomic<bool> fAddedToCache;

    enum {
        kMutable,               // PixelRefs begin mutable.
        kTemporarilyImmutable,  // Considered immutable, but can revert to mutable.
        kImmutable,             // Once set to this state, it never leaves.
    } fMutability : 8;          // easily fits inside a byte

    void needsNewGenID();
    void callGenIDChangeListeners();

    void setTemporarilyImmutable();
    void restoreMutability();
    friend class SkSurface_Raster;   // For the two methods above.

    friend class SkImage_Raster;
    friend class SkSpecialImage_Raster;

    void setImmutableWithID(uint32_t genID);
    friend class SkImage_Gpu;
    friend class SkImage_Lazy;
    friend class SkSpecialImage_Gpu;
    friend void SkBitmapCache_setImmutableWithID(SkPixelRef*, uint32_t);

    typedef SkRefCnt INHERITED;
};

#endif
