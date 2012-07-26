
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
    /** Allocate the specified buffer for pixels. The memory is freed when the
        last owner of this pixelref is gone. If addr is NULL, sk_malloc_throw()
        is called to allocate it.
     */
    SkMallocPixelRef(void* addr, size_t size, SkColorTable* ctable);
    virtual ~SkMallocPixelRef();
    
    //! Return the allocation size for the pixels
    size_t getSize() const { return fSize; }
    void* getAddr() const { return fStorage; }

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkMallocPixelRef)

protected:
    // overrides from SkPixelRef
    virtual void* onLockPixels(SkColorTable**);
    virtual void onUnlockPixels();

    SkMallocPixelRef(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

private:
    void*           fStorage;
    size_t          fSize;
    SkColorTable*   fCTable;

    typedef SkPixelRef INHERITED;
};


#endif
