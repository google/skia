/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPngEncoderBase_DEFINED
#define SkPngEncoderBase_DEFINED

#include <optional>
#include <utility>

#include "src/encode/SkImageEncoderFns.h"

struct SkEncodedInfo;
struct SkImageInfo;

// This namespace implements functionality shared between `SkPngEncoderImpl` and
// `SkPngRustEncoderImpl` (the latter is from `experimental/rust_png`).
namespace SkPngEncoderBase {

// Gets the `SkEncodedInfo` that `srcInfo` should be converted into before
// encoding and a `transform_scanline_proc` that can transform source rows into
// ready-to-encode rows.
//
// For example, `kRGBA_F32_SkColorType` source will be encoded as
// `SkEncodedInfo::kRGBA_Color` with 16 `bitsPerComponent`.  Depending on
// `src`'s alpha type, such transformation can be handled by either
// `transform_scanline_F32` or `transform_scanline_F32_premul`.
//
// Returns `std::nullopt` if `srcInfo` is not supported by the PNG encoder.
std::optional<std::pair<SkEncodedInfo, transform_scanline_proc>> getTargetInfo(
        const SkImageInfo& srcInfo);

}  // namespace SkPngEncoderBase

#endif  // SkPngEncoderBase_DEFINED
