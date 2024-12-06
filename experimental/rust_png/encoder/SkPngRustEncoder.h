/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPngRustEncoder_DEFINED
#define SkPngRustEncoder_DEFINED

#include <cstdint>
#include <memory>

#include "include/private/base/SkAPI.h"

class SkEncoder;
class SkPixmap;
class SkWStream;

namespace SkPngRustEncoder {

/*
 * Compression level.
 */
enum class CompressionLevel : uint8_t {
    // Low compression level - fast, but may result in bigger PNG files.
    kLow,

    // Medium compression level - somewhere in-between `kLow` and `kHigh`.
    kMedium,

    // High compression level - slow, but should results in smaller PNG files.
    kHigh,
};

/*
 * PNG encoding options.
 *
 * TODO(https://crbug.com/379312510): Add support for `SkPngEncoder::Options`
 * like:
 *  - Comments - `tEXt` chunks.
 *  - Color profile - `iCCP` chunk.
 */
struct Options {
    CompressionLevel fCompressionLevel = CompressionLevel::kMedium;
};

/**
 *  Encode the |src| pixels to the |dst| stream.
 *  |options| may be used to control the encoding behavior.
 *
 *  Returns true on success.  Returns false on an invalid or unsupported |src|.
 *
 */
SK_API bool Encode(SkWStream* dst, const SkPixmap& src, const Options& options);

/**
 *  Create a png encoder that will encode the |src| pixels to the |dst| stream.
 *  |options| may be used to control the encoding behavior.
 *
 *  The primary use of this is incremental encoding of the pixels.
 *
 *  |dst| is unowned but must remain valid for the lifetime of the object.
 *
 *  This returns nullptr on an invalid or unsupported |src|.
 */
SK_API std::unique_ptr<SkEncoder> Make(SkWStream* dst, const SkPixmap& src, const Options& options);

}  // namespace SkPngRustEncoder

#endif  // SkPngRustEncoder_DEFINED
