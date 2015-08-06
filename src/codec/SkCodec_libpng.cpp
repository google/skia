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
#include "SkScanlineDecoder.h"
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
    #define png_infopp_NULL NULL
#endif

#ifndef png_bytepp_NULL
    #define png_bytepp_NULL NULL
#endif

#ifndef int_p_NULL
    #define int_p_NULL NULL
#endif

#ifndef png_flush_ptr_NULL
    #define png_flush_ptr_NULL NULL
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

///////////////////////////////////////////////////////////////////////////////
// Helpers
///////////////////////////////////////////////////////////////////////////////

class AutoCleanPng : public SkNoncopyable {
public:
    AutoCleanPng(png_structp png_ptr)
        : fPng_ptr(png_ptr)
        , fInfo_ptr(NULL) {}

    ~AutoCleanPng() {
        // fInfo_ptr will never be non-NULL unless fPng_ptr is.
        if (fPng_ptr) {
            png_infopp info_pp = fInfo_ptr ? &fInfo_ptr : NULL;
            png_destroy_read_struct(&fPng_ptr, info_pp, png_infopp_NULL);
        }
    }

    void setInfoPtr(png_infop info_ptr) {
        SkASSERT(NULL == fInfo_ptr);
        fInfo_ptr = info_ptr;
    }

    void detach() {
        fPng_ptr = NULL;
        fInfo_ptr = NULL;
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
    png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, NULL);
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
        png_get_tRNS(fPng_ptr, fInfo_ptr, &trans, &numTrans, NULL);
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

    fReallyHasAlpha = transLessThanFF < 0;

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
    if (ctableCount != NULL) {
        *ctableCount = colorCount;
    }

    fColorTable.reset(SkNEW_ARGS(SkColorTable, (colorStorage, colorCount)));
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

// Reads the header, and initializes the passed in fields, if not NULL (except
// stream, which is passed to the read function).
// Returns true on success, in which case the caller is responsible for calling
// png_destroy_read_struct. If it returns false, the passed in fields (except
// stream) are unchanged.
static bool read_header(SkStream* stream, png_structp* png_ptrp,
                        png_infop* info_ptrp, SkImageInfo* imageInfo, int* bitDepthPtr) {
    // The image is known to be a PNG. Decode enough to know the SkImageInfo.
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
                                                 sk_error_fn, sk_warning_fn);
    if (!png_ptr) {
        return false;
    }

    AutoCleanPng autoClean(png_ptr);

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        return false;
    }

    autoClean.setInfoPtr(info_ptr);

    // FIXME: Could we use the return value of setjmp to specify the type of
    // error?
    if (setjmp(png_jmpbuf(png_ptr))) {
        return false;
    }

    png_set_read_fn(png_ptr, static_cast<void*>(stream), sk_read_fn);

    // FIXME: This is where the old code hooks up the Peeker. Does it need to
    // be set this early? (i.e. where are the user chunks? early in the stream,
    // potentially?)
    // If it does, we need to figure out a way to set it here.

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
    SkColorType skColorType;
    SkAlphaType skAlphaType;
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

    // FIXME: Also need to check for sRGB (skbug.com/3471).

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

SkCodec* SkPngCodec::NewFromStream(SkStream* stream) {
    SkAutoTDelete<SkStream> streamDeleter(stream);
    png_structp png_ptr;
    png_infop info_ptr;
    SkImageInfo imageInfo;
    int bitDepth;
    if (read_header(stream, &png_ptr, &info_ptr, &imageInfo, &bitDepth)) {
        return SkNEW_ARGS(SkPngCodec, (imageInfo, streamDeleter.detach(), 
                png_ptr, info_ptr, bitDepth));
    }
    return NULL;
}

#define INVALID_NUMBER_PASSES -1
SkPngCodec::SkPngCodec(const SkImageInfo& info, SkStream* stream,
                       png_structp png_ptr, png_infop info_ptr, int bitDepth)
    : INHERITED(info, stream)
    , fPng_ptr(png_ptr)
    , fInfo_ptr(info_ptr)
    , fSrcConfig(SkSwizzler::kUnknown)
    , fNumberPasses(INVALID_NUMBER_PASSES)
    , fReallyHasAlpha(false)
    , fBitDepth(bitDepth)
{}

SkPngCodec::~SkPngCodec() {
    this->destroyReadStruct();
}

void SkPngCodec::destroyReadStruct() {
    if (fPng_ptr) {
        // We will never have a NULL fInfo_ptr with a non-NULL fPng_ptr
        SkASSERT(fInfo_ptr);
        png_destroy_read_struct(&fPng_ptr, &fInfo_ptr, png_infopp_NULL);
        fPng_ptr = NULL;
        fInfo_ptr = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Getting the pixels
///////////////////////////////////////////////////////////////////////////////

static bool conversion_possible(const SkImageInfo& dst, const SkImageInfo& src) {
    // TODO: Support other conversions
    if (dst.profileType() != src.profileType()) {
        return false;
    }

    // Ensure the alpha type is valid
    if (!valid_alpha(dst.alphaType(), src.alphaType())) {
        return false;
    }

    // Check for supported color types
    switch (dst.colorType()) {
        case kN32_SkColorType:
            return true;
        case kRGB_565_SkColorType:
            return src.alphaType() == kOpaque_SkAlphaType;
        default:
            return dst.colorType() == src.colorType();
    }
}

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
    fNumberPasses = png_set_interlace_handling(fPng_ptr);
    png_read_update_info(fPng_ptr, fInfo_ptr);  

    // Set to the default before calling decodePalette, which may change it.
    fReallyHasAlpha = false;

    //srcColorType was determined in readHeader() which determined png color type
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
    fSwizzler.reset(SkSwizzler::CreateSwizzler(fSrcConfig, colors, requestedInfo,
            options.fZeroInitialized));
    if (!fSwizzler) {
        // FIXME: CreateSwizzler could fail for another reason.
        return kUnimplemented;
    }
    return kSuccess;
}


bool SkPngCodec::handleRewind() {
    switch (this->rewindIfNeeded()) {
        case kNoRewindNecessary_RewindState:
            return true;
        case kCouldNotRewind_RewindState:
            return false;
        case kRewound_RewindState: {
            // This sets fPng_ptr and fInfo_ptr to NULL. If read_header
            // succeeds, they will be repopulated, and if it fails, they will
            // remain NULL. Any future accesses to fPng_ptr and fInfo_ptr will
            // come through this function which will rewind and again attempt
            // to reinitialize them.
            this->destroyReadStruct();
            png_structp png_ptr;
            png_infop info_ptr;
            if (read_header(this->stream(), &png_ptr, &info_ptr, NULL, NULL)) {
                fPng_ptr = png_ptr;
                fInfo_ptr = info_ptr;
                return true;
            }
            return false;
        }
        default:
            SkASSERT(false);
            return false;
    }
}

SkCodec::Result SkPngCodec::onGetPixels(const SkImageInfo& requestedInfo, void* dst,
                                        size_t dstRowBytes, const Options& options,
                                        SkPMColor ctable[], int* ctableCount) {
    if (!conversion_possible(requestedInfo, this->getInfo())) {
        return kInvalidConversion;
    }
    if (options.fSubset) {
        // Subsets are not supported.
        return kUnimplemented;
    }
    if (requestedInfo.dimensions() != this->getInfo().dimensions()) {
        return kInvalidScale;
    }
    if (!this->handleRewind()) {
        return kCouldNotRewind;
    }

    // Note that ctable and ctableCount may be modified if there is a color table
    const Result result = this->initializeSwizzler(requestedInfo, options,
                                                   ctable, ctableCount);
    if (result != kSuccess) {
        return result;
    }
    // FIXME: Could we use the return value of setjmp to specify the type of
    // error?
    if (setjmp(png_jmpbuf(fPng_ptr))) {
        SkCodecPrintf("setjmp long jump!\n");
        return kInvalidInput;
    }

    SkASSERT(fNumberPasses != INVALID_NUMBER_PASSES);
    SkAutoMalloc storage;
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
            fReallyHasAlpha |= !SkSwizzler::IsOpaque(fSwizzler->swizzle(dstRow, srcRow));
            dstRow = SkTAddOffset<void>(dstRow, dstRowBytes);
            srcRow += srcRowBytes;
        }
    } else {
        storage.reset(requestedInfo.width() * SkSwizzler::BytesPerPixel(fSrcConfig));
        uint8_t* srcRow = static_cast<uint8_t*>(storage.get());
        for (int y = 0; y < requestedInfo.height(); y++) {
            png_read_rows(fPng_ptr, &srcRow, png_bytepp_NULL, 1);
            fReallyHasAlpha |= !SkSwizzler::IsOpaque(fSwizzler->swizzle(dstRow, srcRow));
            dstRow = SkTAddOffset<void>(dstRow, dstRowBytes);
        }
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

class SkPngScanlineDecoder : public SkScanlineDecoder {
public:
    SkPngScanlineDecoder(const SkImageInfo& srcInfo, SkPngCodec* codec)
        : INHERITED(srcInfo)
        , fCodec(codec)
        , fHasAlpha(false)
    {}

    SkCodec::Result onStart(const SkImageInfo& dstInfo,
                            const SkCodec::Options& options,
                            SkPMColor ctable[], int* ctableCount) override
    {
        if (!fCodec->handleRewind()) {
            return SkCodec::kCouldNotRewind;
        }

        if (!conversion_possible(dstInfo, this->getInfo())) {
            return SkCodec::kInvalidConversion;
        }

        // Check to see if scaling was requested.
        if (dstInfo.dimensions() != this->getInfo().dimensions()) {
            return SkCodec::kInvalidScale;
        }

        const SkCodec::Result result = fCodec->initializeSwizzler(dstInfo, options, ctable,
                                                                  ctableCount);
        if (result != SkCodec::kSuccess) {
            return result;
        }

        fHasAlpha = false;
        fStorage.reset(dstInfo.width() * SkSwizzler::BytesPerPixel(fCodec->fSrcConfig));
        fSrcRow = static_cast<uint8_t*>(fStorage.get());

        return SkCodec::kSuccess;
    }

    SkCodec::Result onGetScanlines(void* dst, int count, size_t rowBytes) override {
        if (setjmp(png_jmpbuf(fCodec->fPng_ptr))) {
            SkCodecPrintf("setjmp long jump!\n");
            return SkCodec::kInvalidInput;
        }

        void* dstRow = dst;
        for (int i = 0; i < count; i++) {
            png_read_rows(fCodec->fPng_ptr, &fSrcRow, png_bytepp_NULL, 1);
            fHasAlpha |= !SkSwizzler::IsOpaque(fCodec->fSwizzler->swizzle(dstRow, fSrcRow));
            dstRow = SkTAddOffset<void>(dstRow, rowBytes);
        }
        return SkCodec::kSuccess;
    }

    SkCodec::Result onSkipScanlines(int count) override {
        // FIXME: Could we use the return value of setjmp to specify the type of
        // error?
        if (setjmp(png_jmpbuf(fCodec->fPng_ptr))) {
            SkCodecPrintf("setjmp long jump!\n");
            return SkCodec::kInvalidInput;
        }
        //there is a potential tradeoff of memory vs speed created by putting this in a loop. 
        //calling png_read_rows in a loop is insignificantly slower than calling it once with count 
        //as png_read_rows has it's own loop which calls png_read_row count times.
        for (int i = 0; i < count; i++) {
            png_read_rows(fCodec->fPng_ptr, &fSrcRow, png_bytepp_NULL, 1);
        }
        return SkCodec::kSuccess;
    }

    bool onReallyHasAlpha() const override { return fHasAlpha; }

private:
    SkAutoTDelete<SkPngCodec>   fCodec;
    bool                        fHasAlpha;
    SkAutoMalloc                fStorage;
    uint8_t*                    fSrcRow;

    typedef SkScanlineDecoder INHERITED;
};


class SkPngInterlacedScanlineDecoder : public SkScanlineDecoder {
public:
    SkPngInterlacedScanlineDecoder(const SkImageInfo& srcInfo, SkPngCodec* codec)
        : INHERITED(srcInfo)
        , fCodec(codec)
        , fHasAlpha(false)
        , fCurrentRow(0)
        , fHeight(srcInfo.height())
        , fCanSkipRewind(false)
    {}

    SkCodec::Result onStart(const SkImageInfo& dstInfo,
                            const SkCodec::Options& options,
                            SkPMColor ctable[], int* ctableCount) override
    {
        if (!fCodec->handleRewind()) {
            return SkCodec::kCouldNotRewind;
        }

        if (!conversion_possible(dstInfo, this->getInfo())) {
            return SkCodec::kInvalidConversion;
        }

        // Check to see if scaling was requested.
        if (dstInfo.dimensions() != this->getInfo().dimensions()) {
            return SkCodec::kInvalidScale;
        }

        const SkCodec::Result result = fCodec->initializeSwizzler(dstInfo, options, ctable,
                                                                  ctableCount);
        if (result != SkCodec::kSuccess) {
            return result;
        }

        fHasAlpha = false;
        fCurrentRow = 0;
        fHeight = dstInfo.height();
        fSrcRowBytes = dstInfo.width() * SkSwizzler::BytesPerPixel(fCodec->fSrcConfig);
        fGarbageRow.reset(fSrcRowBytes);
        fGarbageRowPtr = static_cast<uint8_t*>(fGarbageRow.get());
        fCanSkipRewind = true;

        return SkCodec::kSuccess;
    }

    SkCodec::Result onGetScanlines(void* dst, int count, size_t dstRowBytes) override {
        // rewind stream if have previously called onGetScanlines,
        // since we need entire progressive image to get scanlines
        if (fCanSkipRewind) {
            // We already rewound in onStart, so there is no reason to rewind.
            // Next time onGetScanlines is called, we will need to rewind.
            fCanSkipRewind = false;
        } else if (!fCodec->handleRewind()) {
            return SkCodec::kCouldNotRewind;
        }

        if (setjmp(png_jmpbuf(fCodec->fPng_ptr))) {
            SkCodecPrintf("setjmp long jump!\n");
            return SkCodec::kInvalidInput;
        }
        const int number_passes = png_set_interlace_handling(fCodec->fPng_ptr);
        SkAutoMalloc storage(count * fSrcRowBytes);
        uint8_t* storagePtr = static_cast<uint8_t*>(storage.get());
        uint8_t* srcRow;
        for (int i = 0; i < number_passes; i++) {
            //read rows we planned to skip into garbage row
            for (int y = 0; y < fCurrentRow; y++){
                png_read_rows(fCodec->fPng_ptr, &fGarbageRowPtr, png_bytepp_NULL, 1);
            }
            //read rows we care about into buffer
            srcRow = storagePtr;
            for (int y = 0; y < count; y++) {
                png_read_rows(fCodec->fPng_ptr, &srcRow, png_bytepp_NULL, 1);
                srcRow += fSrcRowBytes;
            }
            //read rows we don't want into garbage buffer
            for (int y = 0; y < fHeight - fCurrentRow - count; y++) {
                png_read_rows(fCodec->fPng_ptr, &fGarbageRowPtr, png_bytepp_NULL, 1);
            }
        }
        //swizzle the rows we care about
        srcRow = storagePtr;
        void* dstRow = dst;
        for (int y = 0; y < count; y++) {
            fHasAlpha |= !SkSwizzler::IsOpaque(fCodec->fSwizzler->swizzle(dstRow, srcRow));
            dstRow = SkTAddOffset<void>(dstRow, dstRowBytes);
            srcRow += fSrcRowBytes;
        }
        fCurrentRow += count;
        return SkCodec::kSuccess;
    }

    SkCodec::Result onSkipScanlines(int count) override {
        //when ongetScanlines is called it will skip to fCurrentRow
        fCurrentRow += count;
        return SkCodec::kSuccess;
    }

    bool onReallyHasAlpha() const override { return fHasAlpha; }

private:
    SkAutoTDelete<SkPngCodec>   fCodec;
    bool                        fHasAlpha;
    int                         fCurrentRow;
    int                         fHeight;
    size_t                      fSrcRowBytes;
    SkAutoMalloc                fGarbageRow;
    uint8_t*                    fGarbageRowPtr;
    // FIXME: This imitates behavior in SkCodec::rewindIfNeeded. That function
    // is called whenever some action is taken that reads the stream and
    // therefore the next call will require a rewind. So it modifies a boolean
    // to note that the *next* time it is called a rewind is needed.
    // SkPngInterlacedScanlineDecoder has an extra wrinkle - calling onStart
    // followed by onGetScanlines does *not* require a rewind. Since
    // rewindIfNeeded does not have this flexibility, we need to add another
    // layer.
    bool                        fCanSkipRewind;

    typedef SkScanlineDecoder INHERITED;
};

SkScanlineDecoder* SkPngCodec::NewSDFromStream(SkStream* stream) {
    SkAutoTDelete<SkPngCodec> codec (static_cast<SkPngCodec*>(SkPngCodec::NewFromStream(stream)));
    if (!codec) {
        return NULL;
    }

    codec->fNumberPasses = png_set_interlace_handling(codec->fPng_ptr);
    SkASSERT(codec->fNumberPasses != INVALID_NUMBER_PASSES);

    const SkImageInfo& srcInfo = codec->getInfo();
    if (codec->fNumberPasses > 1) {
        // interlaced image
        return SkNEW_ARGS(SkPngInterlacedScanlineDecoder, (srcInfo, codec.detach()));
    }

    return SkNEW_ARGS(SkPngScanlineDecoder, (srcInfo, codec.detach()));
}

