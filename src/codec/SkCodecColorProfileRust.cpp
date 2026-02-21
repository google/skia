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
                // Move the Rust profile to the heap so that its Vec<u8> data
                // (grid tables, curve tables) outlives this function.  The
                // skcms_ICCProfile we build below contains raw pointers into
                // that data.
                auto retained = std::shared_ptr<rust_icc::IccProfile>(
                        new rust_icc::IccProfile(std::move(rust_profile)));

                skcms_ICCProfile profile;
                rust_icc::ToSkcmsIccProfile(*retained, &profile);
                auto result = std::unique_ptr<ColorProfile>(
                        new ColorProfile(profile, std::move(data)));
                result->fRetainedData = retained;
                return result;
        }
    }
    return nullptr;
}

}  // namespace SkCodecs
