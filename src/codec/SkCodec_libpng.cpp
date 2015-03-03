/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec_libpng.h"
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
    SkDebugf("------ png error %s\n", msg);
    longjmp(png_jmpbuf(png_ptr), 1);
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

// call only if color_type is PALETTE. Returns true if the ctable has alpha
static bool has_transparency_in_palette(png_structp png_ptr,
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
SkColorTable* decode_palette(png_structp png_ptr, png_infop info_ptr,
                             bool premultiply, SkAlphaType* outAlphaType) {
    SkASSERT(outAlphaType != NULL);
    int numPalette;
    png_colorp palette;
    png_bytep trans;

    if (!png_get_PLTE(png_ptr, info_ptr, &palette, &numPalette)) {
        return NULL;
    }

    /*  BUGGY IMAGE WORKAROUND

        We hit some images (e.g. fruit_.png) who contain bytes that are == colortable_count
        which is a problem since we use the byte as an index. To work around this we grow
        the colortable by 1 (if its < 256) and duplicate the last color into that slot.
    */
    const int colorCount = numPalette + (numPalette < 256);
    // Note: These are not necessarily SkPMColors.
    SkPMColor colorStorage[256];    // worst-case storage
    SkPMColor* colorPtr = colorStorage;

    int numTrans;
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_get_tRNS(png_ptr, info_ptr, &trans, &numTrans, NULL);
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

    if (transLessThanFF < 0) {
        *outAlphaType = premultiply ? kPremul_SkAlphaType : kUnpremul_SkAlphaType;
    } else {
        *outAlphaType = kOpaque_SkAlphaType;
    }

    for (; index < numPalette; index++) {
        *colorPtr++ = SkPackARGB32(0xFF, palette->red, palette->green, palette->blue);
        palette++;
    }

    // see BUGGY IMAGE WORKAROUND comment above
    if (numPalette < 256) {
        *colorPtr = colorPtr[-1];
    }

    return SkNEW_ARGS(SkColorTable, (colorStorage, colorCount));
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

SkCodec* SkPngCodec::NewFromStream(SkStream* stream) {
    // The image is known to be a PNG. Decode enough to know the SkImageInfo.
    // FIXME: Allow silencing warnings.
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
                                                 sk_error_fn, NULL);
    if (!png_ptr) {
        return NULL;
    }

    AutoCleanPng autoClean(png_ptr);

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        return NULL;
    }

    autoClean.setInfoPtr(info_ptr);

    // FIXME: Could we use the return value of setjmp to specify the type of
    // error?
    if (setjmp(png_jmpbuf(png_ptr))) {
        return NULL;
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

    // sanity check for size
    {
        int64_t size = sk_64_mul(origWidth, origHeight);
        // now check that if we are 4-bytes per pixel, we also don't overflow
        if (size < 0 || size > (0x7FFFFFFF >> 2)) {
            return NULL;
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


    // Now determine the default SkColorType and SkAlphaType.
    SkColorType skColorType;
    SkAlphaType skAlphaType;
    switch (colorType) {
        case PNG_COLOR_TYPE_PALETTE:
            // Technically, this is true of the data, but I don't think we want
            // to support it.
            // skColorType = kIndex8_SkColorType;
            skColorType = kN32_SkColorType;
            skAlphaType = has_transparency_in_palette(png_ptr, info_ptr) ?
                    kUnpremul_SkAlphaType : kOpaque_SkAlphaType;
            break;
        case PNG_COLOR_TYPE_GRAY:
            if (false) {
                // FIXME: Is this the wrong default behavior? This means if the
                // caller supplies the info we gave them, they'll get Alpha 8.
                skColorType = kAlpha_8_SkColorType;
                // FIXME: Strangely, the canonical type for Alpha 8 is Premul.
                skAlphaType = kPremul_SkAlphaType;
            } else {
                skColorType = kN32_SkColorType;
                skAlphaType = kOpaque_SkAlphaType;
            }
            break;
        default:
            // Note: This *almost* mimics the code in SkImageDecoder_libpng.
            // has_transparency_in_palette makes an additional check - whether
            // numTrans is greater than 0. Why does the other code not make that
            // check?
            if (has_transparency_in_palette(png_ptr, info_ptr)
                || PNG_COLOR_TYPE_RGB_ALPHA == colorType
                || PNG_COLOR_TYPE_GRAY_ALPHA == colorType)
            {
                skAlphaType = kUnpremul_SkAlphaType;
            } else {
                skAlphaType = kOpaque_SkAlphaType;
            }
            skColorType = kN32_SkColorType;
            break;
    }

    {
        // FIXME: Again, this block needs to go into onGetPixels.
        bool convertGrayToRGB = PNG_COLOR_TYPE_GRAY == colorType && skColorType != kAlpha_8_SkColorType;

        // Unless the user is requesting A8, convert a grayscale image into RGB.
        // GRAY_ALPHA will always be converted to RGB
        if (convertGrayToRGB || colorType == PNG_COLOR_TYPE_GRAY_ALPHA) {
            png_set_gray_to_rgb(png_ptr);
        }

        // Add filler (or alpha) byte (after each RGB triplet) if necessary.
        // FIXME: It seems like we could just use RGB as the SrcConfig here.
        if (colorType == PNG_COLOR_TYPE_RGB || convertGrayToRGB) {
            png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
        }
    }

    // FIXME: Also need to check for sRGB (skbug.com/3471).

    SkImageInfo info = SkImageInfo::Make(origWidth, origHeight, skColorType,
                                         skAlphaType);
    SkCodec* codec = SkNEW_ARGS(SkPngCodec, (info, stream, png_ptr, info_ptr));
    autoClean.detach();
    return codec;
}

SkPngCodec::SkPngCodec(const SkImageInfo& info, SkStream* stream,
                       png_structp png_ptr, png_infop info_ptr)
    : INHERITED(info, stream)
    , fPng_ptr(png_ptr)
    , fInfo_ptr(info_ptr) {}

SkPngCodec::~SkPngCodec() {
    png_destroy_read_struct(&fPng_ptr, &fInfo_ptr, png_infopp_NULL);
}

///////////////////////////////////////////////////////////////////////////////
// Getting the pixels
///////////////////////////////////////////////////////////////////////////////

static bool premul_and_unpremul(SkAlphaType A, SkAlphaType B) {
    return kPremul_SkAlphaType == A && kUnpremul_SkAlphaType == B;
}

static bool conversion_possible(const SkImageInfo& A, const SkImageInfo& B) {
    // TODO: Support other conversions
    if (A.colorType() != B.colorType()) {
        return false;
    }
    if (A.profileType() != B.profileType()) {
        return false;
    }
    if (A.alphaType() == B.alphaType()) {
        return true;
    }
    return premul_and_unpremul(A.alphaType(), B.alphaType())
            || premul_and_unpremul(B.alphaType(), A.alphaType());
}

SkCodec::Result SkPngCodec::onGetPixels(const SkImageInfo& requestedInfo, void* dst,
                                        size_t rowBytes, SkPMColor ctable[],
                                        int* ctableCount) {
    if (!this->rewindIfNeeded()) {
        return kCouldNotRewind;
    }
    if (requestedInfo.dimensions() != this->getOriginalInfo().dimensions()) {
        return kInvalidScale;
    }
    if (!conversion_possible(requestedInfo, this->getOriginalInfo())) {
        return kInvalidConversion;
    }

    SkBitmap decodedBitmap;
    // If installPixels would have failed, getPixels should have failed before
    // calling onGetPixels.
    SkAssertResult(decodedBitmap.installPixels(requestedInfo, dst, rowBytes));

    // Initialize all non-trivial objects before setjmp.
    SkAutoTUnref<SkColorTable> colorTable;
    SkAutoTDelete<SkSwizzler> swizzler;
    SkAutoMalloc storage;                   // Scratch memory for pre-swizzled rows.

    // FIXME: Could we use the return value of setjmp to specify the type of
    // error?
    if (setjmp(png_jmpbuf(fPng_ptr))) {
        SkDebugf("setjmp long jump!\n");
        return kInvalidInput;
    }

    // FIXME: We already retrieved this information. Store it in SkPngCodec?
    png_uint_32 origWidth, origHeight;
    int bitDepth, pngColorType, interlaceType;
    png_get_IHDR(fPng_ptr, fInfo_ptr, &origWidth, &origHeight, &bitDepth,
                 &pngColorType, &interlaceType, int_p_NULL, int_p_NULL);

    const int numberPasses = (interlaceType != PNG_INTERLACE_NONE) ?
            png_set_interlace_handling(fPng_ptr) : 1;

    SkSwizzler::SrcConfig sc;
    bool reallyHasAlpha = false;
    if (PNG_COLOR_TYPE_PALETTE == pngColorType) {
        sc = SkSwizzler::kIndex;
        SkAlphaType at = requestedInfo.alphaType();
        colorTable.reset(decode_palette(fPng_ptr, fInfo_ptr,
                                        kPremul_SkAlphaType == at,
                                        &at));
        if (!colorTable) {
            return kInvalidInput;
        }

        reallyHasAlpha = (at != kOpaque_SkAlphaType);

        if (at != requestedInfo.alphaType()) {
            // It turns out the image is opaque.
            SkASSERT(kOpaque_SkAlphaType == at);
        }
    } else if (kAlpha_8_SkColorType == requestedInfo.colorType()) {
        // Note: we check the destination, since otherwise we would have
        // told png to upscale.
        SkASSERT(PNG_COLOR_TYPE_GRAY == pngColorType);
        sc = SkSwizzler::kGray;
    } else if (this->getOriginalInfo().alphaType() == kOpaque_SkAlphaType) {
        sc = SkSwizzler::kRGBX;
    } else {
        sc = SkSwizzler::kRGBA;
    }
    const SkPMColor* colors = colorTable ? colorTable->readColors() : NULL;
    // TODO: Support skipZeroes.
    swizzler.reset(SkSwizzler::CreateSwizzler(sc, colors, requestedInfo,
                                              dst, rowBytes, false));
    if (!swizzler) {
        // FIXME: CreateSwizzler could fail for another reason.
        return kUnimplemented;
    }

    // FIXME: Here is where we should likely insert some of the modifications
    // made in the factory.
    png_read_update_info(fPng_ptr, fInfo_ptr);

    if (numberPasses > 1) {
        const int width = requestedInfo.width();
        const int height = requestedInfo.height();
        const int bpp = SkSwizzler::BytesPerPixel(sc);
        const size_t rowBytes = width * bpp;

        storage.reset(width * height * bpp);
        uint8_t* const base = static_cast<uint8_t*>(storage.get());

        for (int i = 0; i < numberPasses; i++) {
            uint8_t* row = base;
            for (int y = 0; y < height; y++) {
                uint8_t* bmRow = row;
                png_read_rows(fPng_ptr, &bmRow, png_bytepp_NULL, 1);
                row += rowBytes;
            }
        }

        // Now swizzle it.
        uint8_t* row = base;
        for (int y = 0; y < height; y++) {
            reallyHasAlpha |= swizzler->next(row);
            row += rowBytes;
        }
    } else {
        storage.reset(requestedInfo.width() * SkSwizzler::BytesPerPixel(sc));
        uint8_t* srcRow = static_cast<uint8_t*>(storage.get());
        for (int y = 0; y < requestedInfo.height(); y++) {
            png_read_rows(fPng_ptr, &srcRow, png_bytepp_NULL, 1);
            reallyHasAlpha |= swizzler->next(srcRow);
        }
    }

    /* read rest of file, and get additional chunks in info_ptr - REQUIRED */
    png_read_end(fPng_ptr, fInfo_ptr);

    // FIXME: do we need substituteTranspColor?

    if (reallyHasAlpha && requestedInfo.alphaType() != kOpaque_SkAlphaType) {
        // FIXME: We want to alert the caller. Is this the right way?
        SkImageInfo* modInfo = const_cast<SkImageInfo*>(&requestedInfo);
        *modInfo = requestedInfo.makeAlphaType(kOpaque_SkAlphaType);
    }
    return kSuccess;
}
