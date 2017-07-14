/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkGifCodec_DEFINED
#define SkGifCodec_DEFINED

#include "SkCodec.h"
#include "SkCodecAnimation.h"
#include "SkColorSpace.h"
#include "SkColorTable.h"
#include "SkImageInfo.h"
#include "SkSwizzler.h"

#include "SkGifImageReader.h"

/*
 *
 * This class implements the decoding for gif images
 *
 */
class SkGifCodec : public SkCodec {
public:
    static bool IsGif(const void*, size_t);

    /*
     * Assumes IsGif was called and returned true
     * Reads enough of the stream to determine the image format
     */
    static SkCodec* NewFromStream(SkStream*, Result*);

    // Callback for SkGifImageReader when a row is available.
    bool haveDecodedRow(int frameIndex, const unsigned char* rowBegin,
                        int rowNumber, int repeatCount, bool writeTransparentPixels);
protected:
    /*
     * Performs the full gif decode
     */
    Result onGetPixels(const SkImageInfo&, void*, size_t, const Options&,
            int*) override;

    SkEncodedImageFormat onGetEncodedFormat() const override {
        return SkEncodedImageFormat::kGIF;
    }

    bool onRewind() override;

    uint64_t onGetFillValue(const SkImageInfo&) const override;

    int onGetFrameCount() override;
    bool onGetFrameInfo(int, FrameInfo*) const override;
    int onGetRepetitionCount() override;

    Result onStartIncrementalDecode(const SkImageInfo& /*dstInfo*/, void*, size_t,
            const SkCodec::Options&) override;

    Result onIncrementalDecode(int*) override;

    const SkFrameHolder* getFrameHolder() const override {
        return fReader.get();
    }

private:

    /*
     * Initializes the color table that we will use for decoding.
     *
     * @param dstInfo         Contains the requested dst color type.
     * @param frameIndex      Frame whose color table to use.
     */
    void initializeColorTable(const SkImageInfo& dstInfo, int frameIndex);

   /*
    * Does necessary setup, including setting up the color table and swizzler.
    */
    Result prepareToDecode(const SkImageInfo& dstInfo, const Options& opts);

    /*
     * Initializes the swizzler.
     *
     * @param dstInfo    Output image information.  Dimensions may have been
     *                   adjusted if the image frame size does not match the size
     *                   indicated in the header.
     * @param frameIndex Which frame we are decoding. This determines the frameRect
     *                   to use.
     */
    void initializeSwizzler(const SkImageInfo& dstInfo, int frameIndex);

    SkSampler* getSampler(bool createIfNecessary) override {
        SkASSERT(fSwizzler);
        return fSwizzler.get();
    }

    /*
     * Recursive function to decode a frame.
     *
     * @param firstAttempt Whether this is the first call to decodeFrame since
     *                     starting. e.g. true in onGetPixels, and true in the
     *                     first call to onIncrementalDecode after calling
     *                     onStartIncrementalDecode.
     *                     When true, this method may have to initialize the
     *                     frame, for example by filling or decoding the prior
     *                     frame.
     * @param opts         Options for decoding. May be different from
     *                     this->options() for decoding prior frames. Specifies
     *                     the frame to decode and whether the prior frame has
     *                     already been decoded to fDst. If not, and the frame
     *                     is not independent, this method will recursively
     *                     decode the frame it depends on.
     * @param rowsDecoded  Out-parameter to report the total number of rows
     *                     that have been decoded (or at least written to, if
     *                     it had to fill), including rows decoded by prior
     *                     calls to onIncrementalDecode.
     * @return             kSuccess if the frame is complete, kIncompleteInput
     *                     otherwise.
     */
    Result decodeFrame(bool firstAttempt, const Options& opts, int* rowsDecoded);

    /*
     *  Swizzles and color xforms (if necessary) into dst.
     */
    void applyXformRow(const SkImageInfo& dstInfo, void* dst, const uint8_t* src) const;

    /*
     * Creates an instance of the decoder
     * Called only by NewFromStream
     * Takes ownership of the SkGifImageReader
     */
    SkGifCodec(const SkEncodedInfo&, const SkImageInfo&, SkGifImageReader*);

    std::unique_ptr<SkGifImageReader>   fReader;
    std::unique_ptr<uint8_t[]>          fTmpBuffer;
    std::unique_ptr<SkSwizzler>         fSwizzler;
    sk_sp<SkColorTable>                 fCurrColorTable;
    // We may create a dummy table if there is not a Map in the input data. In
    // that case, we set this value to false, and we can skip a lot of decoding
    // work (which would not be meaningful anyway). We create a "fake"/"dummy"
    // one in that case, so the client and the swizzler have something to draw.
    bool                                fCurrColorTableIsReal;
    // Whether the background was filled.
    bool                                fFilledBackground;
    // True on the first call to onIncrementalDecode. This value is passed to
    // decodeFrame.
    bool                                fFirstCallToIncrementalDecode;

    void*                               fDst;
    size_t                              fDstRowBytes;

    // Updated inside haveDecodedRow when rows are decoded, unless we filled
    // the background, in which case it is set once and left alone.
    int                                 fRowsDecoded;
    std::unique_ptr<uint32_t[]>         fXformBuffer;

    typedef SkCodec INHERITED;
};
#endif  // SkGifCodec_DEFINED
