/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLazyCachingPixelRef_DEFINED
#define SkLazyCachingPixelRef_DEFINED

#include "SkBitmapFactory.h"
#include "SkCachingPixelRef.h"

class SkData;

/**
 *  PixelRef which defers decoding until SkBitmap::lockPixels() is
 *  called.  Makes use of a supplied decode procedure.  Will decode at
 *  the procedure's preferred size.
 */
class SkLazyCachingPixelRef : public SkCachingPixelRef {
public:
    /**
     *  @param data Encoded data representing the pixels.  NULL is
     *         equivalent to an empty data, and will be passed to
     *         DecodeProc with length zero.
     *
     *  @param procedure Called to decode the pixels when
     *         needed. If NULL, use SkImageDecoder::DecodeMemoryToTarget.
     */
    SkLazyCachingPixelRef(SkData* data,
                          SkBitmapFactory::DecodeProc procedure);

    virtual ~SkLazyCachingPixelRef();

    virtual SkData* onRefEncodedData() SK_OVERRIDE { return SkSafeRef(fData); }

    /**
     *  A simplified version of SkBitmapFactory.  Installs a new
     *  SkLazyCachingPixelRef into the provided bitmap.  Will
     *  immediately call onDecodeInfo() to configure the bitmap, but
     *  will defer decoding until the first time the bitmap's pixels
     *  are locked.
     *
     *  @param data Encoded data representing the pixels.  NULL is
     *         equivalent to an empty data, and will be passed to
     *         DecodeProc with length zero.
     *
     *  @param procedure Called to decode the pixels when
     *         needed. If NULL, use SkImageDecoder::DecodeMemoryToTarget.
     *
     *  @param destination Bitmap that will be modified on success.
     *
     *  @returns true on success.
     */
    static bool Install(SkBitmapFactory::DecodeProc procedure,
                        SkData* data,
                        SkBitmap* destination);

    // No need to flatten this object. When flattening an SkBitmap,
    // SkOrderedWriteBuffer will check the encoded data and write that
    // instead.
    // Future implementations of SkFlattenableWriteBuffer will need to
    // special case for onRefEncodedData as well.
    SK_DECLARE_UNFLATTENABLE_OBJECT()

protected:
    /**
     *  Return some information about the pixels, allowing this class
     *  to allocate pixels.  @return false if anything goes wrong.
     *
     *  This implementation calls SkBitmapFactory::DecodeProc with a
     *  NULL target.
     */
    virtual bool onDecodeInfo(SkImageInfo* info) SK_OVERRIDE;
    /**
     *  Decode into the given pixels, a block of memory of size
     *  (info.fHeight * rowBytes) bytes.
     *
     *  @param info Should be identical to the info returned by
     *         onDecodeInfo so that the implementation can confirm
     *         that the caller knows what its asking for (config,
     *         size).
     *
     *  @return false if anything goes wrong.
     */
    virtual bool onDecodePixels(const SkImageInfo& info,
                                void* pixels,
                                size_t rowBytes) SK_OVERRIDE;

private:
    SkData*                     fData;
    SkBitmapFactory::DecodeProc fDecodeProc;

    typedef SkCachingPixelRef INHERITED;
};

#endif  // SkLazyCachingPixelRef_DEFINED

