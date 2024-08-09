/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPngRustDecoder_DEFINED
#define SkPngRustDecoder_DEFINED

#include <memory>

#include "include/codec/SkCodec.h"
#include "include/private/base/SkAPI.h"

class SkStream;

namespace SkPngRustDecoder {

SK_API std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream>, SkCodec::Result*);

}  // namespace SkPngRustDecoder

#endif  // SkPngRustDecoder_DEFINED
