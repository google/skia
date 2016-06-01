/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCodecPriv.h"
#include "SkColorPriv.h"
#include "SkColorSpace.h"
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

// When setjmp is first called, it returns 0, meaning longjmp was not called.
constexpr int kSetJmpOkay   = 0;
// An error internal to libpng.
constexpr int kPngError     = 1;
// Passed to longjmp when we have decoded as many lines as we need.
constexpr int kStopDecoding = 2;

static void sk_error_fn(png_structp png_ptr, png_const_charp msg) {
    SkCodecPrintf("------ png error %s\n", msg);
    longjmp(png_jmpbuf(png_ptr), kPngError);
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
        , fDecodedBounds(false)
        , fReadHeader(false)
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
    bool                fDecodedBounds;
    bool                fReadHeader;
    SkStream*           fStream;
    SkPngChunkReader*   fChunkReader;
    SkCodec**           fOutCodec;

    /**
     *  Supplied to libpng to call when it has read enough data to determine
     *  bounds.
     */
    static void InfoCallback(png_structp png_ptr, png_infop info_ptr) {
        // png_get_progressive_ptr returns the pointer we set on the png_ptr with
        // png_set_progressive_read_fn
        static_cast<AutoCleanPng*>(png_get_progressive_ptr(png_ptr))->infoCallback();
    }

    void infoCallback();

    void releasePngPtrs() {
        fPng_ptr = nullptr;
        fInfo_ptr = nullptr;
    }
};
#define AutoCleanPng(...) SK_REQUIRE_LOCAL_VAR(AutoCleanPng)

bool AutoCleanPng::decodeBounds() {
    if (setjmp(png_jmpbuf(fPng_ptr))) {
        return false;
    }

    png_set_progressive_read_fn(fPng_ptr, this, InfoCallback, nullptr, nullptr);

    // Arbitrary buffer size, though note that it matches (below)
    // SkPngCodec::processData(). FIXME: Can we better suit this to the size of
    // the PNG header?
    constexpr size_t kBufferSize = 4096;
    char buffer[kBufferSize];

    while (true) {
        const size_t bytesRead = fStream->read(buffer, kBufferSize);
        if (!bytesRead) {
            // We have read to the end of the input without decoding bounds.
            break;
        }

        png_process_data(fPng_ptr, fInfo_ptr, (png_bytep) buffer, bytesRead);
        if (fReadHeader) {
            break;
        }
    }

    // For safety, clear the pointer to this object.
    png_set_progressive_read_fn(fPng_ptr, nullptr, nullptr, nullptr, nullptr);
    return fDecodedBounds;
}

void SkPngCodec::processData() {
    switch (setjmp(png_jmpbuf(fPng_ptr))) {
        case kPngError:
            // There was an error. Stop processing data.
            // FIXME: Do we need to discard png_ptr?
            return;
        case kStopDecoding:
            // We decoded all the lines we want.
            return;
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

    while (true) {
        const size_t bytesRead = this->stream()->read(buffer, kBufferSize);
        png_process_data(fPng_ptr, fInfo_ptr, (png_bytep) buffer, bytesRead);

        if (!bytesRead) {
            // We have read to the end of the input. Note that we quit *after*
            // calling png_process_data, because decodeBounds may have told
            // libpng to save the remainder of the buffer, in which case
            // png_process_data will process the saved buffer, though the
            // stream has no more to read.
            break;
        }
    }
}

// Note: SkColorTable claims to store SkPMColors, which is not necessarily
// the case here.
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

        if (PNG_INFO_gAMA == png_get_gAMA_fixed(png_ptr, info_ptr, &gamma)) {
            float value = png_inverted_fixed_point_to_float(gamma);
            gammas[0] = value;
            gammas[1] = value;
            gammas[2] = value;

        } else {
            // Default to sRGB (gamma = 2.2f) if the image has color space information,
            // but does not specify gamma.
            gammas[0] = 2.2f;
            gammas[1] = 2.2f;
            gammas[2] = 2.2f;
        }

        SkMatrix44 mat(SkMatrix44::kUninitialized_Constructor);
        mat.set3x3ColMajorf(toXYZD50);
        return SkColorSpace::NewRGB(gammas, mat);
    }

    // Last, check for gamma.
    if (PNG_INFO_gAMA == png_get_gAMA_fixed(png_ptr, info_ptr, &gamma)) {

        // Guess a default value for cHRM?  Or should we just give up?
        // Here we use the identity matrix as a default.

        // Set the gammas.
        float value = png_inverted_fixed_point_to_float(gamma);
        gammas[0] = value;
        gammas[1] = value;
        gammas[2] = value;

        return SkColorSpace::NewRGB(gammas, SkMatrix44::I());
    }

#endif // LIBPNG >= 1.6

    // Finally, what should we do if there is no color space information in the PNG?
    // The specification says that this indicates "gamma is unknown" and that the
    // "color is device dependent".  I'm assuming we can represent this with NULL.
    // But should we guess sRGB?  Most images are sRGB, even if they don't specify.
    return nullptr;
}

class SkPngNormalDecoder : public SkPngCodec {
public:
    SkPngNormalDecoder(int width, int height, const SkEncodedInfo& info, SkStream* stream,
            SkPngChunkReader* reader, png_structp png_ptr, png_infop info_ptr, int bitDepth,
            sk_sp<SkColorSpace> colorSpace)
        : INHERITED(width, height, info, stream, reader, png_ptr, info_ptr, bitDepth,
                std::move(colorSpace))
        , fLinesDecoded(0)
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
    int                         fLinesDecoded; // FIXME: Move to baseclass?
    void*                       fDst;
    size_t                      fRowBytes;

    // Variables for partial decode
    int                         fFirstRow;  // FIXME: Move to baseclass?
    int                         fLastRow;

    typedef SkPngCodec INHERITED;

    static SkPngNormalDecoder* GetDecoder(png_structp png_ptr) {
        return static_cast<SkPngNormalDecoder*>(png_get_progressive_ptr(png_ptr));
    }

    Result decodeAllRows(void* dst, size_t rowBytes, int* rowsDecoded) override {
        const int height = this->getInfo().height();
        png_set_progressive_read_fn(this->png_ptr(), this, nullptr, AllRowsCallback, nullptr);
        fDst = dst;
        fRowBytes = rowBytes;

        fLinesDecoded = 0;

        this->processData();

        if (fLinesDecoded == height) {
            return SkCodec::kSuccess;
        }

        if (rowsDecoded) {
            *rowsDecoded = fLinesDecoded;
        }

        return SkCodec::kIncompleteInput;
    }

    void allRowsCallback(png_bytep row, int rowNum) {
        SkASSERT(rowNum - fFirstRow == fLinesDecoded);
        fLinesDecoded++;
        this->swizzler()->swizzle(fDst, row);
        fDst = SkTAddOffset<void>(fDst, fRowBytes);
    }

    void setRange(int firstRow, int lastRow, void* dst, size_t rowBytes) override {
        png_set_progressive_read_fn(this->png_ptr(), this, nullptr, RowCallback, nullptr);
        fFirstRow = firstRow;
        fLastRow = lastRow;
        fDst = dst;
        fRowBytes = rowBytes;
        fLinesDecoded = 0;
    }

    SkCodec::Result decode(int* rowsDecoded) override {
        this->processData();

        if (fLinesDecoded == fLastRow - fFirstRow + 1) {
            return SkCodec::kSuccess;
        }

        if (rowsDecoded) {
            *rowsDecoded = fLinesDecoded;
        }

        return SkCodec::kIncompleteInput;
    }

    void rowCallback(png_bytep row, int rowNum) {
        if (rowNum < fFirstRow) {
            // Ignore this row.
            return;
        }

        SkASSERT(rowNum <= fLastRow);

        if (this->swizzler()->rowNeeded(fLinesDecoded)) {
            this->swizzler()->swizzle(fDst, row);
            fDst = SkTAddOffset<void>(fDst, fRowBytes);
        }

        fLinesDecoded++;

        if (rowNum == fLastRow) {
            // Fake error to stop decoding scanlines.
            longjmp(png_jmpbuf(this->png_ptr()), kStopDecoding);
        }
    }
};

class SkPngInterlacedDecoder : public SkPngCodec {
public:
    SkPngInterlacedDecoder(int width, int height, const SkEncodedInfo& info, SkStream* stream,
            SkPngChunkReader* reader, png_structp png_ptr, png_infop info_ptr, int bitDepth,
            sk_sp<SkColorSpace> colorSpace, int numberPasses)
        : INHERITED(width, height, info, stream, reader, png_ptr, info_ptr, bitDepth,
                std::move(colorSpace))
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

    typedef SkPngCodec INHERITED;

    // FIXME: Currently sharing interlaced callback for all rows and subset. It's not
    // as expensive as the subset version of non-interlaced, but it still does extra
    // work.
    void interlacedRowCallback(png_bytep row, int rowNum, int pass) {
        if (rowNum < fFirstRow || rowNum > fLastRow) {
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
                // Last pass, and we have read all of the rows we care about. Note that
                // we do not care about reading anything beyond the end of the image (or
                // beyond the last scanline requested).
                fInterlacedComplete = true;
                // Fake error to stop decoding scanlines.
                longjmp(png_jmpbuf(this->png_ptr()), kStopDecoding);
            }
        }
    }

    SkCodec::Result decodeAllRows(void* dst, size_t rowBytes, int* rowsDecoded) override {
        const int height = this->getInfo().height();
        this->setUpInterlaceBuffer(height);
        png_set_progressive_read_fn(this->png_ptr(), this, nullptr, InterlacedRowCallback, nullptr);

        fFirstRow = 0;
        fLastRow = height - 1;
        fLinesDecoded = 0;

        this->processData();

        png_bytep srcRow = fInterlaceBuffer.get();
        // FIXME: When resuming, this may rewrite rows that did not change.
        for (int rowNum = 0; rowNum < fLinesDecoded; rowNum++) {
            this->swizzler()->swizzle(dst, srcRow);
            dst = SkTAddOffset<void>(dst, rowBytes);
            srcRow = SkTAddOffset<png_byte>(srcRow, fPng_rowbytes);
        }
        if (fInterlacedComplete) {
            return SkCodec::kSuccess;
        }

        if (rowsDecoded) {
            *rowsDecoded = fLinesDecoded;
        }

        return SkCodec::kIncompleteInput;
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

    SkCodec::Result decode(int* rowsDecoded) override {
        this->processData();

        // Now call the callback on all the rows that were decoded.
        if (!fLinesDecoded) {
            return SkCodec::kIncompleteInput;
        }
        const int lastRow = fLinesDecoded + fFirstRow - 1;
        SkASSERT(lastRow <= fLastRow);

        // FIXME: For resuming interlace, we may swizzle a row that hasn't changed. But it
        // may be too tricky/expensive to handle that correctly.
        png_bytep srcRow = fInterlaceBuffer.get();
        const int sampleY = this->swizzler()->sampleY();
        void* dst = fDst;
        for (int rowNum = fFirstRow; rowNum <= lastRow; rowNum += sampleY) {
            this->swizzler()->swizzle(dst, srcRow);
            dst = SkTAddOffset<void>(dst, fRowBytes);
            srcRow = SkTAddOffset<png_byte>(srcRow, fPng_rowbytes * sampleY);
        }

        if (fInterlacedComplete) {
            return SkCodec::kSuccess;
        }

        if (rowsDecoded) {
            *rowsDecoded = fLinesDecoded;
        }
        return SkCodec::kIncompleteInput;
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

    AutoCleanPng autoClean(png_ptr, stream, chunkReader, outCodec);

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
        return false;
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
    return true;
}

void AutoCleanPng::infoCallback() {
    png_uint_32 origWidth, origHeight;
    int bitDepth, encodedColorType;
    png_get_IHDR(fPng_ptr, fInfo_ptr, &origWidth, &origHeight, &bitDepth,
                 &encodedColorType, nullptr, nullptr, nullptr);

    // Tell libpng to strip 16 bit/color files down to 8 bits/color.
    // TODO: Should we handle this in SkSwizzler?  Could this also benefit
    //       RAW decodes?
    if (bitDepth == 16) {
        SkASSERT(PNG_COLOR_TYPE_PALETTE != encodedColorType);
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

    fReadHeader = true;
#if PNG_LIBPNG_VER_MAJOR > 1 || (PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR >= 5)
    // 1 tells libpng to save any extra data. We may be able to be more efficient by saving
    // it ourselves.
    png_process_data_pause(fPng_ptr, 1);
    fDecodedBounds = true;
#else
    // We may have read more than the header. Empty buffer and move to the end of the
    // header so future calls can read the rows.
    fDecodedBounds = fStream->move(-fPng_ptr->buffer_size);
    fPng_ptr->buffer_size = 0;
    if (!fDecodedBounds) {
        // Stream could not be moved to the correct place.
        return;
    }
#endif
    if (fOutCodec) {
        SkASSERT(nullptr == *fOutCodec);
        sk_sp<SkColorSpace> colorSpace = read_color_space(fPng_ptr, fInfo_ptr);
        if (!colorSpace) {
            // Treat unmarked pngs as sRGB.
            colorSpace = SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named);
        }
        SkEncodedInfo info = SkEncodedInfo::Make(color, alpha, 8);
        if (1 == numberPasses) {
            *fOutCodec = new SkPngNormalDecoder(origWidth, origHeight, info, fStream,
                    fChunkReader, fPng_ptr, fInfo_ptr, bitDepth, std::move(colorSpace));
        } else {
            *fOutCodec = new SkPngInterlacedDecoder(origWidth, origHeight, info, fStream,
                    fChunkReader, fPng_ptr, fInfo_ptr, bitDepth, std::move(colorSpace),
                    numberPasses);
        }
    }


    // Release the pointers, which are now owned by the codec or the caller is expected to
    // take ownership.
    this->releasePngPtrs();
}

SkPngCodec::SkPngCodec(int width, int height, const SkEncodedInfo& info, SkStream* stream,
                       SkPngChunkReader* chunkReader, png_structp png_ptr, png_infop info_ptr,
                       int bitDepth, sk_sp<SkColorSpace> colorSpace)
    : INHERITED(width, height, info, stream, colorSpace)
    , fPngChunkReader(SkSafeRef(chunkReader))
    , fPng_ptr(png_ptr)
    , fInfo_ptr(info_ptr)
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

bool SkPngCodec::initializeSwizzler(const SkImageInfo& requestedInfo,
                                    const Options& options,
                                    SkPMColor ctable[],
                                    int* ctableCount) {
    if (setjmp(png_jmpbuf(fPng_ptr))) {
        return false;
    }
    png_read_update_info(fPng_ptr, fInfo_ptr);

    if (SkEncodedInfo::kPalette_Color == this->getEncodedInfo().color()) {
        if (!this->createColorTable(requestedInfo.colorType(),
                kPremul_SkAlphaType == requestedInfo.alphaType(), ctableCount)) {
            return false;
        }
    }

    // Copy the color table to the client if they request kIndex8 mode
    copy_color_table(requestedInfo, fColorTable, ctable, ctableCount);

    // Create the swizzler.  SkPngCodec retains ownership of the color table.
    const SkPMColor* colors = get_color_ptr(fColorTable.get());
    fSwizzler.reset(SkSwizzler::CreateSwizzler(this->getEncodedInfo(), colors, requestedInfo,
            options));
    SkASSERT(fSwizzler);

    return true;
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
    if (!this->initializeSwizzler(requestedInfo, options, ctable, ctableCount)) {
        return kInvalidInput;   // or parameters?
    }

    return this->decodeAllRows(dst, dstRowBytes, rowsDecoded);
}

SkCodec::Result SkPngCodec::onStartIncrementalDecode(const SkImageInfo& dstInfo,
        void* dst, size_t rowBytes, const SkCodec::Options& options,
        SkPMColor* ctable, int* ctableCount) {
    if (!conversion_possible(dstInfo, this->getInfo())) {
        return kInvalidConversion;
    }

    if (!this->initializeSwizzler(dstInfo, options, ctable, ctableCount)) {
        return kInvalidInput;
    }

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
    return this->decode(rowsDecoded);
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

    SkCodec* outCodec = nullptr;
    if (read_header(stream, chunkReader, &outCodec, nullptr, nullptr)) {
        // Codec has taken ownership of the stream.
        SkASSERT(outCodec);
        streamDeleter.release();
        return outCodec;
    }

    return nullptr;
}
