/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageEncoderFns_DEFINED
#define SkImageEncoderFns_DEFINED

#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/encode/SkICC.h"
#include "include/private/SkExif.h"
#include "modules/skcms/skcms.h"

#include <cstring>
#include <optional>

typedef void (*transform_scanline_proc)(char* dst, const char* src, int width, int bpp);

static inline void transform_scanline_memcpy(char* dst, const char* src, int width, int bpp) {
    memcpy(dst, src, width * bpp);
}

static inline void transform_scanline_A8_to_GrayAlpha(char* dst, const char* src, int width, int) {
    for (int i = 0; i < width; i++) {
        *dst++ = 0;
        *dst++ = *src++;
    }
}

static inline sk_sp<SkData> icc_from_color_space(const SkColorSpace* cs,
                                                 const skcms_ICCProfile* profile,
                                                 const char* profile_description) {
    // TODO(ccameron): Remove this check.
    if (!cs) {
        return nullptr;
    }

    if (profile) {
        return SkWriteICCProfile(profile, profile_description);
    }

    skcms_Matrix3x3 toXYZD50;
    if (cs->toXYZD50(&toXYZD50)) {
        skcms_TransferFunction fn;
        cs->transferFn(&fn);
        return SkWriteICCProfile(fn, toXYZD50);
    }
    return nullptr;
}

static inline sk_sp<SkData> icc_from_color_space(const SkImageInfo& info,
                                                 const skcms_ICCProfile* profile,
                                                 const char* profile_description) {
    return icc_from_color_space(info.colorSpace(), profile, profile_description);
}

static inline sk_sp<SkData> exif_from_origin(const SkEncodedOrigin origin) {
    SkExif::Metadata metadata;
    metadata.fOrigin = origin;
    return SkExif::WriteExif(metadata);
}

#endif  // SkImageEncoderFns_DEFINED
