/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkColorTable.h"
#include "SkImageInfo.h"
#include "SkSwizzler.h"

#include "gif_lib.h"

/*
 *
 * This class implements the decoding for gif images
 *
 */
class SkGifCodec : public SkCodec {
public:

    /*
     * Checks the start of the stream to see if the image is a gif
     */
    static bool IsGif(SkStream*);

    /*
     * Assumes IsGif was called and returned true
     * Creates a gif decoder
     * Reads enough of the stream to determine the image format
     */
    static SkCodec* NewFromStream(SkStream*);

protected:

    /*
     * Read enough of the stream to initialize the SkGifCodec.
     * Returns a bool representing success or failure.
     *
     * @param codecOut
     * If it returned true, and codecOut was not nullptr,
     * codecOut will be set to a new SkGifCodec.
     *
     * @param gifOut
     * If it returned true, and codecOut was nullptr,
     * gifOut must be non-nullptr and gifOut will be set to a new
     * GifFileType pointer.
     *
     * @param stream
     * Deleted on failure.
     * codecOut will take ownership of it in the case where we created a codec.
     * Ownership is unchanged when we returned a gifOut.
     *
     */
    static bool ReadHeader(SkStream* stream, SkCodec** codecOut,
            GifFileType** gifOut);

    /*
     * Performs the full gif decode
     */
    Result onGetPixels(const SkImageInfo&, void*, size_t, const Options&,
            SkPMColor*, int*, int*) override;

    SkEncodedFormat onGetEncodedFormat() const override {
        return kGIF_SkEncodedFormat;
    }

    bool onRewind() override;

    uint32_t onGetFillValue(SkColorType colorType, SkAlphaType alphaType) const override;

    int onOutputScanline(int inputScanline) const override;

private:

    /*
     * A gif can contain multiple image frames.  We will only decode the first
     * frame.  This function reads up to the first image frame, processing
     * transparency and/or animation information that comes before the image
     * data.
     *
     * @param gif        Pointer to the library type that manages the gif decode
     * @param transIndex This call will set the transparent index based on the
     *                   extension data.
     */
     static Result ReadUpToFirstImage(GifFileType* gif, uint32_t* transIndex);

     /*
      * A gif may contain many image frames, all of different sizes.
      * This function checks if the gif dimensions are valid, based on the frame
      * dimensions, and corrects the gif dimensions if necessary.
      *
      * @param gif       Pointer to the library type that manages the gif decode
      * @param size      Size of the image that we will decode.
      *                  Will be set by this function if the return value is true.
      * @param frameRect Contains the dimenions and offset of the first image frame.
      *                  Will be set by this function if the return value is true.
      *
      * @return true on success, false otherwise
      */
     static bool GetDimensions(GifFileType* gif, SkISize* size, SkIRect* frameRect);

    /*
     * Initializes the color table that we will use for decoding.
     *
     * @param dstInfo         Contains the requested dst color type.
     * @param inputColorPtr   Copies the encoded color table to the client's
     *                        input color table if the client requests kIndex8.
     * @param inputColorCount If the client requests kIndex8, sets
     *                        inputColorCount to 256.  Since gifs always
     *                        contain 8-bit indices, we need a 256 entry color
     *                        table to ensure that indexing is always in
     *                        bounds.
     */
    void initializeColorTable(const SkImageInfo& dstInfo, SkPMColor* colorPtr,
            int* inputColorCount);

   /*
    * Checks for invalid inputs and calls setFrameDimensions(), and
    * initializeColorTable() in the proper sequence.
    */
    Result prepareToDecode(const SkImageInfo& dstInfo, SkPMColor* inputColorPtr,
            int* inputColorCount, const Options& opts);

    /*
     * Initializes the swizzler.
     *
     * @param dstInfo  Output image information.  Dimensions may have been
     *                 adjusted if the image frame size does not match the size
     *                 indicated in the header.
     * @param options  Informs the swizzler if destination memory is zero initialized.
     *                 Contains subset information.
     */
    Result initializeSwizzler(const SkImageInfo& dstInfo,
            const Options& options);

    SkSampler* getSampler(bool createIfNecessary) override {
        SkASSERT(fSwizzler);
        return fSwizzler;
    }

    /*
     * @return true if the read is successful and false if the read fails.
     */
    bool readRow();

    Result onStartScanlineDecode(const SkImageInfo& dstInfo, const Options& opts,
                   SkPMColor inputColorPtr[], int* inputColorCount) override;

    int onGetScanlines(void* dst, int count, size_t rowBytes) override;

    bool onSkipScanlines(int count) override;

    /*
     * For a scanline decode of "count" lines, this function indicates how
     * many of the "count" lines should be skipped until we reach the top of
     * the image frame and how many of the "count" lines are actually inside
     * the image frame.
     *
     * @param count           The number of scanlines requested.
     * @param rowsBeforeFrame Output variable.  The number of lines before
     *                        we reach the top of the image frame.
     * @param rowsInFrame     Output variable.  The number of lines to decode
     *                        inside the image frame.
     */
    void handleScanlineFrame(int count, int* rowsBeforeFrame, int* rowsInFrame);

    SkScanlineOrder onGetScanlineOrder() const override;

    /*
     * This function cleans up the gif object after the decode completes
     * It is used in a SkAutoTCallIProc template
     */
    static void CloseGif(GifFileType* gif);

    /*
     * Frees any extension data used in the decode
     * Used in a SkAutoTCallVProc
     */
    static void FreeExtension(SavedImage* image);

    /*
     * Creates an instance of the decoder
     * Called only by NewFromStream
     *
     * @param srcInfo contains the source width and height
     * @param stream the stream of image data
     * @param gif pointer to library type that manages gif decode
     *            takes ownership
     * @param transIndex  The transparent index.  An invalid value
     *            indicates that there is no transparent index.
     */
    SkGifCodec(const SkImageInfo& srcInfo, SkStream* stream, GifFileType* gif, uint32_t transIndex,
            const SkIRect& frameRect, bool frameIsSubset);

    SkAutoTCallVProc<GifFileType, CloseGif> fGif; // owned
    SkAutoTDeleteArray<uint8_t>             fSrcBuffer;
    const SkIRect                           fFrameRect;
    const uint32_t                          fTransIndex;
    uint32_t                                fFillIndex;
    const bool                              fFrameIsSubset;
    SkAutoTDelete<SkSwizzler>               fSwizzler;
    SkAutoTUnref<SkColorTable>              fColorTable;

    typedef SkCodec INHERITED;
};
