/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCodecPriv.h"
#include "SkColorPriv.h"
#include "SkColorSpace_Base.h"
#include "SkColorTable.h"
#include "SkMath.h"
#include "SkOpts.h"
#include "SkPngCodec.h"
#include "SkSize.h"
#include "SkStream.h"
#include "SkSwizzler.h"
#include "SkTemplates.h"
#include "SkUtils.h"

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
            png_destroy_read_struct(&fPng_ptr, info_pp, nullptr);
        }
    }

    void setInfoPtr(png_infop info_ptr) {
        SkASSERT(nullptr == fInfo_ptr);
        fInfo_ptr = info_ptr;
    }

    void release() {
        fPng_ptr = nullptr;
        fInfo_ptr = nullptr;
    }

private:
    png_structp     fPng_ptr;
    png_infop       fInfo_ptr;
};
#define AutoCleanPng(...) SK_REQUIRE_LOCAL_VAR(AutoCleanPng)

// Note: SkColorTable claims to store SkPMColors, which is not necessarily
// the case here.
// TODO: If we add support for non-native swizzles, we'll need to handle that here.
bool SkPngCodec::createColorTable(SkColorType dstColorType, bool premultiply, int* ctableCount) {

    int numColors;
    png_color* palette;
    if (!png_get_PLTE(fPng_ptr, fInfo_ptr, &palette, &numColors)) {
        return false;
    }

    // Note: These are not necessarily SkPMColors.
    SkPMColor colorPtr[256];

    png_bytep alphas;
    int numColorsWithAlpha = 0;
    if (png_get_tRNS(fPng_ptr, fInfo_ptr, &alphas, &numColorsWithAlpha, nullptr)) {
        // Choose which function to use to create the color table. If the final destination's
        // colortype is unpremultiplied, the color table will store unpremultiplied colors.
        PackColorProc proc = choose_pack_color_proc(premultiply, dstColorType);

        for (int i = 0; i < numColorsWithAlpha; i++) {
            // We don't have a function in SkOpts that combines a set of alphas with a set
            // of RGBs.  We could write one, but it's hardly worth it, given that this
            // is such a small fraction of the total decode time.
            colorPtr[i] = proc(alphas[i], palette->red, palette->green, palette->blue);
            palette++;
        }
    }

    if (numColorsWithAlpha < numColors) {
        // The optimized code depends on a 3-byte png_color struct with the colors
        // in RGB order.  These checks make sure it is safe to use.
        static_assert(3 == sizeof(png_color), "png_color struct has changed.  Opts are broken.");
#ifdef SK_DEBUG
        SkASSERT(&palette->red < &palette->green);
        SkASSERT(&palette->green < &palette->blue);
#endif

        if (is_rgba(dstColorType)) {
            SkOpts::RGB_to_RGB1(colorPtr + numColorsWithAlpha, palette,
                    numColors - numColorsWithAlpha);
        } else {
            SkOpts::RGB_to_BGR1(colorPtr + numColorsWithAlpha, palette,
                    numColors - numColorsWithAlpha);
        }
    }

    // Pad the color table with the last color in the table (or black) in the case that
    // invalid pixel indices exceed the number of colors in the table.
    const int maxColors = 1 << fBitDepth;
    if (numColors < maxColors) {
        SkPMColor lastColor = numColors > 0 ? colorPtr[numColors - 1] : SK_ColorBLACK;
        sk_memset32(colorPtr + numColors, lastColor, maxColors - numColors);
    }

    // Set the new color count.
    if (ctableCount != nullptr) {
        *ctableCount = maxColors;
    }

    fColorTable.reset(new SkColorTable(colorPtr, maxColors));
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Creation
///////////////////////////////////////////////////////////////////////////////

bool SkPngCodec::IsPng(const char* buf, size_t bytesRead) {
    return !png_sig_cmp((png_bytep) buf, (png_size_t)0, bytesRead);
}

static float png_fixed_point_to_float(png_fixed_point x) {
    // We multiply by the same factor that libpng used to convert
    // fixed point -> double.  Since we want floats, we choose to
    // do the conversion ourselves rather than convert
    // fixed point -> double -> float.
    return ((float) x) * 0.00001f;
}

static float png_inverted_fixed_point_to_float(png_fixed_point x) {
    // This is necessary because the gAMA chunk actually stores 1/gamma.
    return 1.0f / png_fixed_point_to_float(x);
}

static constexpr float gSRGB_toXYZD50[] {
    0.4358f, 0.2224f, 0.0139f,    // * R
    0.3853f, 0.7170f, 0.0971f,    // * G
    0.1430f, 0.0606f, 0.7139f,    // * B
};

// Returns a colorSpace object that represents any color space information in
// the encoded data.  If the encoded data contains no color space, this will
// return NULL.
sk_sp<SkColorSpace> read_color_space(png_structp png_ptr, png_infop info_ptr) {

#if (PNG_LIBPNG_VER_MAJOR > 1) || (PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR >= 6)

    // First check for an ICC profile
    png_bytep profile;
    png_uint_32 length;
    // The below variables are unused, however, we need to pass them in anyway or
    // png_get_iCCP() will return nothing.
    // Could knowing the |name| of the profile ever be interesting?  Maybe for debugging?
    png_charp name;
    // The |compression| is uninteresting since:
    //   (1) libpng has already decompressed the profile for us.
    //   (2) "deflate" is the only mode of decompression that libpng supports.
    int compression;
    if (PNG_INFO_iCCP == png_get_iCCP(png_ptr, info_ptr, &name, &compression, &profile,
            &length)) {
        return SkColorSpace::NewICC(profile, length);
    }

    // Second, check for sRGB.
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_sRGB)) {

        // sRGB chunks also store a rendering intent: Absolute, Relative,
        // Perceptual, and Saturation.
        // FIXME (msarett): Extract this information from the sRGB chunk once
        //                  we are able to handle this information in
        //                  SkColorSpace.
        return SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named);
    }

    // Next, check for chromaticities.
    png_fixed_point XYZ[9];
    float toXYZD50[9];
    png_fixed_point gamma;
    float gammas[3];
    if (png_get_cHRM_XYZ_fixed(png_ptr, info_ptr, &XYZ[0], &XYZ[1], &XYZ[2], &XYZ[3], &XYZ[4],
            &XYZ[5], &XYZ[6], &XYZ[7], &XYZ[8])) {

        // FIXME (msarett): Here we are treating XYZ values as D50 even though the color
        //                  temperature is unspecified.  I suspect that this assumption
        //                  is most often ok, but we could also calculate the color
        //                  temperature (D value) and then convert the XYZ to D50.  Maybe
        //                  we should add a new constructor to SkColorSpace that accepts
        //                  XYZ with D-Unkown?
        for (int i = 0; i < 9; i++) {
            toXYZD50[i] = png_fixed_point_to_float(XYZ[i]);
        }
        SkMatrix44 mat(SkMatrix44::kUninitialized_Constructor);
        mat.set3x3RowMajorf(toXYZD50);

        if (PNG_INFO_gAMA == png_get_gAMA_fixed(png_ptr, info_ptr, &gamma)) {
            float value = png_inverted_fixed_point_to_float(gamma);
            gammas[0] = value;
            gammas[1] = value;
            gammas[2] = value;

            return SkColorSpace_Base::NewRGB(gammas, mat);
        }

        // Default to sRGB gamma if the image has color space information,
        // but does not specify gamma.
        return SkColorSpace::NewRGB(SkColorSpace::kSRGB_GammaNamed, mat);
    }

    // Last, check for gamma.
    if (PNG_INFO_gAMA == png_get_gAMA_fixed(png_ptr, info_ptr, &gamma)) {

        // Set the gammas.
        float value = png_inverted_fixed_point_to_float(gamma);
        gammas[0] = value;
        gammas[1] = value;
        gammas[2] = value;

        // Since there is no cHRM, we will guess sRGB gamut.
        SkMatrix44 mat(SkMatrix44::kUninitialized_Constructor);
        mat.set3x3RowMajorf(gSRGB_toXYZD50);

        return SkColorSpace_Base::NewRGB(gammas, mat);
    }

#endif // LIBPNG >= 1.6

    // Report that there is no color space information in the PNG.  SkPngCodec is currently
    // implemented to guess sRGB in this case.
    return nullptr;
}

static int bytes_per_pixel(int bitsPerPixel) {
    // Note that we will have to change this implementation if we start
    // supporting outputs from libpng that are less than 8-bits per component.
    return bitsPerPixel / 8;
}

// Subclass of SkPngCodec which supports scanline decoding
class SkPngScanlineDecoder : public SkPngCodec {
public:
    SkPngScanlineDecoder(int width, int height, const SkEncodedInfo& info, SkStream* stream,
            SkPngChunkReader* chunkReader, png_structp png_ptr, png_infop info_ptr, int bitDepth,
            sk_sp<SkColorSpace> colorSpace)
        : INHERITED(width, height, info, stream, chunkReader, png_ptr, info_ptr, bitDepth, 1,
                    colorSpace)
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

        fStorage.reset(this->getInfo().width() *
                (bytes_per_pixel(this->getEncodedInfo().bitsPerPixel())));
        fSrcRow = fStorage.get();

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
        for (; row < count; row++) {
            png_read_row(this->png_ptr(), fSrcRow, nullptr);
            this->swizzler()->swizzle(dstRow, fSrcRow);
            dstRow = SkTAddOffset<void>(dstRow, rowBytes);
        }

        return row;
    }

    bool onSkipScanlines(int count) override {
        // Assume that an error in libpng indicates an incomplete input.
        if (setjmp(png_jmpbuf(this->png_ptr()))) {
            SkCodecPrintf("setjmp long jump!\n");
            return false;
        }

        for (int row = 0; row < count; row++) {
            png_read_row(this->png_ptr(), fSrcRow, nullptr);
        }
        return true;
    }

private:
    SkAutoTMalloc<uint8_t>      fStorage;
    uint8_t*                    fSrcRow;

    typedef SkPngCodec INHERITED;
};


class SkPngInterlacedScanlineDecoder : public SkPngCodec {
public:
    SkPngInterlacedScanlineDecoder(int width, int height, const SkEncodedInfo& info,
            SkStream* stream, SkPngChunkReader* chunkReader, png_structp png_ptr,
            png_infop info_ptr, int bitDepth, int numberPasses, sk_sp<SkColorSpace> colorSpace)
        : INHERITED(width, height, info, stream, chunkReader, png_ptr, info_ptr, bitDepth,
                    numberPasses, colorSpace)
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

        fHeight = dstInfo.height();
        // FIXME: This need not be called on a second call to onStartScanlineDecode.
        fSrcRowBytes = this->getInfo().width() *
                (bytes_per_pixel(this->getEncodedInfo().bitsPerPixel()));
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
            this->updateCurrScanline(currScanline);
        }

        if (setjmp(png_jmpbuf(this->png_ptr()))) {
            SkCodecPrintf("setjmp long jump!\n");
            // FIXME (msarett): Returning 0 is pessimistic.  If we can complete a single pass,
            // we may be able to report that all of the memory has been initialized.  Even if we
            // fail on the first pass, we can still report than some scanlines are initialized.
            return 0;
        }
        SkAutoTMalloc<uint8_t> storage(count * fSrcRowBytes);
        uint8_t* storagePtr = storage.get();
        uint8_t* srcRow;
        const int startRow = this->nextScanline();
        for (int i = 0; i < this->numberPasses(); i++) {
            // read rows we planned to skip into garbage row
            for (int y = 0; y < startRow; y++){
                png_read_row(this->png_ptr(), fGarbageRowPtr, nullptr);
            }
            // read rows we care about into buffer
            srcRow = storagePtr;
            for (int y = 0; y < count; y++) {
                png_read_row(this->png_ptr(), srcRow, nullptr);
                srcRow += fSrcRowBytes;
            }
            // read rows we don't want into garbage buffer
            for (int y = 0; y < fHeight - startRow - count; y++) {
                png_read_row(this->png_ptr(), fGarbageRowPtr, nullptr);
            }
        }
        //swizzle the rows we care about
        srcRow = storagePtr;
        void* dstRow = dst;
        for (int y = 0; y < count; y++) {
            this->swizzler()->swizzle(dstRow, srcRow);
            dstRow = SkTAddOffset<void>(dstRow, dstRowBytes);
            srcRow += fSrcRowBytes;
        }

        return count;
    }

    bool onSkipScanlines(int count) override {
        // The non-virtual version will update fCurrScanline.
        return true;
    }

    SkScanlineOrder onGetScanlineOrder() const override {
        return kNone_SkScanlineOrder;
    }

private:
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

// Reads the header and initializes the output fields, if not NULL.
//
// @param stream Input data. Will be read to get enough information to properly
//      setup the codec.
// @param chunkReader SkPngChunkReader, for reading unknown chunks. May be NULL.
//      If not NULL, png_ptr will hold an *unowned* pointer to it. The caller is
//      expected to continue to own it for the lifetime of the png_ptr.
// @param outCodec Optional output variable.  If non-NULL, will be set to a new
//      SkPngCodec on success.
// @param png_ptrp Optional output variable. If non-NULL, will be set to a new
//      png_structp on success.
// @param info_ptrp Optional output variable. If non-NULL, will be set to a new
//      png_infop on success;
// @return true on success, in which case the caller is responsible for calling
//      png_destroy_read_struct(png_ptrp, info_ptrp).
//      If it returns false, the passed in fields (except stream) are unchanged.
static bool read_header(SkStream* stream, SkPngChunkReader* chunkReader, SkCodec** outCodec,
                        png_structp* png_ptrp, png_infop* info_ptrp) {
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
    // Hookup our chunkReader so we can see any user-chunks the caller may be interested in.
    // This needs to be installed before we read the png header.  Android may store ninepatch
    // chunks in the header.
    if (chunkReader) {
        png_set_keep_unknown_chunks(png_ptr, PNG_HANDLE_CHUNK_ALWAYS, (png_byte*)"", 0);
        png_set_read_user_chunk_fn(png_ptr, (png_voidp) chunkReader, sk_read_user_chunk);
    }
#endif

    // The call to png_read_info() gives us all of the information from the
    // PNG file before the first IDAT (image data chunk).
    png_read_info(png_ptr, info_ptr);
    png_uint_32 origWidth, origHeight;
    int bitDepth, encodedColorType;
    png_get_IHDR(png_ptr, info_ptr, &origWidth, &origHeight, &bitDepth,
                 &encodedColorType, nullptr, nullptr, nullptr);

    // Tell libpng to strip 16 bit/color files down to 8 bits/color.
    // TODO: Should we handle this in SkSwizzler?  Could this also benefit
    //       RAW decodes?
    if (bitDepth == 16) {
        SkASSERT(PNG_COLOR_TYPE_PALETTE != encodedColorType);
        png_set_strip_16(png_ptr);
    }

    // Now determine the default colorType and alphaType and set the required transforms.
    // Often, we depend on SkSwizzler to perform any transforms that we need.  However, we
    // still depend on libpng for many of the rare and PNG-specific cases.
    SkEncodedInfo::Color color;
    SkEncodedInfo::Alpha alpha;
    switch (encodedColorType) {
        case PNG_COLOR_TYPE_PALETTE:
            // Extract multiple pixels with bit depths of 1, 2, and 4 from a single
            // byte into separate bytes (useful for paletted and grayscale images).
            if (bitDepth < 8) {
                // TODO: Should we use SkSwizzler here?
                png_set_packing(png_ptr);
            }

            color = SkEncodedInfo::kPalette_Color;
            // Set the alpha depending on if a transparency chunk exists.
            alpha = png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) ?
                    SkEncodedInfo::kUnpremul_Alpha : SkEncodedInfo::kOpaque_Alpha;
            break;
        case PNG_COLOR_TYPE_RGB:
            if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
                // Convert to RGBA if transparency chunk exists.
                png_set_tRNS_to_alpha(png_ptr);
                color = SkEncodedInfo::kRGBA_Color;
                alpha = SkEncodedInfo::kBinary_Alpha;
            } else {
                color = SkEncodedInfo::kRGB_Color;
                alpha = SkEncodedInfo::kOpaque_Alpha;
            }
            break;
        case PNG_COLOR_TYPE_GRAY:
            // Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel.
            if (bitDepth < 8) {
                // TODO: Should we use SkSwizzler here?
                png_set_expand_gray_1_2_4_to_8(png_ptr);
            }

            if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
                png_set_tRNS_to_alpha(png_ptr);
                color = SkEncodedInfo::kGrayAlpha_Color;
                alpha = SkEncodedInfo::kBinary_Alpha;
            } else {
                color = SkEncodedInfo::kGray_Color;
                alpha = SkEncodedInfo::kOpaque_Alpha;
            }
            break;
        case PNG_COLOR_TYPE_GRAY_ALPHA:
            color = SkEncodedInfo::kGrayAlpha_Color;
            alpha = SkEncodedInfo::kUnpremul_Alpha;
            break;
        case PNG_COLOR_TYPE_RGBA:
            color = SkEncodedInfo::kRGBA_Color;
            alpha = SkEncodedInfo::kUnpremul_Alpha;
            break;
        default:
            // All the color types have been covered above.
            SkASSERT(false);
            color = SkEncodedInfo::kRGBA_Color;
            alpha = SkEncodedInfo::kUnpremul_Alpha;
    }

    int numberPasses = png_set_interlace_handling(png_ptr);

    autoClean.release();
    if (png_ptrp) {
        *png_ptrp = png_ptr;
    }
    if (info_ptrp) {
        *info_ptrp = info_ptr;
    }

    if (outCodec) {
        sk_sp<SkColorSpace> colorSpace = read_color_space(png_ptr, info_ptr);
        if (!colorSpace) {
            // Treat unmarked pngs as sRGB.
            colorSpace = SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named);
        }

        SkEncodedInfo info = SkEncodedInfo::Make(color, alpha, 8);

        if (1 == numberPasses) {
            *outCodec = new SkPngScanlineDecoder(origWidth, origHeight, info, stream,
                    chunkReader, png_ptr, info_ptr, bitDepth, colorSpace);
        } else {
            *outCodec = new SkPngInterlacedScanlineDecoder(origWidth, origHeight, info, stream,
                    chunkReader, png_ptr, info_ptr, bitDepth, numberPasses, colorSpace);
        }
    }

    return true;
}

SkPngCodec::SkPngCodec(int width, int height, const SkEncodedInfo& info, SkStream* stream,
                       SkPngChunkReader* chunkReader, png_structp png_ptr, png_infop info_ptr,
                       int bitDepth, int numberPasses, sk_sp<SkColorSpace> colorSpace)
    : INHERITED(width, height, info, stream, colorSpace)
    , fPngChunkReader(SkSafeRef(chunkReader))
    , fPng_ptr(png_ptr)
    , fInfo_ptr(info_ptr)
    , fNumberPasses(numberPasses)
    , fBitDepth(bitDepth)
{}

SkPngCodec::~SkPngCodec() {
    this->destroyReadStruct();
}

void SkPngCodec::destroyReadStruct() {
    if (fPng_ptr) {
        // We will never have a nullptr fInfo_ptr with a non-nullptr fPng_ptr
        SkASSERT(fInfo_ptr);
        png_destroy_read_struct(&fPng_ptr, &fInfo_ptr, nullptr);
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

    if (SkEncodedInfo::kPalette_Color == this->getEncodedInfo().color()) {
        if (!this->createColorTable(requestedInfo.colorType(),
                kPremul_SkAlphaType == requestedInfo.alphaType(), ctableCount)) {
            return kInvalidInput;
        }
    }

    // Copy the color table to the client if they request kIndex8 mode
    copy_color_table(requestedInfo, fColorTable, ctable, ctableCount);

    // Create the swizzler.  SkPngCodec retains ownership of the color table.
    const SkPMColor* colors = get_color_ptr(fColorTable.get());
    fSwizzler.reset(SkSwizzler::CreateSwizzler(this->getEncodedInfo(), colors, requestedInfo,
            options));
    SkASSERT(fSwizzler);

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
    if (!read_header(this->stream(), fPngChunkReader.get(), nullptr, &png_ptr, &info_ptr)) {
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

    const int width = requestedInfo.width();
    const int height = requestedInfo.height();
    const int bpp = bytes_per_pixel(this->getEncodedInfo().bitsPerPixel());
    const size_t srcRowBytes = width * bpp;

    // FIXME: Could we use the return value of setjmp to specify the type of
    // error?
    int row = 0;
    // This must be declared above the call to setjmp to avoid memory leaks on incomplete images.
    SkAutoTMalloc<uint8_t> storage;
    if (setjmp(png_jmpbuf(fPng_ptr))) {
        // Assume that any error that occurs while reading rows is caused by an incomplete input.
        if (fNumberPasses > 1) {
            // FIXME (msarett): Handle incomplete interlaced pngs.
            return (row == height) ? kSuccess : kInvalidInput;
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
        // instead of png_read_row?  Chromium uses png_process_data.
        *rowsDecoded = row;
        return (row == height) ? kSuccess : kIncompleteInput;
    }

    // FIXME: We could split these out based on subclass.
    void* dstRow = dst;
    if (fNumberPasses > 1) {
        storage.reset(height * srcRowBytes);
        uint8_t* const base = storage.get();

        for (int i = 0; i < fNumberPasses; i++) {
            uint8_t* srcRow = base;
            for (int y = 0; y < height; y++) {
                png_read_row(fPng_ptr, srcRow, nullptr);
                srcRow += srcRowBytes;
            }
        }

        // Now swizzle it.
        uint8_t* srcRow = base;
        for (; row < height; row++) {
            fSwizzler->swizzle(dstRow, srcRow);
            dstRow = SkTAddOffset<void>(dstRow, dstRowBytes);
            srcRow += srcRowBytes;
        }
    } else {
        storage.reset(srcRowBytes);
        uint8_t* srcRow = storage.get();
        for (; row < height; row++) {
            png_read_row(fPng_ptr, srcRow, nullptr);
            fSwizzler->swizzle(dstRow, srcRow);
            dstRow = SkTAddOffset<void>(dstRow, dstRowBytes);
        }
    }

    // read rest of file, and get additional comment and time chunks in info_ptr
    png_read_end(fPng_ptr, fInfo_ptr);

    return kSuccess;
}

uint32_t SkPngCodec::onGetFillValue(SkColorType colorType) const {
    const SkPMColor* colorPtr = get_color_ptr(fColorTable.get());
    if (colorPtr) {
        return get_color_table_fill_value(colorType, colorPtr, 0);
    }
    return INHERITED::onGetFillValue(colorType);
}

SkCodec* SkPngCodec::NewFromStream(SkStream* stream, SkPngChunkReader* chunkReader) {
    SkAutoTDelete<SkStream> streamDeleter(stream);

    SkCodec* outCodec;
    if (read_header(stream, chunkReader, &outCodec, nullptr, nullptr)) {
        // Codec has taken ownership of the stream.
        SkASSERT(outCodec);
        streamDeleter.release();
        return outCodec;
    }

    return nullptr;
}
