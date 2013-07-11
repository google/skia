/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapFactory_DEFINED
#define SkBitmapFactory_DEFINED

#include "SkImage.h"
#include "SkTypes.h"

class SkBitmap;
class SkData;
class SkImageCache;

/**
 *  Factory for creating a bitmap from encoded data.
 */
class SkBitmapFactory {

public:
    /**
     *  Struct containing information about a pixel destination.
     */
    struct Target {
        /**
         *  Pre-allocated memory.
         */
        void*  fAddr;

        /**
         *  Rowbytes of the allocated memory.
         */
        size_t fRowBytes;
    };

    /**
     *  Signature for a function to decode an image from encoded data.
     */
    typedef bool (*DecodeProc)(const void* data, size_t length, SkImage::Info*, const Target*);

    /**
     *  Create a bitmap factory which uses DecodeProc for decoding.
     *  @param DecodeProc Must not be NULL.
     */
    SkBitmapFactory(DecodeProc);

    ~SkBitmapFactory();

    /**
     *  Set an image cache to use on pixelrefs provided by installPixelRef. Mutually exclusive
     *  with fCacheSelector.
     */
    void setImageCache(SkImageCache* cache);

    /**
     *  Sets up an SkBitmap from encoded data. On success, the SkBitmap will have its Config,
     *  width, height, rowBytes and pixelref set. If fImageCache is non-NULL, or if fCacheSelector
     *  is set and returns non-NULL, the pixelref will lazily decode, and that SkImageCache will
     *  handle the pixel memory. Otherwise installPixelRef will do an immediate decode.
     *  @param SkData Encoded data.
     *  @param SkBitmap to install the pixel ref on.
     *  @return bool Whether or not a pixel ref was successfully installed.
     */
    bool installPixelRef(SkData*, SkBitmap*);

    /**
     *  An object for selecting an SkImageCache to use based on an SkImage::Info.
     */
    class CacheSelector : public SkRefCnt {

    public:
        SK_DECLARE_INST_COUNT(CacheSelector)
        /**
         *  Return an SkImageCache to use based on the provided SkImage::Info. If the caller decides
         *  to hang on to the result, it will call ref, so the implementation should not add a ref
         *  as a result of this call.
         */
        virtual SkImageCache* selectCache(const SkImage::Info&) = 0;

    private:
        typedef SkRefCnt INHERITED;
    };

    /**
     *  Set the function to be used to select which SkImageCache to use. Mutually exclusive with
     *  fImageCache.
     */
    void setCacheSelector(CacheSelector*);

private:
    DecodeProc     fDecodeProc;
    SkImageCache*  fImageCache;
    CacheSelector* fCacheSelector;
};

#endif // SkBitmapFactory_DEFINED
