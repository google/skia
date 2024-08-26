/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkPngCodecBase.h"

#include <utility>

#include "include/codec/SkEncodedImageFormat.h"
#include "include/core/SkStream.h"
#include "include/private/SkEncodedInfo.h"
#include "modules/skcms/skcms.h"

namespace {

skcms_PixelFormat ToPixelFormat(const SkEncodedInfo& info) {
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

}  // namespace

SkPngCodecBase::~SkPngCodecBase() = default;

// static
bool SkPngCodecBase::isCompatibleColorProfileAndType(const SkEncodedInfo::ICCProfile* profile,
                                                     SkEncodedInfo::Color color) {
    if (profile) {
        switch (profile->profile()->data_color_space) {
            case skcms_Signature_CMYK:
                return false;
            case skcms_Signature_Gray:
                if (SkEncodedInfo::kGray_Color != color &&
                    SkEncodedInfo::kGrayAlpha_Color != color) {
                    return false;
                }
                break;
            default:
                break;
        }
    }

    return true;
}

SkPngCodecBase::SkPngCodecBase(SkEncodedInfo&& encodedInfo, std::unique_ptr<SkStream> stream)
        : SkCodec(std::move(encodedInfo), ToPixelFormat(encodedInfo), std::move(stream)) {}

SkEncodedImageFormat SkPngCodecBase::onGetEncodedFormat() const {
    return SkEncodedImageFormat::kPNG;
}
