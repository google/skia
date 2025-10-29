/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXPSRustPngHelpers_DEFINED
#define SkXPSRustPngHelpers_DEFINED

#include "include/encode/SkPngRustEncoder.h"

class SkWStream;
class SkPixmap;

namespace SkXPS {

/**
 *  Implementation of `SkXPS::EncodePngCallback` based on Rust PNG.
 */
inline bool EncodePngUsingRust(SkWStream* dst, const SkPixmap& src) {
    return SkPngRustEncoder::Encode(dst, src, {});
}

}  // namespace SkXPS

#endif  // SkXPSRustPngHelpers_DEFINED
