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
#include "SkPoint3.h"
#include "SkSize.h"
#include "SkStream.h"
#include "SkSwizzler.h"
#include "SkTemplates.h"
#include "SkUtils.h"

// This warning triggers false postives way too often in here.
#if defined(__GNUC__) && !defined(__clang__)
    #pragma GCC diagnostic ignored "-Wclobbered"
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

static inline SkAlphaType xform_alpha_type(SkAlphaType dstAlphaType, SkAlphaType srcAlphaType) {
    return (kOpaque_SkAlphaType == srcAlphaType) ? kOpaque_SkAlphaType : dstAlphaType;
}

// Note: SkColorTable claims to store SkPMColors, which is not necessarily the case here.
bool SkPngCodec::createColorTable(const SkImageInfo& dstInfo, int* ctableCount) {

    int numColors;
    png_color* palette;
    if (!png_get_PLTE(fPng_ptr, fInfo_ptr, &palette, &numColors)) {
        return false;
    }

    // Contents depend on tableColorType and our choice of if/when to premultiply:
    // { kPremul, kUnpremul, kOpaque } x { RGBA, BGRA }
    SkPMColor colorTable[256];
    SkColorType tableColorType = fColorXform ? kRGBA_8888_SkColorType : dstInfo.colorType();

    png_bytep alphas;
    int numColorsWithAlpha = 0;
    if (png_get_tRNS(fPng_ptr, fInfo_ptr, &alphas, &numColorsWithAlpha, nullptr)) {
        // If we are performing a color xform, it will handle the premultiply.  Otherwise,
        // we'll do it here.
        bool premultiply =  !fColorXform && needs_premul(dstInfo, this->getInfo());

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
            SkOpts::RGB_to_RGB1(colorTable + numColorsWithAlpha, palette,
                    numColors - numColorsWithAlpha);
        } else {
            SkOpts::RGB_to_BGR1(colorTable + numColorsWithAlpha, palette,
                    numColors - numColorsWithAlpha);
        }
    }

    // If we are not decoding to F16, we can color xform now and store the results
    // in the color table.
    if (fColorXform && kRGBA_F16_SkColorType != dstInfo.colorType()) {
        SkColorType xformColorType = is_rgba(dstInfo.colorType()) ?
                kRGBA_8888_SkColorType : kBGRA_8888_SkColorType;
        SkAlphaType xformAlphaType = xform_alpha_type(dstInfo.alphaType(),
                                                      this->getInfo().alphaType());
        fColorXform->apply(colorTable, colorTable, numColors, xformColorType, xformAlphaType);
    }

    // Pad the color table with the last color in the table (or black) in the case that
    // invalid pixel indices exceed the number of colors in the table.
    const int maxColors = 1 << fBitDepth;
    if (numColors < maxColors) {
        SkPMColor lastColor = numColors > 0 ? colorTable[numColors - 1] : SK_ColorBLACK;
        sk_memset32(colorTable + numColors, lastColor, maxColors - numColors);
    }

    // Set the new color count.
    if (ctableCount != nullptr) {
        *ctableCount = maxColors;
    }

    fColorTable.reset(new SkColorTable(colorTable, maxColors));
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

static bool convert_to_D50(SkMatrix44* toXYZD50, float toXYZ[9], float whitePoint[2]) {
    float wX = whitePoint[0];
    float wY = whitePoint[1];
    if (wX < 0.0f || wY < 0.0f || (wX + wY > 1.0f)) {
        return false;
    }

    // Calculate the XYZ illuminant.  Call this the src illuminant.
    float wZ = 1.0f - wX - wY;
    float scale = 1.0f / wY;
    // TODO (msarett):
    // What are common src illuminants?  I'm guessing we will almost always see D65.  Should
    // we go ahead and save a precomputed D65->D50 Bradford matrix?  Should we exit early if
    // if the src illuminant is D50?
    SkVector3 srcXYZ = SkVector3::Make(wX * scale, 1.0f, wZ * scale);

    // The D50 illuminant.
    SkVector3 dstXYZ = SkVector3::Make(0.96422f, 1.0f, 0.82521f);

    // Calculate the chromatic adaptation matrix.  We will use the Bradford method, thus
    // the matrices below.  The Bradford method is used by Adobe and is widely considered
    // to be the best.
    // http://www.brucelindbloom.com/index.html?Eqn_ChromAdapt.html
    SkMatrix mA, mAInv;
    mA.setAll(0.8951f, 0.2664f, -0.1614f, -0.7502f, 1.7135f, 0.0367f, 0.0389f, -0.0685f, 1.0296f);
    mAInv.setAll(0.9869929f, -0.1470543f, 0.1599627f, 0.4323053f, 0.5183603f, 0.0492912f,
                 -0.0085287f, 0.0400428f, 0.9684867f);

    // Map illuminant into cone response domain.
    SkVector3 srcCone;
    srcCone.fX = mA[0] * srcXYZ.fX + mA[1] * srcXYZ.fY + mA[2] * srcXYZ.fZ;
    srcCone.fY = mA[3] * srcXYZ.fX + mA[4] * srcXYZ.fY + mA[5] * srcXYZ.fZ;
    srcCone.fZ = mA[6] * srcXYZ.fX + mA[7] * srcXYZ.fY + mA[8] * srcXYZ.fZ;
    SkVector3 dstCone;
    dstCone.fX = mA[0] * dstXYZ.fX + mA[1] * dstXYZ.fY + mA[2] * dstXYZ.fZ;
    dstCone.fY = mA[3] * dstXYZ.fX + mA[4] * dstXYZ.fY + mA[5] * dstXYZ.fZ;
    dstCone.fZ = mA[6] * dstXYZ.fX + mA[7] * dstXYZ.fY + mA[8] * dstXYZ.fZ;

    SkMatrix DXToD50;
    DXToD50.setIdentity();
    DXToD50[0] = dstCone.fX / srcCone.fX;
    DXToD50[4] = dstCone.fY / srcCone.fY;
    DXToD50[8] = dstCone.fZ / srcCone.fZ;
    DXToD50.postConcat(mAInv);
    DXToD50.preConcat(mA);

    SkMatrix toXYZ3x3;
    toXYZ3x3.setAll(toXYZ[0], toXYZ[3], toXYZ[6], toXYZ[1], toXYZ[4], toXYZ[7], toXYZ[2], toXYZ[5],
                    toXYZ[8]);
    toXYZ3x3.postConcat(DXToD50);

    toXYZD50->set3x3(toXYZ3x3[0], toXYZ3x3[1], toXYZ3x3[2], toXYZ3x3[3], toXYZ3x3[4], toXYZ3x3[5],
                     toXYZ3x3[6], toXYZ3x3[7], toXYZ3x3[8]);
    return true;
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
    png_fixed_point toXYZFixed[9];
    float toXYZ[9];
    png_fixed_point whitePointFixed[2];
    float whitePoint[2];
    png_fixed_point gamma;
    float gammas[3];
    if (png_get_cHRM_XYZ_fixed(png_ptr, info_ptr, &toXYZFixed[0], &toXYZFixed[1], &toXYZFixed[2],
                               &toXYZFixed[3], &toXYZFixed[4], &toXYZFixed[5], &toXYZFixed[6],
                               &toXYZFixed[7], &toXYZFixed[8]) &&
        png_get_cHRM_fixed(png_ptr, info_ptr, &whitePointFixed[0], &whitePointFixed[1], nullptr,
                           nullptr, nullptr, nullptr, nullptr, nullptr))
    {
        for (int i = 0; i < 9; i++) {
            toXYZ[i] = png_fixed_point_to_float(toXYZFixed[i]);
        }
        whitePoint[0] = png_fixed_point_to_float(whitePointFixed[0]);
        whitePoint[1] = png_fixed_point_to_float(whitePointFixed[1]);

        SkMatrix44 toXYZD50(SkMatrix44::kUninitialized_Constructor);
        if (!convert_to_D50(&toXYZD50, toXYZ, whitePoint)) {
            toXYZD50.set3x3RowMajorf(gSRGB_toXYZD50);
        }

        if (PNG_INFO_gAMA == png_get_gAMA_fixed(png_ptr, info_ptr, &gamma)) {
            float value = png_inverted_fixed_point_to_float(gamma);
            gammas[0] = value;
            gammas[1] = value;
            gammas[2] = value;

            return SkColorSpace_Base::NewRGB(gammas, toXYZD50);
        }

        // Default to sRGB gamma if the image has color space information,
        // but does not specify gamma.
        return SkColorSpace::NewRGB(SkColorSpace::kSRGB_GammaNamed, toXYZD50);
    }

    // Last, check for gamma.
    if (PNG_INFO_gAMA == png_get_gAMA_fixed(png_ptr, info_ptr, &gamma)) {

        // Set the gammas.
        float value = png_inverted_fixed_point_to_float(gamma);
        gammas[0] = value;
        gammas[1] = value;
        gammas[2] = value;

        // Since there is no cHRM, we will guess sRGB gamut.
        SkMatrix44 toXYZD50(SkMatrix44::kUninitialized_Constructor);
        toXYZD50.set3x3RowMajorf(gSRGB_toXYZD50);

        return SkColorSpace_Base::NewRGB(gammas, toXYZD50);
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

static bool png_conversion_possible(const SkImageInfo& dst, const SkImageInfo& src) {
    // Ensure the alpha type is valid
    if (!valid_alpha(dst.alphaType(), src.alphaType())) {
        return false;
    }

    // Check for supported color types
    switch (dst.colorType()) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
        case kRGBA_F16_SkColorType:
            return true;
        case kRGB_565_SkColorType:
            return kOpaque_SkAlphaType == src.alphaType();
        default:
            return dst.colorType() == src.colorType();
    }
}

void SkPngCodec::allocateStorage() {
    size_t colorXformBytes = fColorXform ? fSwizzler->swizzleWidth() * sizeof(uint32_t) : 0;

    fStorage.reset(SkAlign4(fSrcRowBytes) + colorXformBytes);
    fSwizzlerSrcRow = fStorage.get();
    fColorXformSrcRow =
            fColorXform ? SkTAddOffset<uint32_t>(fSwizzlerSrcRow, SkAlign4(fSrcRowBytes)) : 0;
}

static inline bool apply_xform_on_decode(SkColorType dstColorType, SkEncodedInfo::Color srcColor) {
    // We will apply the color xform when reading the color table, unless F16 is requested.
    return SkEncodedInfo::kPalette_Color != srcColor || kRGBA_F16_SkColorType == dstColorType;
}

class SkPngNormalCodec : public SkPngCodec {
public:
    SkPngNormalCodec(const SkEncodedInfo& encodedInfo, const SkImageInfo& imageInfo,
            SkStream* stream, SkPngChunkReader* chunkReader, png_structp png_ptr,
            png_infop info_ptr, int bitDepth)
        : INHERITED(encodedInfo, imageInfo, stream, chunkReader, png_ptr, info_ptr, bitDepth, 1)
    {}

    Result onStartScanlineDecode(const SkImageInfo& dstInfo, const Options& options,
            SkPMColor ctable[], int* ctableCount) override {
        if (!png_conversion_possible(dstInfo, this->getInfo()) ||
            !this->initializeXforms(dstInfo, options, ctable, ctableCount))
        {
            return kInvalidConversion;
        }

        this->allocateStorage();
        return kSuccess;
    }

    int readRows(const SkImageInfo& dstInfo, void* dst, size_t rowBytes, int count, int startRow)
    override {
        SkASSERT(0 == startRow);

        // Assume that an error in libpng indicates an incomplete input.
        int y = 0;
        if (setjmp(png_jmpbuf(fPng_ptr))) {
            SkCodecPrintf("Failed to read row.\n");
            return y;
        }

        void* swizzlerDstRow = dst;
        size_t swizzlerDstRowBytes = rowBytes;

        bool colorXform = fColorXform &&
                apply_xform_on_decode(dstInfo.colorType(), this->getEncodedInfo().color());
        if (colorXform) {
            swizzlerDstRow = fColorXformSrcRow;
            swizzlerDstRowBytes = 0;
        }

        SkAlphaType xformAlphaType = xform_alpha_type(dstInfo.alphaType(),
                                                      this->getInfo().alphaType());
        for (; y < count; y++) {
            png_read_row(fPng_ptr, fSwizzlerSrcRow, nullptr);
            fSwizzler->swizzle(swizzlerDstRow, fSwizzlerSrcRow);

            if (colorXform) {
                fColorXform->apply(dst, (const uint32_t*) swizzlerDstRow, fSwizzler->swizzleWidth(),
                                   dstInfo.colorType(), xformAlphaType);
                dst = SkTAddOffset<void>(dst, rowBytes);
            }

            swizzlerDstRow = SkTAddOffset<void>(swizzlerDstRow, swizzlerDstRowBytes);
        }

        return y;
    }

    int onGetScanlines(void* dst, int count, size_t rowBytes) override {
        return this->readRows(this->dstInfo(), dst, rowBytes, count, 0);
    }

    bool onSkipScanlines(int count) override {
        if (setjmp(png_jmpbuf(fPng_ptr))) {
            SkCodecPrintf("Failed to skip row.\n");
            return false;
        }

        for (int row = 0; row < count; row++) {
            png_read_row(fPng_ptr, fSwizzlerSrcRow, nullptr);
        }
        return true;
    }

    typedef SkPngCodec INHERITED;
};

class SkPngInterlacedCodec : public SkPngCodec {
public:
    SkPngInterlacedCodec(const SkEncodedInfo& encodedInfo, const SkImageInfo& imageInfo,
            SkStream* stream, SkPngChunkReader* chunkReader, png_structp png_ptr,
            png_infop info_ptr, int bitDepth, int numberPasses)
        : INHERITED(encodedInfo, imageInfo, stream, chunkReader, png_ptr, info_ptr, bitDepth,
                    numberPasses)
        , fCanSkipRewind(false)
    {
        SkASSERT(numberPasses != 1);
    }

    Result onStartScanlineDecode(const SkImageInfo& dstInfo, const Options& options,
            SkPMColor ctable[], int* ctableCount) override {
        if (!png_conversion_possible(dstInfo, this->getInfo()) ||
            !this->initializeXforms(dstInfo, options, ctable, ctableCount))
        {
            return kInvalidConversion;
        }

        this->allocateStorage();
        fCanSkipRewind = true;
        return SkCodec::kSuccess;
    }

    int readRows(const SkImageInfo& dstInfo, void* dst, size_t rowBytes, int count, int startRow)
    override {
        if (setjmp(png_jmpbuf(fPng_ptr))) {
            SkCodecPrintf("Failed to get scanlines.\n");
            // FIXME (msarett): Returning 0 is pessimistic.  If we can complete a single pass,
            // we may be able to report that all of the memory has been initialized.  Even if we
            // fail on the first pass, we can still report than some scanlines are initialized.
            return 0;
        }

        SkAutoTMalloc<uint8_t> storage(count * fSrcRowBytes);
        uint8_t* srcRow;
        for (int i = 0; i < fNumberPasses; i++) {
            // Discard rows that we planned to skip.
            for (int y = 0; y < startRow; y++){
                png_read_row(fPng_ptr, fSwizzlerSrcRow, nullptr);
            }
            // Read rows we care about into storage.
            srcRow = storage.get();
            for (int y = 0; y < count; y++) {
                png_read_row(fPng_ptr, srcRow, nullptr);
                srcRow += fSrcRowBytes;
            }
            // Discard rows that we don't need.
            for (int y = 0; y < this->getInfo().height() - startRow - count; y++) {
                png_read_row(fPng_ptr, fSwizzlerSrcRow, nullptr);
            }
        }

        // Swizzle and xform the rows we care about
        void* swizzlerDstRow = dst;
        size_t swizzlerDstRowBytes = rowBytes;

        bool colorXform = fColorXform &&
                apply_xform_on_decode(dstInfo.colorType(), this->getEncodedInfo().color());
        if (colorXform) {
            swizzlerDstRow = fColorXformSrcRow;
            swizzlerDstRowBytes = 0;
        }

        SkAlphaType xformAlphaType = xform_alpha_type(dstInfo.alphaType(),
                                                      this->getInfo().alphaType());
        srcRow = storage.get();
        for (int y = 0; y < count; y++) {
            fSwizzler->swizzle(swizzlerDstRow, srcRow);
            srcRow = SkTAddOffset<uint8_t>(srcRow, fSrcRowBytes);

            if (colorXform) {
                fColorXform->apply(dst, (const uint32_t*) swizzlerDstRow, fSwizzler->swizzleWidth(),
                                   dstInfo.colorType(), xformAlphaType);
                dst = SkTAddOffset<void>(dst, rowBytes);
            }

            swizzlerDstRow = SkTAddOffset<void>(swizzlerDstRow, swizzlerDstRowBytes);
        }

        return count;
    }

    int onGetScanlines(void* dst, int count, size_t rowBytes) override {
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

        return this->readRows(this->dstInfo(), dst, rowBytes, count, this->nextScanline());
    }

    bool onSkipScanlines(int count) override {
        // The non-virtual version will update fCurrScanline.
        return true;
    }

    SkScanlineOrder onGetScanlineOrder() const override {
        return kNone_SkScanlineOrder;
    }

private:
    // FIXME: This imitates behavior in SkCodec::rewindIfNeeded. That function
    // is called whenever some action is taken that reads the stream and
    // therefore the next call will require a rewind. So it modifies a boolean
    // to note that the *next* time it is called a rewind is needed.
    // SkPngInterlacedCodec has an extra wrinkle - calling
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

        SkEncodedInfo encodedInfo = SkEncodedInfo::Make(color, alpha, 8);
        SkImageInfo imageInfo = encodedInfo.makeImageInfo(origWidth, origHeight, colorSpace);

        if (SkEncodedInfo::kOpaque_Alpha == alpha) {
            png_color_8p sigBits;
            if (png_get_sBIT(png_ptr, info_ptr, &sigBits)) {
                if (5 == sigBits->red && 6 == sigBits->green && 5 == sigBits->blue) {
                    // Recommend a decode to 565 if the sBIT indicates 565.
                    imageInfo = imageInfo.makeColorType(kRGB_565_SkColorType);
                }
            }
        }

        if (1 == numberPasses) {
            *outCodec = new SkPngNormalCodec(encodedInfo, imageInfo, stream,
                    chunkReader, png_ptr, info_ptr, bitDepth);
        } else {
            *outCodec = new SkPngInterlacedCodec(encodedInfo, imageInfo, stream,
                    chunkReader, png_ptr, info_ptr, bitDepth, numberPasses);
        }
    }

    return true;
}

SkPngCodec::SkPngCodec(const SkEncodedInfo& encodedInfo, const SkImageInfo& imageInfo,
                       SkStream* stream, SkPngChunkReader* chunkReader, png_structp png_ptr,
                       png_infop info_ptr, int bitDepth, int numberPasses)
    : INHERITED(encodedInfo, imageInfo, stream)
    , fPngChunkReader(SkSafeRef(chunkReader))
    , fPng_ptr(png_ptr)
    , fInfo_ptr(info_ptr)
    , fSwizzlerSrcRow(nullptr)
    , fColorXformSrcRow(nullptr)
    , fSrcRowBytes(imageInfo.width() * (bytes_per_pixel(this->getEncodedInfo().bitsPerPixel())))
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

bool SkPngCodec::initializeXforms(const SkImageInfo& dstInfo, const Options& options,
                                  SkPMColor ctable[], int* ctableCount) {
    if (setjmp(png_jmpbuf(fPng_ptr))) {
        SkCodecPrintf("Failed on png_read_update_info.\n");
        return false;
    }
    png_read_update_info(fPng_ptr, fInfo_ptr);

    // It's important to reset fColorXform to nullptr.  We don't do this on rewinding
    // because the interlaced scanline decoder may need to rewind.
    fColorXform = nullptr;
    SkImageInfo swizzlerInfo = dstInfo;
    bool needsColorXform = needs_color_xform(dstInfo, this->getInfo());
    if (needsColorXform) {
        switch (dstInfo.colorType()) {
            case kRGBA_8888_SkColorType:
            case kBGRA_8888_SkColorType:
            case kRGBA_F16_SkColorType:
                swizzlerInfo = swizzlerInfo.makeColorType(kRGBA_8888_SkColorType);
                if (kPremul_SkAlphaType == dstInfo.alphaType()) {
                    swizzlerInfo = swizzlerInfo.makeAlphaType(kUnpremul_SkAlphaType);
                }
                break;
            case kIndex_8_SkColorType:
                break;
            default:
                return false;
        }

        fColorXform = SkColorSpaceXform::New(sk_ref_sp(this->getInfo().colorSpace()),
                                             sk_ref_sp(dstInfo.colorSpace()));

        if (!fColorXform && kRGBA_F16_SkColorType == dstInfo.colorType()) {
            return false;
        }
    }

    if (SkEncodedInfo::kPalette_Color == this->getEncodedInfo().color()) {
        if (!this->createColorTable(dstInfo, ctableCount)) {
            return false;
        }
    }

    // Copy the color table to the client if they request kIndex8 mode
    copy_color_table(swizzlerInfo, fColorTable, ctable, ctableCount);

    // Create the swizzler.  SkPngCodec retains ownership of the color table.
    const SkPMColor* colors = get_color_ptr(fColorTable.get());
    fSwizzler.reset(SkSwizzler::CreateSwizzler(this->getEncodedInfo(), colors, swizzlerInfo,
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

SkCodec::Result SkPngCodec::onGetPixels(const SkImageInfo& dstInfo, void* dst,
                                        size_t rowBytes, const Options& options,
                                        SkPMColor ctable[], int* ctableCount,
                                        int* rowsDecoded) {
    if (!png_conversion_possible(dstInfo, this->getInfo()) ||
        !this->initializeXforms(dstInfo, options, ctable, ctableCount))
    {
        return kInvalidConversion;
    }

    if (options.fSubset) {
        return kUnimplemented;
    }

    this->allocateStorage();
    int count = this->readRows(dstInfo, dst, rowBytes, dstInfo.height(), 0);
    if (count > dstInfo.height()) {
        *rowsDecoded = count;
        return kIncompleteInput;
    }

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
