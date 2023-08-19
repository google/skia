/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/surface_manager/SurfaceManager.h"
#include "include/core/SkCanvas.h"
#include "src/core/SkColorSpacePriv.h"
#include "tools/ToolUtils.h"

// Based on
// https://skia.googlesource.com/skia/+/88d5e1daa3ba3aae65139d4a3ded1e1b7078d59b/dm/DM.cpp#1315.
static std::string identify_gamut(SkColorSpace* cs) {
    if (!cs) {
        return "untagged";
    }

    skcms_Matrix3x3 gamut;
    if (cs->toXYZD50(&gamut)) {
        auto eq = [](skcms_Matrix3x3 x, skcms_Matrix3x3 y) {
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (x.vals[i][j] != y.vals[i][j]) {
                        return false;
                    }
                }
            }
            return true;
        };

        if (eq(gamut, SkNamedGamut::kSRGB)) {
            return "sRGB";
        }
        if (eq(gamut, SkNamedGamut::kAdobeRGB)) {
            return "Adobe";
        }
        if (eq(gamut, SkNamedGamut::kDisplayP3)) {
            return "P3";
        }
        if (eq(gamut, SkNamedGamut::kRec2020)) {
            return "2020";
        }
        if (eq(gamut, SkNamedGamut::kXYZ)) {
            return "XYZ";
        }
        if (eq(gamut, gNarrow_toXYZD50)) {
            return "narrow";
        }
        return "other";
    }
    return "non-XYZ";
}

// Based on
// https://skia.googlesource.com/skia/+/88d5e1daa3ba3aae65139d4a3ded1e1b7078d59b/dm/DM.cpp#1341.
static std::string identify_transfer_fn(SkColorSpace* cs) {
    if (!cs) {
        return "untagged";
    }

    auto eq = [](skcms_TransferFunction x, skcms_TransferFunction y) {
        return x.g == y.g && x.a == y.a && x.b == y.b && x.c == y.c && x.d == y.d && x.e == y.e &&
               x.f == y.f;
    };

    skcms_TransferFunction tf;
    cs->transferFn(&tf);
    switch (skcms_TransferFunction_getType(&tf)) {
        case skcms_TFType_sRGBish:
            if (tf.a == 1 && tf.b == 0 && tf.c == 0 && tf.d == 0 && tf.e == 0 && tf.f == 0) {
                return SkStringPrintf("gamma %.3g", tf.g).c_str();
            }
            if (eq(tf, SkNamedTransferFn::kSRGB)) {
                return "sRGB";
            }
            if (eq(tf, SkNamedTransferFn::kRec2020)) {
                return "2020";
            }
            return SkStringPrintf("%.3g %.3g %.3g %.3g %.3g %.3g %.3g",
                                  tf.g,
                                  tf.a,
                                  tf.b,
                                  tf.c,
                                  tf.d,
                                  tf.e,
                                  tf.f)
                    .c_str();

        case skcms_TFType_PQish:
            if (eq(tf, SkNamedTransferFn::kPQ)) {
                return "PQ";
            }
            return SkStringPrintf("PQish %.3g %.3g %.3g %.3g %.3g %.3g",
                                  tf.a,
                                  tf.b,
                                  tf.c,
                                  tf.d,
                                  tf.e,
                                  tf.f)
                    .c_str();

        case skcms_TFType_HLGish:
            if (eq(tf, SkNamedTransferFn::kHLG)) {
                return "HLG";
            }
            return SkStringPrintf("HLGish %.3g %.3g %.3g %.3g %.3g (%.3g)",
                                  tf.a,
                                  tf.b,
                                  tf.c,
                                  tf.d,
                                  tf.e,
                                  tf.f + 1)
                    .c_str();

        case skcms_TFType_HLGinvish:
            break;
        case skcms_TFType_Invalid:
            break;
    }
    return "non-numeric";
}

std::map<std::string, std::string> SurfaceManager::getGoldKeys() const {
    return std::map<std::string, std::string>{
            {"surface_config", fConfig},
            {"gamut", identify_gamut(fColorInfo.colorSpace())},
            {"transfer_fn", identify_transfer_fn(fColorInfo.colorSpace())},
            {"color_type", std::string(ToolUtils::colortype_name(fColorInfo.colorType()))},
            {"alpha_type", std::string(ToolUtils::alphatype_name(fColorInfo.alphaType()))},
            {"color_depth", std::string(ToolUtils::colortype_depth(fColorInfo.colorType()))},
    };
}
