/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXPSLibpngHelpers_DEFINED
#define SkXPSLibpngHelpers_DEFINED

#include "include/encode/SkPngEncoder.h"

class SkWStream;
class SkPixmap;

namespace SkXPS {

/**
 *  Implementation of `SkXPS::EncodePngCallback` based on `libpng`.
 */
inline bool EncodePngUsingLibpng(SkWStream* dst, const SkPixmap& src) {
    return SkPngEncoder::Encode(dst, src, {});
}

}  // namespace SkXPS

#endif  // SkXPSLibpngHelpers_DEFINED
