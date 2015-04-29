/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkScanlineDecoder_DEFINED
#define SkScanlineDecoder_DEFINED

#include "SkTypes.h"
#include "SkTemplates.h"
#include "SkImageGenerator.h"
#include "SkImageInfo.h"

class SkScanlineDecoder : public SkNoncopyable {
public:
    // Note for implementations: An SkScanlineDecoder will be deleted by (and
    // therefore *before*) its associated SkCodec, in case the order matters.
    virtual ~SkScanlineDecoder() {}

    /**
     *  Write the next countLines scanlines into dst.
     *
     *  @param dst Must be non-null, and large enough to hold countLines
     *      scanlines of size rowBytes.
     *  @param countLines Number of lines to write.
     *  @param rowBytes Number of bytes per row. Must be large enough to hold
     *      a scanline based on the SkImageInfo used to create this object.
     */
    SkImageGenerator::Result getScanlines(void* dst, int countLines, size_t rowBytes) {
        if ((rowBytes < fDstInfo.minRowBytes() && countLines > 1 ) || countLines <= 0
                || fCurrScanline + countLines > fDstInfo.height()) {
            return SkImageGenerator::kInvalidParameters;
        }
        const SkImageGenerator::Result result = this->onGetScanlines(dst, countLines, rowBytes);
        this->checkForFinish(countLines);
        return result;
    }

    /**
     *  Skip count scanlines.
     *
     *  The default version just calls onGetScanlines and discards the dst.
     *  NOTE: If skipped lines are the only lines with alpha, this default
     *  will make reallyHasAlpha return true, when it could have returned
     *  false.
     */
    SkImageGenerator::Result skipScanlines(int countLines) {
        if (fCurrScanline + countLines > fDstInfo.height()) {
            // Arguably, we could just skip the scanlines which are remaining,
            // and return kSuccess. We choose to return invalid so the client
            // can catch their bug.
            return SkImageGenerator::kInvalidParameters;
        }
        const SkImageGenerator::Result result = this->onSkipScanlines(countLines);
        this->checkForFinish(countLines);
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
    SkScanlineDecoder(const SkImageInfo& requested)
        : fDstInfo(requested)
        , fCurrScanline(0) {}

    virtual bool onReallyHasAlpha() const { return false; }

    const SkImageInfo& dstInfo() const { return fDstInfo; }

private:
    const SkImageInfo   fDstInfo;
    int                 fCurrScanline;

    // Naive default version just calls onGetScanlines on temp memory.
    virtual SkImageGenerator::Result onSkipScanlines(int countLines) {
        SkAutoMalloc storage(fDstInfo.minRowBytes());
        // Note that we pass 0 to rowBytes so we continue to use the same memory.
        // Also note that while getScanlines checks that rowBytes is big enough,
        // onGetScanlines bypasses that check.
        // Calling the virtual method also means we do not double count
        // countLines.
        return this->onGetScanlines(storage.get(), countLines, 0);
    }

    virtual SkImageGenerator::Result onGetScanlines(void* dst, int countLines,
                                                    size_t rowBytes) = 0;

    /**
     *  Called after any set of scanlines read/skipped. Updates fCurrScanline,
     *  and, if we are at the end, calls onFinish().
     */
    void checkForFinish(int countLines) {
        fCurrScanline += countLines;
        if (fCurrScanline >= fDstInfo.height()) {
            this->onFinish();
        }
    }

    /**
     *  This function will be called after reading/skipping all scanlines to do
     *  any necessary cleanups.
     */
    virtual void onFinish() {} // Default does nothing.
};
#endif // SkScanlineDecoder_DEFINED
