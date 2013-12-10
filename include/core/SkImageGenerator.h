/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageGenerator_DEFINED
#define SkImageGenerator_DEFINED

#include "SkDiscardableMemory.h"
#include "SkImageInfo.h"

class SkBitmap;
class SkData;
class SkImageGenerator;

/**
 *  Takes ownership of SkImageGenerator.  If this method fails for
 *  whatever reason, it will return false and immediatetely delete
 *  the generator.  If it succeeds, it will modify destination
 *  bitmap.
 *
 *  If this fails or when the SkDiscardablePixelRef that is
 *  installed into destination is destroyed, it will call
 *  SkDELETE() on the generator.  Therefore, generator should be
 *  allocated with SkNEW() or SkNEW_ARGS().
 *
 *  @param destination Upon success, this bitmap will be
 *  configured and have a pixelref installed.
 *
 *  @param factory If not NULL, this object will be used as a
 *  source of discardable memory when decoding.  If NULL, then
 *  SkDiscardableMemory::Create() will be called.
 *
 *  @return true iff successful.
 */
SK_API bool SkInstallDiscardablePixelRef(SkImageGenerator* generator,
                                         SkBitmap* destination,
                                         SkDiscardableMemory::Factory* factory = NULL);


/**
 *  An interface that allows a purgeable PixelRef (such as a
 *  SkDiscardablePixelRef) to decode and re-decode an image as needed.
 */
class SK_API SkImageGenerator {
public:
    /**
     *  The PixelRef which takes ownership of this SkImageGenerator
     *  will call the image generator's destructor.
     */
    virtual ~SkImageGenerator() { }

    /**
     *  Return a ref to the encoded (i.e. compressed) representation,
     *  of this data.
     *
     *  If non-NULL is returned, the caller is responsible for calling
     *  unref() on the data when it is finished.
     */
    virtual SkData* refEncodedData() { return NULL; }

    /**
     *  Return some information about the image, allowing the owner of
     *  this object to allocate pixels.
     *
     *  Repeated calls to this function should give the same results,
     *  allowing the PixelRef to be immutable.
     *
     *  @return false if anything goes wrong.
     */
    virtual bool getInfo(SkImageInfo* info) = 0;

    /**
     *  Decode into the given pixels, a block of memory of size at
     *  least (info.fHeight - 1) * rowBytes + (info.fWidth *
     *  bytesPerPixel)
     *
     *  Repeated calls to this function should give the same results,
     *  allowing the PixelRef to be immutable.
     *
     *  @param info A description of the format (config, size)
     *         expected by the caller.  This can simply be identical
     *         to the info returned by getInfo().
     *
     *         This contract also allows the caller to specify
     *         different output-configs, which the implementation can
     *         decide to support or not.
     *
     *  @return false if anything goes wrong or if the image info is
     *          unsupported.
     */
    virtual bool getPixels(const SkImageInfo& info,
                           void* pixels,
                           size_t rowBytes) = 0;
};

#endif  // SkImageGenerator_DEFINED
