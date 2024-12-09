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

#include "include/core/SkDataTable.h"
#include "include/core/SkRefCnt.h"
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

    /**
     *  Represents comments to be written into tEXt chunks of the png.
     *
     *  The 2i-th entry is the keyword for the i-th comment,
     *  and the (2i + 1)-th entry is the text for the i-th comment.
     *
     *  All entries are treated as strings encoded as Latin-1 (i.e.
     *  ISO-8859-1).  The strings may, but don't have to be NUL-terminated
     *  (trailing NUL characters will be stripped).  Encoding will fail if
     *  keyword or text don't meet the requirements of the PNG spec - text may
     *  have any length and contain any of the 191 Latin-1 characters (and/or
     *  the linefeed character), but keyword's length is restricted to at most
     *  79 characters and it can't contain a non-breaking space character.
     */
    sk_sp<SkDataTable> fComments;
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
