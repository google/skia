
/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkMallocPixelRef_DEFINED
#define SkMallocPixelRef_DEFINED

#include "SkPixelRef.h"

/** We explicitly use the same allocator for our pixels that SkMask does,
    so that we can freely assign memory allocated by one class to the other.
*/
class SkMallocPixelRef : public SkPixelRef {
public:
    /**
     *  Return a new SkMallocPixelRef with the provided pixel storage, rowBytes,
     *  and optional colortable. The caller is responsible for managing the
     *  lifetime of the pixel storage buffer, as the pixelref will not try
     *  to delete the storage.
     *
     *  This pixelref will ref() the specified colortable (if not NULL).
     *
     *  Returns NULL on failure.
     */
    static SkMallocPixelRef* NewDirect(const SkImageInfo&, void* addr,
                                       size_t rowBytes, SkColorTable*);

    /**
     *  Return a new SkMallocPixelRef, automatically allocating storage for the
     *  pixels. If rowBytes are 0, an optimal value will be chosen automatically.
     *  If rowBytes is > 0, then it will be respected, or NULL will be returned
     *  if rowBytes is invalid for the specified info.
     *
     *  This pixelref will ref() the specified colortable (if not NULL).
     *
     *  Returns NULL on failure.
     */
    static SkMallocPixelRef* NewAllocate(const SkImageInfo& info,
                                         size_t rowBytes, SkColorTable*);

    void* getAddr() const { return fStorage; }

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkMallocPixelRef)

protected:
    virtual bool onNewLockPixels(LockRec*) SK_OVERRIDE;
    virtual void onUnlockPixels() SK_OVERRIDE;
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;
    virtual size_t getAllocatedSizeInBytes() const SK_OVERRIDE;

    SkMallocPixelRef(SkFlattenableReadBuffer& buffer);
    SkMallocPixelRef(const SkImageInfo&, void* addr, size_t rb, SkColorTable*,
                     bool ownsPixels);
    virtual ~SkMallocPixelRef();

private:
    void*               fStorage;
    SkColorTable*       fCTable;
    size_t              fRB;
    const bool          fOwnPixels;

    typedef SkPixelRef INHERITED;
};


#endif
