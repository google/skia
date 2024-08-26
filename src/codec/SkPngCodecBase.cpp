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

SkPngCodecBase::SkPngCodecBase(SkEncodedInfo&& encodedInfo,
                               XformFormat srcFormat,
                               std::unique_ptr<SkStream> stream)
        : SkCodec(std::move(encodedInfo), srcFormat, std::move(stream)) {}

SkEncodedImageFormat SkPngCodecBase::onGetEncodedFormat() const {
    return SkEncodedImageFormat::kPNG;
}
