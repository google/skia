
/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPixelRef_DEFINED
#define SkPixelRef_DEFINED

#include "SkBitmap.h"
#include "SkRefCnt.h"
#include "SkString.h"
#include "SkFlattenable.h"

#ifdef SK_DEBUG
    /**
     *  Defining SK_IGNORE_PIXELREF_SETPRELOCKED will force all pixelref
     *  subclasses to correctly handle lock/unlock pixels. For performance
     *  reasons, simple malloc-based subclasses call setPreLocked() to skip
     *  the overhead of implementing these calls.
     *
     *  This build-flag disables that optimization, to add in debugging our
     *  call-sites, to ensure that they correctly balance their calls of
     *  lock and unlock.
     */
//    #define SK_IGNORE_PIXELREF_SETPRELOCKED
#endif

class SkColorTable;
class SkData;
struct SkIRect;
class SkMutex;

// this is an opaque class, not interpreted by skia
class SkGpuTexture;

/** \class SkPixelRef

    This class is the smart container for pixel memory, and is used with
    SkBitmap. A pixelref is installed into a bitmap, and then the bitmap can
    access the actual pixel memory by calling lockPixels/unlockPixels.

    This class can be shared/accessed between multiple threads.
*/
class SK_API SkPixelRef : public SkFlattenable {
public:
    SK_DECLARE_INST_COUNT(SkPixelRef)

    explicit SkPixelRef(SkBaseMutex* mutex = NULL);

    /** Return the pixel memory returned from lockPixels, or null if the
        lockCount is 0.
    */
    void* pixels() const { return fPixels; }

    /** Return the current colorTable (if any) if pixels are locked, or null.
    */
    SkColorTable* colorTable() const { return fColorTable; }

    /**
     *  Returns true if the lockcount > 0
     */
    bool isLocked() const { return fLockCount > 0; }

    SkDEBUGCODE(int getLockCount() const { return fLockCount; })

    /** Call to access the pixel memory, which is returned. Balance with a call
        to unlockPixels().
    */
    void lockPixels();
    /** Call to balanace a previous call to lockPixels(). Returns the pixels
        (or null) after the unlock. NOTE: lock calls can be nested, but the
        matching number of unlock calls must be made in order to free the
        memory (if the subclass implements caching/deferred-decoding.)
    */
    void unlockPixels();

    /**
     *  Some bitmaps can return a copy of their pixels for lockPixels(), but
     *  that copy, if modified, will not be pushed back. These bitmaps should
     *  not be used as targets for a raster device/canvas (since all pixels
     *  modifications will be lost when unlockPixels() is called.)
     */
    bool lockPixelsAreWritable() const;

    /** Returns a non-zero, unique value corresponding to the pixels in this
        pixelref. Each time the pixels are changed (and notifyPixelsChanged is
        called), a different generation ID will be returned.
    */
    uint32_t getGenerationID() const;

    /** Call this if you have changed the contents of the pixels. This will in-
        turn cause a different generation ID value to be returned from
        getGenerationID().
    */
    void notifyPixelsChanged();

    /** Returns true if this pixelref is marked as immutable, meaning that the
        contents of its pixels will not change for the lifetime of the pixelref.
    */
    bool isImmutable() const { return fIsImmutable; }

    /** Marks this pixelref is immutable, meaning that the contents of its
        pixels will not change for the lifetime of the pixelref. This state can
        be set on a pixelref, but it cannot be cleared once it is set.
    */
    void setImmutable();

    /** Return the optional URI string associated with this pixelref. May be
        null.
    */
    const char* getURI() const { return fURI.size() ? fURI.c_str() : NULL; }

    /** Copy a URI string to this pixelref, or clear the URI if the uri is null
     */
    void setURI(const char uri[]) {
        fURI.set(uri);
    }

    /** Copy a URI string to this pixelref
     */
    void setURI(const char uri[], size_t len) {
        fURI.set(uri, len);
    }

    /** Assign a URI string to this pixelref.
    */
    void setURI(const SkString& uri) { fURI = uri; }

    /**
     *  If the pixelRef has an encoded (i.e. compressed) representation,
     *  return a ref to its data. If the pixelRef
     *  is uncompressed or otherwise does not have this form, return NULL.
     *
     *  If non-null is returned, the caller is responsible for calling unref()
     *  on the data when it is finished.
     */
    SkData* refEncodedData() {
        return this->onRefEncodedData();
    }

    /** Are we really wrapping a texture instead of a bitmap?
     */
    virtual SkGpuTexture* getTexture() { return NULL; }

    bool readPixels(SkBitmap* dst, const SkIRect* subset = NULL);

    /**
     *  Makes a deep copy of this PixelRef, respecting the requested config.
     *  @param config Desired config.
     *  @param subset Subset of this PixelRef to copy. Must be fully contained within the bounds of
     *         of this PixelRef.
     *  @return A new SkPixelRef, or NULL if either there is an error (e.g. the destination could
     *          not be created with the given config), or this PixelRef does not support deep
     *          copies.
     */
    virtual SkPixelRef* deepCopy(SkBitmap::Config config, const SkIRect* subset = NULL) {
        return NULL;
    }

#ifdef SK_BUILD_FOR_ANDROID
    /**
     *  Acquire a "global" ref on this object.
     *  The default implementation just calls ref(), but subclasses can override
     *  this method to implement additional behavior.
     */
    virtual void globalRef(void* data=NULL);

    /**
     *  Release a "global" ref on this object.
     *  The default implementation just calls unref(), but subclasses can override
     *  this method to implement additional behavior.
     */
    virtual void globalUnref();
#endif

protected:
    /** Called when the lockCount goes from 0 to 1. The caller will have already
        acquire a mutex for thread safety, so this method need not do that.
    */
    virtual void* onLockPixels(SkColorTable**) = 0;
    /** Called when the lock count goes from 1 to 0. The caller will have
        already acquire a mutex for thread safety, so this method need not do
        that.
    */
    virtual void onUnlockPixels() = 0;

    /** Default impl returns true */
    virtual bool onLockPixelsAreWritable() const;

    /**
     *  For pixelrefs that don't have access to their raw pixels, they may be
     *  able to make a copy of them (e.g. if the pixels are on the GPU).
     *
     *  The base class implementation returns false;
     */
    virtual bool onReadPixels(SkBitmap* dst, const SkIRect* subsetOrNull);

    // default impl returns NULL.
    virtual SkData* onRefEncodedData();

    /** Return the mutex associated with this pixelref. This value is assigned
        in the constructor, and cannot change during the lifetime of the object.
    */
    SkBaseMutex* mutex() const { return fMutex; }

    // serialization
    SkPixelRef(SkFlattenableReadBuffer&, SkBaseMutex*);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

    // only call from constructor. Flags this to always be locked, removing
    // the need to grab the mutex and call onLockPixels/onUnlockPixels.
    // Performance tweak to avoid those calls (esp. in multi-thread use case).
    void setPreLocked(void* pixels, SkColorTable* ctable);

private:

    SkBaseMutex*    fMutex; // must remain in scope for the life of this object
    void*           fPixels;
    SkColorTable*   fColorTable;    // we do not track ownership, subclass does
    int             fLockCount;

    mutable uint32_t fGenerationID;

    // SkBitmap is only a friend so that when copying, it can modify the new SkPixelRef to have the
    // same fGenerationID as the original.
    friend class SkBitmap;

    SkString    fURI;

    // can go from false to true, but never from true to false
    bool    fIsImmutable;
    // only ever set in constructor, const after that
    bool    fPreLocked;

    void setMutex(SkBaseMutex* mutex);

    typedef SkFlattenable INHERITED;
};

#endif
