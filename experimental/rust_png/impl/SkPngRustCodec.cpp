/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_png/impl/SkPngRustCodec.h"

#include <utility>

#include "experimental/rust_png/ffi/FFI.rs.h"
#include "include/core/SkStream.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTemplates.h"
#include "modules/skcms/src/skcms_public.h"
#include "src/codec/SkSwizzler.h"
#include "third_party/rust/cxx/v1/cxx.h"

namespace {

SkEncodedInfo::Color ToColor(rust_png::ColorType colorType) {
    switch (colorType) {
        case rust_png::ColorType::Grayscale:
            return SkEncodedInfo::kGray_Color;
        case rust_png::ColorType::Rgb:
            return SkEncodedInfo::kRGB_Color;
        case rust_png::ColorType::GrayscaleAlpha:
            return SkEncodedInfo::kGrayAlpha_Color;
        case rust_png::ColorType::Rgba:
            return SkEncodedInfo::kRGBA_Color;
        // `Indexed` is impossible, because of `png::Transformations::EXPAND`.
        case rust_png::ColorType::Indexed:
            break;
    }
    SK_ABORT("Unexpected `rust_png::ColorType`: %d", static_cast<int>(colorType));
}

SkEncodedInfo::Alpha ToAlpha(rust_png::ColorType colorType) {
    switch (colorType) {
        case rust_png::ColorType::Grayscale:
        case rust_png::ColorType::Rgb:
            return SkEncodedInfo::kOpaque_Alpha;
        case rust_png::ColorType::GrayscaleAlpha:
        case rust_png::ColorType::Rgba:
            return SkEncodedInfo::kUnpremul_Alpha;
        // `Indexed` is impossible, because of `png::Transformations::EXPAND`.
        case rust_png::ColorType::Indexed:
            break;
    }
    SK_ABORT("Unexpected `rust_png::ColorType`: %d", static_cast<int>(colorType));
}

SkEncodedInfo CreateEncodedInfo(const rust_png::DecodedImage& image) {
    return SkEncodedInfo::Make(image.width,
                               image.height,
                               ToColor(image.color),
                               ToAlpha(image.color),
                               image.bits_per_component);
}

// This helper class adapts `SkStream` to expose the API required by Rust FFI
// (i.e. the `ReadTrait` API).
class ReadTraitAdapterForSkStream final : public rust_png::ReadTrait {
public:
    // SAFETY: The caller needs to guarantee that `stream` will be alive for
    // as long as `ReadTraitAdapterForSkStream`.
    explicit ReadTraitAdapterForSkStream(SkStream* stream) : fStream(stream) {}

    ~ReadTraitAdapterForSkStream() override = default;

    // Non-copyable and non-movable (we want a stable `this` pointer, because we
    // will be passing a `ReadTrait*` pointer over the FFI boundary and
    // retaining it inside `png::Reader`).
    ReadTraitAdapterForSkStream(const ReadTraitAdapterForSkStream&) = delete;
    ReadTraitAdapterForSkStream& operator=(const ReadTraitAdapterForSkStream&) = delete;
    ReadTraitAdapterForSkStream(ReadTraitAdapterForSkStream&&) = delete;
    ReadTraitAdapterForSkStream& operator=(ReadTraitAdapterForSkStream&&) = delete;

    // Implementation of the `std::io::Read::read` method.  See `RustTrait`'s
    // doc comments and
    // https://doc.rust-lang.org/nightly/std/io/trait.Read.html#tymethod.read
    // for guidance on the desired implementation and behavior of this method.
    size_t read(rust::Slice<uint8_t> buffer) override {
        // Avoiding operating on `buffer.data()` if the slice is empty helps to avoid
        // UB risk described at https://davidben.net/2024/01/15/empty-slices.html.
        if (buffer.empty()) {
            return 0;
        }

        return fStream->read(buffer.data(), buffer.size());
    }

private:
    SkStream* fStream = nullptr;  // Non-owning pointer.
};

// The current implementation does *not* call into
// `SkCodec::applyColorXform` and therefore it doesn't really matter what we
// pass to the constructor of `SkCodec` as `XformFormat srcFormat`.  Passing
// `kInvalidSkcmsFormat` will ensure that `skcms_Transform` will return
// false if (unexpectedly) called.
//
// Note that `skcms_PixelFormat` doesn't currently cover all the possible
// output formats from the PNG decoder - e.g. it has no support for handling
// G16 or GA16.
//
// TODO(https://crbug.com/356879515): Remove this constant and start using
// valid pixel format constants before depending on `skcms_Transform`.
#pragma clang diagnostic ignored "-Wenum-constexpr-conversion"
constexpr skcms_PixelFormat kInvalidSkcmsPixelFormat = static_cast<skcms_PixelFormat>(-1);

}  // namespace

// static
std::unique_ptr<SkPngRustCodec> SkPngRustCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                               Result* result) {
    SkASSERT(stream);
    SkASSERT(result);

    auto readTraitAdapter = std::make_unique<ReadTraitAdapterForSkStream>(stream.get());
    std::unique_ptr<rust_png::DecodedImage> image = rust_png::read_png(std::move(readTraitAdapter));
    if (!image) {
        // TODO(https://crbug.com/356878144): Provide more granular conversion
        // from `png::DecodingError` to `SkCodec::Result`.
        *result = kErrorInInput;
        return nullptr;
    }

    *result = kSuccess;
    return std::make_unique<SkPngRustCodec>(
            CreateEncodedInfo(*image), std::move(stream), std::move(image->data));
}

SkPngRustCodec::SkPngRustCodec(SkEncodedInfo&& encodedInfo,
                               std::unique_ptr<SkStream> stream,
                               rust::Vec<uint8_t> decodedData)
        : SkCodec(std::move(encodedInfo), kInvalidSkcmsPixelFormat, std::move(stream))
        , fDecodedData(std::move(decodedData)) {}

SkPngRustCodec::~SkPngRustCodec() = default;

SkEncodedImageFormat SkPngRustCodec::onGetEncodedFormat() const {
    return SkEncodedImageFormat::kPNG;
}

SkCodec::Result SkPngRustCodec::onGetPixels(const SkImageInfo& dstInfo,
                                            void* dst,
                                            size_t rowBytes,
                                            const Options& options,
                                            int* rowsDecoded) {
    // TODO(https://crbug.com/356922876): Expose `png` crate's ability to decode
    // multiple frames.
    if (options.fFrameIndex != 0) {
        return kUnimplemented;
    }

    if (options.fSubset) {
        return kUnimplemented;
    }

    // We can assume that the source and destination have the same dimensions,
    // because `SkPngRustCodec` inherits the default implementation of
    // `SkCodec::onDimensionsSupported` which returns false (and
    // `SkCodec::getPixels` checks `dimensionsSupported` before proceeding).
    int width = dstInfo.width();
    int height = dstInfo.height();
    SkASSERT(width == getEncodedInfo().width());
    SkASSERT(height == getEncodedInfo().height());

    // Palette expansion currently takes place within the `png` crate, via
    // `png::Transformations::EXPAND`.
    //
    // TODO(https://crbug.com/356882657): Measure if populating `SkPMColor`
    // table may have some runtime performance benefits.
    SkPMColor* kColorTable = nullptr;

    std::unique_ptr<SkSwizzler> swizzler =
            SkSwizzler::Make(this->getEncodedInfo(), kColorTable, dstInfo, options);

    // The assertion below is based on `png::Transformations::EXPAND`.  The
    // assertion helps to ensure that dividing by 8 in `srcRowSize` calculations
    // is okay.
    SkASSERT(getEncodedInfo().bitsPerComponent() % 8 == 0);
    size_t srcRowSize = static_cast<size_t>(getEncodedInfo().bitsPerPixel()) / 8 * width;

    SkSpan<const uint8_t> src = fDecodedData;
    void* dstRow = dst;
    for (int y = 0; y < height; ++y) {
        swizzler->swizzle(dstRow, src.data());
        dstRow = SkTAddOffset<void>(dstRow, rowBytes);
        src = src.subspan(srcRowSize);
    }
    *rowsDecoded = height;

    return kSuccess;
}
