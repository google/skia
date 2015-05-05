/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCodec_DEFINED
#define SkCodec_DEFINED

#include "SkEncodedFormat.h"
#include "SkImageGenerator.h"
#include "SkImageInfo.h"
#include "SkScanlineDecoder.h"
#include "SkSize.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkTypes.h"

class SkData;

/**
 *  Abstraction layer directly on top of an image codec.
 */
class SkCodec : public SkImageGenerator {
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

    /**
     *  Return a size that approximately supports the desired scale factor.
     *  The codec may not be able to scale efficiently to the exact scale
     *  factor requested, so return a size that approximates that scale.
     *
     *  FIXME: Move to SkImageGenerator?
     */
    SkISize getScaledDimensions(float desiredScale) const {
        return this->onGetScaledDimensions(desiredScale);
    }

    /**
     *  Format of the encoded data.
     */
    SkEncodedFormat getEncodedFormat() const { return this->onGetEncodedFormat(); }

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

    /*
     *
     * Get method for the input stream
     *
     */
    SkStream* stream() {
        return fStream.get();
    }

private:
    SkAutoTDelete<SkStream>             fStream;
    bool                                fNeedsRewind;
    SkAutoTDelete<SkScanlineDecoder>    fScanlineDecoder;

    typedef SkImageGenerator INHERITED;
};
#endif // SkCodec_DEFINED
