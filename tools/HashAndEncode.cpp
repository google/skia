// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "include/core/SkICC.h"
#include "include/core/SkString.h"
#include "tools/HashAndEncode.h"
#include "png.h"

static sk_sp<SkColorSpace> rec2020() {
    return SkColorSpace::MakeRGB(SkNamedTransferFn::kRec2020, SkNamedGamut::kRec2020);
}

HashAndEncode::HashAndEncode(const SkBitmap& bitmap) : fSize(bitmap.info().dimensions()) {
    skcms_AlphaFormat srcAlpha;
    switch (bitmap.alphaType()) {
        case kUnknown_SkAlphaType: return;

        case kOpaque_SkAlphaType:
        case kUnpremul_SkAlphaType: srcAlpha = skcms_AlphaFormat_Unpremul;        break;
        case kPremul_SkAlphaType:   srcAlpha = skcms_AlphaFormat_PremulAsEncoded; break;
    }

    skcms_PixelFormat srcFmt;
    switch (bitmap.colorType()) {
        case kUnknown_SkColorType:            return;

        case kAlpha_8_SkColorType:            srcFmt = skcms_PixelFormat_A_8;          break;
        case kRGB_565_SkColorType:            srcFmt = skcms_PixelFormat_BGR_565;      break;
        case kARGB_4444_SkColorType:          srcFmt = skcms_PixelFormat_ABGR_4444;    break;
        case kRGBA_8888_SkColorType:          srcFmt = skcms_PixelFormat_RGBA_8888;    break;
        case kBGRA_8888_SkColorType:          srcFmt = skcms_PixelFormat_BGRA_8888;    break;
        case kRGBA_1010102_SkColorType:       srcFmt = skcms_PixelFormat_RGBA_1010102; break;
        case kBGRA_1010102_SkColorType:       srcFmt = skcms_PixelFormat_BGRA_1010102; break;
        case kGray_8_SkColorType:             srcFmt = skcms_PixelFormat_G_8;          break;
        case kRGBA_F16Norm_SkColorType:       srcFmt = skcms_PixelFormat_RGBA_hhhh;    break;
        case kRGBA_F16_SkColorType:           srcFmt = skcms_PixelFormat_RGBA_hhhh;    break;
        case kRGBA_F32_SkColorType:           srcFmt = skcms_PixelFormat_RGBA_ffff;    break;

        case kRGB_888x_SkColorType:           srcFmt = skcms_PixelFormat_RGBA_8888;
                                              srcAlpha = skcms_AlphaFormat_Opaque;     break;
        case kRGB_101010x_SkColorType:        srcFmt = skcms_PixelFormat_RGBA_1010102;
                                              srcAlpha = skcms_AlphaFormat_Opaque;     break;
        case kBGR_101010x_SkColorType:        srcFmt = skcms_PixelFormat_BGRA_1010102;
                                              srcAlpha = skcms_AlphaFormat_Opaque;     break;
        case kR8G8_unorm_SkColorType:         return;
        case kR16G16_unorm_SkColorType:       return;
        case kR16G16_float_SkColorType:       return;
        case kA16_unorm_SkColorType:          return;
        case kA16_float_SkColorType:          return;
        case kR16G16B16A16_unorm_SkColorType: return;
    }

    skcms_ICCProfile srcProfile = *skcms_sRGB_profile();
    if (auto cs = bitmap.colorSpace()) {
        cs->toProfile(&srcProfile);
    }

    // Our common format that can represent anything we draw and encode as a PNG:
    //   - 16-bit big-endian RGBA
    //   - unpremul
    //   - Rec. 2020 gamut and transfer function
    skcms_PixelFormat dstFmt   = skcms_PixelFormat_RGBA_16161616BE;
    skcms_AlphaFormat dstAlpha = skcms_AlphaFormat_Unpremul;
    skcms_ICCProfile dstProfile;
    rec2020()->toProfile(&dstProfile);

    int N = fSize.width() * fSize.height();
    fPixels.reset(new uint64_t[N]);

    if (!skcms_Transform(bitmap.getPixels(), srcFmt, srcAlpha, &srcProfile,
                         fPixels.get(),      dstFmt, dstAlpha, &dstProfile, N)) {
        SkASSERT(false);
        fPixels.reset(nullptr);
    }
}

void HashAndEncode::write(SkWStream* st) const {
    st->write(&fSize, sizeof(fSize));
    if (const uint64_t* px = fPixels.get()) {
        st->write(px, sizeof(*px) * fSize.width() * fSize.height());
    }

    // N.B. changing salt will change the hash of all images produced by DM,
    // and will cause tens of thousands of new images to be uploaded to Gold.
    int salt = 1;
    st->write(&salt, sizeof(salt));
}

bool HashAndEncode::writePngTo(const char* path,
                               const char* md5,
                               CommandLineFlags::StringArray key,
                               CommandLineFlags::StringArray properties) const {
    if (!fPixels) {
        return false;
    }

    FILE* f = fopen(path, "wb");
    if (!f) {
        return false;
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) {
        fclose(f);
        return false;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_write_struct(&png, &info);
        fclose(f);
        return false;
    }

    SkString description;
    description.append("Key: ");
    for (int i = 0; i < key.count(); i++) {
        description.appendf("%s ", key[i]);
    }
    description.append("Properties: ");
    for (int i = 0; i < properties.count(); i++) {
        description.appendf("%s ", properties[i]);
    }
    description.appendf("MD5: %s", md5);

    png_text text[2];
    text[0].key  = (png_charp)"Author";
    text[0].text = (png_charp)"DM unified Rec.2020";
    text[0].compression = PNG_TEXT_COMPRESSION_NONE;
    text[1].key  = (png_charp)"Description";
    text[1].text = (png_charp)description.c_str();
    text[1].compression = PNG_TEXT_COMPRESSION_NONE;
    png_set_text(png, info, text, SK_ARRAY_COUNT(text));

    png_init_io(png, f);
    png_set_IHDR(png, info, (png_uint_32)fSize.width()
                          , (png_uint_32)fSize.height()
                          , 16/*bits per channel*/
                          , PNG_COLOR_TYPE_RGB_ALPHA
                          , PNG_INTERLACE_NONE
                          , PNG_COMPRESSION_TYPE_DEFAULT
                          , PNG_FILTER_TYPE_DEFAULT);

    // Fastest encoding and decoding, at slight file size cost is no filtering, compression 1.
    png_set_filter(png, PNG_FILTER_TYPE_BASE, PNG_FILTER_NONE);
    png_set_compression_level(png, 1);

    static const sk_sp<SkData> profile =
        SkWriteICCProfile(SkNamedTransferFn::kRec2020, SkNamedGamut::kRec2020);
    png_set_iCCP(png, info,
                 "Rec.2020",
                 0/*compression type... no idea what options are available here*/,
                 (png_const_bytep)profile->data(),
                 (png_uint_32)    profile->size());

    png_write_info(png, info);
    for (int y = 0; y < fSize.height(); y++) {
        png_write_row(png, (png_bytep)(fPixels.get() + y*fSize.width()));
    }
    png_write_end(png, info);

    png_destroy_write_struct(&png, &info);
    fclose(f);
    return true;
}

