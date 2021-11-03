/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkMath.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/private/SkColorData.h"
#include "include/private/SkMacros.h"
#include "include/private/SkTemplates.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkColorTable.h"
#include "src/codec/SkPngCodec.h"
#include "src/codec/SkPngPriv.h"
#include "src/codec/SkSwizzler.h"
#include "src/core/SkOpts.h"

#include "png.h"
#include <algorithm>

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    #include "include/android/SkAndroidFrameworkUtils.h"
#endif

// This warning triggers false postives way too often in here.
#if defined(__GNUC__) && !defined(__clang__)
    #pragma GCC diagnostic ignored "-Wclobbered"
#endif

// FIXME (scroggo): We can use png_jumpbuf directly once Google3 is on 1.6
#define PNG_JMPBUF(x) png_jmpbuf((png_structp) x)

///////////////////////////////////////////////////////////////////////////////
// Callback functions
///////////////////////////////////////////////////////////////////////////////

// When setjmp is first called, it returns 0, meaning longjmp was not called.
constexpr int kSetJmpOkay   = 0;
// An error internal to libpng.
constexpr int kPngError     = 1;
// Passed to longjmp when we have decoded as many lines as we need.
constexpr int kStopDecoding = 2;

static void sk_error_fn(png_structp png_ptr, png_const_charp msg) {
    SkCodecPrintf("------ png error %s\n", msg);
    longjmp(PNG_JMPBUF(png_ptr), kPngError);
}

void sk_warning_fn(png_structp, png_const_charp msg) {
    SkCodecPrintf("----- png warning %s\n", msg);
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
    /*
     *  This class does not take ownership of stream or reader, but if codecPtr
     *  is non-NULL, and decodeBounds succeeds, it will have created a new
     *  SkCodec (pointed to by *codecPtr) which will own/ref them, as well as
     *  the png_ptr and info_ptr.
     */
    AutoCleanPng(png_structp png_ptr, SkStream* stream, SkPngChunkReader* reader,
            SkCodec** codecPtr)
        : fPng_ptr(png_ptr)
        , fInfo_ptr(nullptr)
        , fStream(stream)
        , fChunkReader(reader)
        , fOutCodec(codecPtr)
    {}

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

    /**
     *  Reads enough of the input stream to decode the bounds.
     *  @return false if the stream is not a valid PNG (or too short).
     *          true if it read enough of the stream to determine the bounds.
     *          In the latter case, the stream may have been read beyond the
     *          point to determine the bounds, and the png_ptr will have saved
     *          any extra data. Further, if the codecPtr supplied to the
     *          constructor was not NULL, it will now point to a new SkCodec,
     *          which owns (or refs, in the case of the SkPngChunkReader) the
     *          inputs. If codecPtr was NULL, the png_ptr and info_ptr are
     *          unowned, and it is up to the caller to destroy them.
     */
    bool decodeBounds();

private:
    png_structp         fPng_ptr;
    png_infop           fInfo_ptr;
    SkStream*           fStream;
    SkPngChunkReader*   fChunkReader;
    SkCodec**           fOutCodec;

    void infoCallback(size_t idatLength);

    void releasePngPtrs() {
        fPng_ptr = nullptr;
        fInfo_ptr = nullptr;
    }
};

static inline bool is_chunk(const png_byte* chunk, const char* tag) {
    return memcmp(chunk + 4, tag, 4) == 0;
}

static inline bool process_data(png_structp png_ptr, png_infop info_ptr,
        SkStream* stream, void* buffer, size_t bufferSize, size_t length) {
    while (length > 0) {
        const size_t bytesToProcess = std::min(bufferSize, length);
        const size_t bytesRead = stream->read(buffer, bytesToProcess);
        png_process_data(png_ptr, info_ptr, (png_bytep) buffer, bytesRead);
        if (bytesRead < bytesToProcess) {
            return false;
        }
        length -= bytesToProcess;
    }
    return true;
}

bool AutoCleanPng::decodeBounds() {
    if (setjmp(PNG_JMPBUF(fPng_ptr))) {
        return false;
    }

    png_set_progressive_read_fn(fPng_ptr, nullptr, nullptr, nullptr, nullptr);

    // Arbitrary buffer size, though note that it matches (below)
    // SkPngCodec::processData(). FIXME: Can we better suit this to the size of
    // the PNG header?
    constexpr size_t kBufferSize = 4096;
    char buffer[kBufferSize];

    {
        // Parse the signature.
        if (fStream->read(buffer, 8) < 8) {
            return false;
        }

        png_process_data(fPng_ptr, fInfo_ptr, (png_bytep) buffer, 8);
    }

    while (true) {
        // Parse chunk length and type.
        if (fStream->read(buffer, 8) < 8) {
            // We have read to the end of the input without decoding bounds.
            break;
        }

        png_byte* chunk = reinterpret_cast<png_byte*>(buffer);
        const size_t length = png_get_uint_32(chunk);

        if (is_chunk(chunk, "IDAT")) {
            this->infoCallback(length);
            return true;
        }

        png_process_data(fPng_ptr, fInfo_ptr, chunk, 8);
        // Process the full chunk + CRC.
        if (!process_data(fPng_ptr, fInfo_ptr, fStream, buffer, kBufferSize, length + 4)) {
            return false;
        }
    }

    return false;
}

bool SkPngCodec::processData() {
    switch (setjmp(PNG_JMPBUF(fPng_ptr))) {
        case kPngError:
            // There was an error. Stop processing data.
            // FIXME: Do we need to discard png_ptr?
            return false;
        case kStopDecoding:
            // We decoded all the lines we want.
            return true;
        case kSetJmpOkay:
            // Everything is okay.
            break;
        default:
            // No other values should be passed to longjmp.
            SkASSERT(false);
    }

    // Arbitrary buffer size
    constexpr size_t kBufferSize = 4096;
    char buffer[kBufferSize];

    bool iend = false;
    while (true) {
        size_t length;
        if (fDecodedIdat) {
            // Parse chunk length and type.
            if (this->stream()->read(buffer, 8) < 8) {
                break;
            }

            png_byte* chunk = reinterpret_cast<png_byte*>(buffer);
            png_process_data(fPng_ptr, fInfo_ptr, chunk, 8);
            if (is_chunk(chunk, "IEND")) {
                iend = true;
            }

            length = png_get_uint_32(chunk);
        } else {
            length = fIdatLength;
            png_byte idat[] = {0, 0, 0, 0, 'I', 'D', 'A', 'T'};
            png_save_uint_32(idat, length);
            png_process_data(fPng_ptr, fInfo_ptr, idat, 8);
            fDecodedIdat = true;
        }

        // Process the full chunk + CRC.
        if (!process_data(fPng_ptr, fInfo_ptr, this->stream(), buffer, kBufferSize, length + 4)
                || iend) {
            break;
        }
    }

    return true;
}

static constexpr SkColorType kXformSrcColorType = kRGBA_8888_SkColorType;

static inline bool needs_premul(SkAlphaType dstAT, SkEncodedInfo::Alpha encodedAlpha) {
    return kPremul_SkAlphaType == dstAT && SkEncodedInfo::kUnpremul_Alpha == encodedAlpha;
}

// Note: SkColorTable claims to store SkPMColors, which is not necessarily the case here.
bool SkPngCodec::createColorTable(const SkImageInfo& dstInfo) {

    int numColors;
    png_color* palette;
    if (!png_get_PLTE(fPng_ptr, fInfo_ptr, &palette, &numColors)) {
        return false;
    }

    // Contents depend on tableColorType and our choice of if/when to premultiply:
    // { kPremul, kUnpremul, kOpaque } x { RGBA, BGRA }
    SkPMColor colorTable[256];
    SkColorType tableColorType = this->colorXform() ? kXformSrcColorType : dstInfo.colorType();

    png_bytep alphas;
    int numColorsWithAlpha = 0;
    if (png_get_tRNS(fPng_ptr, fInfo_ptr, &alphas, &numColorsWithAlpha, nullptr)) {
        bool premultiply = needs_premul(dstInfo.alphaType(), this->getEncodedInfo().alpha());

        // Choose which function to use to create the color table. If the final destination's
        // colortype is unpremultiplied, the color table will store unpremultiplied colors.
        PackColorProc proc = choose_pack_color_proc(premultiply, tableColorType);

        for (int i = 0; i < numColorsWithAlpha; i++) {
            // We don't have a function in SkOpts that combines a set of alphas with a set
            // of RGBs.  We could write one, but it's hardly worth it, given that this
            // is such a small fraction of the total decode time.
            colorTable[i] = proc(alphas[i], palette->red, palette->green, palette->blue);
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

        if (is_rgba(tableColorType)) {
            SkOpts::RGB_to_RGB1(colorTable + numColorsWithAlpha, (const uint8_t*)palette,
                    numColors - numColorsWithAlpha);
        } else {
            SkOpts::RGB_to_BGR1(colorTable + numColorsWithAlpha, (const uint8_t*)palette,
                    numColors - numColorsWithAlpha);
        }
    }

    if (this->colorXform() && !this->xformOnDecode()) {
        this->applyColorXform(colorTable, colorTable, numColors);
    }

    // Pad the color table with the last color in the table (or black) in the case that
    // invalid pixel indices exceed the number of colors in the table.
    const int maxColors = 1 << fBitDepth;
    if (numColors < maxColors) {
        SkPMColor lastColor = numColors > 0 ? colorTable[numColors - 1] : SK_ColorBLACK;
        sk_memset32(colorTable + numColors, lastColor, maxColors - numColors);
    }

    fColorTable.reset(new SkColorTable(colorTable, maxColors));
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Creation
///////////////////////////////////////////////////////////////////////////////

bool SkPngCodec::IsPng(const void* buf, size_t bytesRead) {
    return !png_sig_cmp((png_bytep) buf, (png_size_t)0, bytesRead);
}

#if (PNG_LIBPNG_VER_MAJOR > 1) || (PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR >= 6)

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

#endif // LIBPNG >= 1.6

// If there is no color profile information, it will use sRGB.
std::unique_ptr<SkEncodedInfo::ICCProfile> read_color_profile(png_structp png_ptr,
                                                              png_infop info_ptr) {

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
        auto data = SkData::MakeWithCopy(profile, length);
        return SkEncodedInfo::ICCProfile::Make(std::move(data));
    }

    // Second, check for sRGB.
    // Note that Blink does this first. This code checks ICC first, with the thinking that
    // an image has both truly wants the potentially more specific ICC chunk, with sRGB as a
    // backup in case the decoder does not support full color management.
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_sRGB)) {
        // sRGB chunks also store a rendering intent: Absolute, Relative,
        // Perceptual, and Saturation.
        // FIXME (scroggo): Extract this information from the sRGB chunk once
        //                  we are able to handle this information in
        //                  skcms_ICCProfile
        return nullptr;
    }

    // Default to SRGB gamut.
    skcms_Matrix3x3 toXYZD50 = skcms_sRGB_profile()->toXYZD50;
    // Next, check for chromaticities.
    png_fixed_point chrm[8];
    png_fixed_point gamma;
    if (png_get_cHRM_fixed(png_ptr, info_ptr, &chrm[0], &chrm[1], &chrm[2], &chrm[3], &chrm[4],
                           &chrm[5], &chrm[6], &chrm[7]))
    {
        float rx = png_fixed_point_to_float(chrm[2]);
        float ry = png_fixed_point_to_float(chrm[3]);
        float gx = png_fixed_point_to_float(chrm[4]);
        float gy = png_fixed_point_to_float(chrm[5]);
        float bx = png_fixed_point_to_float(chrm[6]);
        float by = png_fixed_point_to_float(chrm[7]);
        float wx = png_fixed_point_to_float(chrm[0]);
        float wy = png_fixed_point_to_float(chrm[1]);

        skcms_Matrix3x3 tmp;
        if (skcms_PrimariesToXYZD50(rx, ry, gx, gy, bx, by, wx, wy, &tmp)) {
            toXYZD50 = tmp;
        } else {
            // Note that Blink simply returns nullptr in this case. We'll fall
            // back to srgb.
        }
    }

    skcms_TransferFunction fn;
    if (PNG_INFO_gAMA == png_get_gAMA_fixed(png_ptr, info_ptr, &gamma)) {
        fn.a = 1.0f;
        fn.b = fn.c = fn.d = fn.e = fn.f = 0.0f;
        fn.g = png_inverted_fixed_point_to_float(gamma);
    } else {
        // Default to sRGB gamma if the image has color space information,
        // but does not specify gamma.
        // Note that Blink would again return nullptr in this case.
        fn = *skcms_sRGB_TransferFunction();
    }

    skcms_ICCProfile skcmsProfile;
    skcms_Init(&skcmsProfile);
    skcms_SetTransferFunction(&skcmsProfile, &fn);
    skcms_SetXYZD50(&skcmsProfile, &toXYZD50);

    return SkEncodedInfo::ICCProfile::Make(skcmsProfile);
#else // LIBPNG >= 1.6
    return nullptr;
#endif // LIBPNG >= 1.6
}

void SkPngCodec::allocateStorage(const SkImageInfo& dstInfo) {
    switch (fXformMode) {
        case kSwizzleOnly_XformMode:
            break;
        case kColorOnly_XformMode:
            // Intentional fall through.  A swizzler hasn't been created yet, but one will
            // be created later if we are sampling.  We'll go ahead and allocate
            // enough memory to swizzle if necessary.
        case kSwizzleColor_XformMode: {
            const int bitsPerPixel = this->getEncodedInfo().bitsPerPixel();

            // If we have more than 8-bits (per component) of precision, we will keep that
            // extra precision.  Otherwise, we will swizzle to RGBA_8888 before transforming.
            const size_t bytesPerPixel = (bitsPerPixel > 32) ? bitsPerPixel / 8 : 4;
            const size_t colorXformBytes = dstInfo.width() * bytesPerPixel;
            fStorage.reset(colorXformBytes);
            fColorXformSrcRow = fStorage.get();
            break;
        }
    }
}

static skcms_PixelFormat png_select_xform_format(const SkEncodedInfo& info) {
    // We use kRGB and kRGBA formats because color PNGs are always RGB or RGBA.
    if (16 == info.bitsPerComponent()) {
        if (SkEncodedInfo::kRGBA_Color == info.color()) {
            return skcms_PixelFormat_RGBA_16161616BE;
        } else if (SkEncodedInfo::kRGB_Color == info.color()) {
            return skcms_PixelFormat_RGB_161616BE;
        }
    } else if (SkEncodedInfo::kGray_Color == info.color()) {
        return skcms_PixelFormat_G_8;
    }

    return skcms_PixelFormat_RGBA_8888;
}

void SkPngCodec::applyXformRow(void* dst, const void* src) {
    switch (fXformMode) {
        case kSwizzleOnly_XformMode:
            fSwizzler->swizzle(dst, (const uint8_t*) src);
            break;
        case kColorOnly_XformMode:
            this->applyColorXform(dst, src, fXformWidth);
            break;
        case kSwizzleColor_XformMode:
            fSwizzler->swizzle(fColorXformSrcRow, (const uint8_t*) src);
            this->applyColorXform(dst, fColorXformSrcRow, fXformWidth);
            break;
    }
}

static SkCodec::Result log_and_return_error(bool success) {
    if (success) return SkCodec::kIncompleteInput;
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    SkAndroidFrameworkUtils::SafetyNetLog("117838472");
#endif
    return SkCodec::kErrorInInput;
}

class SkPngNormalDecoder : public SkPngCodec {
public:
    SkPngNormalDecoder(SkEncodedInfo&& info, std::unique_ptr<SkStream> stream,
            SkPngChunkReader* reader, png_structp png_ptr, png_infop info_ptr, int bitDepth)
        : INHERITED(std::move(info), std::move(stream), reader, png_ptr, info_ptr, bitDepth)
        , fRowsWrittenToOutput(0)
        , fDst(nullptr)
        , fRowBytes(0)
        , fFirstRow(0)
        , fLastRow(0)
    {}

    static void AllRowsCallback(png_structp png_ptr, png_bytep row, png_uint_32 rowNum, int /*pass*/) {
        GetDecoder(png_ptr)->allRowsCallback(row, rowNum);
    }

    static void RowCallback(png_structp png_ptr, png_bytep row, png_uint_32 rowNum, int /*pass*/) {
        GetDecoder(png_ptr)->rowCallback(row, rowNum);
    }

private:
    int                         fRowsWrittenToOutput;
    void*                       fDst;
    size_t                      fRowBytes;

    // Variables for partial decode
    int                         fFirstRow;  // FIXME: Move to baseclass?
    int                         fLastRow;
    int                         fRowsNeeded;

    using INHERITED = SkPngCodec;

    static SkPngNormalDecoder* GetDecoder(png_structp png_ptr) {
        return static_cast<SkPngNormalDecoder*>(png_get_progressive_ptr(png_ptr));
    }

    Result decodeAllRows(void* dst, size_t rowBytes, int* rowsDecoded) override {
        const int height = this->dimensions().height();
        png_set_progressive_read_fn(this->png_ptr(), this, nullptr, AllRowsCallback, nullptr);
        fDst = dst;
        fRowBytes = rowBytes;

        fRowsWrittenToOutput = 0;
        fFirstRow = 0;
        fLastRow = height - 1;

        const bool success = this->processData();
        if (success && fRowsWrittenToOutput == height) {
            return kSuccess;
        }

        if (rowsDecoded) {
            *rowsDecoded = fRowsWrittenToOutput;
        }

        return log_and_return_error(success);
    }

    void allRowsCallback(png_bytep row, int rowNum) {
        SkASSERT(rowNum == fRowsWrittenToOutput);
        fRowsWrittenToOutput++;
        this->applyXformRow(fDst, row);
        fDst = SkTAddOffset<void>(fDst, fRowBytes);
    }

    void setRange(int firstRow, int lastRow, void* dst, size_t rowBytes) override {
        png_set_progressive_read_fn(this->png_ptr(), this, nullptr, RowCallback, nullptr);
        fFirstRow = firstRow;
        fLastRow = lastRow;
        fDst = dst;
        fRowBytes = rowBytes;
        fRowsWrittenToOutput = 0;
        fRowsNeeded = fLastRow - fFirstRow + 1;
    }

    Result decode(int* rowsDecoded) override {
        if (this->swizzler()) {
            const int sampleY = this->swizzler()->sampleY();
            fRowsNeeded = get_scaled_dimension(fLastRow - fFirstRow + 1, sampleY);
        }

        const bool success = this->processData();
        if (success && fRowsWrittenToOutput == fRowsNeeded) {
            return kSuccess;
        }

        if (rowsDecoded) {
            *rowsDecoded = fRowsWrittenToOutput;
        }

        return log_and_return_error(success);
    }

    void rowCallback(png_bytep row, int rowNum) {
        if (rowNum < fFirstRow) {
            // Ignore this row.
            return;
        }

        SkASSERT(rowNum <= fLastRow);
        SkASSERT(fRowsWrittenToOutput < fRowsNeeded);

        // If there is no swizzler, all rows are needed.
        if (!this->swizzler() || this->swizzler()->rowNeeded(rowNum - fFirstRow)) {
            this->applyXformRow(fDst, row);
            fDst = SkTAddOffset<void>(fDst, fRowBytes);
            fRowsWrittenToOutput++;
        }

        if (fRowsWrittenToOutput == fRowsNeeded) {
            // Fake error to stop decoding scanlines.
            longjmp(PNG_JMPBUF(this->png_ptr()), kStopDecoding);
        }
    }
};

class SkPngInterlacedDecoder : public SkPngCodec {
public:
    SkPngInterlacedDecoder(SkEncodedInfo&& info, std::unique_ptr<SkStream> stream,
            SkPngChunkReader* reader, png_structp png_ptr,
            png_infop info_ptr, int bitDepth, int numberPasses)
        : INHERITED(std::move(info), std::move(stream), reader, png_ptr, info_ptr, bitDepth)
        , fNumberPasses(numberPasses)
        , fFirstRow(0)
        , fLastRow(0)
        , fLinesDecoded(0)
        , fInterlacedComplete(false)
        , fPng_rowbytes(0)
    {}

    static void InterlacedRowCallback(png_structp png_ptr, png_bytep row, png_uint_32 rowNum, int pass) {
        auto decoder = static_cast<SkPngInterlacedDecoder*>(png_get_progressive_ptr(png_ptr));
        decoder->interlacedRowCallback(row, rowNum, pass);
    }

private:
    const int               fNumberPasses;
    int                     fFirstRow;
    int                     fLastRow;
    void*                   fDst;
    size_t                  fRowBytes;
    int                     fLinesDecoded;
    bool                    fInterlacedComplete;
    size_t                  fPng_rowbytes;
    SkAutoTMalloc<png_byte> fInterlaceBuffer;

    using INHERITED = SkPngCodec;

    // FIXME: Currently sharing interlaced callback for all rows and subset. It's not
    // as expensive as the subset version of non-interlaced, but it still does extra
    // work.
    void interlacedRowCallback(png_bytep row, int rowNum, int pass) {
        if (rowNum < fFirstRow || rowNum > fLastRow || fInterlacedComplete) {
            // Ignore this row
            return;
        }

        png_bytep oldRow = fInterlaceBuffer.get() + (rowNum - fFirstRow) * fPng_rowbytes;
        png_progressive_combine_row(this->png_ptr(), oldRow, row);

        if (0 == pass) {
            // The first pass initializes all rows.
            SkASSERT(row);
            SkASSERT(fLinesDecoded == rowNum - fFirstRow);
            fLinesDecoded++;
        } else {
            SkASSERT(fLinesDecoded == fLastRow - fFirstRow + 1);
            if (fNumberPasses - 1 == pass && rowNum == fLastRow) {
                // Last pass, and we have read all of the rows we care about.
                fInterlacedComplete = true;
                if (fLastRow != this->dimensions().height() - 1 ||
                        (this->swizzler() && this->swizzler()->sampleY() != 1)) {
                    // Fake error to stop decoding scanlines. Only stop if we're not decoding the
                    // whole image, in which case processing the rest of the image might be
                    // expensive. When decoding the whole image, read through the IEND chunk to
                    // preserve Android behavior of leaving the input stream in the right place.
                    longjmp(PNG_JMPBUF(this->png_ptr()), kStopDecoding);
                }
            }
        }
    }

    Result decodeAllRows(void* dst, size_t rowBytes, int* rowsDecoded) override {
        const int height = this->dimensions().height();
        this->setUpInterlaceBuffer(height);
        png_set_progressive_read_fn(this->png_ptr(), this, nullptr, InterlacedRowCallback,
                                    nullptr);

        fFirstRow = 0;
        fLastRow = height - 1;
        fLinesDecoded = 0;

        const bool success = this->processData();
        png_bytep srcRow = fInterlaceBuffer.get();
        // FIXME: When resuming, this may rewrite rows that did not change.
        for (int rowNum = 0; rowNum < fLinesDecoded; rowNum++) {
            this->applyXformRow(dst, srcRow);
            dst = SkTAddOffset<void>(dst, rowBytes);
            srcRow = SkTAddOffset<png_byte>(srcRow, fPng_rowbytes);
        }
        if (success && fInterlacedComplete) {
            return kSuccess;
        }

        if (rowsDecoded) {
            *rowsDecoded = fLinesDecoded;
        }

        return log_and_return_error(success);
    }

    void setRange(int firstRow, int lastRow, void* dst, size_t rowBytes) override {
        // FIXME: We could skip rows in the interlace buffer that we won't put in the output.
        this->setUpInterlaceBuffer(lastRow - firstRow + 1);
        png_set_progressive_read_fn(this->png_ptr(), this, nullptr, InterlacedRowCallback, nullptr);
        fFirstRow = firstRow;
        fLastRow = lastRow;
        fDst = dst;
        fRowBytes = rowBytes;
        fLinesDecoded = 0;
    }

    Result decode(int* rowsDecoded) override {
        const bool success = this->processData();

        // Now apply Xforms on all the rows that were decoded.
        if (!fLinesDecoded) {
            if (rowsDecoded) {
                *rowsDecoded = 0;
            }
            return log_and_return_error(success);
        }

        const int sampleY = this->swizzler() ? this->swizzler()->sampleY() : 1;
        const int rowsNeeded = get_scaled_dimension(fLastRow - fFirstRow + 1, sampleY);

        // FIXME: For resuming interlace, we may swizzle a row that hasn't changed. But it
        // may be too tricky/expensive to handle that correctly.

        // Offset srcRow by get_start_coord rows. We do not need to account for fFirstRow,
        // since the first row in fInterlaceBuffer corresponds to fFirstRow.
        int srcRow = get_start_coord(sampleY);
        void* dst = fDst;
        int rowsWrittenToOutput = 0;
        while (rowsWrittenToOutput < rowsNeeded && srcRow < fLinesDecoded) {
            png_bytep src = SkTAddOffset<png_byte>(fInterlaceBuffer.get(), fPng_rowbytes * srcRow);
            this->applyXformRow(dst, src);
            dst = SkTAddOffset<void>(dst, fRowBytes);

            rowsWrittenToOutput++;
            srcRow += sampleY;
        }

        if (success && fInterlacedComplete) {
            return kSuccess;
        }

        if (rowsDecoded) {
            *rowsDecoded = rowsWrittenToOutput;
        }
        return log_and_return_error(success);
    }

    void setUpInterlaceBuffer(int height) {
        fPng_rowbytes = png_get_rowbytes(this->png_ptr(), this->info_ptr());
        fInterlaceBuffer.reset(fPng_rowbytes * height);
        fInterlacedComplete = false;
    }
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
// @return if kSuccess, the caller is responsible for calling
//      png_destroy_read_struct(png_ptrp, info_ptrp).
//      Otherwise, the passed in fields (except stream) are unchanged.
static SkCodec::Result read_header(SkStream* stream, SkPngChunkReader* chunkReader,
                                   SkCodec** outCodec,
                                   png_structp* png_ptrp, png_infop* info_ptrp) {
    // The image is known to be a PNG. Decode enough to know the SkImageInfo.
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr,
                                                 sk_error_fn, sk_warning_fn);
    if (!png_ptr) {
        return SkCodec::kInternalError;
    }

#ifdef PNG_SET_OPTION_SUPPORTED
    // This setting ensures that we display images with incorrect CMF bytes.
    // See crbug.com/807324.
    png_set_option(png_ptr, PNG_MAXIMUM_INFLATE_WINDOW, PNG_OPTION_ON);
#endif

    AutoCleanPng autoClean(png_ptr, stream, chunkReader, outCodec);

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == nullptr) {
        return SkCodec::kInternalError;
    }

    autoClean.setInfoPtr(info_ptr);

    if (setjmp(PNG_JMPBUF(png_ptr))) {
        return SkCodec::kInvalidInput;
    }

#ifdef PNG_READ_UNKNOWN_CHUNKS_SUPPORTED
    // Hookup our chunkReader so we can see any user-chunks the caller may be interested in.
    // This needs to be installed before we read the png header.  Android may store ninepatch
    // chunks in the header.
    if (chunkReader) {
        png_set_keep_unknown_chunks(png_ptr, PNG_HANDLE_CHUNK_ALWAYS, (png_byte*)"", 0);
        png_set_read_user_chunk_fn(png_ptr, (png_voidp) chunkReader, sk_read_user_chunk);
    }
#endif

    const bool decodedBounds = autoClean.decodeBounds();

    if (!decodedBounds) {
        return SkCodec::kIncompleteInput;
    }

    // On success, decodeBounds releases ownership of png_ptr and info_ptr.
    if (png_ptrp) {
        *png_ptrp = png_ptr;
    }
    if (info_ptrp) {
        *info_ptrp = info_ptr;
    }

    // decodeBounds takes care of setting outCodec
    if (outCodec) {
        SkASSERT(*outCodec);
    }
    return SkCodec::kSuccess;
}

void AutoCleanPng::infoCallback(size_t idatLength) {
    png_uint_32 origWidth, origHeight;
    int bitDepth, encodedColorType;
    png_get_IHDR(fPng_ptr, fInfo_ptr, &origWidth, &origHeight, &bitDepth,
                 &encodedColorType, nullptr, nullptr, nullptr);

    // TODO: Should we support 16-bits of precision for gray images?
    if (bitDepth == 16 && (PNG_COLOR_TYPE_GRAY == encodedColorType ||
                           PNG_COLOR_TYPE_GRAY_ALPHA == encodedColorType)) {
        bitDepth = 8;
        png_set_strip_16(fPng_ptr);
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
                bitDepth = 8;
                png_set_packing(fPng_ptr);
            }

            color = SkEncodedInfo::kPalette_Color;
            // Set the alpha depending on if a transparency chunk exists.
            alpha = png_get_valid(fPng_ptr, fInfo_ptr, PNG_INFO_tRNS) ?
                    SkEncodedInfo::kUnpremul_Alpha : SkEncodedInfo::kOpaque_Alpha;
            break;
        case PNG_COLOR_TYPE_RGB:
            if (png_get_valid(fPng_ptr, fInfo_ptr, PNG_INFO_tRNS)) {
                // Convert to RGBA if transparency chunk exists.
                png_set_tRNS_to_alpha(fPng_ptr);
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
                bitDepth = 8;
                png_set_expand_gray_1_2_4_to_8(fPng_ptr);
            }

            if (png_get_valid(fPng_ptr, fInfo_ptr, PNG_INFO_tRNS)) {
                png_set_tRNS_to_alpha(fPng_ptr);
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

    const int numberPasses = png_set_interlace_handling(fPng_ptr);

    if (fOutCodec) {
        SkASSERT(nullptr == *fOutCodec);
        auto profile = read_color_profile(fPng_ptr, fInfo_ptr);
        if (profile) {
            switch (profile->profile()->data_color_space) {
                case skcms_Signature_CMYK:
                    profile = nullptr;
                    break;
                case skcms_Signature_Gray:
                    if (SkEncodedInfo::kGray_Color != color &&
                        SkEncodedInfo::kGrayAlpha_Color != color)
                    {
                        profile = nullptr;
                    }
                    break;
                default:
                    break;
            }
        }

        switch (encodedColorType) {
            case PNG_COLOR_TYPE_GRAY_ALPHA:{
                png_color_8p sigBits;
                if (png_get_sBIT(fPng_ptr, fInfo_ptr, &sigBits)) {
                    if (8 == sigBits->alpha && kGraySigBit_GrayAlphaIsJustAlpha == sigBits->gray) {
                        color = SkEncodedInfo::kXAlpha_Color;
                    }
                }
                break;
            }
            case PNG_COLOR_TYPE_RGB:{
                png_color_8p sigBits;
                if (png_get_sBIT(fPng_ptr, fInfo_ptr, &sigBits)) {
                    if (5 == sigBits->red && 6 == sigBits->green && 5 == sigBits->blue) {
                        // Recommend a decode to 565 if the sBIT indicates 565.
                        color = SkEncodedInfo::k565_Color;
                    }
                }
                break;
            }
        }

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
        if (encodedColorType != PNG_COLOR_TYPE_GRAY_ALPHA
            && SkEncodedInfo::kOpaque_Alpha == alpha) {
            png_color_8p sigBits;
            if (png_get_sBIT(fPng_ptr, fInfo_ptr, &sigBits)) {
                if (5 == sigBits->red && 6 == sigBits->green && 5 == sigBits->blue) {
                    SkAndroidFrameworkUtils::SafetyNetLog("190188264");
                }
            }
        }
#endif // SK_BUILD_FOR_ANDROID_FRAMEWORK

        SkEncodedInfo encodedInfo = SkEncodedInfo::Make(origWidth, origHeight, color, alpha,
                                                        bitDepth, std::move(profile));
        if (1 == numberPasses) {
            *fOutCodec = new SkPngNormalDecoder(std::move(encodedInfo),
                   std::unique_ptr<SkStream>(fStream), fChunkReader, fPng_ptr, fInfo_ptr, bitDepth);
        } else {
            *fOutCodec = new SkPngInterlacedDecoder(std::move(encodedInfo),
                    std::unique_ptr<SkStream>(fStream), fChunkReader, fPng_ptr, fInfo_ptr, bitDepth,
                    numberPasses);
        }
        static_cast<SkPngCodec*>(*fOutCodec)->setIdatLength(idatLength);
    }

    // Release the pointers, which are now owned by the codec or the caller is expected to
    // take ownership.
    this->releasePngPtrs();
}

SkPngCodec::SkPngCodec(SkEncodedInfo&& encodedInfo, std::unique_ptr<SkStream> stream,
                       SkPngChunkReader* chunkReader, void* png_ptr, void* info_ptr, int bitDepth)
    : INHERITED(std::move(encodedInfo), png_select_xform_format(encodedInfo), std::move(stream))
    , fPngChunkReader(SkSafeRef(chunkReader))
    , fPng_ptr(png_ptr)
    , fInfo_ptr(info_ptr)
    , fColorXformSrcRow(nullptr)
    , fBitDepth(bitDepth)
    , fIdatLength(0)
    , fDecodedIdat(false)
{}

SkPngCodec::~SkPngCodec() {
    this->destroyReadStruct();
}

void SkPngCodec::destroyReadStruct() {
    if (fPng_ptr) {
        // We will never have a nullptr fInfo_ptr with a non-nullptr fPng_ptr
        SkASSERT(fInfo_ptr);
        png_destroy_read_struct((png_struct**)&fPng_ptr, (png_info**)&fInfo_ptr, nullptr);
        fPng_ptr = nullptr;
        fInfo_ptr = nullptr;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Getting the pixels
///////////////////////////////////////////////////////////////////////////////

SkCodec::Result SkPngCodec::initializeXforms(const SkImageInfo& dstInfo, const Options& options) {
    if (setjmp(PNG_JMPBUF((png_struct*)fPng_ptr))) {
        SkCodecPrintf("Failed on png_read_update_info.\n");
        return kInvalidInput;
    }
    png_read_update_info(fPng_ptr, fInfo_ptr);

    // Reset fSwizzler and this->colorXform().  We can't do this in onRewind() because the
    // interlaced scanline decoder may need to rewind.
    fSwizzler.reset(nullptr);

    // If skcms directly supports the encoded PNG format, we should skip format
    // conversion in the swizzler (or skip swizzling altogether).
    bool skipFormatConversion = false;
    switch (this->getEncodedInfo().color()) {
        case SkEncodedInfo::kRGB_Color:
            if (this->getEncodedInfo().bitsPerComponent() != 16) {
                break;
            }
            [[fallthrough]];
        case SkEncodedInfo::kRGBA_Color:
        case SkEncodedInfo::kGray_Color:
            skipFormatConversion = this->colorXform();
            break;
        default:
            break;
    }
    if (skipFormatConversion && !options.fSubset) {
        fXformMode = kColorOnly_XformMode;
        return kSuccess;
    }

    if (SkEncodedInfo::kPalette_Color == this->getEncodedInfo().color()) {
        if (!this->createColorTable(dstInfo)) {
            return kInvalidInput;
        }
    }

    this->initializeSwizzler(dstInfo, options, skipFormatConversion);
    return kSuccess;
}

void SkPngCodec::initializeXformParams() {
    switch (fXformMode) {
        case kColorOnly_XformMode:
            fXformWidth = this->dstInfo().width();
            break;
        case kSwizzleColor_XformMode:
            fXformWidth = this->swizzler()->swizzleWidth();
            break;
        default:
            break;
    }
}

void SkPngCodec::initializeSwizzler(const SkImageInfo& dstInfo, const Options& options,
                                    bool skipFormatConversion) {
    SkImageInfo swizzlerInfo = dstInfo;
    Options swizzlerOptions = options;
    fXformMode = kSwizzleOnly_XformMode;
    if (this->colorXform() && this->xformOnDecode()) {
        if (SkEncodedInfo::kGray_Color == this->getEncodedInfo().color()) {
            swizzlerInfo = swizzlerInfo.makeColorType(kGray_8_SkColorType);
        } else {
            swizzlerInfo = swizzlerInfo.makeColorType(kXformSrcColorType);
        }
        if (kPremul_SkAlphaType == dstInfo.alphaType()) {
            swizzlerInfo = swizzlerInfo.makeAlphaType(kUnpremul_SkAlphaType);
        }

        fXformMode = kSwizzleColor_XformMode;

        // Here, we swizzle into temporary memory, which is not zero initialized.
        // FIXME (msarett):
        // Is this a problem?
        swizzlerOptions.fZeroInitialized = kNo_ZeroInitialized;
    }

    if (skipFormatConversion) {
        // We cannot skip format conversion when there is a color table.
        SkASSERT(!fColorTable);
        int srcBPP = 0;
        switch (this->getEncodedInfo().color()) {
            case SkEncodedInfo::kRGB_Color:
                SkASSERT(this->getEncodedInfo().bitsPerComponent() == 16);
                srcBPP = 6;
                break;
            case SkEncodedInfo::kRGBA_Color:
                srcBPP = this->getEncodedInfo().bitsPerComponent() / 2;
                break;
            case SkEncodedInfo::kGray_Color:
                srcBPP = 1;
                break;
            default:
                SkASSERT(false);
                break;
        }
        fSwizzler = SkSwizzler::MakeSimple(srcBPP, swizzlerInfo, swizzlerOptions);
    } else {
        const SkPMColor* colors = get_color_ptr(fColorTable.get());
        fSwizzler = SkSwizzler::Make(this->getEncodedInfo(), colors, swizzlerInfo,
                                     swizzlerOptions);
    }
    SkASSERT(fSwizzler);
}

SkSampler* SkPngCodec::getSampler(bool createIfNecessary) {
    if (fSwizzler || !createIfNecessary) {
        return fSwizzler.get();
    }

    this->initializeSwizzler(this->dstInfo(), this->options(), true);
    return fSwizzler.get();
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
    if (kSuccess != read_header(this->stream(), fPngChunkReader.get(), nullptr,
                                &png_ptr, &info_ptr)) {
        return false;
    }

    fPng_ptr = png_ptr;
    fInfo_ptr = info_ptr;
    fDecodedIdat = false;
    return true;
}

SkCodec::Result SkPngCodec::onGetPixels(const SkImageInfo& dstInfo, void* dst,
                                        size_t rowBytes, const Options& options,
                                        int* rowsDecoded) {
    Result result = this->initializeXforms(dstInfo, options);
    if (kSuccess != result) {
        return result;
    }

    if (options.fSubset) {
        return kUnimplemented;
    }

    this->allocateStorage(dstInfo);
    this->initializeXformParams();
    return this->decodeAllRows(dst, rowBytes, rowsDecoded);
}

SkCodec::Result SkPngCodec::onStartIncrementalDecode(const SkImageInfo& dstInfo,
        void* dst, size_t rowBytes, const SkCodec::Options& options) {
    Result result = this->initializeXforms(dstInfo, options);
    if (kSuccess != result) {
        return result;
    }

    this->allocateStorage(dstInfo);

    int firstRow, lastRow;
    if (options.fSubset) {
        firstRow = options.fSubset->top();
        lastRow = options.fSubset->bottom() - 1;
    } else {
        firstRow = 0;
        lastRow = dstInfo.height() - 1;
    }
    this->setRange(firstRow, lastRow, dst, rowBytes);
    return kSuccess;
}

SkCodec::Result SkPngCodec::onIncrementalDecode(int* rowsDecoded) {
    // FIXME: Only necessary on the first call.
    this->initializeXformParams();

    return this->decode(rowsDecoded);
}

std::unique_ptr<SkCodec> SkPngCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                    Result* result, SkPngChunkReader* chunkReader) {
    SkCodec* outCodec = nullptr;
    *result = read_header(stream.get(), chunkReader, &outCodec, nullptr, nullptr);
    if (kSuccess == *result) {
        // Codec has taken ownership of the stream.
        SkASSERT(outCodec);
        stream.release();
    }
    return std::unique_ptr<SkCodec>(outCodec);
}
