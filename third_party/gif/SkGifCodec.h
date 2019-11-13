// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE_BSD_3_CLAUSE.md file.

#ifndef SkGifCodec_DEFINED
#define SkGifCodec_DEFINED

#include "include/codec/SkCodec.h"

namespace SkGifCodec {

bool IsGif(const void*, size_t);

/*
 * Assumes IsGif was called and returned true
 * Reads enough of the stream to determine the image format
 */
std::unique_ptr<SkCodec> MakeFromStream(std::unique_ptr<SkStream>, SkCodec::Result*);

}  // namespace SkGifCodec
#endif  // SkGifCodec_DEFINED
