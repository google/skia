/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkScanlineDecoder_DEFINED
#define SkScanlineDecoder_DEFINED

#include "SkTypes.h"
#include "SkCodec.h"
#include "SkTemplates.h"
#include "SkImageInfo.h"

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

protected:
    SkScanlineDecoder(const SkImageInfo& srcInfo)
        : fSrcInfo(srcInfo)
        , fDstInfo()
        , fCurrScanline(0) {}

    virtual bool onReallyHasAlpha() const { return false; }

    const SkImageInfo& dstInfo() const { return fDstInfo; }

private:
    const SkImageInfo   fSrcInfo;
    SkImageInfo         fDstInfo;
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
