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
class SkScanlineDecoder;

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
            : fZeroInitialized(kNo_ZeroInitialized) {}

        ZeroInitialized fZeroInitialized;
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
     *  Return an object which can be used to decode individual scanlines.
     *
     *  This object is owned by the SkCodec, which will handle its lifetime. The
     *  returned object is only valid until the SkCodec is deleted or the next
     *  call to getScanlineDecoder, whichever comes first.
     *
     *  Calling a second time will rewind and replace the existing one with a
     *  new one. If the stream cannot be rewound, this will delete the existing
     *  one and return NULL.
     *
     *  @param dstInfo Info of the destination. If the dimensions do not match
     *      those of getInfo, this implies a scale.
     *  @param options Contains decoding options, including if memory is zero
     *      initialized.
     *  @param ctable A pointer to a color table.  When dstInfo.colorType() is
     *      kIndex8, this should be non-NULL and have enough storage for 256
     *      colors.  The color table will be populated after decoding the palette.
     *  @param ctableCount A pointer to the size of the color table.  When
     *      dstInfo.colorType() is kIndex8, this should be non-NULL.  It will
     *      be modified to the true size of the color table (<= 256) after
     *      decoding the palette.
     *  @return New SkScanlineDecoder, or NULL on failure.
     *
     *  NOTE: If any rows were previously decoded, this requires rewinding the
     *  SkStream.
     *
     *  NOTE: The scanline decoder is owned by the SkCodec and will delete it
     *  when the SkCodec is deleted.
     */
    SkScanlineDecoder* getScanlineDecoder(const SkImageInfo& dstInfo, const Options* options,
                                          SkPMColor ctable[], int* ctableCount);

    /**
     *  Simplified version of getScanlineDecoder() that asserts that info is NOT
     *  kIndex8_SkColorType and uses the default Options.
     */
    SkScanlineDecoder* getScanlineDecoder(const SkImageInfo& dstInfo);

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

    /**
     *  Override if your codec supports scanline decoding.
     *
     *  As in onGetPixels(), the implementation must call rewindIfNeeded() and
     *  handle as appropriate.
     *
     *  @param dstInfo Info of the destination. If the dimensions do not match
     *      those of getInfo, this implies a scale.
     *  @param options Contains decoding options, including if memory is zero
     *      initialized.
     *  @param ctable A pointer to a color table.  When dstInfo.colorType() is
     *      kIndex8, this should be non-NULL and have enough storage for 256
     *      colors.  The color table will be populated after decoding the palette.
     *  @param ctableCount A pointer to the size of the color table.  When
     *      dstInfo.colorType() is kIndex8, this should be non-NULL.  It will
     *      be modified to the true size of the color table (<= 256) after
     *      decoding the palette.
     *  @return New SkScanlineDecoder on success, NULL otherwise. The SkCodec
     *      will take ownership of the returned scanline decoder.
     */
    virtual SkScanlineDecoder* onGetScanlineDecoder(const SkImageInfo& dstInfo,
                                                    const Options& options,
                                                    SkPMColor ctable[],
                                                    int* ctableCount) {
        return NULL;
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

    /**
     * If the codec has a scanline decoder, return it (no ownership change occurs)
     * else return NULL.
     * The returned decoder is valid while the codec exists and the client has not
     * created a new scanline decoder.
     */
    SkScanlineDecoder* scanlineDecoder() {
        return fScanlineDecoder;
    }

    /**
     * Allow the codec subclass to detach and take ownership of the scanline decoder.
     * This will likely be used when the scanline decoder needs to be destroyed
     * in the destructor of the subclass.
     */
    SkScanlineDecoder* detachScanlineDecoder() {
        SkScanlineDecoder* scanlineDecoder = fScanlineDecoder;
        fScanlineDecoder = NULL;
        return scanlineDecoder;
    }

private:
    const SkImageInfo       fInfo;
    SkAutoTDelete<SkStream> fStream;
    bool                    fNeedsRewind;
    SkScanlineDecoder*      fScanlineDecoder;
};
#endif // SkCodec_DEFINED
