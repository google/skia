/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAndroidCodec_DEFINED
#define SkAndroidCodec_DEFINED

#include "SkCodec.h"
#include "SkEncodedFormat.h"
#include "SkStream.h"
#include "SkTypes.h"

/**
 *  Abstract interface defining image codec functionality that is necessary for
 *  Android.
 */
class SkAndroidCodec : SkNoncopyable {
public:
    /**
     *  If this stream represents an encoded image that we know how to decode,
     *  return an SkAndroidCodec that can decode it. Otherwise return NULL.
     *
     *  The SkPngChunkReader handles unknown chunks in PNGs.
     *  See SkCodec.h for more details.
     *
     *  If NULL is returned, the stream is deleted immediately. Otherwise, the
     *  SkCodec takes ownership of it, and will delete it when done with it.
     */
    static SkAndroidCodec* NewFromStream(SkStream*, SkPngChunkReader* = NULL);

    /**
     *  If this data represents an encoded image that we know how to decode,
     *  return an SkAndroidCodec that can decode it. Otherwise return NULL.
     *
     *  The SkPngChunkReader handles unknown chunks in PNGs.
     *  See SkCodec.h for more details.
     *
     *  Will take a ref if it returns a codec, else will not affect the data.
     */
    static SkAndroidCodec* NewFromData(SkData*, SkPngChunkReader* = NULL);

    virtual ~SkAndroidCodec() {}


    const SkImageInfo& getInfo() const { return fInfo; }

    /**
     *  Format of the encoded data.
     */
    SkEncodedFormat getEncodedFormat() const { return fCodec->getEncodedFormat(); }

    /**
     *  @param requestedColorType Color type requested by the client
     *
     *  If it is possible to decode to requestedColorType, this returns
     *  requestedColorType.  Otherwise, this returns whichever color type
     *  is suggested by the codec as the best match for the encoded data.
     */
    SkColorType computeOutputColorType(SkColorType requestedColorType);

    /**
     *  @param requestedUnpremul  Indicates if the client requested
     *                            unpremultiplied output
     *
     *  Returns the appropriate alpha type to decode to.  If the image
     *  has alpha, the value of requestedUnpremul will be honored.
     */
    SkAlphaType computeOutputAlphaType(bool requestedUnpremul);

    /**
     *  Returns the dimensions of the scaled output image, for an input
     *  sampleSize.
     *
     *  When the sample size divides evenly into the original dimensions, the
     *  scaled output dimensions will simply be equal to the original
     *  dimensions divided by the sample size.
     *
     *  When the sample size does not divide even into the original
     *  dimensions, the codec may round up or down, depending on what is most
     *  efficient to decode.
     *
     *  Finally, the codec will always recommend a non-zero output, so the output
     *  dimension will always be one if the sampleSize is greater than the
     *  original dimension.
     */
    SkISize getSampledDimensions(int sampleSize) const;

    /**
     *  Return (via desiredSubset) a subset which can decoded from this codec,
     *  or false if the input subset is invalid.
     *
     *  @param desiredSubset in/out parameter
     *                       As input, a desired subset of the original bounds
     *                       (as specified by getInfo).
     *                       As output, if true is returned, desiredSubset may
     *                       have been modified to a subset which is
     *                       supported. Although a particular change may have
     *                       been made to desiredSubset to create something
     *                       supported, it is possible other changes could
     *                       result in a valid subset.  If false is returned,
     *                       desiredSubset's value is undefined.
     *  @return true         If the input desiredSubset is valid.
     *                       desiredSubset may be modified to a subset
     *                       supported by the codec.
     *          false        If desiredSubset is invalid (NULL or not fully
     *                       contained within the image).
     */
    bool getSupportedSubset(SkIRect* desiredSubset) const;
    // TODO: Rename SkCodec::getValidSubset() to getSupportedSubset()

    /**
     *  Returns the dimensions of the scaled, partial output image, for an
     *  input sampleSize and subset.
     *
     *  @param sampleSize Factor to scale down by.
     *  @param subset     Must be a valid subset of the original image
     *                    dimensions and a subset supported by SkAndroidCodec.
     *                    getSubset() can be used to obtain a subset supported
     *                    by SkAndroidCodec.
     *  @return           Size of the scaled partial image.  Or zero size
     *                    if either of the inputs is invalid.
     */
    SkISize getSampledSubsetDimensions(int sampleSize, const SkIRect& subset) const;

    /**
     *  Additional options to pass to getAndroidPixels().
     */
    // FIXME: It's a bit redundant to name these AndroidOptions when this class is already
    //        called SkAndroidCodec.  On the other hand, it's may be a bit confusing to call
    //        these Options when SkCodec has a slightly different set of Options.  Maybe these
    //        should be DecodeOptions or SamplingOptions?
    struct AndroidOptions {
        AndroidOptions()
            : fZeroInitialized(SkCodec::kNo_ZeroInitialized)
            , fSubset(nullptr)
            , fColorPtr(nullptr)
            , fColorCount(nullptr)
            , fSampleSize(1)
        {}

        /**
         *  Indicates is destination pixel memory is zero initialized.
         */
        SkCodec::ZeroInitialized fZeroInitialized;

        /**
         *  If not NULL, represents a subset of the original image to decode.
         *
         *  Must be within the bounds returned by getInfo().
         *
         *  If the EncodedFormat is kWEBP_SkEncodedFormat, the top and left
         *  values must be even.
         */
        SkIRect* fSubset;

        /**
         *  If the client has requested a decode to kIndex8_SkColorType
         *  (specified in the SkImageInfo), then the caller must provide
         *  storage for up to 256 SkPMColor values in fColorPtr.  On success,
         *  the codec must copy N colors into that storage, (where N is the
         *  logical number of table entries) and set fColorCount to N.
         *
         *  If the client does not request kIndex8_SkColorType, then the last
         *  two parameters may be NULL. If fColorCount is not null, it will be
         *  set to 0.
         */
        SkPMColor* fColorPtr;
        int*       fColorCount;

        /**
         *  The client may provide an integer downscale factor for the decode.
         *  The codec may implement this downscaling by sampling or another
         *  method if it is more efficient.
         */
        int fSampleSize;
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
     *         to scale or subset. If the codec cannot perform this
     *         scaling or subsetting, it will return an error code.
     *
     *  If info is kIndex8_SkColorType, then the caller must provide storage for up to 256
     *  SkPMColor values in options->fColorPtr. On success the codec must copy N colors into
     *  that storage, (where N is the logical number of table entries) and set
     *  options->fColorCount to N.
     *
     *  If info is not kIndex8_SkColorType, options->fColorPtr and options->fColorCount may
     *  be nullptr.
     *
     *  The AndroidOptions object is also used to specify any requested scaling or subsetting
     *  using options->fSampleSize and options->fSubset.
     *
     *  @return Result kSuccess, or another value explaining the type of failure.
     */
    // FIXME: It's a bit redundant to name this getAndroidPixels() when this class is already
    //        called SkAndroidCodec.  On the other hand, it's may be a bit confusing to call
    //        this getPixels() when it is a slightly different API than SkCodec's getPixels().
    //        Maybe this should be decode() or decodeSubset()?
    SkCodec::Result getAndroidPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
            const AndroidOptions* options);

    /**
     *  Simplified version of getAndroidPixels() where we supply the default AndroidOptions.
     *
     *  This will return an error if the info is kIndex_8_SkColorType and also will not perform
     *  any scaling or subsetting.
     */
    SkCodec::Result getAndroidPixels(const SkImageInfo& info, void* pixels, size_t rowBytes);

protected:

    SkAndroidCodec(SkCodec*);

    SkCodec* codec() const { return fCodec.get(); }

    virtual SkISize onGetSampledDimensions(int sampleSize) const = 0;

    virtual bool onGetSupportedSubset(SkIRect* desiredSubset) const = 0;

    virtual SkCodec::Result onGetAndroidPixels(const SkImageInfo& info, void* pixels,
            size_t rowBytes, const AndroidOptions& options) = 0;

private:

    // This will always be a reference to the info that is contained by the
    // embedded SkCodec.
    const SkImageInfo& fInfo;

    SkAutoTDelete<SkCodec> fCodec;
};
#endif // SkAndroidCodec_DEFINED
