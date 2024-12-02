/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPngRustEncoderImpl_DEFINED
#define SkPngRustEncoderImpl_DEFINED

#include <memory>

#include "include/encode/SkEncoder.h"

class SkPixmap;
class SkWStream;

// This class provides the Skia image encoding API (`SkEncoder`) on top of the
// third-party `png` crate (PNG compression and encoding implemented in Rust).
//
// TODO(https://crbug.com/379312510): Derive from `SkPngEncoderBase` (see
// http://review.skia.org/923336 and http://review.skia.org/922676).
class SkPngRustEncoderImpl final : public SkEncoder {
public:
    static std::unique_ptr<SkEncoder> Make(SkWStream*, const SkPixmap&);

    // `public` to support `std::make_unique<SkPngRustEncoderImpl>(...)`.
    explicit SkPngRustEncoderImpl(const SkPixmap&);

    ~SkPngRustEncoderImpl() override;

protected:
    bool onEncodeRows(int numRows) override;
};

#endif  // SkPngRustEncoderImpl_DEFINED
