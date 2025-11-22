/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPngRustEncoderImpl_DEFINED
#define SkPngRustEncoderImpl_DEFINED

#include <memory>

#include "rust/png/FFI.rs.h"
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
class SkPngRustEncoderImpl final : public SkPngEncoderBase {
public:
    enum ExtraRowTransform {
      // `kNone...` indicates that pixels from `SkPngEncoderBase` can be fed
      // directly into Rust `png`.
      kNone_ExtraRowTransform,

      // `kRgba8ToRgb8...` indicates that `SkPngEncoderBase` gives RGBx (8-bit)
      // pixels to `onEncodeRow`.  And since Rust `png` can't ignore the
      // alpha channel (`libpng` can do this via `png_set_filler`), we need to
      // do an extra RGBA => RGB transformation before feeding the data into
      // Rust.
      kRgba8ToRgb8_ExtraRowTransform,

      // `kRgba16leToRgba16be...` indicates that `SkPngEncoderBase` gives RGBA (16-bit)
      // little endian pixels to `onEncodeRow`.  And since Rust `png` can't swap the
      // endianess (`libpng` can do this via `png_set_swap`), we need to
      // do an extra LE => BE transformation before feeding the data into
      // Rust.
      kRgba16leToRgba16be_ExtraRowTransform,

      // `kRgba16leToRgb16be...` indicates that `SkPngEncoderBase` gives RGBA (16-bit)
      // little endian pixels to `onEncodeRow`. And we must ignore alpha + swap endianess.
      kRgba16leToRgb16be_ExtraRowTransform,
    };

    static std::unique_ptr<SkEncoder> Make(SkWStream*,
                                           const SkPixmap&,
                                           const SkPngRustEncoder::Options& options);

    // `public` to support `std::make_unique<SkPngRustEncoderImpl>(...)`.
    SkPngRustEncoderImpl(TargetInfo targetInfo,
                         const SkPixmap& src,
                         rust::Box<rust_png::StreamWriter> streamWriter,
                         ExtraRowTransform extraRowTransform);

    ~SkPngRustEncoderImpl() override;

protected:
    bool onEncodeRow(SkSpan<const uint8_t> row) override;
    bool onFinishEncoding() override;

private:
    rust::Box<rust_png::StreamWriter> fStreamWriter;

    ExtraRowTransform fExtraRowTransform;
    std::vector<uint8_t> fExtraRowBuffer;
};

#endif  // SkPngRustEncoderImpl_DEFINED
