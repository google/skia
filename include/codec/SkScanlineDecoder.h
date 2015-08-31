/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkScanlineDecoder_DEFINED
#define SkScanlineDecoder_DEFINED

#include "../private/SkTemplates.h"
#include "SkCodec.h"
#include "SkImageInfo.h"
#include "SkTypes.h"

class SkScanlineDecoder : public SkNoncopyable {
public:
    /**
     *  If this stream represents an encoded image that we know how to decode
     *  in scanlines, return an SkScanlineDecoder that can decode it. Otherwise
     *  return NULL.
     *
     *  start() must be called in order to decode any scanlines.
     *
     *  If NULL is returned, the stream is deleted immediately. Otherwise, the
     *  SkScanlineDecoder takes ownership of it, and will delete it when done
     *  with it.
     */
    static SkScanlineDecoder* NewFromStream(SkStream*);

    /**
     *  Similar to NewFromStream, but reads from an SkData.
     *
     *  Will take a ref if it returns a scanline decoder, else will not affect
     *  the data.
     */
    static SkScanlineDecoder* NewFromData(SkData*);

    /**
     *  Clean up after reading/skipping scanlines.
     *
     *  It is possible that not all scanlines will have been read/skipped.  In
     *  fact, in the case of subset decodes, it is likely that there will be
     *  scanlines at the bottom of the image that have been ignored.
     */
    virtual ~SkScanlineDecoder() {}

    /**
     *  Return a size that approximately supports the desired scale factor.
     *  The codec may not be able to scale efficiently to the exact scale
     *  factor requested, so return a size that approximates that scale.
     *  The returned value is the codec's suggestion for the closest valid
     *  scale that it can natively support
     *  FIXME: share this with SkCodec
     */
    SkISize getScaledDimensions(float desiredScale) {
        return this->onGetScaledDimensions(desiredScale);
    }

    /**
     *  Returns the default info, corresponding to the encoded data.
     */
    const SkImageInfo& getInfo() { return fSrcInfo; }

    /**
     *  Initialize on the first scanline, with the specified options.
     *
     *  This must be called in order to call getScanlnies or skipScanlines. If
     *  it has been called before, this will require rewinding the stream.
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
    SkCodec::Result start(const SkImageInfo& dstInfo, const SkCodec::Options* options,
                          SkPMColor ctable[], int* ctableCount);

    /**
     *  Simplified version of start() that asserts that info is NOT
     *  kIndex8_SkColorType and uses the default Options.
     */
    SkCodec::Result start(const SkImageInfo& dstInfo);

    /**
     *  Write the next countLines scanlines into dst.
     *
     *  Not valid to call before calling start().
     *
     *  @param dst Must be non-null, and large enough to hold countLines
     *      scanlines of size rowBytes.
     *  @param countLines Number of lines to write.
     *  @param rowBytes Number of bytes per row. Must be large enough to hold
     *      a scanline based on the SkImageInfo used to create this object.
     */
    SkCodec::Result getScanlines(void* dst, int countLines, size_t rowBytes) {
        SkASSERT(!fDstInfo.isEmpty());
        if ((rowBytes < fDstInfo.minRowBytes() && countLines > 1 ) || countLines <= 0
                || fCurrScanline + countLines > fDstInfo.height()) {
            return SkCodec::kInvalidParameters;
        }
        const SkCodec::Result result = this->onGetScanlines(dst, countLines, rowBytes);
        fCurrScanline += countLines;
        return result;
    }

    /**
     *  Skip count scanlines.
     *
     *  Not valid to call before calling start().
     *
     *  The default version just calls onGetScanlines and discards the dst.
     *  NOTE: If skipped lines are the only lines with alpha, this default
     *  will make reallyHasAlpha return true, when it could have returned
     *  false.
     */
    SkCodec::Result skipScanlines(int countLines) {
        SkASSERT(!fDstInfo.isEmpty());
        if (fCurrScanline + countLines > fDstInfo.height()) {
            // Arguably, we could just skip the scanlines which are remaining,
            // and return kSuccess. We choose to return invalid so the client
            // can catch their bug.
            return SkCodec::kInvalidParameters;
        }
        const SkCodec::Result result = this->onSkipScanlines(countLines);
        fCurrScanline += countLines;
        return result;
    }

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

    /**
     *  Format of the encoded data.
     */
    SkEncodedFormat getEncodedFormat() const { return this->onGetEncodedFormat(); }

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
         * kBottomUp, the getY() API can be used to determine the actual
         * y-coordinate of the next output row, but the client is not forced
         * to take advantage of this, given that it's not too tough to keep
         * track independently.
         *
         * For full image decodes, it is safe to get all of the scanlines at
         * once, since the decoder will handle inverting the rows as it
         * decodes.
         *
         * For subset decodes and sampling, it is simplest to get and skip
         * scanlines one at a time, using the getY() API.  It is possible to
         * ask for larger chunks at a time, but this should be used with
         * caution.  As with full image decodes, the decoder will handle
         * inverting the requested rows, but rows will still be delivered
         * starting from the bottom of the image.
         *
         * Upside down bmps are an example.
         */
        kBottomUp_SkScanlineOrder,

        /*
         * This indicates that the scanline decoder reliably outputs rows, but
         * they will not be in logical order.  If the scanline format is
         * kOutOfOrder, the getY() API should be used to determine the actual
         * y-coordinate of the next output row.
         *
         * For this scanline ordering, it is advisable to get and skip
         * scanlines one at a time.
         *
         * Interlaced gifs are an example.
         */
        kOutOfOrder_SkScanlineOrder,

        /*
         * Indicates that the entire image must be decoded in order to output
         * any amount of scanlines.  In this case, it is a REALLY BAD IDEA to
         * request scanlines 1-by-1 or in small chunks.  The client should
         * determine which scanlines are needed and ask for all of them in
         * a single call to getScanlines().
         *
         * Interlaced pngs are an example.
         */
        kNone_SkScanlineOrder,
    };

    /**
     *  An enum representing the order in which scanlines will be returned by
     *  the scanline decoder.
     */
    SkScanlineOrder getScanlineOrder() const { return this->onGetScanlineOrder(); }

    /**
     *  Returns the y-coordinate of the next row to be returned by the scanline
     *  decoder.  This will be overridden in the case of
     *  kOutOfOrder_SkScanlineOrder and should be unnecessary in the case of
     *  kNone_SkScanlineOrder.
     */
    int getY() const {
        SkASSERT(kNone_SkScanlineOrder != this->getScanlineOrder());
        return this->onGetY();
    }

protected:
    SkScanlineDecoder(const SkImageInfo& srcInfo)
        : fSrcInfo(srcInfo)
        , fDstInfo()
        , fOptions()
        , fCurrScanline(0) {}

    virtual SkISize onGetScaledDimensions(float /* desiredScale */) {
        // By default, scaling is not supported.
        return this->getInfo().dimensions();
    }

    virtual SkEncodedFormat onGetEncodedFormat() const = 0;

    virtual bool onReallyHasAlpha() const { return false; }

    /**
     *  Most images types will be kTopDown and will not need to override this function.
     */
    virtual SkScanlineOrder onGetScanlineOrder() const { return kTopDown_SkScanlineOrder; }

    /**
     *  Most images will be kTopDown and will not need to override this function.
     */
    virtual int onGetY() const { return fCurrScanline; }

    const SkImageInfo& dstInfo() const { return fDstInfo; }

    const SkCodec::Options& options() const { return fOptions; }

private:
    const SkImageInfo   fSrcInfo;
    SkImageInfo         fDstInfo;
    SkCodec::Options    fOptions;
    int                 fCurrScanline;

    virtual SkCodec::Result onStart(const SkImageInfo& dstInfo,
                                    const SkCodec::Options& options,
                                    SkPMColor ctable[], int* ctableCount) = 0;

    // Naive default version just calls onGetScanlines on temp memory.
    virtual SkCodec::Result onSkipScanlines(int countLines) {
        SkAutoMalloc storage(fDstInfo.minRowBytes());
        // Note that we pass 0 to rowBytes so we continue to use the same memory.
        // Also note that while getScanlines checks that rowBytes is big enough,
        // onGetScanlines bypasses that check.
        // Calling the virtual method also means we do not double count
        // countLines.
        return this->onGetScanlines(storage.get(), countLines, 0);
    }

    virtual SkCodec::Result onGetScanlines(void* dst, int countLines,
                                                    size_t rowBytes) = 0;

};
#endif // SkScanlineDecoder_DEFINED
