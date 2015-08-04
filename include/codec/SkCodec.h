/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCodec_DEFINED
#define SkCodec_DEFINED

#include "SkColor.h"
#include "SkEncodedFormat.h"
#include "SkImageInfo.h"
#include "SkSize.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkTypes.h"

class SkData;

/**
 *  Abstraction layer directly on top of an image codec.
 */
class SkCodec : SkNoncopyable {
public:
    /**
     *  If this stream represents an encoded image that we know how to decode,
     *  return an SkCodec that can decode it. Otherwise return NULL.
     *
     *  If NULL is returned, the stream is deleted immediately. Otherwise, the
     *  SkCodec takes ownership of it, and will delete it when done with it.
     */
    static SkCodec* NewFromStream(SkStream*);

    /**
     *  If this data represents an encoded image that we know how to decode,
     *  return an SkCodec that can decode it. Otherwise return NULL.
     *
     *  Will take a ref if it returns a codec, else will not affect the data.
     */
    static SkCodec* NewFromData(SkData*);

    virtual ~SkCodec();

    /**
     *  Return the ImageInfo associated with this codec.
     */
    const SkImageInfo& getInfo() const { return fInfo; }

    /**
     *  Return a size that approximately supports the desired scale factor.
     *  The codec may not be able to scale efficiently to the exact scale
     *  factor requested, so return a size that approximates that scale.
     */
    SkISize getScaledDimensions(float desiredScale) const {
        return this->onGetScaledDimensions(desiredScale);
    }

    /**
     *  Return (via desiredSubset) a subset which can decoded from this codec,
     *  or false if this codec cannot decode subsets or anything similar to
     *  desiredSubset.
     *
     *  @param desiredSubset In/out parameter. As input, a desired subset of
     *      the original bounds (as specified by getInfo). If true is returned,
     *      desiredSubset may have been modified to a subset which is
     *      supported. Although a particular change may have been made to
     *      desiredSubset to create something supported, it is possible other
     *      changes could result in a valid subset.
     *      If false is returned, desiredSubset's value is undefined.
     *  @return true if this codec supports decoding desiredSubset (as
     *      returned, potentially modified)
     */
    bool getValidSubset(SkIRect* desiredSubset) const {
        return this->onGetValidSubset(desiredSubset);
    }

    /**
     *  Format of the encoded data.
     */
    SkEncodedFormat getEncodedFormat() const { return this->onGetEncodedFormat(); }

    /**
     *  Used to describe the result of a call to getPixels().
     *
     *  Result is the union of possible results from subclasses.
     */
    enum Result {
        /**
         *  General return value for success.
         */
        kSuccess,
        /**
         *  The input is incomplete. A partial image was generated.
         */
        kIncompleteInput,
        /**
         *  The generator cannot convert to match the request, ignoring
         *  dimensions.
         */
        kInvalidConversion,
        /**
         *  The generator cannot scale to requested size.
         */
        kInvalidScale,
        /**
         *  Parameters (besides info) are invalid. e.g. NULL pixels, rowBytes
         *  too small, etc.
         */
        kInvalidParameters,
        /**
         *  The input did not contain a valid image.
         */
        kInvalidInput,
        /**
         *  Fulfilling this request requires rewinding the input, which is not
         *  supported for this input.
         */
        kCouldNotRewind,
        /**
         *  This method is not implemented by this generator.
         */
        kUnimplemented,
    };

    /**
     *  Whether or not the memory passed to getPixels is zero initialized.
     */
    enum ZeroInitialized {
        /**
         *  The memory passed to getPixels is zero initialized. The SkCodec
         *  may take advantage of this by skipping writing zeroes.
         */
        kYes_ZeroInitialized,
        /**
         *  The memory passed to getPixels has not been initialized to zero,
         *  so the SkCodec must write all zeroes to memory.
         *
         *  This is the default. It will be used if no Options struct is used.
         */
        kNo_ZeroInitialized,
    };

    /**
     *  Additional options to pass to getPixels.
     */
    struct Options {
        Options()
            : fZeroInitialized(kNo_ZeroInitialized)
            , fSubset(NULL)
        {}

        ZeroInitialized fZeroInitialized;
        /**
         *  If not NULL, represents a subset of the original image to decode.
         *
         *  Must be within the bounds returned by getInfo().
         *
         *  If the EncodedFormat is kWEBP_SkEncodedFormat (the only one which
         *  currently supports subsets), the top and left values must be even.
         */
        SkIRect*        fSubset;
    };

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
     *  @return Result kSuccess, or another value explaining the type of failure.
     */
    Result getPixels(const SkImageInfo& info, void* pixels, size_t rowBytes, const Options*,
                     SkPMColor ctable[], int* ctableCount);

    /**
     *  Simplified version of getPixels() that asserts that info is NOT kIndex8_SkColorType and
     *  uses the default Options.
     */
    Result getPixels(const SkImageInfo& info, void* pixels, size_t rowBytes);

    /**
     *  Some images may initially report that they have alpha due to the format
     *  of the encoded data, but then never use any colors which have alpha
     *  less than 100%. This function can be called *after* decoding to
     *  determine if such an image truly had alpha. Calling it before decoding
     *  is undefined.
     *  FIXME: see skbug.com/3582.
     */
    bool reallyHasAlpha() const {
        return this->onReallyHasAlpha();
    }

protected:
    SkCodec(const SkImageInfo&, SkStream*);

    virtual SkISize onGetScaledDimensions(float /* desiredScale */) const {
        // By default, scaling is not supported.
        return this->getInfo().dimensions();
    }

    virtual SkEncodedFormat onGetEncodedFormat() const = 0;

    virtual Result onGetPixels(const SkImageInfo& info,
                               void* pixels, size_t rowBytes, const Options&,
                               SkPMColor ctable[], int* ctableCount) = 0;

    virtual bool onGetValidSubset(SkIRect* /* desiredSubset */) const {
        // By default, subsets are not supported.
        return false;
    }

    virtual bool onReallyHasAlpha() const { return false; }

    enum RewindState {
        kRewound_RewindState,
        kNoRewindNecessary_RewindState,
        kCouldNotRewind_RewindState
    };
    /**
     *  If the stream was previously read, attempt to rewind.
     *  @returns:
     *      kRewound if the stream needed to be rewound, and the
     *               rewind succeeded.
     *      kNoRewindNecessary if the stream did not need to be
     *                         rewound.
     *      kCouldNotRewind if the stream needed to be rewound, and
     *                      rewind failed.
     *
     *  Subclasses MUST call this function before reading the stream (e.g. in
     *  onGetPixels). If it returns false, onGetPixels should return
     *  kCouldNotRewind.
     */
    RewindState SK_WARN_UNUSED_RESULT rewindIfNeeded();

    /**
     * Get method for the input stream
     */
    SkStream* stream() {
        return fStream.get();
    }

private:
    const SkImageInfo       fInfo;
    SkAutoTDelete<SkStream> fStream;
    bool                    fNeedsRewind;
};
#endif // SkCodec_DEFINED
