/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCodec_DEFINED
#define SkCodec_DEFINED

#include "../private/SkTemplates.h"
#include "SkColor.h"
#include "SkEncodedImageFormat.h"
#include "SkEncodedInfo.h"
#include "SkImageInfo.h"
#include "SkSize.h"
#include "SkStream.h"
#include "SkTypes.h"
#include "SkYUVSizeInfo.h"

#include <vector>

class SkColorSpace;
class SkColorSpaceXform;
class SkData;
class SkPngChunkReader;
class SkSampler;

namespace DM {
class CodecSrc;
class ColorCodecSrc;
}
class ColorCodecBench;

/**
 *  Abstraction layer directly on top of an image codec.
 */
class SK_API SkCodec : SkNoncopyable {
public:
    /**
     *  Minimum number of bytes that must be buffered in SkStream input.
     *
     *  An SkStream passed to NewFromStream must be able to use this many
     *  bytes to determine the image type. Then the same SkStream must be
     *  passed to the correct decoder to read from the beginning.
     *
     *  This can be accomplished by implementing peek() to support peeking
     *  this many bytes, or by implementing rewind() to be able to rewind()
     *  after reading this many bytes.
     */
    static size_t MinBufferedBytesNeeded();

    /**
     *  If this stream represents an encoded image that we know how to decode,
     *  return an SkCodec that can decode it. Otherwise return NULL.
     *
     *  As stated above, this call must be able to peek or read
     *  MinBufferedBytesNeeded to determine the correct format, and then start
     *  reading from the beginning. First it will attempt to peek, and it
     *  assumes that if less than MinBufferedBytesNeeded bytes (but more than
     *  zero) are returned, this is because the stream is shorter than this,
     *  so falling back to reading would not provide more data. If peek()
     *  returns zero bytes, this call will instead attempt to read(). This
     *  will require that the stream can be rewind()ed.
     *
     *  If SkPngChunkReader is not NULL, take a ref and pass it to libpng if
     *  the image is a png.
     *
     *  If the SkPngChunkReader is not NULL then:
     *      If the image is not a PNG, the SkPngChunkReader will be ignored.
     *      If the image is a PNG, the SkPngChunkReader will be reffed.
     *      If the PNG has unknown chunks, the SkPngChunkReader will be used
     *      to handle these chunks.  SkPngChunkReader will be called to read
     *      any unknown chunk at any point during the creation of the codec
     *      or the decode.  Note that if SkPngChunkReader fails to read a
     *      chunk, this could result in a failure to create the codec or a
     *      failure to decode the image.
     *      If the PNG does not contain unknown chunks, the SkPngChunkReader
     *      will not be used or modified.
     *
     *  If NULL is returned, the stream is deleted immediately. Otherwise, the
     *  SkCodec takes ownership of it, and will delete it when done with it.
     */
    static SkCodec* NewFromStream(SkStream*, SkPngChunkReader* = NULL);

    /**
     *  If this data represents an encoded image that we know how to decode,
     *  return an SkCodec that can decode it. Otherwise return NULL.
     *
     *  If the SkPngChunkReader is not NULL then:
     *      If the image is not a PNG, the SkPngChunkReader will be ignored.
     *      If the image is a PNG, the SkPngChunkReader will be reffed.
     *      If the PNG has unknown chunks, the SkPngChunkReader will be used
     *      to handle these chunks.  SkPngChunkReader will be called to read
     *      any unknown chunk at any point during the creation of the codec
     *      or the decode.  Note that if SkPngChunkReader fails to read a
     *      chunk, this could result in a failure to create the codec or a
     *      failure to decode the image.
     *      If the PNG does not contain unknown chunks, the SkPngChunkReader
     *      will not be used or modified.
     */
    static SkCodec* NewFromData(sk_sp<SkData>, SkPngChunkReader* = NULL);
    static SkCodec* NewFromData(SkData* data, SkPngChunkReader* reader) {
        return NewFromData(sk_ref_sp(data), reader);
    }

    virtual ~SkCodec();

    /**
     *  Return the ImageInfo associated with this codec.
     */
    const SkImageInfo& getInfo() const { return fSrcInfo; }

    const SkEncodedInfo& getEncodedInfo() const { return fEncodedInfo; }

    enum Origin {
        kTopLeft_Origin     = 1, // Default
        kTopRight_Origin    = 2, // Reflected across y-axis
        kBottomRight_Origin = 3, // Rotated 180
        kBottomLeft_Origin  = 4, // Reflected across x-axis
        kLeftTop_Origin     = 5, // Reflected across x-axis, Rotated 90 CCW
        kRightTop_Origin    = 6, // Rotated 90 CW
        kRightBottom_Origin = 7, // Reflected across x-axis, Rotated 90 CW
        kLeftBottom_Origin  = 8, // Rotated 90 CCW
        kDefault_Origin     = kTopLeft_Origin,
        kLast_Origin        = kLeftBottom_Origin,
    };

    /**
     *  Returns the image orientation stored in the EXIF data.
     *  If there is no EXIF data, or if we cannot read the EXIF data, returns kTopLeft.
     */
    Origin getOrigin() const { return fOrigin; }

    /**
     *  Return a size that approximately supports the desired scale factor.
     *  The codec may not be able to scale efficiently to the exact scale
     *  factor requested, so return a size that approximates that scale.
     *  The returned value is the codec's suggestion for the closest valid
     *  scale that it can natively support
     */
    SkISize getScaledDimensions(float desiredScale) const {
        // Negative and zero scales are errors.
        SkASSERT(desiredScale > 0.0f);
        if (desiredScale <= 0.0f) {
            return SkISize::Make(0, 0);
        }

        // Upscaling is not supported. Return the original size if the client
        // requests an upscale.
        if (desiredScale >= 1.0f) {
            return this->getInfo().dimensions();
        }
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
    SkEncodedImageFormat getEncodedFormat() const { return this->onGetEncodedFormat(); }

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
         *  This method is not implemented by this codec.
         *  FIXME: Perhaps this should be kUnsupported?
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
            , fSubset(nullptr)
            , fFrameIndex(0)
            , fHasPriorFrame(false)
            , fPremulBehavior(SkTransferFunctionBehavior::kRespect)
        {}

        ZeroInitialized            fZeroInitialized;
        /**
         *  If not NULL, represents a subset of the original image to decode.
         *  Must be within the bounds returned by getInfo().
         *  If the EncodedFormat is SkEncodedImageFormat::kWEBP (the only one which
         *  currently supports subsets), the top and left values must be even.
         *
         *  In getPixels and incremental decode, we will attempt to decode the
         *  exact rectangular subset specified by fSubset.
         *
         *  In a scanline decode, it does not make sense to specify a subset
         *  top or subset height, since the client already controls which rows
         *  to get and which rows to skip.  During scanline decodes, we will
         *  require that the subset top be zero and the subset height be equal
         *  to the full height.  We will, however, use the values of
         *  subset left and subset width to decode partial scanlines on calls
         *  to getScanlines().
         */
        const SkIRect*             fSubset;

        /**
         *  The frame to decode.
         *
         *  Only meaningful for multi-frame images.
         */
        size_t                     fFrameIndex;

        /**
         *  If true, the dst already contains the prior frame.
         *
         *  Only meaningful for multi-frame images.
         *
         *  If fFrameIndex needs to be blended with a prior frame (as reported by
         *  getFrameInfo[fFrameIndex].fRequiredFrame), the client can set this to
         *  either true or false:
         *
         *  true means that the prior frame is already in the dst, and this
         *  codec only needs to decode fFrameIndex and blend it with the dst.
         *  Options.fZeroInitialized is ignored in this case.
         *
         *  false means that the dst does not contain the prior frame, so this
         *  codec needs to first decode the prior frame (which in turn may need
         *  to decode its prior frame).
         */
        bool                       fHasPriorFrame;

        /**
         *  Indicates whether we should do a linear premultiply or a legacy premultiply.
         *
         *  In the case where the dst SkColorSpace is nullptr, this flag is ignored and
         *  we will always do a legacy premultiply.
         */
        SkTransferFunctionBehavior fPremulBehavior;
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
     *         If the info contains a non-null SkColorSpace, the codec
     *         will perform the appropriate color space transformation.
     *         If the caller passes in the same color space that was
     *         reported by the codec, the color space transformation is
     *         a no-op.
     *
     *  If info is kIndex8_SkColorType, then the caller must provide storage for up to 256
     *  SkPMColor values in ctable. On success the generator must copy N colors into that storage,
     *  (where N is the logical number of table entries) and set ctableCount to N.
     *
     *  If info is not kIndex8_SkColorType, then the last two parameters may be NULL. If ctableCount
     *  is not null, it will be set to 0.
     *
     *  If a scanline decode is in progress, scanline mode will end, requiring the client to call
     *  startScanlineDecode() in order to return to decoding scanlines.
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
     *  If decoding to YUV is supported, this returns true.  Otherwise, this
     *  returns false and does not modify any of the parameters.
     *
     *  @param sizeInfo   Output parameter indicating the sizes and required
     *                    allocation widths of the Y, U, and V planes.
     *  @param colorSpace Output parameter.  If non-NULL this is set to kJPEG,
     *                    otherwise this is ignored.
     */
    bool queryYUV8(SkYUVSizeInfo* sizeInfo, SkYUVColorSpace* colorSpace) const {
        if (nullptr == sizeInfo) {
            return false;
        }

        return this->onQueryYUV8(sizeInfo, colorSpace);
    }

    /**
     *  Returns kSuccess, or another value explaining the type of failure.
     *  This always attempts to perform a full decode.  If the client only
     *  wants size, it should call queryYUV8().
     *
     *  @param sizeInfo   Needs to exactly match the values returned by the
     *                    query, except the WidthBytes may be larger than the
     *                    recommendation (but not smaller).
     *  @param planes     Memory for each of the Y, U, and V planes.
     */
    Result getYUV8Planes(const SkYUVSizeInfo& sizeInfo, void* planes[3]) {
        if (nullptr == planes || nullptr == planes[0] || nullptr == planes[1] ||
                nullptr == planes[2]) {
            return kInvalidInput;
        }

        if (!this->rewindIfNeeded()) {
            return kCouldNotRewind;
        }

        return this->onGetYUV8Planes(sizeInfo, planes);
    }

    /**
     *  Prepare for an incremental decode with the specified options.
     *
     *  This may require a rewind.
     *
     *  @param dstInfo Info of the destination. If the dimensions do not match
     *      those of getInfo, this implies a scale.
     *  @param dst Memory to write to. Needs to be large enough to hold the subset,
     *      if present, or the full image as described in dstInfo.
     *  @param options Contains decoding options, including if memory is zero
     *      initialized and whether to decode a subset.
     *  @param ctable A pointer to a color table.  When dstInfo.colorType() is
     *      kIndex8, this should be non-NULL and have enough storage for 256
     *      colors.  The color table will be populated after decoding the palette.
     *  @param ctableCount A pointer to the size of the color table.  When
     *      dstInfo.colorType() is kIndex8, this should be non-NULL.  It will
     *      be modified to the true size of the color table (<= 256) after
     *      decoding the palette.
     *  @return Enum representing success or reason for failure.
     */
    Result startIncrementalDecode(const SkImageInfo& dstInfo, void* dst, size_t rowBytes,
            const SkCodec::Options*, SkPMColor* ctable, int* ctableCount);

    Result startIncrementalDecode(const SkImageInfo& dstInfo, void* dst, size_t rowBytes,
            const SkCodec::Options* options) {
        return this->startIncrementalDecode(dstInfo, dst, rowBytes, options, nullptr, nullptr);
    }

    Result startIncrementalDecode(const SkImageInfo& dstInfo, void* dst, size_t rowBytes) {
        return this->startIncrementalDecode(dstInfo, dst, rowBytes, nullptr, nullptr, nullptr);
    }

    /**
     *  Start/continue the incremental decode.
     *
     *  Not valid to call before calling startIncrementalDecode().
     *
     *  After the first call, should only be called again if more data has been
     *  provided to the source SkStream.
     *
     *  Unlike getPixels and getScanlines, this does not do any filling. This is
     *  left up to the caller, since they may be skipping lines or continuing the
     *  decode later. In the latter case, they may choose to initialize all lines
     *  first, or only initialize the remaining lines after the first call.
     *
     *  @param rowsDecoded Optional output variable returning the total number of
     *      lines initialized. Only meaningful if this method returns kIncompleteInput.
     *      Otherwise the implementation may not set it.
     *      Note that some implementations may have initialized this many rows, but
     *      not necessarily finished those rows (e.g. interlaced PNG). This may be
     *      useful for determining what rows the client needs to initialize.
     *  @return kSuccess if all lines requested in startIncrementalDecode have
     *      been completely decoded. kIncompleteInput otherwise.
     */
    Result incrementalDecode(int* rowsDecoded = nullptr) {
        if (!fStartedIncrementalDecode) {
            return kInvalidParameters;
        }
        return this->onIncrementalDecode(rowsDecoded);
    }

    /**
     * The remaining functions revolve around decoding scanlines.
     */

    /**
     *  Prepare for a scanline decode with the specified options.
     *
     *  After this call, this class will be ready to decode the first scanline.
     *
     *  This must be called in order to call getScanlines or skipScanlines.
     *
     *  This may require rewinding the stream.
     *
     *  Not all SkCodecs support this.
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
     *  @return Enum representing success or reason for failure.
     */
    Result startScanlineDecode(const SkImageInfo& dstInfo, const SkCodec::Options* options,
            SkPMColor ctable[], int* ctableCount);

    /**
     *  Simplified version of startScanlineDecode() that asserts that info is NOT
     *  kIndex8_SkColorType and uses the default Options.
     */
    Result startScanlineDecode(const SkImageInfo& dstInfo);

    /**
     *  Write the next countLines scanlines into dst.
     *
     *  Not valid to call before calling startScanlineDecode().
     *
     *  @param dst Must be non-null, and large enough to hold countLines
     *      scanlines of size rowBytes.
     *  @param countLines Number of lines to write.
     *  @param rowBytes Number of bytes per row. Must be large enough to hold
     *      a scanline based on the SkImageInfo used to create this object.
     *  @return the number of lines successfully decoded.  If this value is
     *      less than countLines, this will fill the remaining lines with a
     *      default value.
     */
    int getScanlines(void* dst, int countLines, size_t rowBytes);

    /**
     *  Skip count scanlines.
     *
     *  Not valid to call before calling startScanlineDecode().
     *
     *  The default version just calls onGetScanlines and discards the dst.
     *  NOTE: If skipped lines are the only lines with alpha, this default
     *  will make reallyHasAlpha return true, when it could have returned
     *  false.
     *
     *  @return true if the scanlines were successfully skipped
     *          false on failure, possible reasons for failure include:
     *              An incomplete input image stream.
     *              Calling this function before calling startScanlineDecode().
     *              If countLines is less than zero or so large that it moves
     *                  the current scanline past the end of the image.
     */
    bool skipScanlines(int countLines);

    /**
     *  The order in which rows are output from the scanline decoder is not the
     *  same for all variations of all image types.  This explains the possible
     *  output row orderings.
     */
    enum SkScanlineOrder {
        /*
         * By far the most common, this indicates that the image can be decoded
         * reliably using the scanline decoder, and that rows will be output in
         * the logical order.
         */
        kTopDown_SkScanlineOrder,

        /*
         * This indicates that the scanline decoder reliably outputs rows, but
         * they will be returned in reverse order.  If the scanline format is
         * kBottomUp, the nextScanline() API can be used to determine the actual
         * y-coordinate of the next output row, but the client is not forced
         * to take advantage of this, given that it's not too tough to keep
         * track independently.
         *
         * For full image decodes, it is safe to get all of the scanlines at
         * once, since the decoder will handle inverting the rows as it
         * decodes.
         *
         * For subset decodes and sampling, it is simplest to get and skip
         * scanlines one at a time, using the nextScanline() API.  It is
         * possible to ask for larger chunks at a time, but this should be used
         * with caution.  As with full image decodes, the decoder will handle
         * inverting the requested rows, but rows will still be delivered
         * starting from the bottom of the image.
         *
         * Upside down bmps are an example.
         */
        kBottomUp_SkScanlineOrder,
    };

    /**
     *  An enum representing the order in which scanlines will be returned by
     *  the scanline decoder.
     *
     *  This is undefined before startScanlineDecode() is called.
     */
    SkScanlineOrder getScanlineOrder() const { return this->onGetScanlineOrder(); }

    /**
     *  Returns the y-coordinate of the next row to be returned by the scanline
     *  decoder.
     *
     *  This will equal fCurrScanline, except in the case of strangely
     *  encoded image types (bottom-up bmps).
     *
     *  Results are undefined when not in scanline decoding mode.
     */
    int nextScanline() const { return this->outputScanline(fCurrScanline); }

    /**
     *  Returns the output y-coordinate of the row that corresponds to an input
     *  y-coordinate.  The input y-coordinate represents where the scanline
     *  is located in the encoded data.
     *
     *  This will equal inputScanline, except in the case of strangely
     *  encoded image types (bottom-up bmps, interlaced gifs).
     */
    int outputScanline(int inputScanline) const;

    /**
     *  Return the number of frames in the image.
     *
     *  May require reading through the stream.
     */
    size_t getFrameCount() {
        return this->onGetFrameCount();
    }

    // The required frame for an independent frame is marked as
    // kNone.
    static constexpr size_t kNone = static_cast<size_t>(-1);

    /**
     *  Information about individual frames in a multi-framed image.
     */
    struct FrameInfo {
        /**
         *  The frame that this frame needs to be blended with, or
         *  kNone.
         */
        size_t fRequiredFrame;

        /**
         *  Number of milliseconds to show this frame.
         */
        size_t fDuration;

        /**
         *  Whether the end marker for this frame is contained in the stream.
         *
         *  Note: this does not guarantee that an attempt to decode will be complete.
         *  There could be an error in the stream.
         */
        bool fFullyReceived;

        /**
         *  This is conservative; it will still return non-opaque if e.g. a
         *  color index-based frame has a color with alpha but does not use it.
         */
        SkAlphaType fAlphaType;
    };

    /**
     *  Return info about a single frame.
     *
     *  Only supported by multi-frame images. Does not read through the stream,
     *  so it should be called after getFrameCount() to parse any frames that
     *  have not already been parsed.
     */
    bool getFrameInfo(size_t index, FrameInfo* info) const {
        return this->onGetFrameInfo(index, info);
    }

    /**
     *  Return info about all the frames in the image.
     *
     *  May require reading through the stream to determine info about the
     *  frames (including the count).
     *
     *  As such, future decoding calls may require a rewind.
     *
     *  For single-frame images, this will return an empty vector.
     */
    std::vector<FrameInfo> getFrameInfo();

    static constexpr int kRepetitionCountInfinite = -1;

    /**
     *  Return the number of times to repeat, if this image is animated.
     *
     *  May require reading the stream to find the repetition count.
     *
     *  As such, future decoding calls may require a rewind.
     *
     *  For single-frame images, this will return 0.
     */
    int getRepetitionCount() {
        return this->onGetRepetitionCount();
    }

protected:
    /**
     *  Takes ownership of SkStream*
     */
    SkCodec(int width,
            int height,
            const SkEncodedInfo&,
            SkStream*,
            sk_sp<SkColorSpace>,
            Origin = kTopLeft_Origin);

    /**
     *  Takes ownership of SkStream*
     *  Allows the subclass to set the recommended SkImageInfo
     */
    SkCodec(const SkEncodedInfo&,
            const SkImageInfo&,
            SkStream*,
            Origin = kTopLeft_Origin);

    virtual SkISize onGetScaledDimensions(float /*desiredScale*/) const {
        // By default, scaling is not supported.
        return this->getInfo().dimensions();
    }

    // FIXME: What to do about subsets??
    /**
     *  Subclasses should override if they support dimensions other than the
     *  srcInfo's.
     */
    virtual bool onDimensionsSupported(const SkISize&) {
        return false;
    }

    virtual SkEncodedImageFormat onGetEncodedFormat() const = 0;

    /**
     * @param rowsDecoded When the encoded image stream is incomplete, this function
     *                    will return kIncompleteInput and rowsDecoded will be set to
     *                    the number of scanlines that were successfully decoded.
     *                    This will allow getPixels() to fill the uninitialized memory.
     */
    virtual Result onGetPixels(const SkImageInfo& info,
                               void* pixels, size_t rowBytes, const Options&,
                               SkPMColor ctable[], int* ctableCount,
                               int* rowsDecoded) = 0;

    virtual bool onQueryYUV8(SkYUVSizeInfo*, SkYUVColorSpace*) const {
        return false;
    }

    virtual Result onGetYUV8Planes(const SkYUVSizeInfo&, void*[3] /*planes*/) {
        return kUnimplemented;
    }

    virtual bool onGetValidSubset(SkIRect* /*desiredSubset*/) const {
        // By default, subsets are not supported.
        return false;
    }

    /**
     *  If the stream was previously read, attempt to rewind.
     *
     *  If the stream needed to be rewound, call onRewind.
     *  @returns true if the codec is at the right position and can be used.
     *      false if there was a failure to rewind.
     *
     *  This is called by getPixels() and start(). Subclasses may call if they
     *  need to rewind at another time.
     */
    bool SK_WARN_UNUSED_RESULT rewindIfNeeded();

    /**
     *  Called by rewindIfNeeded, if the stream needed to be rewound.
     *
     *  Subclasses should do any set up needed after a rewind.
     */
    virtual bool onRewind() {
        return true;
    }

    /**
     * On an incomplete input, getPixels() and getScanlines() will fill any uninitialized
     * scanlines.  This allows the subclass to indicate what value to fill with.
     *
     * @param dstInfo   Describes the destination.
     * @return          The value with which to fill uninitialized pixels.
     *
     * Note that we can interpret the return value as a 64-bit Float16 color, a SkPMColor,
     * a 16-bit 565 color, an 8-bit gray color, or an 8-bit index into a color table,
     * depending on the color type.
     */
    uint64_t getFillValue(const SkImageInfo& dstInfo) const {
        return this->onGetFillValue(dstInfo);
    }

    /**
     * Some subclasses will override this function, but this is a useful default for the color
     * types that we support.  Note that for color types that do not use the full 64-bits,
     * we will simply take the low bits of the fill value.
     *
     * The defaults are:
     * kRGBA_F16_SkColorType: Transparent or Black, depending on the src alpha type
     * kN32_SkColorType: Transparent or Black, depending on the src alpha type
     * kRGB_565_SkColorType: Black
     * kGray_8_SkColorType: Black
     * kIndex_8_SkColorType: First color in color table
     */
    virtual uint64_t onGetFillValue(const SkImageInfo& dstInfo) const;

    /**
     * Get method for the input stream
     */
    SkStream* stream() {
        return fStream.get();
    }

    /**
     *  The remaining functions revolve around decoding scanlines.
     */

    /**
     *  Most images types will be kTopDown and will not need to override this function.
     */
    virtual SkScanlineOrder onGetScanlineOrder() const { return kTopDown_SkScanlineOrder; }

    const SkImageInfo& dstInfo() const { return fDstInfo; }

    const SkCodec::Options& options() const { return fOptions; }

    /**
     *  Returns the number of scanlines that have been decoded so far.
     *  This is unaffected by the SkScanlineOrder.
     *
     *  Returns -1 if we have not started a scanline decode.
     */
    int currScanline() const { return fCurrScanline; }

    virtual int onOutputScanline(int inputScanline) const;

    bool initializeColorXform(const SkImageInfo& dstInfo,
                              SkTransferFunctionBehavior premulBehavior);
    SkColorSpaceXform* colorXform() const { return fColorXform.get(); }

    virtual size_t onGetFrameCount() {
        return 1;
    }

    virtual bool onGetFrameInfo(size_t, FrameInfo*) const {
        return false;
    }

    virtual int onGetRepetitionCount() {
        return 0;
    }

    void setUnsupportedICC(bool SkDEBUGCODE(value)) { SkDEBUGCODE(fUnsupportedICC = value); }

private:
    const SkEncodedInfo                fEncodedInfo;
    const SkImageInfo                  fSrcInfo;
    std::unique_ptr<SkStream>          fStream;
    bool                               fNeedsRewind;
    const Origin                       fOrigin;

    SkImageInfo                        fDstInfo;
    SkCodec::Options                   fOptions;
    std::unique_ptr<SkColorSpaceXform> fColorXform;

    // Only meaningful during scanline decodes.
    int                                fCurrScanline;

    bool                               fStartedIncrementalDecode;
#ifdef SK_DEBUG
    bool                               fUnsupportedICC = false;
#endif

    /**
     *  Return whether these dimensions are supported as a scale.
     *
     *  The codec may choose to cache the information about scale and subset.
     *  Either way, the same information will be passed to onGetPixels/onStart
     *  on success.
     *
     *  This must return true for a size returned from getScaledDimensions.
     */
    bool dimensionsSupported(const SkISize& dim) {
        return dim == fSrcInfo.dimensions() || this->onDimensionsSupported(dim);
    }

    // Methods for scanline decoding.
    virtual SkCodec::Result onStartScanlineDecode(const SkImageInfo& /*dstInfo*/,
            const SkCodec::Options& /*options*/, SkPMColor* /*ctable*/, int* /*ctableCount*/) {
        return kUnimplemented;
    }

    virtual Result onStartIncrementalDecode(const SkImageInfo& /*dstInfo*/, void*, size_t,
            const SkCodec::Options&, SkPMColor*, int*) {
        return kUnimplemented;
    }

    virtual Result onIncrementalDecode(int*) {
        return kUnimplemented;
    }


    virtual bool onSkipScanlines(int /*countLines*/) { return false; }

    virtual int onGetScanlines(void* /*dst*/, int /*countLines*/, size_t /*rowBytes*/) { return 0; }

    /**
     * On an incomplete decode, getPixels() and getScanlines() will call this function
     * to fill any uinitialized memory.
     *
     * @param dstInfo        Contains the destination color type
     *                       Contains the destination alpha type
     *                       Contains the destination width
     *                       The height stored in this info is unused
     * @param dst            Pointer to the start of destination pixel memory
     * @param rowBytes       Stride length in destination pixel memory
     * @param zeroInit       Indicates if memory is zero initialized
     * @param linesRequested Number of lines that the client requested
     * @param linesDecoded   Number of lines that were successfully decoded
     */
    void fillIncompleteImage(const SkImageInfo& dstInfo, void* dst, size_t rowBytes,
            ZeroInitialized zeroInit, int linesRequested, int linesDecoded);

    /**
     *  Return an object which will allow forcing scanline decodes to sample in X.
     *
     *  May create a sampler, if one is not currently being used. Otherwise, does
     *  not affect ownership.
     *
     *  Only valid during scanline decoding or incremental decoding.
     */
    virtual SkSampler* getSampler(bool /*createIfNecessary*/) { return nullptr; }

    friend class DM::CodecSrc;  // for fillIncompleteImage
    friend class SkSampledCodec;
    friend class SkIcoCodec;
    friend struct Sniffer; // for fUnsupportedICC
    friend class AutoCleanPng; // for setUnsupportedICC()
};
#endif // SkCodec_DEFINED
