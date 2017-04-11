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
class SK_API SkMallocPixelRef : public SkPixelRef {
public:
    /**
     *  Return a new SkMallocPixelRef with the provided pixel storage, rowBytes,
     *  and optional colortable. The caller is responsible for managing the
     *  lifetime of the pixel storage buffer, as this pixelref will not try
     *  to delete it.
     *
     *  Returns NULL on failure.
     */
    static sk_sp<SkPixelRef> MakeDirect(const SkImageInfo&, void* addr,
                                       size_t rowBytes, sk_sp<SkColorTable>);

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
    static sk_sp<SkPixelRef> MakeAllocate(const SkImageInfo&, size_t rowBytes, sk_sp<SkColorTable>);

    /**
     *  Identical to MakeAllocate, except all pixel bytes are zeroed.
     */
    static sk_sp<SkPixelRef> MakeZeroed(const SkImageInfo&, size_t rowBytes, sk_sp<SkColorTable>);

    /**
     *  Return a new SkMallocPixelRef with the provided pixel storage,
     *  rowBytes, and optional colortable. On destruction, ReleaseProc
     *  will be called.
     *
     *  If ReleaseProc is NULL, the pixels will never be released. This
     *  can be useful if the pixels were stack allocated. However, such an
     *  SkMallocPixelRef must not live beyond its pixels (e.g. by copying
     *  an SkBitmap pointing to it, or drawing to an SkPicture).
     *
     *  Returns NULL on failure.
     */
    typedef void (*ReleaseProc)(void* addr, void* context);
    static sk_sp<SkPixelRef> MakeWithProc(const SkImageInfo& info,
                                          size_t rowBytes, sk_sp<SkColorTable>,
                                          void* addr, ReleaseProc proc,
                                          void* context);

    /**
     *  Return a new SkMallocPixelRef that will use the provided
     *  SkData, rowBytes, and optional colortable as pixel storage.
     *  The SkData will be ref()ed and on destruction of the PielRef,
     *  the SkData will be unref()ed.
     *
     *  Returns NULL on failure.
     */
    static sk_sp<SkPixelRef> MakeWithData(const SkImageInfo& info,
                                          size_t rowBytes,
                                          sk_sp<SkColorTable>,
                                          sk_sp<SkData> data);
    
#ifdef SK_SUPPORT_LEGACY_PIXELREFFACTORY
    static SkMallocPixelRef* NewDirect(const SkImageInfo& info, void* addr,
                                       size_t rowBytes, SkColorTable* ctable) {
        return (SkMallocPixelRef*)MakeDirect(info, addr, rowBytes, sk_ref_sp(ctable)).release();
    }
    static SkMallocPixelRef* NewAllocate(const SkImageInfo& info, size_t rb, SkColorTable* ct) {
        return (SkMallocPixelRef*)MakeAllocate(info, rb, sk_ref_sp(ct)).release();
    }
    static SkMallocPixelRef* NewZeroed(const SkImageInfo& info, size_t rowBytes, SkColorTable* ct) {
        return (SkMallocPixelRef*)MakeZeroed(info, rowBytes, sk_ref_sp(ct)).release();
    }
    static SkMallocPixelRef* NewWithProc(const SkImageInfo& info,
                                               size_t rowBytes, SkColorTable* ctable,
                                               void* addr, ReleaseProc proc,
                                               void* ctx) {
        return (SkMallocPixelRef*)MakeWithProc(info, rowBytes, sk_ref_sp(ctable), addr, proc, ctx).release();
    }
    static SkMallocPixelRef* NewWithData(const SkImageInfo& info,
                                         size_t rowBytes,
                                         SkColorTable* ctable,
                                         SkData* data);
#endif

protected:
    ~SkMallocPixelRef() override;

#ifdef SK_SUPPORT_LEGACY_NO_ADDR_PIXELREF
    bool onNewLockPixels(LockRec*) override;
    void onUnlockPixels() override;
#endif
    size_t getAllocatedSizeInBytes() const override;

private:
    // Uses alloc to implement NewAllocate or NewZeroed.
    static sk_sp<SkPixelRef> MakeUsing(void*(*alloc)(size_t),
                                       const SkImageInfo&,
                                       size_t rowBytes,
                                       sk_sp<SkColorTable>);

    ReleaseProc fReleaseProc;
    void*       fReleaseProcContext;

    SkMallocPixelRef(const SkImageInfo&, void* addr, size_t rb, sk_sp<SkColorTable>,
                     ReleaseProc proc, void* context);

    typedef SkPixelRef INHERITED;
};


#endif
