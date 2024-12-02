/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPngRustEncoder_DEFINED
#define SkPngRustEncoder_DEFINED

#include <memory>

#include "include/private/base/SkAPI.h"

class SkEncoder;
class SkPixmap;
class SkWStream;

namespace SkPngRustEncoder {

/**
 *  Encode the |src| pixels to the |dst| stream.
 *  |options| may be used to control the encoding behavior.
 *
 *  Returns true on success.  Returns false on an invalid or unsupported |src|.
 *
 *  TODO(https://crbug.com/379312510): Add support for `SkPngEncoder::Options`
 *  like:
 *  * Comments - `tEXt` chunks.
 *  * Color profile - `iCCP` chunk.
 *  * Filter choice and compression level
 */
SK_API bool Encode(SkWStream* dst, const SkPixmap& src);

/**
 *  Create a png encoder that will encode the |src| pixels to the |dst| stream.
 *  |options| may be used to control the encoding behavior.
 *
 *  The primary use of this is incremental encoding of the pixels.
 *
 *  |dst| is unowned but must remain valid for the lifetime of the object.
 *
 *  This returns nullptr on an invalid or unsupported |src|.
 *
 *  TODO(https://crbug.com/379312510): Add support for `SkPngEncoder::Options`.
 */
SK_API std::unique_ptr<SkEncoder> Make(SkWStream* dst, const SkPixmap& src);

}  // namespace SkPngRustEncoder

#endif  // SkPngRustEncoder_DEFINED
