/*
 * Copyright 2026 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkJpegRustEncoder_DEFINED
#define SkJpegRustEncoder_DEFINED

#include "include/private/SkAPI.h"

class SkPixmap;
class SkWStream;

namespace SkJpegEncoder {
struct Options;
}

namespace SkJpegRustEncoder {

/**
 * Encode the given pixmap as JPEG using the Rust jpeg-encoder crate.
 * Returns true on success. Options.fQuality controls compression quality (0-100).
 */
SK_API bool Encode(SkWStream* dst, const SkPixmap& src,
                   const SkJpegEncoder::Options& options);

}  // namespace SkJpegRustEncoder

#endif  // SkJpegRustEncoder_DEFINED
