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
#include "SkYUVSizeInfo.h"

class SkColorTable;
class SkData;
struct SkIRect;

class GrTexture;
class SkDiscardableMemory;

/** \class SkPixelRef

    This class is the smart container for pixel memory, and is used with
    SkBitmap. A pixelref is installed into a bitmap, and then the bitmap can
    access the actual pixel memory by calling lockPixels/unlockPixels.

    This class can be shared/accessed between multiple threads.
*/
class SK_API SkPixelRef : public SkRefCnt {
public:
    explicit SkPixelRef(const SkImageInfo&);
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
        LockRec() : fPixels(NULL), fColorTable(NULL) {}

        void*           fPixels;
        SkColorTable*   fColorTable;
        size_t          fRowBytes;

        void zero() { sk_bzero(this, sizeof(*this)); }

        bool isZero() const {
            return NULL == fPixels && NULL == fColorTable && 0 == fRowBytes;
        }
    };

    SkDEBUGCODE(bool isLocked() const { return fLockCount > 0; })
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

    /**
     *  Change the info's AlphaType. Note that this does not automatically
     *  invalidate the generation ID. If the pixel values themselves have
     *  changed, then you must explicitly call notifyPixelsChanged() as well.
     */
    void changeAlphaType(SkAlphaType at);

    /** Returns true if this pixelref is marked as immutable, meaning that the
        contents of its pixels will not change for the lifetime of the pixelref.
    */
    bool isImmutable() const { return fMutability != kMutable; }

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

    struct LockRequest {
        SkISize         fSize;
        SkFilterQuality fQuality;
    };

    struct LockResult {
        LockResult() : fPixels(NULL), fCTable(NULL) {}

        void        (*fUnlockProc)(void* ctx);
        void*       fUnlockContext;

        const void* fPixels;
        SkColorTable* fCTable;  // should be NULL unless colortype is kIndex8
        size_t      fRowBytes;
        SkISize     fSize;

        void unlock() {
            if (fUnlockProc) {
                fUnlockProc(fUnlockContext);
                fUnlockProc = NULL; // can't unlock twice!
            }
        }
    };

    bool requestLock(const LockRequest&, LockResult*);

    /** Are we really wrapping a texture instead of a bitmap?
     */
    virtual GrTexture* getTexture() { return NULL; }

    /**
     *  If this can efficiently return YUV data, this should return true.
     *  Otherwise this returns false and does not modify any of the parameters.
     *
     *  @param sizeInfo   Output parameter indicating the sizes and required
     *                    allocation widths of the Y, U, and V planes.
     *  @param colorSpace Output parameter.
     */
    bool queryYUV8(SkYUVSizeInfo* sizeInfo, SkYUVColorSpace* colorSpace) const {
        return this->onQueryYUV8(sizeInfo, colorSpace);
    }

    /**
     *  Returns true on success and false on failure.
     *  Copies YUV data into the provided YUV planes.
     *
     *  @param sizeInfo   Needs to exactly match the values returned by the
     *                    query, except the WidthBytes may be larger than the
     *                    recommendation (but not smaller).
     *  @param planes     Memory for each of the Y, U, and V planes.
     */
    bool getYUV8Planes(const SkYUVSizeInfo& sizeInfo, void* planes[3]) {
        return this->onGetYUV8Planes(sizeInfo, planes);
    }

    /** Populates dst with the pixels of this pixelRef, converting them to colorType. */
    bool readPixels(SkBitmap* dst, SkColorType colorType, const SkIRect* subset = NULL);

    /**
     *  Makes a deep copy of this PixelRef, respecting the requested config.
     *  @param colorType Desired colortype.
     *  @param profileType Desired colorprofiletype.
     *  @param subset Subset of this PixelRef to copy. Must be fully contained within the bounds of
     *         of this PixelRef.
     *  @return A new SkPixelRef, or NULL if either there is an error (e.g. the destination could
     *          not be created with the given config), or this PixelRef does not support deep
     *          copies.
     */
    virtual SkPixelRef* deepCopy(SkColorType, SkColorProfileType, const SkIRect* /*subset*/) {
        return NULL;
    }

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

    /**
     *  Returns true if the pixels are generated on-the-fly (when required).
     */
    bool isLazyGenerated() const { return this->onIsLazyGenerated(); }

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

    /**
     *  For pixelrefs that don't have access to their raw pixels, they may be
     *  able to make a copy of them (e.g. if the pixels are on the GPU).
     *
     *  The base class implementation returns false;
     */
    virtual bool onReadPixels(SkBitmap* dst, SkColorType colorType, const SkIRect* subsetOrNull);

    // default impl returns NULL.
    virtual SkData* onRefEncodedData();

    // default impl does nothing.
    virtual void onNotifyPixelsChanged();

    virtual bool onQueryYUV8(SkYUVSizeInfo*, SkYUVColorSpace*) const {
        return false;
    }
    virtual bool onGetYUV8Planes(const SkYUVSizeInfo&, void*[3] /*planes*/) {
        return false;
    }

    /**
     *  Returns the size (in bytes) of the internally allocated memory.
     *  This should be implemented in all serializable SkPixelRef derived classes.
     *  SkBitmap::fPixelRefOffset + SkBitmap::getSafeSize() should never overflow this value,
     *  otherwise the rendering code may attempt to read memory out of bounds.
     *
     *  @return default impl returns 0.
     */
    virtual size_t getAllocatedSizeInBytes() const;

    virtual bool onRequestLock(const LockRequest&, LockResult*);

    virtual bool onIsLazyGenerated() const { return false; }

    /** Return the mutex associated with this pixelref. This value is assigned
        in the constructor, and cannot change during the lifetime of the object.
    */
    SkBaseMutex* mutex() const { return &fMutex; }

    // only call from constructor. Flags this to always be locked, removing
    // the need to grab the mutex and call onLockPixels/onUnlockPixels.
    // Performance tweak to avoid those calls (esp. in multi-thread use case).
    void setPreLocked(void*, size_t rowBytes, SkColorTable*);

private:
    mutable SkMutex fMutex;

    // mostly const. fInfo.fAlpahType can be changed at runtime.
    const SkImageInfo fInfo;

    // LockRec is only valid if we're in a locked state (isLocked())
    LockRec         fRec;
    int             fLockCount;

    bool lockPixelsInsideMutex();

    // Bottom bit indicates the Gen ID is unique.
    bool genIDIsUnique() const { return SkToBool(fTaggedGenID.load() & 1); }
    mutable SkAtomic<uint32_t> fTaggedGenID;

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    const uint32_t fStableID;
#endif

    SkTDArray<GenIDChangeListener*> fGenIDChangeListeners;  // pointers are owned

    SkString    fURI;

    // Set true by caches when they cache content that's derived from the current pixels.
    SkAtomic<bool> fAddedToCache;

    enum {
        kMutable,               // PixelRefs begin mutable.
        kTemporarilyImmutable,  // Considered immutable, but can revert to mutable.
        kImmutable,             // Once set to this state, it never leaves.
    } fMutability : 8;          // easily fits inside a byte

    // only ever set in constructor, const after that
    bool fPreLocked;

    void needsNewGenID();
    void callGenIDChangeListeners();

    void setTemporarilyImmutable();
    void restoreMutability();
    friend class SkSurface_Raster;   // For the two methods above.

    bool isPreLocked() const { return fPreLocked; }
    friend class SkImage_Raster;
    friend class SkSpecialImage_Raster;

    // When copying a bitmap to another with the same shape and config, we can safely
    // clone the pixelref generation ID too, which makes them equivalent under caching.
    friend class SkBitmap;  // only for cloneGenID
    void cloneGenID(const SkPixelRef&);

    void setImmutableWithID(uint32_t genID);
    friend class SkImage_Gpu;
    friend class SkImageCacherator;

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
