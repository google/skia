/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_png/encoder/impl/SkPngRustEncoderImpl.h"

#include <memory>

#include "include/core/SkStream.h"
#include "src/encode/SkImageEncoderPriv.h"

#ifdef __clang__
#pragma clang diagnostic error "-Wconversion"
#endif

// static
std::unique_ptr<SkEncoder> SkPngRustEncoderImpl::Make(SkWStream* dst, const SkPixmap& src) {
    if (!SkPixmapIsValid(src)) {
        return nullptr;
    }

    // TODO(https://crbug.com/379312510): Actually call into Rust PNG crate
    // here to write the IHDR, iCCP, and/or tEXt chunks.
    return std::make_unique<SkPngRustEncoderImpl>(src);
}

// TODO(https://crbug.com/379312510): Reusing `SkPngEncoderBase` here will make
// it unnecessary to directly pass this size into `SkEncoder`'s constructor.
constexpr size_t kInvalidEncodedRowSize = 0;

SkPngRustEncoderImpl::SkPngRustEncoderImpl(const SkPixmap& src)
        : SkEncoder(src, kInvalidEncodedRowSize) {}

SkPngRustEncoderImpl::~SkPngRustEncoderImpl() = default;

bool SkPngRustEncoderImpl::onEncodeRows(int numRows) {
    // TODO(https://crbug.com/379312510): Actually call into Rust PNG crate
    // here to write IDAT and IEND chunks.
    return false;
}
