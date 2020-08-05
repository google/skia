/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/ports/SkNDKConversions.h"

namespace {
static const struct {
    SkColorType         colorType;
    AndroidBitmapFormat format;
} gColorTypeTable[] = {
    { kRGBA_8888_SkColorType, ANDROID_BITMAP_FORMAT_RGBA_8888 },
    { kRGBA_F16_SkColorType,  ANDROID_BITMAP_FORMAT_RGBA_F16 },
    { kRGB_565_SkColorType,   ANDROID_BITMAP_FORMAT_RGB_565 },
    // Android allows using its alpha 8 format to get 8 bit gray pixels.
    { kGray_8_SkColorType,    ANDROID_BITMAP_FORMAT_A_8 },
};

} // anonymous namespace

namespace SkNDKConversions {
    AndroidBitmapFormat toAndroidBitmapFormat(SkColorType colorType) {
        for (const auto& entry : gColorTypeTable) {
            if (entry.colorType == colorType) {
                return entry.format;
            }
        }
        return ANDROID_BITMAP_FORMAT_NONE;
    }

    SkColorType toColorType(AndroidBitmapFormat format) {
        for (const auto& entry : gColorTypeTable) {
            if (entry.format == format) {
                return entry.colorType;
            }
        }
        return kUnknown_SkColorType;
    }

} // SkNDKConversions

static constexpr skcms_TransferFunction k2Dot6 = {2.6f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

static constexpr skcms_Matrix3x3 kDCIP3 = {{
        {0.486143, 0.323835, 0.154234},
        {0.226676, 0.710327, 0.0629966},
        {0.000800549, 0.0432385, 0.78275},
}};

namespace {
static const struct {
    ADataSpace             dataSpace;
    skcms_TransferFunction transferFunction;
    skcms_Matrix3x3        gamut;
} gColorSpaceTable[] = {
    { ADATASPACE_SRGB,         SkNamedTransferFn::kSRGB,    SkNamedGamut::kSRGB },
    { ADATASPACE_SCRGB,        SkNamedTransferFn::kSRGB,    SkNamedGamut::kSRGB },
    { ADATASPACE_SCRGB_LINEAR, SkNamedTransferFn::kLinear,  SkNamedGamut::kSRGB },
    { ADATASPACE_SRGB_LINEAR,  SkNamedTransferFn::kLinear,  SkNamedGamut::kSRGB },
    { ADATASPACE_ADOBE_RGB,    SkNamedTransferFn::k2Dot2,   SkNamedGamut::kAdobeRGB },
    { ADATASPACE_DISPLAY_P3,   SkNamedTransferFn::kSRGB,    SkNamedGamut::kDisplayP3 },
    { ADATASPACE_BT2020,       SkNamedTransferFn::kRec2020, SkNamedGamut::kRec2020 },
    { ADATASPACE_BT709,        SkNamedTransferFn::kRec2020, SkNamedGamut::kSRGB },
    { ADATASPACE_DCI_P3,       k2Dot6,                      kDCIP3 },
};

} // anonymous namespace

static bool nearly_equal(float a, float b) {
    return fabs(a - b) < .002f;
}

static bool nearly_equal(const skcms_TransferFunction& x, const skcms_TransferFunction& y) {
    return nearly_equal(x.g, y.g)
        && nearly_equal(x.a, y.a)
        && nearly_equal(x.b, y.b)
        && nearly_equal(x.c, y.c)
        && nearly_equal(x.d, y.d)
        && nearly_equal(x.e, y.e)
        && nearly_equal(x.f, y.f);
}

static bool nearly_equal(const skcms_Matrix3x3& a, const skcms_Matrix3x3& b) {
    for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++) {
        if (!nearly_equal(a.vals[i][j], b.vals[i][j])) return false;
    }
    return true;
}

namespace SkNDKConversions {
    ADataSpace toDataSpace(SkColorSpace* cs) {
        if (!cs) return ADATASPACE_SRGB;

        skcms_TransferFunction fn;
        skcms_Matrix3x3 gamut;
        if (cs->isNumericalTransferFn(&fn) && cs->toXYZD50(&gamut)) {
            for (const auto& entry : gColorSpaceTable) {
                if (nearly_equal(gamut, entry.gamut) && nearly_equal(fn, entry.transferFunction)) {
                    return entry.dataSpace;
                }
            }
        }
        return ADATASPACE_UNKNOWN;
    }

    sk_sp<SkColorSpace> toColorSpace(ADataSpace dataSpace) {
        for (const auto& entry : gColorSpaceTable) {
            if (entry.dataSpace == dataSpace) {
                return SkColorSpace::MakeRGB(entry.transferFunction, entry.gamut);
            }
        }
        return nullptr;
    }
}

