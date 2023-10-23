/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkParseEncodedOrigin.h"

#include "include/core/SkData.h"
#include "include/private/SkExif.h"
#include "include/private/base/SkAssert.h"

bool SkParseEncodedOrigin(const void* data, size_t data_length, SkEncodedOrigin* orientation) {
    SkASSERT(orientation);
    SkExifMetadata exif(SkData::MakeWithoutCopy(data, data_length));
    return exif.getOrigin(orientation);
}
