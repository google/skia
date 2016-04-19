/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodecPriv.h"
#include "SkColorPriv.h"
#include "SkColorTable.h"
#include "SkGifCodec.h"
#include "SkStream.h"
#include "SkSwizzler.h"
#include "SkUtils.h"

#include "gif_lib.h"

/*
 * Checks the start of the stream to see if the image is a gif
 */
bool SkGifCodec::IsGif(const void* buf, size_t bytesRead) {
    if (bytesRead >= GIF_STAMP_LEN) {
        if (memcmp(GIF_STAMP,   buf, GIF_STAMP_LEN) == 0 ||
            memcmp(GIF87_STAMP, buf, GIF_STAMP_LEN) == 0 ||
            memcmp(GIF89_STAMP, buf, GIF_STAMP_LEN) == 0)
        {
            return true;
        }
    }
    return false;
}

/*
 * Error function
 */
static SkCodec::Result gif_error(const char* msg, SkCodec::Result result = SkCodec::kInvalidInput) {
    SkCodecPrintf("Gif Error: %s\n", msg);
    return result;
}


/*
 * Read function that will be passed to gif_lib
 */
static int32_t read_bytes_callback(GifFileType* fileType, GifByteType* out, int32_t size) {
    SkStream* stream = (SkStream*) fileType->UserData;
    return (int32_t) stream->read(out, size);
}

/*
 * Open the gif file
 */
static GifFileType* open_gif(SkStream* stream) {
#if GIFLIB_MAJOR < 5
    return DGifOpen(stream, read_bytes_callback);
#else
    return DGifOpen(stream, read_bytes_callback, nullptr);
#endif
}

/*
 * Check if a there is an index of the color table for a transparent pixel
 */
static uint32_t find_trans_index(const SavedImage& image) {
    // If there is a transparent index specified, it will be contained in an
    // extension block.  We will loop through extension blocks in reverse order
    // to check the most recent extension blocks first.
    for (int32_t i = image.ExtensionBlockCount - 1; i >= 0; i--) {
        // Get an extension block
        const ExtensionBlock& extBlock = image.ExtensionBlocks[i];

        // Specifically, we need to check for a graphics control extension,
        // which may contain transparency information.  Also, note that a valid
        // graphics control extension is always four bytes.  The fourth byte
        // is the transparent index (if it exists), so we need at least four
        // bytes.
        if (GRAPHICS_EXT_FUNC_CODE == extBlock.Function && extBlock.ByteCount >= 4) {
            // Check the transparent color flag which indicates whether a
            // transparent index exists.  It is the least significant bit of
            // the first byte of the extension block.
            if (1 == (extBlock.Bytes[0] & 1)) {
                // Use uint32_t to prevent sign extending
                return extBlock.Bytes[3];
            }

            // There should only be one graphics control extension for the image frame
            break;
        }
    }

    // Use maximum unsigned int (surely an invalid index) to indicate that a valid
    // index was not found.
    return SK_MaxU32;
}

inline uint32_t ceil_div(uint32_t a, uint32_t b) {
    return (a + b - 1) / b;
}

/*
 * Gets the output row corresponding to the encoded row for interlaced gifs
 */
inline uint32_t get_output_row_interlaced(uint32_t encodedRow, uint32_t height) {
    SkASSERT(encodedRow < height);
    // First pass
    if (encodedRow * 8 < height) {
        return encodedRow * 8;
    }
    // Second pass
    if (encodedRow * 4 < height) {
        return 4 + 8 * (encodedRow - ceil_div(height, 8));
    }
    // Third pass
    if (encodedRow * 2 < height) {
        return 2 + 4 * (encodedRow - ceil_div(height, 4));
    }
    // Fourth pass
    return 1 + 2 * (encodedRow - ceil_div(height, 2));
}

/*
 * This function cleans up the gif object after the decode completes
 * It is used in a SkAutoTCallIProc template
 */
void SkGifCodec::CloseGif(GifFileType* gif) {
#if GIFLIB_MAJOR < 5 || (GIFLIB_MAJOR == 5 && GIFLIB_MINOR == 0)
    DGifCloseFile(gif);
#else
    DGifCloseFile(gif, nullptr);
#endif
}

/*
 * This function free extension data that has been saved to assist the image
 * decoder
 */
void SkGifCodec::FreeExtension(SavedImage* image) {
    if (NULL != image->ExtensionBlocks) {
#if GIFLIB_MAJOR < 5
        FreeExtension(image);
#else
        GifFreeExtensions(&image->ExtensionBlockCount, &image->ExtensionBlocks);
#endif
    }
}

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
bool SkGifCodec::ReadHeader(SkStream* stream, SkCodec** codecOut, GifFileType** gifOut) {
    SkAutoTDelete<SkStream> streamDeleter(stream);

    // Read gif header, logical screen descriptor, and global color table
    SkAutoTCallVProc<GifFileType, CloseGif> gif(open_gif(stream));

    if (nullptr == gif) {
        gif_error("DGifOpen failed.\n");
        return false;
    }

    // Read through gif extensions to get to the image data.  Set the
    // transparent index based on the extension data.
    uint32_t transIndex;
    SkCodec::Result result = ReadUpToFirstImage(gif, &transIndex);
    if (kSuccess != result){
        return false;
    }

    // Read the image descriptor
    if (GIF_ERROR == DGifGetImageDesc(gif)) {
        return false;
    }
    // If reading the image descriptor is successful, the image count will be
    // incremented.
    SkASSERT(gif->ImageCount >= 1);

    if (nullptr != codecOut) {
        SkISize size;
        SkIRect frameRect;
        if (!GetDimensions(gif, &size, &frameRect)) {
            gif_error("Invalid gif size.\n");
            return false;
        }
        bool frameIsSubset = (size != frameRect.size());

        // Determine the recommended alpha type.  The transIndex might be valid if it less
        // than 256.  We are not certain that the index is valid until we process the color
        // table, since some gifs have color tables with less than 256 colors.  If
        // there might be a valid transparent index, we must indicate that the image has
        // alpha.
        // In the case where we must support alpha, we have the option to set the
        // suggested alpha type to kPremul or kUnpremul.  Both are valid since the alpha
        // component will always be 0xFF or the entire 32-bit pixel will be set to zero.
        // We prefer kPremul because we support kPremul, and it is more efficient to use
        // kPremul directly even when kUnpremul is supported.
        SkAlphaType alphaType = (transIndex < 256) ? kPremul_SkAlphaType : kOpaque_SkAlphaType;

        // Return the codec
        // kIndex is the most natural color type for gifs, so we set this as
        // the default.
        SkImageInfo imageInfo = SkImageInfo::Make(size.width(), size.height(), kIndex_8_SkColorType,
                alphaType);
        *codecOut = new SkGifCodec(imageInfo, streamDeleter.release(), gif.release(), transIndex,
                frameRect, frameIsSubset);
    } else {
        SkASSERT(nullptr != gifOut);
        streamDeleter.release();
        *gifOut = gif.release();
    }
    return true;
}

/*
 * Assumes IsGif was called and returned true
 * Creates a gif decoder
 * Reads enough of the stream to determine the image format
 */
SkCodec* SkGifCodec::NewFromStream(SkStream* stream) {
    SkCodec* codec = nullptr;
    if (ReadHeader(stream, &codec, nullptr)) {
        return codec;
    }
    return nullptr;
}

SkGifCodec::SkGifCodec(const SkImageInfo& srcInfo, SkStream* stream, GifFileType* gif,
        uint32_t transIndex, const SkIRect& frameRect, bool frameIsSubset)
    : INHERITED(srcInfo, stream)
    , fGif(gif)
    , fSrcBuffer(new uint8_t[this->getInfo().width()])
    , fFrameRect(frameRect)
    // If it is valid, fTransIndex will be used to set fFillIndex.  We don't know if
    // fTransIndex is valid until we process the color table, since fTransIndex may
    // be greater than the size of the color table.
    , fTransIndex(transIndex)
    // Default fFillIndex is 0.  We will overwrite this if fTransIndex is valid, or if
    // there is a valid background color.
    , fFillIndex(0)
    , fFrameIsSubset(frameIsSubset)
    , fSwizzler(NULL)
    , fColorTable(NULL)
{}

bool SkGifCodec::onRewind() {
    GifFileType* gifOut = nullptr;
    if (!ReadHeader(this->stream(), nullptr, &gifOut)) {
        return false;
    }

    SkASSERT(nullptr != gifOut);
    fGif.reset(gifOut);
    return true;
}

SkCodec::Result SkGifCodec::ReadUpToFirstImage(GifFileType* gif, uint32_t* transIndex) {
    // Use this as a container to hold information about any gif extension
    // blocks.  This generally stores transparency and animation instructions.
    SavedImage saveExt;
    SkAutoTCallVProc<SavedImage, FreeExtension> autoFreeExt(&saveExt);
    saveExt.ExtensionBlocks = nullptr;
    saveExt.ExtensionBlockCount = 0;
    GifByteType* extData;
    int32_t extFunction;

    // We will loop over components of gif images until we find an image.  Once
    // we find an image, we will decode and return it.  While many gif files
    // contain more than one image, we will simply decode the first image.
    GifRecordType recordType;
    do {
        // Get the current record type
        if (GIF_ERROR == DGifGetRecordType(gif, &recordType)) {
            return gif_error("DGifGetRecordType failed.\n", kInvalidInput);
        }
        switch (recordType) {
            case IMAGE_DESC_RECORD_TYPE: {
                *transIndex = find_trans_index(saveExt);

                // FIXME: Gif files may have multiple images stored in a single
                //        file.  This is most commonly used to enable
                //        animations.  Since we are leaving animated gifs as a
                //        TODO, we will return kSuccess after decoding the
                //        first image in the file.  This is the same behavior
                //        as SkImageDecoder_libgif.
                //
                //        Most times this works pretty well, but sometimes it
                //        doesn't.  For example, I have an animated test image
                //        where the first image in the file is 1x1, but the
                //        subsequent images are meaningful.  This currently
                //        displays the 1x1 image, which is not ideal.  Right
                //        now I am leaving this as an issue that will be
                //        addressed when we implement animated gifs.
                //
                //        It is also possible (not explicitly disallowed in the
                //        specification) that gif files provide multiple
                //        images in a single file that are all meant to be
                //        displayed in the same frame together.  I will
                //        currently leave this unimplemented until I find a
                //        test case that expects this behavior.
                return kSuccess;
            }
            // Extensions are used to specify special properties of the image
            // such as transparency or animation.
            case EXTENSION_RECORD_TYPE:
                // Read extension data
                if (GIF_ERROR == DGifGetExtension(gif, &extFunction, &extData)) {
                    return gif_error("Could not get extension.\n", kIncompleteInput);
                }

                // Create an extension block with our data
                while (nullptr != extData) {
                    // Add a single block

#if GIFLIB_MAJOR < 5
                    if (AddExtensionBlock(&saveExt, extData[0],
                                          &extData[1]) == GIF_ERROR) {
#else
                    if (GIF_ERROR == GifAddExtensionBlock(&saveExt.ExtensionBlockCount,
                                                          &saveExt.ExtensionBlocks,
                                                          extFunction, extData[0], &extData[1])) {
#endif
                        return gif_error("Could not add extension block.\n", kIncompleteInput);
                    }
                    // Move to the next block
                    if (GIF_ERROR == DGifGetExtensionNext(gif, &extData)) {
                        return gif_error("Could not get next extension.\n", kIncompleteInput);
                    }
                }
                break;

            // Signals the end of the gif file
            case TERMINATE_RECORD_TYPE:
                break;

            default:
                // DGifGetRecordType returns an error if the record type does
                // not match one of the above cases.  This should not be
                // reached.
                SkASSERT(false);
                break;
        }
    } while (TERMINATE_RECORD_TYPE != recordType);

    return gif_error("Could not find any images to decode in gif file.\n", kInvalidInput);
}

bool SkGifCodec::GetDimensions(GifFileType* gif, SkISize* size, SkIRect* frameRect) {
    // Get the encoded dimension values
    SavedImage* image = &gif->SavedImages[gif->ImageCount - 1];
    const GifImageDesc& desc = image->ImageDesc;
    int frameLeft = desc.Left;
    int frameTop = desc.Top;
    int frameWidth = desc.Width;
    int frameHeight = desc.Height;
    int width = gif->SWidth;
    int height = gif->SHeight;

    // Ensure that the decode dimensions are large enough to contain the frame
    width = SkTMax(width, frameWidth + frameLeft);
    height = SkTMax(height, frameHeight + frameTop);

    // All of these dimensions should be positive, as they are encoded as unsigned 16-bit integers.
    // It is unclear why giflib casts them to ints.  We will go ahead and check that they are
    // in fact positive.
    if (frameLeft < 0 || frameTop < 0 || frameWidth < 0 || frameHeight < 0 || width <= 0 ||
            height <= 0) {
        return false;
    }

    frameRect->setXYWH(frameLeft, frameTop, frameWidth, frameHeight);
    size->set(width, height);
    return true;
}

void SkGifCodec::initializeColorTable(const SkImageInfo& dstInfo, SkPMColor* inputColorPtr,
        int* inputColorCount) {
    // Set up our own color table
    const uint32_t maxColors = 256;
    SkPMColor colorPtr[256];
    if (NULL != inputColorCount) {
        // We set the number of colors to maxColors in order to ensure
        // safe memory accesses.  Otherwise, an invalid pixel could
        // access memory outside of our color table array.
        *inputColorCount = maxColors;
    }

    // Get local color table
    ColorMapObject* colorMap = fGif->Image.ColorMap;
    // If there is no local color table, use the global color table
    if (NULL == colorMap) {
        colorMap = fGif->SColorMap;
    }

    uint32_t colorCount = 0;
    if (NULL != colorMap) {
        colorCount = colorMap->ColorCount;
        // giflib guarantees these properties
        SkASSERT(colorCount == (unsigned) (1 << (colorMap->BitsPerPixel)));
        SkASSERT(colorCount <= 256);
        for (uint32_t i = 0; i < colorCount; i++) {
            colorPtr[i] = SkPackARGB32(0xFF, colorMap->Colors[i].Red,
                    colorMap->Colors[i].Green, colorMap->Colors[i].Blue);
        }
    }

    // Fill in the color table for indices greater than color count.
    // This allows for predictable, safe behavior.
    if (colorCount > 0) {
        // Gifs have the option to specify the color at a single index of the color
        // table as transparent.  If the transparent index is greater than the
        // colorCount, we know that there is no valid transparent color in the color
        // table.  If there is not valid transparent index, we will try to use the
        // backgroundIndex as the fill index.  If the backgroundIndex is also not
        // valid, we will let fFillIndex default to 0 (it is set to zero in the
        // constructor).  This behavior is not specified but matches
        // SkImageDecoder_libgif.
        uint32_t backgroundIndex = fGif->SBackGroundColor;
        if (fTransIndex < colorCount) {
            colorPtr[fTransIndex] = SK_ColorTRANSPARENT;
            fFillIndex = fTransIndex;
        } else if (backgroundIndex < colorCount) {
            fFillIndex = backgroundIndex;
        }

        for (uint32_t i = colorCount; i < maxColors; i++) {
            colorPtr[i] = colorPtr[fFillIndex];
        }
    } else {
        sk_memset32(colorPtr, 0xFF000000, maxColors);
    }

    fColorTable.reset(new SkColorTable(colorPtr, maxColors));
    copy_color_table(dstInfo, this->fColorTable, inputColorPtr, inputColorCount);
}

SkCodec::Result SkGifCodec::prepareToDecode(const SkImageInfo& dstInfo, SkPMColor* inputColorPtr,
        int* inputColorCount, const Options& opts) {
    // Check for valid input parameters
    if (!conversion_possible(dstInfo, this->getInfo())) {
        return gif_error("Cannot convert input type to output type.\n",
                kInvalidConversion);
    }

    // Initialize color table and copy to the client if necessary
    this->initializeColorTable(dstInfo, inputColorPtr, inputColorCount);

    this->initializeSwizzler(dstInfo, opts);
    return kSuccess;
}

void SkGifCodec::initializeSwizzler(const SkImageInfo& dstInfo, const Options& opts) {
    const SkPMColor* colorPtr = get_color_ptr(fColorTable.get());
    const SkIRect* frameRect = fFrameIsSubset ? &fFrameRect : nullptr;
    fSwizzler.reset(SkSwizzler::CreateSwizzler(SkSwizzler::kIndex, colorPtr, dstInfo, opts,
            frameRect));
    SkASSERT(fSwizzler);
}

bool SkGifCodec::readRow() {
    return GIF_ERROR != DGifGetLine(fGif, fSrcBuffer.get(), fFrameRect.width());
}

/*
 * Initiates the gif decode
 */
SkCodec::Result SkGifCodec::onGetPixels(const SkImageInfo& dstInfo,
                                        void* dst, size_t dstRowBytes,
                                        const Options& opts,
                                        SkPMColor* inputColorPtr,
                                        int* inputColorCount,
                                        int* rowsDecoded) {
    Result result = this->prepareToDecode(dstInfo, inputColorPtr, inputColorCount, opts);
    if (kSuccess != result) {
        return result;
    }

    if (dstInfo.dimensions() != this->getInfo().dimensions()) {
        return gif_error("Scaling not supported.\n", kInvalidScale);
    }

    // Initialize the swizzler
    if (fFrameIsSubset) {
        // Fill the background
        SkSampler::Fill(dstInfo, dst, dstRowBytes, this->getFillValue(dstInfo.colorType()),
                opts.fZeroInitialized);
    }

    // Iterate over rows of the input
    for (int y = fFrameRect.top(); y < fFrameRect.bottom(); y++) {
        if (!this->readRow()) {
            *rowsDecoded = y;
            return gif_error("Could not decode line.\n", kIncompleteInput);
        }
        void* dstRow = SkTAddOffset<void>(dst, dstRowBytes * this->outputScanline(y));
        fSwizzler->swizzle(dstRow, fSrcBuffer.get());
    }
    return kSuccess;
}

// FIXME: This is similar to the implementation for bmp and png.  Can we share more code or
//        possibly make this non-virtual?
uint32_t SkGifCodec::onGetFillValue(SkColorType colorType) const {
    const SkPMColor* colorPtr = get_color_ptr(fColorTable.get());
    return get_color_table_fill_value(colorType, colorPtr, fFillIndex);
}

SkCodec::Result SkGifCodec::onStartScanlineDecode(const SkImageInfo& dstInfo,
        const SkCodec::Options& opts, SkPMColor inputColorPtr[], int* inputColorCount) {
    return this->prepareToDecode(dstInfo, inputColorPtr, inputColorCount, this->options());
}

void SkGifCodec::handleScanlineFrame(int count, int* rowsBeforeFrame, int* rowsInFrame) {
    if (fFrameIsSubset) {
        const int currRow = this->currScanline();

        // The number of rows that remain to be skipped before reaching rows that we
        // actually must decode into.
        // This must be at least zero.  We also make sure that it is less than or
        // equal to count, since we will skip at most count rows.
        *rowsBeforeFrame = SkTMin(count, SkTMax(0, fFrameRect.top() - currRow));

        // Rows left to decode once we reach the start of the frame.
        const int rowsLeft = count - *rowsBeforeFrame;

        // Count the number of that extend beyond the bottom of the frame.  We do not
        // need to decode into these rows.
        const int rowsAfterFrame = SkTMax(0, currRow + rowsLeft - fFrameRect.bottom());

        // Set the actual number of source rows that we need to decode.
        *rowsInFrame = rowsLeft - rowsAfterFrame;
    } else {
        *rowsBeforeFrame = 0;
        *rowsInFrame = count;
    }
}

int SkGifCodec::onGetScanlines(void* dst, int count, size_t rowBytes) {
    int rowsBeforeFrame;
    int rowsInFrame;
    this->handleScanlineFrame(count, &rowsBeforeFrame, &rowsInFrame);

    if (fFrameIsSubset) {
        // Fill the requested rows
        SkImageInfo fillInfo = this->dstInfo().makeWH(this->dstInfo().width(), count);
        uint32_t fillValue = this->onGetFillValue(this->dstInfo().colorType());
        fSwizzler->fill(fillInfo, dst, rowBytes, fillValue, this->options().fZeroInitialized);

        // Start to write pixels at the start of the image frame
        dst = SkTAddOffset<void>(dst, rowBytes * rowsBeforeFrame);
    }

    for (int i = 0; i < rowsInFrame; i++) {
        if (!this->readRow()) {
            return i + rowsBeforeFrame;
        }
        fSwizzler->swizzle(dst, fSrcBuffer.get());
        dst = SkTAddOffset<void>(dst, rowBytes);
    }

    return count;
}

bool SkGifCodec::onSkipScanlines(int count) {
    int rowsBeforeFrame;
    int rowsInFrame;
    this->handleScanlineFrame(count, &rowsBeforeFrame, &rowsInFrame);

    for (int i = 0; i < rowsInFrame; i++) {
        if (!this->readRow()) {
            return false;
        }
    }

    return true;
}

SkCodec::SkScanlineOrder SkGifCodec::onGetScanlineOrder() const {
    if (fGif->Image.Interlace) {
        return kOutOfOrder_SkScanlineOrder;
    }
    return kTopDown_SkScanlineOrder;
}

int SkGifCodec::onOutputScanline(int inputScanline) const {
    if (fGif->Image.Interlace) {
        if (inputScanline < fFrameRect.top() || inputScanline >= fFrameRect.bottom()) {
            return inputScanline;
        }
        return get_output_row_interlaced(inputScanline - fFrameRect.top(), fFrameRect.height()) +
                fFrameRect.top();
    }
    return inputScanline;
}
