/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec_libgif.h"
#include "SkCodecPriv.h"
#include "SkColorPriv.h"
#include "SkColorTable.h"
#include "SkGifInterlaceIter.h"
#include "SkStream.h"
#include "SkSwizzler.h"
#include "SkUtils.h"

/*
 * Checks the start of the stream to see if the image is a gif
 */
bool SkGifCodec::IsGif(SkStream* stream) {
    char buf[GIF_STAMP_LEN];
    if (stream->read(buf, GIF_STAMP_LEN) == GIF_STAMP_LEN) {
        if (memcmp(GIF_STAMP,   buf, GIF_STAMP_LEN) == 0 ||
                memcmp(GIF87_STAMP, buf, GIF_STAMP_LEN) == 0 ||
                memcmp(GIF89_STAMP, buf, GIF_STAMP_LEN) == 0) {
            return true;
        }
    }
    return false;
}

/*
 * Warning reporting function
 */
static void gif_warning(const char* msg) {
    SkCodecPrintf("Gif Warning: %s\n", msg);
}

/*
 * Error function
 */
static SkCodec::Result gif_error(const char* msg,
        SkCodec::Result result = SkCodec::kInvalidInput) {
    SkCodecPrintf("Gif Error: %s\n", msg);
    return result;
}


/*
 * Read function that will be passed to gif_lib
 */
static int32_t read_bytes_callback(GifFileType* fileType, GifByteType* out,
        int32_t size) {
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
    return DGifOpen(stream, read_bytes_callback, NULL);
#endif
}

 /*
 * This function cleans up the gif object after the decode completes
 * It is used in a SkAutoTCallIProc template
 */
void SkGifCodec::CloseGif(GifFileType* gif) {
#if GIFLIB_MAJOR < 5 || (GIFLIB_MAJOR == 5 && GIFLIB_MINOR == 0)
    DGifCloseFile(gif);
#else
    DGifCloseFile(gif, NULL);
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
        if (GRAPHICS_EXT_FUNC_CODE == extBlock.Function &&
                extBlock.ByteCount >= 4) {

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

/*
 * Read enough of the stream to initialize the SkGifCodec.
 * Returns a bool representing success or failure.
 *
 * @param codecOut
 * If it returned true, and codecOut was not NULL,
 * codecOut will be set to a new SkGifCodec.
 *
 * @param gifOut
 * If it returned true, and codecOut was NULL,
 * gifOut must be non-NULL and gifOut will be set to a new
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

    if (NULL == gif) {
        gif_error("DGifOpen failed.\n");
        return false;
    }

    if (NULL != codecOut) {
        // Get fields from header
        const int32_t width = gif->SWidth;
        const int32_t height = gif->SHeight;
        if (width <= 0 || height <= 0) {
            gif_error("Invalid dimensions.\n");
            return false;
        }

        // Return the codec
        // kIndex is the most natural color type for gifs, so we set this as
        // the default.
        // Many gifs specify a color table index for transparent pixels.  Every
        // other pixel is guaranteed to be opaque.  Despite this, because of the
        // possiblity of transparent pixels, we cannot assume that the image is
        // opaque.  We have the option to set the alpha type as kPremul or
        // kUnpremul.  Both are valid since the alpha component will always be
        // 0xFF or the entire 32-bit pixel will be set to zero.  We prefer
        // kPremul because we support kPremul, and it is more efficient to
        // use kPremul directly even when kUnpremul is supported.
        const SkImageInfo& imageInfo = SkImageInfo::Make(width, height,
                kIndex_8_SkColorType, kPremul_SkAlphaType);
        *codecOut = SkNEW_ARGS(SkGifCodec, (imageInfo, streamDeleter.detach(), gif.detach()));
    } else {
        SkASSERT(NULL != gifOut);
        streamDeleter.detach();
        *gifOut = gif.detach();
    }
    return true;
}

/*
 * Assumes IsGif was called and returned true
 * Creates a gif decoder
 * Reads enough of the stream to determine the image format
 */
SkCodec* SkGifCodec::NewFromStream(SkStream* stream) {
    SkCodec* codec = NULL;
    if (ReadHeader(stream, &codec, NULL)) {
        return codec;
    }
    return NULL;
}

SkGifCodec::SkGifCodec(const SkImageInfo& srcInfo, SkStream* stream,
                       GifFileType* gif)
    : INHERITED(srcInfo, stream)
    , fGif(gif)
{}

/*
 * Checks if the conversion between the input image and the requested output
 * image has been implemented
 */
static bool conversion_possible(const SkImageInfo& dst,
                                const SkImageInfo& src) {
    // Ensure that the profile type is unchanged
    if (dst.profileType() != src.profileType()) {
        return false;
    }

    // Check for supported color and alpha types
    switch (dst.colorType()) {
        case kN32_SkColorType:
            return kPremul_SkAlphaType == dst.alphaType() ||
                    kUnpremul_SkAlphaType == dst.alphaType();
        case kIndex_8_SkColorType:
            return kPremul_SkAlphaType == dst.alphaType() ||
                    kUnpremul_SkAlphaType == dst.alphaType();
        default:
            return false;
    }
}

/*
 * Initiates the gif decode
 */
SkCodec::Result SkGifCodec::onGetPixels(const SkImageInfo& dstInfo,
                                        void* dst, size_t dstRowBytes,
                                        const Options& opts,
                                        SkPMColor* inputColorPtr,
                                        int* inputColorCount) {
    // Rewind if necessary
    SkCodec::RewindState rewindState = this->rewindIfNeeded();
    if (rewindState == kCouldNotRewind_RewindState) {
        return kCouldNotRewind;
    } else if (rewindState == kRewound_RewindState) {
        GifFileType* gifOut = NULL;
        if (!ReadHeader(this->stream(), NULL, &gifOut)) {
            return kCouldNotRewind;
        } else {
            SkASSERT(NULL != gifOut);
            fGif.reset(gifOut);
        }
    }

    // Check for valid input parameters
    if (opts.fSubset) {
        // Subsets are not supported.
        return kUnimplemented;
    }
    if (dstInfo.dimensions() != this->getInfo().dimensions()) {
        return gif_error("Scaling not supported.\n", kInvalidScale);
    }
    if (!conversion_possible(dstInfo, this->getInfo())) {
        return gif_error("Cannot convert input type to output type.\n",
                kInvalidConversion);
    }

    // Use this as a container to hold information about any gif extension
    // blocks.  This generally stores transparency and animation instructions.
    SavedImage saveExt;
    SkAutoTCallVProc<SavedImage, FreeExtension> autoFreeExt(&saveExt);
    saveExt.ExtensionBlocks = NULL;
    saveExt.ExtensionBlockCount = 0;
    GifByteType* extData;
#if GIFLIB_MAJOR >= 5
    int32_t extFunction;
#endif

    // We will loop over components of gif images until we find an image.  Once
    // we find an image, we will decode and return it.  While many gif files
    // contain more than one image, we will simply decode the first image.
    const int32_t width = dstInfo.width();
    const int32_t height = dstInfo.height();
    GifRecordType recordType;
    do {
        // Get the current record type
        if (GIF_ERROR == DGifGetRecordType(fGif, &recordType)) {
            return gif_error("DGifGetRecordType failed.\n", kInvalidInput);
        }

        switch (recordType) {
            case IMAGE_DESC_RECORD_TYPE: {
                // Read the image descriptor
                if (GIF_ERROR == DGifGetImageDesc(fGif)) {
                    return gif_error("DGifGetImageDesc failed.\n",
                            kInvalidInput);
                }

                // If reading the image descriptor is successful, the image
                // count will be incremented
                SkASSERT(fGif->ImageCount >= 1);
                SavedImage* image = &fGif->SavedImages[fGif->ImageCount - 1];

                // Process the descriptor
                const GifImageDesc& desc = image->ImageDesc;
                int32_t imageLeft = desc.Left;
                int32_t imageTop = desc.Top;
                int32_t innerWidth = desc.Width;
                int32_t innerHeight = desc.Height;
                // Fail on non-positive dimensions
                if (innerWidth <= 0 || innerHeight <= 0) {
                    return gif_error("Invalid dimensions for inner image.\n",
                            kInvalidInput);
                }
                // Treat the following cases as warnings and try to fix
                if (innerWidth > width) {
                    gif_warning("Inner image too wide, shrinking.\n");
                    innerWidth = width;
                    imageLeft = 0;
                } else if (imageLeft + innerWidth > width) {
                    gif_warning("Shifting inner image to left to fit.\n");
                    imageLeft = width - innerWidth;
                } else if (imageLeft < 0) {
                    gif_warning("Shifting image to right to fit\n");
                    imageLeft = 0;
                }
                if (innerHeight > height) {
                    gif_warning("Inner image too tall, shrinking.\n");
                    innerHeight = height;
                    imageTop = 0;
                } else if (imageTop + innerHeight > height) {
                    gif_warning("Shifting inner image up to fit.\n");
                    imageTop = height - innerHeight;
                } else if (imageTop < 0) {
                    gif_warning("Shifting image down to fit\n");
                    imageTop = 0;
                }

                // Create a color table to store colors the giflib colorMap
                SkPMColor alternateColorPtr[256];
                SkPMColor* colorTable;
                SkColorType dstColorType = dstInfo.colorType();
                if (kIndex_8_SkColorType == dstColorType) {
                    SkASSERT(NULL != inputColorPtr);
                    SkASSERT(NULL != inputColorCount);
                    colorTable = inputColorPtr;
                } else {
                    colorTable = alternateColorPtr;
                }

                // Set up the color table
                uint32_t colorCount = 0;
                // Allocate maximum storage to deal with invalid indices safely
                const uint32_t maxColors = 256;
                ColorMapObject* colorMap = fGif->Image.ColorMap;
                // If there is no local color table, use the global color table
                if (NULL == colorMap) {
                    colorMap = fGif->SColorMap;
                }
                if (NULL != colorMap) {
                    colorCount = colorMap->ColorCount;
                    SkASSERT(colorCount ==
                            (unsigned) (1 << (colorMap->BitsPerPixel)));
                    SkASSERT(colorCount <= 256);
                    for (uint32_t i = 0; i < colorCount; i++) {
                        colorTable[i] = SkPackARGB32(0xFF,
                                                     colorMap->Colors[i].Red,
                                                     colorMap->Colors[i].Green,
                                                     colorMap->Colors[i].Blue);
                    }
                }

                // This is used to fill unspecified pixels in the image data.
                uint32_t fillIndex = fGif->SBackGroundColor;
                ZeroInitialized zeroInit = opts.fZeroInitialized;

                // Gifs have the option to specify the color at a single
                // index of the color table as transparent.
                {
                    // Get the transparent index.  If the return value of this
                    // function is greater than the colorCount, we know that
                    // there is no valid transparent color in the color table.
                    // This occurs if there is no graphics control extension or
                    // if the index specified by the graphics control extension
                    // is out of range.
                    uint32_t transIndex = find_trans_index(saveExt);

                    if (transIndex < colorCount) {
                        colorTable[transIndex] = SK_ColorTRANSPARENT;
                        // If there is a transparent index, we also use this as
                        // the fill index.
                        fillIndex = transIndex;
                    } else if (fillIndex >= colorCount) {
                        // If the fill index is invalid, we default to 0.  This
                        // behavior is unspecified but matches SkImageDecoder.
                        fillIndex = 0;
                    }
                }

                // Check if we can skip filling the background of the image.  We
                // may be able to if the memory is zero initialized.
                bool skipBackground =
                        ((kN32_SkColorType == dstColorType && colorTable[fillIndex] == 0) ||
                        (kIndex_8_SkColorType == dstColorType && fillIndex == 0)) &&
                        kYes_ZeroInitialized == zeroInit;


                // Fill in the color table for indices greater than color count.
                // This allows for predictable, safe behavior.
                for (uint32_t i = colorCount; i < maxColors; i++) {
                    colorTable[i] = colorTable[fillIndex];
                } 

                // Check if image is only a subset of the image frame
                SkAutoTDelete<SkSwizzler> swizzler(NULL);
                if (innerWidth < width || innerHeight < height) {

                    // Modify the destination info
                    const SkImageInfo subsetDstInfo =
                            dstInfo.makeWH(innerWidth, innerHeight);

                    // Fill the destination with the fill color
                    // FIXME: This may not be the behavior that we want for
                    //        animated gifs where we draw on top of the
                    //        previous frame.
                    if (!skipBackground) {
                        SkSwizzler::Fill(dst, dstInfo, dstRowBytes, height,
                                fillIndex, colorTable);
                    }

                    // Modify the dst pointer
                    const int32_t dstBytesPerPixel =
                            SkColorTypeBytesPerPixel(dstColorType);
                    dst = SkTAddOffset<void*>(dst,
                            dstRowBytes * imageTop +
                            dstBytesPerPixel * imageLeft);

                    // Create the subset swizzler
                    swizzler.reset(SkSwizzler::CreateSwizzler(
                            SkSwizzler::kIndex, colorTable, subsetDstInfo,
                            zeroInit));
                } else {
                    // Create the fully dimensional swizzler
                    swizzler.reset(SkSwizzler::CreateSwizzler(
                            SkSwizzler::kIndex, colorTable, dstInfo, zeroInit));
                }

                // Stores output from dgiflib and input to the swizzler
                SkAutoTDeleteArray<uint8_t>
                        buffer(SkNEW_ARRAY(uint8_t, innerWidth));

                // Check the interlace flag and iterate over rows of the input
                if (fGif->Image.Interlace) {
                    // In interlace mode, the rows of input are rearranged in
                    // the output image.  We use an iterator to take care of
                    // the rearranging.
                    SkGifInterlaceIter iter(innerHeight);
                    for (int32_t y = 0; y < innerHeight; y++) {
                        if (GIF_ERROR == DGifGetLine(fGif, buffer.get(),
                                innerWidth)) {
                            // Recover from error by filling remainder of image
                            if (!skipBackground) {
                                memset(buffer.get(), fillIndex, innerWidth);
                                for (; y < innerHeight; y++) {
                                    void* dstRow = SkTAddOffset<void>(dst,
                                            dstRowBytes * iter.nextY());
                                    swizzler->swizzle(dstRow, buffer.get());
                                }
                            }
                            return gif_error(SkStringPrintf(
                                    "Could not decode line %d of %d.\n",
                                    y, height - 1).c_str(), kIncompleteInput);
                        }
                        void* dstRow = SkTAddOffset<void>(
                                dst, dstRowBytes * iter.nextY());
                        swizzler->swizzle(dstRow, buffer.get());
                    }
                } else {
                    // Standard mode
                    void* dstRow = dst;
                    for (int32_t y = 0; y < innerHeight; y++) {
                        if (GIF_ERROR == DGifGetLine(fGif, buffer.get(),
                                innerWidth)) {
                            if (!skipBackground) {
                                SkSwizzler::Fill(dstRow, dstInfo, dstRowBytes,
                                        innerHeight - y, fillIndex, colorTable);
                            }
                            return gif_error(SkStringPrintf(
                                    "Could not decode line %d of %d.\n",
                                    y, height - 1).c_str(), kIncompleteInput);
                        }
                        swizzler->swizzle(dstRow, buffer.get());
                        dstRow = SkTAddOffset<void>(dstRow, dstRowBytes);
                    }
                }

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
#if GIFLIB_MAJOR < 5
                if (GIF_ERROR ==
                        DGifGetExtension(fGif, &saveExt.Function, &extData)) {
#else
                if (GIF_ERROR ==
                        DGifGetExtension(fGif, &extFunction, &extData)) {
#endif
                    return gif_error("Could not get extension.\n",
                            kIncompleteInput);
                }

                // Create an extension block with our data
                while (NULL != extData) {
                    // Add a single block
#if GIFLIB_MAJOR < 5
                    if (GIF_ERROR == AddExtensionBlock(&saveExt, extData[0],
                            &extData[1])) {
#else
                    if (GIF_ERROR ==
                            GifAddExtensionBlock(&saveExt.ExtensionBlockCount,
                            &saveExt.ExtensionBlocks, extFunction, extData[0],
                            &extData[1])) {
#endif
                        return gif_error("Could not add extension block.\n",
                                kIncompleteInput);
                    }
                    // Move to the next block
                    if (GIF_ERROR == DGifGetExtensionNext(fGif, &extData)) {
                        return gif_error("Could not get next extension.\n",
                                kIncompleteInput);
                    }
#if GIFLIB_MAJOR < 5
                    saveExt.Function = 0;
#endif
                }
                break;

            // Signals the end of the gif file
            case TERMINATE_RECORD_TYPE:
                break;

            default:
                // giflib returns an error code if the record type is not known.
                // We should catch this error immediately.
                SkASSERT(false);
                break;
        }
    } while (TERMINATE_RECORD_TYPE != recordType);

    return gif_error("Could not find any images to decode in gif file.\n",
            kInvalidInput);
}
