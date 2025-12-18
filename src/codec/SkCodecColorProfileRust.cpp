/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkCodecColorProfileRust.h"

#include "rust/icc/FFI.rs.h"

namespace SkCodecs {

std::unique_ptr<ColorProfile> MakeICCProfileWithRust(
        sk_sp<const SkData> data) {
    if (data) {
        rust_icc::IccProfile rust_profile;

        if (rust_icc::parse_icc_profile(
                    rust::Slice<const uint8_t>(data->bytes(), data->size()), rust_profile)) {
                skcms_ICCProfile profile;
                rust_icc::ToSkcmsIccProfile(rust_profile, &profile);
                return std::unique_ptr<ColorProfile>(
                        new ColorProfile(profile, std::move(data)));
        }
    }
    return nullptr;
}

}  // namespace SkCodecs
