/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkParseEncodedOrigin_DEFINED
#define SkParseEncodedOrigin_DEFINED

#include "include/codec/SkEncodedOrigin.h"

/**
 * If |data| is an EXIF tag representing an SkEncodedOrigin, return true and set |out|
 * appropriately. Otherwise return false.
 */
bool SkParseEncodedOrigin(const uint8_t* data, size_t data_length, SkEncodedOrigin* out);

#endif // SkParseEncodedOrigin_DEFINED
