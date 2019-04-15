// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "HashAndEncode.h"
#include "PngTool.h"
#include "SkICC.h"
#include "SkString.h"

static constexpr skcms_TransferFunction k2020_TF =
    {2.22222f, 0.909672f, 0.0903276f, 0.222222f, 0.0812429f, 0, 0};

static sk_sp<SkColorSpace> rec2020() {
    return SkColorSpace::MakeRGB(k2020_TF, SkNamedGamut::kRec2020);
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
        case kUnknown_SkColorType: return;

        case kAlpha_8_SkColorType:      srcFmt = skcms_PixelFormat_A_8;          break;
        case kRGB_565_SkColorType:      srcFmt = skcms_PixelFormat_BGR_565;      break;
        case kARGB_4444_SkColorType:    srcFmt = skcms_PixelFormat_ABGR_4444;    break;
        case kRGBA_8888_SkColorType:    srcFmt = skcms_PixelFormat_RGBA_8888;    break;
        case kBGRA_8888_SkColorType:    srcFmt = skcms_PixelFormat_BGRA_8888;    break;
        case kRGBA_1010102_SkColorType: srcFmt = skcms_PixelFormat_RGBA_1010102; break;
        case kGray_8_SkColorType:       srcFmt = skcms_PixelFormat_G_8;          break;
        case kRGBA_F16Norm_SkColorType: srcFmt = skcms_PixelFormat_RGBA_hhhh;    break;
        case kRGBA_F16_SkColorType:     srcFmt = skcms_PixelFormat_RGBA_hhhh;    break;
        case kRGBA_F32_SkColorType:     srcFmt = skcms_PixelFormat_RGBA_ffff;    break;

        case kRGB_888x_SkColorType:     srcFmt = skcms_PixelFormat_RGBA_8888;
                                        srcAlpha = skcms_AlphaFormat_Opaque;       break;
        case kRGB_101010x_SkColorType:  srcFmt = skcms_PixelFormat_RGBA_1010102;
                                        srcAlpha = skcms_AlphaFormat_Opaque;       break;
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

    static const sk_sp<SkData> profile = SkWriteICCProfile(k2020_TF, SkNamedGamut::kRec2020);

    PngTool::WriteOptions options;
    options.iccName = "Rec.2020";
    options.iccData = profile->data();
    options.iccDataLength = profile->size();
    options.author = "DM unified Rec.2020";
    options.description = description.c_str();
    options.fast = true;
    PngTool::Pixmap pixmap = {(unsigned char*)fPixels.get(),
                              (unsigned)fSize.width(),
                              (unsigned)fSize.height(),
                              16, PngTool::ColorType::kRGBA};
    return PngTool::Write(path, pixmap, options);
}
