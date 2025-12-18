/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCodecColorProfileRust_DEFINED
#define SkCodecColorProfileRust_DEFINED

#include "include/core/SkData.h"
#include "src/codec/SkCodecPriv.h"

namespace SkCodecs {

// Parses an ICC profile using the Rust implementation (moxcms).
// This function is only available when SK_CODEC_COLOR_PROFILE_PARSE_WITH_RUST is defined.
// For testing purposes only - production code should use MakeICCProfile() which
// delegates to the appropriate implementation.
std::unique_ptr<ColorProfile> MakeICCProfileWithRust(sk_sp<const SkData> data);

}  // namespace SkCodecs

#endif  // SkCodecColorProfileRust_DEFINED
