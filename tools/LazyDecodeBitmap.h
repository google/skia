/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef LazyDecodeBitmap_DEFINED
#define LazyDecodeBitmap_DEFINED

#include "SkTypes.h"

class SkBitmap;

namespace sk_tools {

/**
 * Decode the image with DecodeMemoryToTarget but defer the process until it is needed.
 */
bool LazyDecodeBitmap(const void* buffer, size_t size, SkBitmap* bitmap);

}

#endif  // LazyDecodeBitmap_DEFINED
