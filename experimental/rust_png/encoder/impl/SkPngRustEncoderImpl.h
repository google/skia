/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPngRustEncoderImpl_DEFINED
#define SkPngRustEncoderImpl_DEFINED

#include <memory>

#include "experimental/rust_png/ffi/FFI.rs.h"
#include "src/encode/SkPngEncoderBase.h"
#include "third_party/rust/cxx/v1/cxx.h"

class SkWStream;
struct SkEncodedInfo;
class SkPixmap;
class SkPngEncoderMgr;
template <typename T> class SkSpan;

namespace SkPngRustEncoder {
struct Options;
}  // namespace SkPngRustEncoder

// This class provides the Skia image encoding API (`SkEncoder`) on top of the
// third-party `png` crate (PNG compression and encoding implemented in Rust).
//
// TODO(https://crbug.com/379312510): Derive from `SkPngEncoderBase` (see
// http://review.skia.org/923336 and http://review.skia.org/922676).
class SkPngRustEncoderImpl final : public SkPngEncoderBase {
public:
    static std::unique_ptr<SkEncoder> Make(SkWStream*,
                                           const SkPixmap&,
                                           const SkPngRustEncoder::Options& options);

    // `public` to support `std::make_unique<SkPngRustEncoderImpl>(...)`.
    SkPngRustEncoderImpl(TargetInfo targetInfo,
                         const SkPixmap& src,
                         rust::Box<rust_png::StreamWriter> streamWriter);

    ~SkPngRustEncoderImpl() override;

protected:
    bool onEncodeRow(SkSpan<const uint8_t> row) override;
    bool onFinishEncoding() override;

private:
    rust::Box<rust_png::StreamWriter> fStreamWriter;
};

#endif  // SkPngRustEncoderImpl_DEFINED
