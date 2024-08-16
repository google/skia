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

SkPngCodecBase::~SkPngCodecBase() = default;

SkPngCodecBase::SkPngCodecBase(SkEncodedInfo&& encodedInfo,
                               XformFormat srcFormat,
                               std::unique_ptr<SkStream> stream)
        : SkCodec(std::move(encodedInfo), srcFormat, std::move(stream)) {}

SkEncodedImageFormat SkPngCodecBase::onGetEncodedFormat() const {
    return SkEncodedImageFormat::kPNG;
}
