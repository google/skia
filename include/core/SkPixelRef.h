/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPixelRef_DEFINED
#define SkPixelRef_DEFINED

#include "SkBitmap.h"
#include "SkDynamicAnnotations.h"
#include "SkRefCnt.h"
#include "SkString.h"
#include "SkImageInfo.h"
#include "SkSize.h"
#include "SkTDArray.h"

//#define xed

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

class GrTexture;

/** \class SkPixelRef

    This class is the smart container for pixel memory, and is used with
    SkBitmap. A pixelref is installed into a bitmap, and then the bitmap can
    access the actual pixel memory by calling lockPixels/unlockPixels.

    This class can be shared/accessed between multiple threads.
*/
class SK_API SkPixelRef : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkPixelRef)

    explicit SkPixelRef(const SkImageInfo&);
    SkPixelRef(const SkImageInfo&, SkBaseMutex* mutex);
    virtual ~SkPixelRef();

    const SkImageInfo& info() const {
        return fInfo;
    }

    /** Return the pixel memory returned from lockPixels, or null if the
        lockCount is 0.
    */
    void* pixels() const { return fRec.fPixels; }

    /** Return the current colorTable (if any) if pixels are locked, or null.
    */
    SkColorTable* colorTable() const { return fRec.fColorTable; }

    size_t rowBytes() const { return fRec.fRowBytes; }

    /**
     *  To access the actual pixels of a pixelref, it must be "locked".
     *  Calling lockPixels returns a LockRec struct (on success).
     */
    struct LockRec {
        void*           fPixels;
        SkColorTable*   fColorTable;
        size_t          fRowBytes;

        void zero() { sk_bzero(this, sizeof(*this)); }

        bool isZero() const {
            return NULL == fPixels && NULL == fColorTable && 0 == fRowBytes;
        }
    };

    /**
     *  Returns true if the lockcount > 0
     */
    bool isLocked() const { return fLockCount > 0; }

    SkDEBUGCODE(int getLockCount() const { return fLockCount; })

    /**
     *  Call to access the pixel memory. Return true on success. Balance this
     *  with a call to unlockPixels().
     */
    bool lockPixels();

    /**
     *  Call to access the pixel memory. On success, return true and fill out
     *  the specified rec. On failure, return false and ignore the rec parameter.
     *  Balance this with a call to unlockPixels().
     */
    bool lockPixels(LockRec* rec);

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

    /**
     *  Call this if you have changed the contents of the pixels. This will in-
     *  turn cause a different generation ID value to be returned from
     *  getGenerationID().
     */
    void notifyPixelsChanged();

    /**
     *  Change the info's AlphaType. Note that this does not automatically
     *  invalidate the generation ID. If the pixel values themselves have
     *  changed, then you must explicitly call notifyPixelsChanged() as well.
     */
    void changeAlphaType(SkAlphaType at);

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

    /**
     *  Experimental -- tells the caller if it is worth it to call decodeInto().
     *  Just an optimization at this point, to avoid checking the cache first.
     *  We may remove/change this call in the future.
     */
    bool implementsDecodeInto() {
        return this->onImplementsDecodeInto();
    }

    /**
     *  Return a decoded instance of this pixelRef in bitmap. If this cannot be
     *  done, return false and the bitmap parameter is ignored/unchanged.
     *
     *  pow2 is the requeste power-of-two downscale that the caller needs. This
     *  can be ignored, and the "original" size can be returned, but if the
     *  underlying codec can efficiently return a smaller size, that should be
     *  done. Some examples:
     *
     *  To request the "base" version (original scale), pass 0 for pow2
     *  To request 1/2 scale version (1/2 width, 1/2 height), pass 1 for pow2
     *  To request 1/4 scale version (1/4 width, 1/4 height), pass 2 for pow2
     *  ...
     *
     *  If this returns true, then bitmap must be "locked" such that
     *  bitmap->getPixels() will return the correct address.
     */
    bool decodeInto(int pow2, SkBitmap* bitmap) {
        SkASSERT(pow2 >= 0);
        return this->onDecodeInto(pow2, bitmap);
    }

    /** Are we really wrapping a texture instead of a bitmap?
     */
    virtual GrTexture* getTexture() { return NULL; }

    /**
     *  If any planes or rowBytes is NULL, this should output the sizes and return true
     *  if it can efficiently return YUV planar data. If it cannot, it should return false.
     *
     *  If all planes and rowBytes are not NULL, then it should copy the associated Y,U,V data
     *  into those planes of memory supplied by the caller. It should validate that the sizes
     *  match what it expected. If the sizes do not match, it should return false.
     */
    bool getYUV8Planes(SkISize sizes[3], void* planes[3], size_t rowBytes[3]) {
        return this->onGetYUV8Planes(sizes, planes, rowBytes);
    }

    bool readPixels(SkBitmap* dst, const SkIRect* subset = NULL);

    /**
     *  Makes a deep copy of this PixelRef, respecting the requested config.
     *  @param colorType Desired colortype.
     *  @param subset Subset of this PixelRef to copy. Must be fully contained within the bounds of
     *         of this PixelRef.
     *  @return A new SkPixelRef, or NULL if either there is an error (e.g. the destination could
     *          not be created with the given config), or this PixelRef does not support deep
     *          copies.
     */
    virtual SkPixelRef* deepCopy(SkColorType colortype, const SkIRect* subset) {
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

protected:
    /**
     *  On success, returns true and fills out the LockRec for the pixels. On
     *  failure returns false and ignores the LockRec parameter.
     *
     *  The caller will have already acquired a mutex for thread safety, so this
     *  method need not do that.
     */
    virtual bool onNewLockPixels(LockRec*) = 0;

    /**
     *  Balancing the previous successful call to onNewLockPixels. The locked
     *  pixel address will no longer be referenced, so the subclass is free to
     *  move or discard that memory.
     *
     *  The caller will have already acquired a mutex for thread safety, so this
     *  method need not do that.
     */
    virtual void onUnlockPixels() = 0;

    /** Default impl returns true */
    virtual bool onLockPixelsAreWritable() const;

    // returns false;
    virtual bool onImplementsDecodeInto();
    // returns false;
    virtual bool onDecodeInto(int pow2, SkBitmap* bitmap);

    /**
     *  For pixelrefs that don't have access to their raw pixels, they may be
     *  able to make a copy of them (e.g. if the pixels are on the GPU).
     *
     *  The base class implementation returns false;
     */
    virtual bool onReadPixels(SkBitmap* dst, const SkIRect* subsetOrNull);

    // default impl returns NULL.
    virtual SkData* onRefEncodedData();

    // default impl returns false.
    virtual bool onGetYUV8Planes(SkISize sizes[3], void* planes[3], size_t rowBytes[3]);

    /**
     *  Returns the size (in bytes) of the internally allocated memory.
     *  This should be implemented in all serializable SkPixelRef derived classes.
     *  SkBitmap::fPixelRefOffset + SkBitmap::getSafeSize() should never overflow this value,
     *  otherwise the rendering code may attempt to read memory out of bounds.
     *
     *  @return default impl returns 0.
     */
    virtual size_t getAllocatedSizeInBytes() const;

    /** Return the mutex associated with this pixelref. This value is assigned
        in the constructor, and cannot change during the lifetime of the object.
    */
    SkBaseMutex* mutex() const { return fMutex; }

    // only call from constructor. Flags this to always be locked, removing
    // the need to grab the mutex and call onLockPixels/onUnlockPixels.
    // Performance tweak to avoid those calls (esp. in multi-thread use case).
    void setPreLocked(void*, size_t rowBytes, SkColorTable*);

private:
    SkBaseMutex*    fMutex; // must remain in scope for the life of this object

    // mostly const. fInfo.fAlpahType can be changed at runtime.
    const SkImageInfo fInfo;

    // LockRec is only valid if we're in a locked state (isLocked())
    LockRec         fRec;
    int             fLockCount;

    mutable SkTRacy<uint32_t> fGenerationID;
    mutable SkTRacy<bool>     fUniqueGenerationID;

    SkTDArray<GenIDChangeListener*> fGenIDChangeListeners;  // pointers are owned

    SkString    fURI;

    // can go from false to true, but never from true to false
    bool    fIsImmutable;
    // only ever set in constructor, const after that
    bool    fPreLocked;

    void needsNewGenID();
    void callGenIDChangeListeners();

    void setMutex(SkBaseMutex* mutex);

    // When copying a bitmap to another with the same shape and config, we can safely
    // clone the pixelref generation ID too, which makes them equivalent under caching.
    friend class SkBitmap;  // only for cloneGenID
    void cloneGenID(const SkPixelRef&);

    typedef SkRefCnt INHERITED;
};

class SkPixelRefFactory : public SkRefCnt {
public:
    /**
     *  Allocate a new pixelref matching the specified ImageInfo, allocating
     *  the memory for the pixels. If the ImageInfo requires a ColorTable,
     *  the pixelref will ref() the colortable.
     *  On failure return NULL.
     */
    virtual SkPixelRef* create(const SkImageInfo&, size_t rowBytes, SkColorTable*) = 0;
};

#endif
