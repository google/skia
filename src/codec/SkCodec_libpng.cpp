/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec_libpng.h"
#include "SkCodecPriv.h"
#include "SkColorPriv.h"
#include "SkColorTable.h"
#include "SkBitmap.h"
#include "SkMath.h"
#include "SkSize.h"
#include "SkStream.h"
#include "SkSwizzler.h"

///////////////////////////////////////////////////////////////////////////////
// Helper macros
///////////////////////////////////////////////////////////////////////////////

#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

/* These were dropped in libpng >= 1.4 */
#ifndef png_infopp_NULL
    #define png_infopp_NULL nullptr
#endif

#ifndef png_bytepp_NULL
    #define png_bytepp_NULL nullptr
#endif

#ifndef int_p_NULL
    #define int_p_NULL nullptr
#endif

#ifndef png_flush_ptr_NULL
    #define png_flush_ptr_NULL nullptr
#endif

///////////////////////////////////////////////////////////////////////////////
// Callback functions
///////////////////////////////////////////////////////////////////////////////

static void sk_error_fn(png_structp png_ptr, png_const_charp msg) {
    SkCodecPrintf("------ png error %s\n", msg);
    longjmp(png_jmpbuf(png_ptr), 1);
}

void sk_warning_fn(png_structp, png_const_charp msg) {
    SkCodecPrintf("----- png warning %s\n", msg);
}

static void sk_read_fn(png_structp png_ptr, png_bytep data,
                       png_size_t length) {
    SkStream* stream = static_cast<SkStream*>(png_get_io_ptr(png_ptr));
    const size_t bytes = stream->read(data, length);
    if (bytes != length) {
        // FIXME: We want to report the fact that the stream was truncated.
        // One way to do that might be to pass a enum to longjmp so setjmp can
        // specify the failure.
        png_error(png_ptr, "Read Error!");
    }
}

#ifdef PNG_READ_UNKNOWN_CHUNKS_SUPPORTED
static int sk_read_user_chunk(png_structp png_ptr, png_unknown_chunkp chunk) {
    SkPngChunkReader* chunkReader = (SkPngChunkReader*)png_get_user_chunk_ptr(png_ptr);
    // readChunk() returning true means continue decoding
    return chunkReader->readChunk((const char*)chunk->name, chunk->data, chunk->size) ? 1 : -1;
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Helpers
///////////////////////////////////////////////////////////////////////////////

class AutoCleanPng : public SkNoncopyable {
public:
    AutoCleanPng(png_structp png_ptr)
        : fPng_ptr(png_ptr)
        , fInfo_ptr(nullptr) {}

    ~AutoCleanPng() {
        // fInfo_ptr will never be non-nullptr unless fPng_ptr is.
        if (fPng_ptr) {
            png_infopp info_pp = fInfo_ptr ? &fInfo_ptr : nullptr;
            png_destroy_read_struct(&fPng_ptr, info_pp, png_infopp_NULL);
        }
    }

    void setInfoPtr(png_infop info_ptr) {
        SkASSERT(nullptr == fInfo_ptr);
        fInfo_ptr = info_ptr;
    }

    void detach() {
        fPng_ptr = nullptr;
        fInfo_ptr = nullptr;
    }

private:
    png_structp     fPng_ptr;
    png_infop       fInfo_ptr;
};
#define AutoCleanPng(...) SK_REQUIRE_LOCAL_VAR(AutoCleanPng)

//checks if there is transparency info in the tRNS chunk
//image types which could have data in the tRNS chunk include: Index8, Gray8, RGB
static bool has_transparency_in_tRNS(png_structp png_ptr,
                                        png_infop info_ptr) {
    if (!png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        return false;
    }

    png_bytep trans;
    int num_trans;
    png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, nullptr);
    return num_trans > 0;
}

// Method for coverting to either an SkPMColor or a similarly packed
// unpremultiplied color.
typedef uint32_t (*PackColorProc)(U8CPU a, U8CPU r, U8CPU g, U8CPU b);

// Note: SkColorTable claims to store SkPMColors, which is not necessarily
// the case here.
bool SkPngCodec::decodePalette(bool premultiply, int* ctableCount) {
    int numPalette;
    png_colorp palette;
    png_bytep trans;

    if (!png_get_PLTE(fPng_ptr, fInfo_ptr, &palette, &numPalette)) {
        return false;
    }

    // Note: These are not necessarily SkPMColors
    SkPMColor colorStorage[256];    // worst-case storage
    SkPMColor* colorPtr = colorStorage;

    int numTrans;
    if (png_get_valid(fPng_ptr, fInfo_ptr, PNG_INFO_tRNS)) {
        png_get_tRNS(fPng_ptr, fInfo_ptr, &trans, &numTrans, nullptr);
    } else {
        numTrans = 0;
    }

    // check for bad images that might make us crash
    if (numTrans > numPalette) {
        numTrans = numPalette;
    }

    int index = 0;
    int transLessThanFF = 0;

    // Choose which function to use to create the color table. If the final destination's
    // colortype is unpremultiplied, the color table will store unpremultiplied colors.
    PackColorProc proc;
    if (premultiply) {
        proc = &SkPreMultiplyARGB;
    } else {
        proc = &SkPackARGB32NoCheck;
    }
    for (; index < numTrans; index++) {
        transLessThanFF |= (int)*trans - 0xFF;
        *colorPtr++ = proc(*trans++, palette->red, palette->green, palette->blue);
        palette++;
    }

    if (transLessThanFF >= 0) {
        // No transparent colors were found.
        fAlphaState = kOpaque_AlphaState;
    }

    for (; index < numPalette; index++) {
        *colorPtr++ = SkPackARGB32(0xFF, palette->red, palette->green, palette->blue);
        palette++;
    }

    /*  BUGGY IMAGE WORKAROUND
        Invalid images could contain pixel values that are greater than the number of palette
        entries. Since we use pixel values as indices into the palette this could result in reading
        beyond the end of the palette which could leak the contents of uninitialized memory. To
        ensure this doesn't happen, we grow the colortable to the maximum size that can be
        addressed by the bitdepth of the image and fill it with the last palette color or black if
        the palette is empty (really broken image).
    */
    int colorCount = SkTMax(numPalette, 1 << SkTMin(fBitDepth, 8));
    SkPMColor lastColor = index > 0 ? colorPtr[-1] : SkPackARGB32(0xFF, 0, 0, 0);
    for (; index < colorCount; index++) {
        *colorPtr++ = lastColor;
    }

    // Set the new color count
    if (ctableCount != nullptr) {
        *ctableCount = colorCount;
    }

    fColorTable.reset(new SkColorTable(colorStorage, colorCount));
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Creation
///////////////////////////////////////////////////////////////////////////////

#define PNG_BYTES_TO_CHECK 4

bool SkPngCodec::IsPng(SkStream* stream) {
    char buf[PNG_BYTES_TO_CHECK];
    if (stream->read(buf, PNG_BYTES_TO_CHECK) != PNG_BYTES_TO_CHECK) {
        return false;
    }
    if (png_sig_cmp((png_bytep) buf, (png_size_t)0, PNG_BYTES_TO_CHECK)) {
        return false;
    }
    return true;
}

// Reads the header and initializes the output fields, if not NULL.
//
// @param stream Input data. Will be read to get enough information to properly
//      setup the codec.
// @param chunkReader SkPngChunkReader, for reading unknown chunks. May be NULL.
//      If not NULL, png_ptr will hold an *unowned* pointer to it. The caller is
//      expected to continue to own it for the lifetime of the png_ptr.
// @param png_ptrp Optional output variable. If non-NULL, will be set to a new
//      png_structp on success.
// @param info_ptrp Optional output variable. If non-NULL, will be set to a new
//      png_infop on success;
// @param imageInfo Optional output variable. If non-NULL, will be set to
//      reflect the properties of the encoded image on success.
// @param bitDepthPtr Optional output variable. If non-NULL, will be set to the
//      bit depth of the encoded image on success.
// @param numberPassesPtr Optional output variable. If non-NULL, will be set to
//      the number_passes of the encoded image on success.
// @return true on success, in which case the caller is responsible for calling
//      png_destroy_read_struct(png_ptrp, info_ptrp).
//      If it returns false, the passed in fields (except stream) are unchanged.
static bool read_header(SkStream* stream, SkPngChunkReader* chunkReader,
                        png_structp* png_ptrp, png_infop* info_ptrp,
                        SkImageInfo* imageInfo, int* bitDepthPtr, int* numberPassesPtr) {
    // The image is known to be a PNG. Decode enough to know the SkImageInfo.
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr,
                                                 sk_error_fn, sk_warning_fn);
    if (!png_ptr) {
        return false;
    }

    AutoCleanPng autoClean(png_ptr);

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == nullptr) {
        return false;
    }

    autoClean.setInfoPtr(info_ptr);

    // FIXME: Could we use the return value of setjmp to specify the type of
    // error?
    if (setjmp(png_jmpbuf(png_ptr))) {
        return false;
    }

    png_set_read_fn(png_ptr, static_cast<void*>(stream), sk_read_fn);

#ifdef PNG_READ_UNKNOWN_CHUNKS_SUPPORTED
    // FIXME: Does this need to be installed so early?
    // hookup our chunkReader so we can see any user-chunks the caller may be interested in
    if (chunkReader) {
        png_set_keep_unknown_chunks(png_ptr, PNG_HANDLE_CHUNK_ALWAYS, (png_byte*)"", 0);
        png_set_read_user_chunk_fn(png_ptr, (png_voidp) chunkReader, sk_read_user_chunk);
    }
#endif

    // The call to png_read_info() gives us all of the information from the
    // PNG file before the first IDAT (image data chunk).
    png_read_info(png_ptr, info_ptr);
    png_uint_32 origWidth, origHeight;
    int bitDepth, colorType;
    png_get_IHDR(png_ptr, info_ptr, &origWidth, &origHeight, &bitDepth,
                 &colorType, int_p_NULL, int_p_NULL, int_p_NULL);

    if (bitDepthPtr) {
        *bitDepthPtr = bitDepth;
    }

    // sanity check for size
    {
        int64_t size = sk_64_mul(origWidth, origHeight);
        // now check that if we are 4-bytes per pixel, we also don't overflow
        if (size < 0 || size > (0x7FFFFFFF >> 2)) {
            return false;
        }
    }

    // Tell libpng to strip 16 bit/color files down to 8 bits/color
    if (bitDepth == 16) {
        png_set_strip_16(png_ptr);
    }
#ifdef PNG_READ_PACK_SUPPORTED
    // Extract multiple pixels with bit depths of 1, 2, and 4 from a single
    // byte into separate bytes (useful for paletted and grayscale images).
    if (bitDepth < 8) {
        png_set_packing(png_ptr);
    }
#endif
    // Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel.
    if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }

    // Now determine the default SkColorType and SkAlphaType and set required transforms
    SkColorType skColorType = kUnknown_SkColorType;
    SkAlphaType skAlphaType = kUnknown_SkAlphaType;
    switch (colorType) {
        case PNG_COLOR_TYPE_PALETTE:
            skColorType = kIndex_8_SkColorType;
            skAlphaType = has_transparency_in_tRNS(png_ptr, info_ptr) ?
                    kUnpremul_SkAlphaType : kOpaque_SkAlphaType;
            break;
        case PNG_COLOR_TYPE_RGB:
            if (has_transparency_in_tRNS(png_ptr, info_ptr)) {
                //convert to RGBA with tranparency information in tRNS chunk if it exists
                png_set_tRNS_to_alpha(png_ptr);
                skAlphaType = kUnpremul_SkAlphaType;
            } else {
                //convert to RGBA with Opaque Alpha 
                png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
                skAlphaType = kOpaque_SkAlphaType;
            }
            skColorType = kN32_SkColorType;
            break;
        case PNG_COLOR_TYPE_GRAY:
            if (has_transparency_in_tRNS(png_ptr, info_ptr)) {
                //FIXME: support gray with alpha as a color type
                //convert to RGBA if there is transparentcy info in the tRNS chunk
                png_set_tRNS_to_alpha(png_ptr);
                png_set_gray_to_rgb(png_ptr);
                skColorType = kN32_SkColorType;
                skAlphaType = kUnpremul_SkAlphaType;
            } else {
                skColorType = kGray_8_SkColorType;
                skAlphaType = kOpaque_SkAlphaType;
            }
            break;
        case PNG_COLOR_TYPE_GRAY_ALPHA:
            //FIXME: support gray with alpha as a color type 
            //convert to RGBA 
            png_set_gray_to_rgb(png_ptr);
            skColorType = kN32_SkColorType;
            skAlphaType = kUnpremul_SkAlphaType;
            break;
        case PNG_COLOR_TYPE_RGBA:
            skColorType = kN32_SkColorType;
            skAlphaType = kUnpremul_SkAlphaType;
            break;
        default:
            //all the color types have been covered above
            SkASSERT(false);
    }

    int numberPasses = png_set_interlace_handling(png_ptr);
    if (numberPassesPtr) {
        *numberPassesPtr = numberPasses;
    }

    // FIXME: Also need to check for sRGB ( https://bug.skia.org/3471 ).

    if (imageInfo) {
        *imageInfo = SkImageInfo::Make(origWidth, origHeight, skColorType, skAlphaType);
    }
    autoClean.detach();
    if (png_ptrp) {
        *png_ptrp = png_ptr;
    }
    if (info_ptrp) {
        *info_ptrp = info_ptr;
    }

    return true;
}

SkPngCodec::SkPngCodec(const SkImageInfo& info, SkStream* stream, SkPngChunkReader* chunkReader,
                       png_structp png_ptr, png_infop info_ptr, int bitDepth, int numberPasses)
    : INHERITED(info, stream)
    , fPngChunkReader(SkSafeRef(chunkReader))
    , fPng_ptr(png_ptr)
    , fInfo_ptr(info_ptr)
    , fSrcConfig(SkSwizzler::kUnknown)
    , fNumberPasses(numberPasses)
    , fBitDepth(bitDepth)
{
    if (info.alphaType() == kOpaque_SkAlphaType) {
        fAlphaState = kOpaque_AlphaState;
    } else {
        fAlphaState = kUnknown_AlphaState;
    }
}

SkPngCodec::~SkPngCodec() {
    this->destroyReadStruct();
}

void SkPngCodec::destroyReadStruct() {
    if (fPng_ptr) {
        // We will never have a nullptr fInfo_ptr with a non-nullptr fPng_ptr
        SkASSERT(fInfo_ptr);
        png_destroy_read_struct(&fPng_ptr, &fInfo_ptr, png_infopp_NULL);
        fPng_ptr = nullptr;
        fInfo_ptr = nullptr;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Getting the pixels
///////////////////////////////////////////////////////////////////////////////

SkCodec::Result SkPngCodec::initializeSwizzler(const SkImageInfo& requestedInfo,
                                               const Options& options,
                                               SkPMColor ctable[],
                                               int* ctableCount) {
    // FIXME: Could we use the return value of setjmp to specify the type of
    // error?
    if (setjmp(png_jmpbuf(fPng_ptr))) {
        SkCodecPrintf("setjmp long jump!\n");
        return kInvalidInput;
    }
    png_read_update_info(fPng_ptr, fInfo_ptr);  

    //srcColorType was determined in read_header() which determined png color type
    const SkColorType srcColorType = this->getInfo().colorType();

    switch (srcColorType) {
        case kIndex_8_SkColorType:
            //decode palette to Skia format
            fSrcConfig = SkSwizzler::kIndex;
            if (!this->decodePalette(kPremul_SkAlphaType == requestedInfo.alphaType(),
                    ctableCount)) {
                return kInvalidInput;
            }
            break;
        case kGray_8_SkColorType:
            fSrcConfig = SkSwizzler::kGray;
            break; 
        case kN32_SkColorType:
            if (this->getInfo().alphaType() == kOpaque_SkAlphaType) {
                    fSrcConfig = SkSwizzler::kRGBX;
                } else {
                    fSrcConfig = SkSwizzler::kRGBA;
            }
            break;
        default:
            //would have exited before now if the colorType was supported by png
            SkASSERT(false);
    }  

    // Copy the color table to the client if they request kIndex8 mode
    copy_color_table(requestedInfo, fColorTable, ctable, ctableCount);

    // Create the swizzler.  SkPngCodec retains ownership of the color table.
    const SkPMColor* colors = get_color_ptr(fColorTable.get());
    fSwizzler.reset(SkSwizzler::CreateSwizzler(fSrcConfig, colors, requestedInfo, options));
    if (!fSwizzler) {
        // FIXME: CreateSwizzler could fail for another reason.
        return kUnimplemented;
    }
    return kSuccess;
}


bool SkPngCodec::onRewind() {
    // This sets fPng_ptr and fInfo_ptr to nullptr. If read_header
    // succeeds, they will be repopulated, and if it fails, they will
    // remain nullptr. Any future accesses to fPng_ptr and fInfo_ptr will
    // come through this function which will rewind and again attempt
    // to reinitialize them.
    this->destroyReadStruct();

    png_structp png_ptr;
    png_infop info_ptr;
    if (!read_header(this->stream(), fPngChunkReader.get(), &png_ptr, &info_ptr,
                     nullptr, nullptr, nullptr)) {
        return false;
    }

    fPng_ptr = png_ptr;
    fInfo_ptr = info_ptr;
    return true;
}

SkCodec::Result SkPngCodec::onGetPixels(const SkImageInfo& requestedInfo, void* dst,
                                        size_t dstRowBytes, const Options& options,
                                        SkPMColor ctable[], int* ctableCount,
                                        int* rowsDecoded) {
    if (!conversion_possible(requestedInfo, this->getInfo())) {
        return kInvalidConversion;
    }
    if (options.fSubset) {
        // Subsets are not supported.
        return kUnimplemented;
    }

    // Note that ctable and ctableCount may be modified if there is a color table
    const Result result = this->initializeSwizzler(requestedInfo, options, ctable, ctableCount);
    if (result != kSuccess) {
        return result;
    }
    // FIXME: Could we use the return value of setjmp to specify the type of
    // error?
    int row = 0;
    // This must be declared above the call to setjmp to avoid memory leaks on incomplete images.
    SkAutoMalloc storage;
    if (setjmp(png_jmpbuf(fPng_ptr))) {
        // Assume that any error that occurs while reading rows is caused by an incomplete input.
        if (fNumberPasses > 1) {
            // FIXME (msarett): Handle incomplete interlaced pngs.
            return kInvalidInput;
        }
        // FIXME: We do a poor job on incomplete pngs compared to other decoders (ex: Chromium,
        // Ubuntu Image Viewer).  This is because we use the default buffer size in libpng (8192
        // bytes), and if we can't fill the buffer, we immediately fail.
        // For example, if we try to read 8192 bytes, and the image (incorrectly) only contains
        // half that, which may have been enough to contain a non-zero number of lines, we fail
        // when we could have decoded a few more lines and then failed.
        // The read function that we provide for libpng has no way of indicating that we have
        // made a partial read.
        // Making our buffer size smaller improves our incomplete decodes, but what impact does
        // it have on regular decode performance?  Should we investigate using a different API
        // instead of png_read_row(s)?  Chromium uses png_process_data.
        *rowsDecoded = row;
        return kIncompleteInput;
    }

    bool hasAlpha = false;
    // FIXME: We could split these out based on subclass.
    void* dstRow = dst;
    if (fNumberPasses > 1) {
        const int width = requestedInfo.width();
        const int height = requestedInfo.height();
        const int bpp = SkSwizzler::BytesPerPixel(fSrcConfig);
        const size_t srcRowBytes = width * bpp;

        storage.reset(width * height * bpp);
        uint8_t* const base = static_cast<uint8_t*>(storage.get());

        for (int i = 0; i < fNumberPasses; i++) {
            uint8_t* srcRow = base;
            for (int y = 0; y < height; y++) {
                uint8_t* bmRow = srcRow;
                png_read_rows(fPng_ptr, &bmRow, png_bytepp_NULL, 1);
                srcRow += srcRowBytes;
            }
        }

        // Now swizzle it.
        uint8_t* srcRow = base;
        for (int y = 0; y < height; y++) {
            hasAlpha |= !SkSwizzler::IsOpaque(fSwizzler->swizzle(dstRow, srcRow));
            dstRow = SkTAddOffset<void>(dstRow, dstRowBytes);
            srcRow += srcRowBytes;
        }
    } else {
        storage.reset(requestedInfo.width() * SkSwizzler::BytesPerPixel(fSrcConfig));
        uint8_t* srcRow = static_cast<uint8_t*>(storage.get());
        for (; row < requestedInfo.height(); row++) {
            png_read_rows(fPng_ptr, &srcRow, png_bytepp_NULL, 1);
            // FIXME: Only call IsOpaque once, outside the loop. Same for onGetScanlines.
            hasAlpha |= !SkSwizzler::IsOpaque(fSwizzler->swizzle(dstRow, srcRow));
            dstRow = SkTAddOffset<void>(dstRow, dstRowBytes);
        }
    }

    if (hasAlpha) {
        fAlphaState = kHasAlpha_AlphaState;
    } else {
        fAlphaState = kOpaque_AlphaState;
    }

    // FIXME: do we need substituteTranspColor? Note that we cannot do it for
    // scanline decoding, but we could do it here. Alternatively, we could do
    // it as we go, instead of in post-processing like SkPNGImageDecoder.

    if (setjmp(png_jmpbuf(fPng_ptr))) {
        // We've already read all the scanlines. This is a success.
        return kSuccess;
    }

    // read rest of file, and get additional comment and time chunks in info_ptr
    png_read_end(fPng_ptr, fInfo_ptr);

    return kSuccess;
}

uint32_t SkPngCodec::onGetFillValue(SkColorType colorType, SkAlphaType alphaType) const {
    const SkPMColor* colorPtr = get_color_ptr(fColorTable.get());
    if (colorPtr) {
        return get_color_table_fill_value(colorType, colorPtr, 0);
    }
    return INHERITED::onGetFillValue(colorType, alphaType);
}

bool SkPngCodec::onReallyHasAlpha() const {
    switch (fAlphaState) {
        case kOpaque_AlphaState:
            return false;
        case kUnknown_AlphaState:
            // Maybe the subclass knows?
            return this->alphaInScanlineDecode() == kHasAlpha_AlphaState;
        case kHasAlpha_AlphaState:
            switch (this->alphaInScanlineDecode()) {
                case kUnknown_AlphaState:
                    // Scanline decoder must not have been used. Return our knowledge.
                    return true;
                case kOpaque_AlphaState:
                    // Scanline decoder was used, and did not find alpha in its subset.
                    return false;
                case kHasAlpha_AlphaState:
                    return true;
            }
    }

    // All valid AlphaStates have been covered, so this should not be reached.
    SkASSERT(false);
    return true;
}

// Subclass of SkPngCodec which supports scanline decoding
class SkPngScanlineDecoder : public SkPngCodec {
public:
    SkPngScanlineDecoder(const SkImageInfo& srcInfo, SkStream* stream,
            SkPngChunkReader* chunkReader, png_structp png_ptr, png_infop info_ptr, int bitDepth)
        : INHERITED(srcInfo, stream, chunkReader, png_ptr, info_ptr, bitDepth, 1)
        , fAlphaState(kUnknown_AlphaState)
        , fSrcRow(nullptr)
    {}

    Result onStartScanlineDecode(const SkImageInfo& dstInfo, const Options& options,
            SkPMColor ctable[], int* ctableCount) override {
        if (!conversion_possible(dstInfo, this->getInfo())) {
            return kInvalidConversion;
        }

        const Result result = this->initializeSwizzler(dstInfo, options, ctable,
                                                       ctableCount);
        if (result != kSuccess) {
            return result;
        }

        fAlphaState = kUnknown_AlphaState;
        fStorage.reset(this->getInfo().width() * SkSwizzler::BytesPerPixel(this->srcConfig()));
        fSrcRow = static_cast<uint8_t*>(fStorage.get());

        return kSuccess;
    }

    int onGetScanlines(void* dst, int count, size_t rowBytes) override {
        // Assume that an error in libpng indicates an incomplete input.
        int row = 0;
        if (setjmp(png_jmpbuf(this->png_ptr()))) {
            SkCodecPrintf("setjmp long jump!\n");
            return row;
        }

        void* dstRow = dst;
        bool hasAlpha = false;
        for (; row < count; row++) {
            png_read_rows(this->png_ptr(), &fSrcRow, png_bytepp_NULL, 1);
            hasAlpha |= !SkSwizzler::IsOpaque(this->swizzler()->swizzle(dstRow, fSrcRow));
            dstRow = SkTAddOffset<void>(dstRow, rowBytes);
        }

        if (hasAlpha) {
            fAlphaState = kHasAlpha_AlphaState;
        } else {
            if (kUnknown_AlphaState == fAlphaState) {
                fAlphaState = kOpaque_AlphaState;
            }
            // Otherwise, the AlphaState is unchanged.
        }

        return row;
    }

    bool onSkipScanlines(int count) override {
        // Assume that an error in libpng indicates an incomplete input.
        if (setjmp(png_jmpbuf(this->png_ptr()))) {
            SkCodecPrintf("setjmp long jump!\n");
            return false;
        }
        //there is a potential tradeoff of memory vs speed created by putting this in a loop. 
        //calling png_read_rows in a loop is insignificantly slower than calling it once with count 
        //as png_read_rows has it's own loop which calls png_read_row count times.
        for (int row = 0; row < count; row++) {
            png_read_rows(this->png_ptr(), &fSrcRow, png_bytepp_NULL, 1);
        }
        return true;
    }

    AlphaState alphaInScanlineDecode() const override {
        return fAlphaState;
    }

private:
    AlphaState                  fAlphaState;
    SkAutoMalloc                fStorage;
    uint8_t*                    fSrcRow;

    typedef SkPngCodec INHERITED;
};


class SkPngInterlacedScanlineDecoder : public SkPngCodec {
public:
    SkPngInterlacedScanlineDecoder(const SkImageInfo& srcInfo, SkStream* stream,
            SkPngChunkReader* chunkReader, png_structp png_ptr, png_infop info_ptr,
            int bitDepth, int numberPasses)
        : INHERITED(srcInfo, stream, chunkReader, png_ptr, info_ptr, bitDepth, numberPasses)
        , fAlphaState(kUnknown_AlphaState)
        , fHeight(-1)
        , fCanSkipRewind(false)
    {
        SkASSERT(numberPasses != 1);
    }

    Result onStartScanlineDecode(const SkImageInfo& dstInfo, const Options& options,
            SkPMColor ctable[], int* ctableCount) override {
        if (!conversion_possible(dstInfo, this->getInfo())) {
            return kInvalidConversion;    
        }

        const Result result = this->initializeSwizzler(dstInfo, options, ctable,
                                                       ctableCount);
        if (result != kSuccess) {
            return result;
        }

        fAlphaState = kUnknown_AlphaState;
        fHeight = dstInfo.height();
        // FIXME: This need not be called on a second call to onStartScanlineDecode.
        fSrcRowBytes = this->getInfo().width() * SkSwizzler::BytesPerPixel(this->srcConfig());
        fGarbageRow.reset(fSrcRowBytes);
        fGarbageRowPtr = static_cast<uint8_t*>(fGarbageRow.get());
        fCanSkipRewind = true;

        return SkCodec::kSuccess;
    }

    int onGetScanlines(void* dst, int count, size_t dstRowBytes) override {
        // rewind stream if have previously called onGetScanlines,
        // since we need entire progressive image to get scanlines
        if (fCanSkipRewind) {
            // We already rewound in onStartScanlineDecode, so there is no reason to rewind.
            // Next time onGetScanlines is called, we will need to rewind.
            fCanSkipRewind = false;
        } else {
            // rewindIfNeeded resets fCurrScanline, since it assumes that start
            // needs to be called again before scanline decoding. PNG scanline
            // decoding is the exception, since it needs to rewind between
            // calls to getScanlines. Keep track of fCurrScanline, to undo the
            // reset.
            const int currScanline = this->nextScanline();
            // This method would never be called if currScanline is -1
            SkASSERT(currScanline != -1);

            if (!this->rewindIfNeeded()) {
                return kCouldNotRewind;
            }
            this->updateNextScanline(currScanline);
        }

        if (setjmp(png_jmpbuf(this->png_ptr()))) {
            SkCodecPrintf("setjmp long jump!\n");
            // FIXME (msarett): Returning 0 is pessimistic.  If we can complete a single pass,
            // we may be able to report that all of the memory has been initialized.  Even if we
            // fail on the first pass, we can still report than some scanlines are initialized.
            return 0;
        }
        SkAutoMalloc storage(count * fSrcRowBytes);
        uint8_t* storagePtr = static_cast<uint8_t*>(storage.get());
        uint8_t* srcRow;
        const int startRow = this->nextScanline();
        for (int i = 0; i < this->numberPasses(); i++) {
            // read rows we planned to skip into garbage row
            for (int y = 0; y < startRow; y++){
                png_read_rows(this->png_ptr(), &fGarbageRowPtr, png_bytepp_NULL, 1);
            }
            // read rows we care about into buffer
            srcRow = storagePtr;
            for (int y = 0; y < count; y++) {
                png_read_rows(this->png_ptr(), &srcRow, png_bytepp_NULL, 1);
                srcRow += fSrcRowBytes;
            }
            // read rows we don't want into garbage buffer
            for (int y = 0; y < fHeight - startRow - count; y++) {
                png_read_rows(this->png_ptr(), &fGarbageRowPtr, png_bytepp_NULL, 1);
            }
        }
        //swizzle the rows we care about
        srcRow = storagePtr;
        void* dstRow = dst;
        bool hasAlpha = false;
        for (int y = 0; y < count; y++) {
            hasAlpha |= !SkSwizzler::IsOpaque(this->swizzler()->swizzle(dstRow, srcRow));
            dstRow = SkTAddOffset<void>(dstRow, dstRowBytes);
            srcRow += fSrcRowBytes;
        }

        if (hasAlpha) {
            fAlphaState = kHasAlpha_AlphaState;
        } else {
            if (kUnknown_AlphaState == fAlphaState) {
                fAlphaState = kOpaque_AlphaState;
            }
            // Otherwise, the AlphaState is unchanged.
        }

        return count;
    }

    bool onSkipScanlines(int count) override {
        // The non-virtual version will update fCurrScanline.
        return true;
    }

    AlphaState alphaInScanlineDecode() const override {
        return fAlphaState;
    }

    SkScanlineOrder onGetScanlineOrder() const override {
        return kNone_SkScanlineOrder;
    }

private:
    AlphaState                  fAlphaState;
    int                         fHeight;
    size_t                      fSrcRowBytes;
    SkAutoMalloc                fGarbageRow;
    uint8_t*                    fGarbageRowPtr;
    // FIXME: This imitates behavior in SkCodec::rewindIfNeeded. That function
    // is called whenever some action is taken that reads the stream and
    // therefore the next call will require a rewind. So it modifies a boolean
    // to note that the *next* time it is called a rewind is needed.
    // SkPngInterlacedScanlineDecoder has an extra wrinkle - calling
    // onStartScanlineDecode followed by onGetScanlines does *not* require a
    // rewind. Since rewindIfNeeded does not have this flexibility, we need to
    // add another layer.
    bool                        fCanSkipRewind;

    typedef SkPngCodec INHERITED;
};

SkCodec* SkPngCodec::NewFromStream(SkStream* stream, SkPngChunkReader* chunkReader) {
    SkAutoTDelete<SkStream> streamDeleter(stream);
    png_structp png_ptr;
    png_infop info_ptr;
    SkImageInfo imageInfo;
    int bitDepth;
    int numberPasses;

    if (!read_header(stream, chunkReader, &png_ptr, &info_ptr, &imageInfo, &bitDepth,
                     &numberPasses)) {
        return nullptr;
    }

    if (1 == numberPasses) {
        return new SkPngScanlineDecoder(imageInfo, streamDeleter.detach(), chunkReader,
                                        png_ptr, info_ptr, bitDepth);
    }

    return new SkPngInterlacedScanlineDecoder(imageInfo, streamDeleter.detach(), chunkReader,
                                              png_ptr, info_ptr, bitDepth, numberPasses);
}
