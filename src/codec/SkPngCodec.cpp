/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkPngCodec.h"

#include "include/codec/SkEncodedOrigin.h"
#include "include/codec/SkPngChunkReader.h"
#include "include/codec/SkPngDecoder.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkNoncopyable.h"
#include "include/private/base/SkTemplates.h"
#include "modules/skcms/skcms.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkPngCompositeChunkReader.h"
#include "src/codec/SkPngPriv.h"
#include "src/codec/SkSwizzler.h"

#include <csetjmp>
#include <algorithm>
#include <cstring>
#include <utility>

#include <png.h>
#include <pngconf.h>

using namespace skia_private;

#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    #include "include/android/SkAndroidFrameworkUtils.h"
#endif

// This warning triggers false positives way too often in here.
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
    AutoCleanPng(png_structp png_ptr,
                 SkStream* stream,
                 sk_sp<SkPngCompositeChunkReader> reader,
                 SkCodec** codecPtr)
            : fPng_ptr(png_ptr)
            , fInfo_ptr(nullptr)
            , fStream(stream)
            , fChunkReader(std::move(reader))
            , fOutCodec(codecPtr) {}

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
    sk_sp<SkPngCompositeChunkReader> fChunkReader;
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
    SkASSERT(fStream);
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

std::optional<SkSpan<const SkPngCodecBase::PaletteColorEntry>> SkPngCodec::onTryGetPlteChunk() {
    int numColors;
    png_color* palette;
    if (!png_get_PLTE(fPng_ptr, fInfo_ptr, &palette, &numColors)) {
        return std::nullopt;
    }

    static_assert(sizeof(png_color) == sizeof(PaletteColorEntry));
    return SkSpan(reinterpret_cast<const PaletteColorEntry*>(palette), numColors);
}

std::optional<SkSpan<const uint8_t>> SkPngCodec::onTryGetTrnsChunk() {
    png_bytep alphas;
    int numColorsWithAlpha = 0;
    if (!png_get_tRNS(fPng_ptr, fInfo_ptr, &alphas, &numColorsWithAlpha, nullptr)) {
        return std::nullopt;
    }
    return SkSpan(alphas, numColorsWithAlpha);
}

///////////////////////////////////////////////////////////////////////////////
// Creation
///////////////////////////////////////////////////////////////////////////////

bool SkPngCodec::IsPng(const void* buf, size_t bytesRead) {
    return !png_sig_cmp((png_const_bytep) buf, (png_size_t)0, bytesRead);
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
        // TODO(https://crbug.com/362304558): Consider the intent field from the
        // `sRGB` chunk.
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

static SkCodec::Result log_and_return_error(bool success) {
    if (success) return SkCodec::kIncompleteInput;
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
    SkAndroidFrameworkUtils::SafetyNetLog("117838472");
#endif
    return SkCodec::kErrorInInput;
}

class SkPngNormalDecoder : public SkPngCodec {
public:
    SkPngNormalDecoder(SkEncodedInfo&& info,
                       std::unique_ptr<SkStream> stream,
                       sk_sp<SkPngCompositeChunkReader> reader,
                       png_structp png_ptr,
                       png_infop info_ptr,
                       std::unique_ptr<SkStream> gainmapStream,
                       std::optional<SkGainmapInfo> gainmapInfo)
            : SkPngCodec(std::move(info),
                         std::move(stream),
                         std::move(reader),
                         png_ptr,
                         info_ptr,
                         std::move(gainmapStream),
                         gainmapInfo)
            , fRowsWrittenToOutput(0)
            , fDst(nullptr)
            , fRowBytes(0)
            , fFirstRow(0)
            , fLastRow(0) {}

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

    Result setRange(int firstRow, int lastRow, void* dst, size_t rowBytes) override {
        png_set_progressive_read_fn(this->png_ptr(), this, nullptr, RowCallback, nullptr);
        fFirstRow = firstRow;
        fLastRow = lastRow;
        fDst = dst;
        fRowBytes = rowBytes;
        fRowsWrittenToOutput = 0;
        fRowsNeeded = fLastRow - fFirstRow + 1;
        return kSuccess;
    }

    Result decode(int* rowsDecoded) override {
        if (this->swizzler()) {
            const int sampleY = this->swizzler()->sampleY();
            fRowsNeeded = SkCodecPriv::GetSampledDimension(fLastRow - fFirstRow + 1, sampleY);
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
    SkPngInterlacedDecoder(SkEncodedInfo&& info,
                           std::unique_ptr<SkStream> stream,
                           sk_sp<SkPngCompositeChunkReader> reader,
                           png_structp png_ptr,
                           png_infop info_ptr,
                           int numberPasses,
                           std::unique_ptr<SkStream> gainmapStream,
                           std::optional<SkGainmapInfo> gainmapInfo)
            : SkPngCodec(std::move(info),
                         std::move(stream),
                         std::move(reader),
                         png_ptr,
                         info_ptr,
                         std::move(gainmapStream),
                         gainmapInfo)
            , fNumberPasses(numberPasses)
            , fFirstRow(0)
            , fLastRow(0)
            , fLinesDecoded(0)
            , fInterlacedComplete(false)
            , fPng_rowbytes(0) {}

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
    std::unique_ptr<png_byte, SkOverloadedFunctionObject<void(void*), sk_free>> fInterlaceBuffer;

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
        Result res = this->setUpInterlaceBuffer(height);
        if (res != kSuccess) {
          return res;
        }
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

    Result setRange(int firstRow, int lastRow, void* dst, size_t rowBytes) override {
        // FIXME: We could skip rows in the interlace buffer that we won't put in the output.
        Result res = this->setUpInterlaceBuffer(lastRow - firstRow + 1);
        if (res != kSuccess) {
          return res;
        }
        png_set_progressive_read_fn(this->png_ptr(), this, nullptr, InterlacedRowCallback, nullptr);
        fFirstRow = firstRow;
        fLastRow = lastRow;
        fDst = dst;
        fRowBytes = rowBytes;
        fLinesDecoded = 0;
        return kSuccess;
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
        const int rowsNeeded = SkCodecPriv::GetSampledDimension(fLastRow - fFirstRow + 1, sampleY);

        // FIXME: For resuming interlace, we may swizzle a row that hasn't changed. But it
        // may be too tricky/expensive to handle that correctly.

        // Offset srcRow by SkCodecPriv::GetStartCoord rows. We do not need to account for
        // fFirstRow, since the first row in fInterlaceBuffer corresponds to fFirstRow.
        int srcRow = SkCodecPriv::GetStartCoord(sampleY);
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

    Result setUpInterlaceBuffer(int height) {
        fPng_rowbytes = png_get_rowbytes(this->png_ptr(), this->info_ptr());
        size_t interlaceBufferSize = fPng_rowbytes * height;
        void* interlaceBufferRaw = nullptr;
        if (interlaceBufferSize) {
           interlaceBufferRaw = sk_malloc_canfail(interlaceBufferSize, sizeof(png_byte));
           if (!interlaceBufferRaw) {
             return kInternalError;
           }
        }
        fInterlaceBuffer.reset(reinterpret_cast<png_byte*>(interlaceBufferRaw));
        fInterlacedComplete = false;
        return kSuccess;
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
static SkCodec::Result read_header(SkStream* stream, const sk_sp<SkPngCompositeChunkReader>& chunkReader,
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
        png_set_keep_unknown_chunks(png_ptr, PNG_HANDLE_CHUNK_ALWAYS, (png_const_bytep)"", 0);
        png_set_read_user_chunk_fn(png_ptr, (png_voidp)chunkReader.get(), sk_read_user_chunk);
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

    // TODO(https://crbug.com/359245096): Should we support 16-bits of precision
    // for gray images?
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
        if (!SkPngCodecBase::isCompatibleColorProfileAndType(profile.get(), color)) {
            profile = nullptr;
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
                                                std::unique_ptr<SkStream>(fStream),
                                                fChunkReader,
                                                fPng_ptr,
                                                fInfo_ptr,
                                                fChunkReader->takeGaimapStream(),
                                                fChunkReader->getGainmapInfo());
        } else {
            *fOutCodec = new SkPngInterlacedDecoder(std::move(encodedInfo),
                                                    std::unique_ptr<SkStream>(fStream),
                                                    fChunkReader,
                                                    fPng_ptr,
                                                    fInfo_ptr,
                                                    numberPasses,
                                                    fChunkReader->takeGaimapStream(),
                                                    fChunkReader->getGainmapInfo());
        }
        static_cast<SkPngCodec*>(*fOutCodec)->setIdatLength(idatLength);
    }

    // Release the pointers, which are now owned by the codec or the caller is expected to
    // take ownership.
    this->releasePngPtrs();
}

// TODO(https://crbug.com/390707316): Consider adding handling of eXIF chunks
// for parity with Blink.
constexpr SkEncodedOrigin kDefaultEncodedOrigin = kTopLeft_SkEncodedOrigin;

SkPngCodec::SkPngCodec(SkEncodedInfo&& encodedInfo,
                       std::unique_ptr<SkStream> stream,
                       sk_sp<SkPngCompositeChunkReader> chunkReader,
                       void* png_ptr,
                       void* info_ptr,
                       std::unique_ptr<SkStream> gainmapStream,
                       std::optional<SkGainmapInfo> gainmapInfo)
        : SkPngCodecBase(std::move(encodedInfo), std::move(stream), kDefaultEncodedOrigin)
        , fPngChunkReader(std::move(chunkReader))
        , fPng_ptr(png_ptr)
        , fInfo_ptr(info_ptr)
        , fIdatLength(0)
        , fDecodedIdat(false)
        , fGainmapStream(std::move(gainmapStream))
        , fGainmapInfo(gainmapInfo) {}

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

    // `SkPngCodec` doesn't support APNG - the `frameWidth` is always the same
    // as the full image width.
    int frameWidth = dstInfo.width();

    return SkPngCodecBase::initializeXforms(dstInfo, options, frameWidth);
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
    if (kSuccess != read_header(this->stream(), fPngChunkReader, nullptr,
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

    this->initializeXformParams();
    return this->decodeAllRows(dst, rowBytes, rowsDecoded);
}

SkCodec::Result SkPngCodec::onStartIncrementalDecode(const SkImageInfo& dstInfo,
        void* dst, size_t rowBytes, const SkCodec::Options& options) {
    Result result = this->initializeXforms(dstInfo, options);
    if (kSuccess != result) {
        return result;
    }

    int firstRow, lastRow;
    if (options.fSubset) {
        firstRow = options.fSubset->top();
        lastRow = options.fSubset->bottom() - 1;
    } else {
        firstRow = 0;
        lastRow = dstInfo.height() - 1;
    }
    return this->setRange(firstRow, lastRow, dst, rowBytes);
}

SkCodec::Result SkPngCodec::onIncrementalDecode(int* rowsDecoded) {
    // FIXME: Only necessary on the first call.
    this->initializeXformParams();

    return this->decode(rowsDecoded);
}

std::unique_ptr<SkCodec> SkPngCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                    Result* result, SkPngChunkReader* chunkReader) {
    SkASSERT(result);
    if (!stream) {
        *result = SkCodec::kInvalidInput;
        return nullptr;
    }
    SkCodec* outCodec = nullptr;
    auto compositeReader = sk_make_sp<SkPngCompositeChunkReader>(chunkReader);
    *result = read_header(stream.get(), compositeReader, &outCodec, nullptr, nullptr);
    if (kSuccess == *result) {
        // Codec has taken ownership of the stream.
        SkASSERT(outCodec);
        stream.release();
    }
    return std::unique_ptr<SkCodec>(outCodec);
}

bool SkPngCodec::onGetGainmapCodec(SkGainmapInfo* info, std::unique_ptr<SkCodec>* gainmapCodec) {
    if (!fGainmapStream) {
        return false;
    }

    sk_sp<SkData> data = fGainmapStream->getData();
    if (!data) {
        return false;
    }

    if (!SkPngDecoder::IsPng(data->bytes(), data->size())) {
        return false;
    }

    // The gainmap information lives on the gainmap image itself, so we need to
    // create the gainmap codec first, then check if it has a metadata chunk.
    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkPngCodec::MakeFromStream(fGainmapStream->duplicate(), &result, fPngChunkReader.get());

    if (result != SkCodec::Result::kSuccess) {
        return false;
    }

    bool hasInfo = codec->onGetGainmapInfo(info);

    if (hasInfo && gainmapCodec) {
        // The ISO gainmap payload does not contain the actual alterative image
        // primaries, so we need to query the ICC profile stored on the gainmap.
        if (info->fGainmapMathColorSpace) {
            const auto iccProfile = codec->getEncodedInfo().profile();
            if (iccProfile) {
                auto colorSpace = SkColorSpace::Make(*iccProfile);
                if (colorSpace) {
                    info->fGainmapMathColorSpace = std::move(colorSpace);
                }
            }
        }

        *gainmapCodec = std::move(codec);
    }

    return hasInfo;
}

bool SkPngCodec::onGetGainmapInfo(SkGainmapInfo* info) {
    if (fGainmapInfo) {
        if (info) {
            *info = *fGainmapInfo;
        }
        return true;
    }

    return false;
}

namespace SkPngDecoder {
bool IsPng(const void* data, size_t len) {
    return SkPngCodec::IsPng(data, len);
}

std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream> stream,
                                SkCodec::Result* outResult,
                                SkCodecs::DecodeContext ctx) {
    SkCodec::Result resultStorage;
    if (!outResult) {
        outResult = &resultStorage;
    }
    SkPngChunkReader* chunkReader = nullptr;
    if (ctx) {
        chunkReader = static_cast<SkPngChunkReader*>(ctx);
    }
    return SkPngCodec::MakeFromStream(std::move(stream), outResult, chunkReader);
}

std::unique_ptr<SkCodec> Decode(sk_sp<SkData> data,
                                SkCodec::Result* outResult,
                                SkCodecs::DecodeContext ctx) {
    if (!data) {
        if (outResult) {
            *outResult = SkCodec::kInvalidInput;
        }
        return nullptr;
    }
    return Decode(SkMemoryStream::Make(std::move(data)), outResult, ctx);
}
}  // namespace SkPngDecoder
