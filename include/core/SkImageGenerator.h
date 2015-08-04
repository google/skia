/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageGenerator_DEFINED
#define SkImageGenerator_DEFINED

#include "SkColor.h"
#include "SkImageInfo.h"

class SkBitmap;
class SkData;
class SkImageGenerator;
class SkMatrix;
class SkPaint;
class SkPicture;

/**
 *  Takes ownership of SkImageGenerator.  If this method fails for
 *  whatever reason, it will return false and immediatetely delete
 *  the generator.  If it succeeds, it will modify destination
 *  bitmap.
 *
 *  If generator is NULL, will safely return false.
 *
 *  If this fails or when the SkDiscardablePixelRef that is
 *  installed into destination is destroyed, it will call
 *  SkDELETE() on the generator.  Therefore, generator should be
 *  allocated with SkNEW() or SkNEW_ARGS().
 *
 *  @param destination Upon success, this bitmap will be
 *  configured and have a pixelref installed.
 *
 *  @return true iff successful.
 */
SK_API bool SkInstallDiscardablePixelRef(SkImageGenerator*, SkBitmap* destination);

/**
 *  On success, installs a discardable pixelref into destination, based on encoded data.
 *  Regardless of success or failure, the caller must still balance their ownership of encoded.
 */
SK_API bool SkInstallDiscardablePixelRef(SkData* encoded, SkBitmap* destination);

/**
 *  An interface that allows a purgeable PixelRef (such as a
 *  SkDiscardablePixelRef) to decode and re-decode an image as needed.
 */
class SK_API SkImageGenerator : public SkNoncopyable {
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
    SkData* refEncodedData() { return this->onRefEncodedData(); }

    /**
     *  Return the ImageInfo associated with this generator.
     */
    const SkImageInfo& getInfo() const { return fInfo; }

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
     *         A size that does not match getInfo() implies a request
     *         to scale. If the generator cannot perform this scale,
     *         it will return kInvalidScale.
     *
     *  If info is kIndex8_SkColorType, then the caller must provide storage for up to 256
     *  SkPMColor values in ctable. On success the generator must copy N colors into that storage,
     *  (where N is the logical number of table entries) and set ctableCount to N.
     *
     *  If info is not kIndex8_SkColorType, then the last two parameters may be NULL. If ctableCount
     *  is not null, it will be set to 0.
     *
     *  @return true on success.
     */
    bool getPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                   SkPMColor ctable[], int* ctableCount);

    /**
     *  Simplified version of getPixels() that asserts that info is NOT kIndex8_SkColorType and
     *  uses the default Options.
     */
    bool getPixels(const SkImageInfo& info, void* pixels, size_t rowBytes);

    /**
     *  If planes or rowBytes is NULL or if any entry in planes is NULL or if any entry in rowBytes
     *  is 0, this imagegenerator should output the sizes and return true if it can efficiently
     *  return YUV planar data. If it cannot, it should return false. Note that either planes and
     *  rowBytes are both fully defined and non NULL/non 0 or they are both NULL or have NULL or 0
     *  entries only. Having only partial planes/rowBytes information is not supported.
     *
     *  If all planes and rowBytes entries are non NULL or non 0, then it should copy the
     *  associated YUV data into those planes of memory supplied by the caller. It should validate
     *  that the sizes match what it expected. If the sizes do not match, it should return false.
     */
    bool getYUV8Planes(SkISize sizes[3], void* planes[3], size_t rowBytes[3],
                       SkYUVColorSpace* colorSpace);

    /**
     *  If the default image decoder system can interpret the specified (encoded) data, then
     *  this returns a new ImageGenerator for it. Otherwise this returns NULL. Either way
     *  the caller is still responsible for managing their ownership of the data.
     */
    static SkImageGenerator* NewFromEncoded(SkData*);

    /** Return a new image generator backed by the specified picture.  If the size is empty or
     *  the picture is NULL, this returns NULL.
     *  The optional matrix and paint arguments are passed to drawPicture() at rasterization
     *  time.
     */
    static SkImageGenerator* NewFromPicture(const SkISize&, const SkPicture*, const SkMatrix*,
                                            const SkPaint*);

protected:
    SkImageGenerator(const SkImageInfo& info) : fInfo(info) {}

    virtual SkData* onRefEncodedData();

    virtual bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                             SkPMColor ctable[], int* ctableCount);
    virtual bool onGetYUV8Planes(SkISize sizes[3], void* planes[3], size_t rowBytes[3]);
    virtual bool onGetYUV8Planes(SkISize sizes[3], void* planes[3], size_t rowBytes[3],
                                 SkYUVColorSpace* colorSpace);

private:
    const SkImageInfo fInfo;

    // This is our default impl, which may be different on different platforms.
    // It is called from NewFromEncoded() after it has checked for any runtime factory.
    // The SkData will never be NULL, as that will have been checked by NewFromEncoded.
    static SkImageGenerator* NewFromEncodedImpl(SkData*);
};

#endif  // SkImageGenerator_DEFINED
