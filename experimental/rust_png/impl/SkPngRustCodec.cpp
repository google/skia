/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_png/impl/SkPngRustCodec.h"

#include <memory>
#include <utility>

#include "experimental/rust_png/ffi/FFI.rs.h"
#include "include/core/SkStream.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTemplates.h"
#include "modules/skcms/skcms.h"
#include "src/base/SkAutoMalloc.h"
#include "src/codec/SkSwizzler.h"
#include "third_party/rust/cxx/v1/cxx.h"

namespace {

SkEncodedInfo::Color ToColor(rust_png::ColorType colorType) {
    // TODO(https://crbug.com/359279096): Take `sBIT` chunk into account to
    // sometimes return `kXAlpha_Color` or `k565_Color`.  This may require
    // a small PR to expose `sBIT` chunk from the `png` crate.

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

std::unique_ptr<SkEncodedInfo::ICCProfile> CreateColorProfile(const rust_png::Reader& reader) {
    // NOTE: This method is based on `read_color_profile` in
    // `src/codec/SkPngCodec.cpp` but has been refactored to use Rust inputs
    // instead of `libpng`.

    rust::Slice<const uint8_t> iccp;
    if (reader.try_get_iccp(iccp)) {
        skcms_ICCProfile profile;
        skcms_Init(&profile);
        if (skcms_Parse(iccp.data(), iccp.size(), &profile)) {
            return SkEncodedInfo::ICCProfile::Make(profile);
        }
    }

    if (reader.is_srgb()) {
        // TODO(https://crbug.com/362304558): Consider the intent field from the
        // `sRGB` chunk.
        return nullptr;
    }

    // Default to SRGB gamut.
    skcms_Matrix3x3 toXYZD50 = skcms_sRGB_profile()->toXYZD50;

    // Next, check for chromaticities.
    float rx = 0.0;
    float ry = 0.0;
    float gx = 0.0;
    float gy = 0.0;
    float bx = 0.0;
    float by = 0.0;
    float wx = 0.0;
    float wy = 0.0;
    if (reader.try_get_chrm(wx, wy, rx, ry, gx, gy, bx, by)) {
        skcms_Matrix3x3 tmp;
        if (skcms_PrimariesToXYZD50(rx, ry, gx, gy, bx, by, wx, wy, &tmp)) {
            toXYZD50 = tmp;
        } else {
            // Note that Blink simply returns nullptr in this case. We'll fall
            // back to srgb.
            //
            // TODO(https://crbug.com/362306048): If this implementation ends up
            // replacing the one from Blink, then we should 1) double-check that
            // we are comfortable with the difference and 2) remove this comment
            // (since the Blink code that it refers to will get removed).
        }
    }

    skcms_TransferFunction fn;
    float gamma;
    if (reader.try_get_gama(gamma)) {
        fn.a = 1.0f;
        fn.b = fn.c = fn.d = fn.e = fn.f = 0.0f;
        fn.g = 1.0f / gamma;
    } else {
        // Default to sRGB gamma if the image has color space information,
        // but does not specify gamma.
        // Note that Blink would again return nullptr in this case.
        fn = *skcms_sRGB_TransferFunction();
    }

    skcms_ICCProfile profile;
    skcms_Init(&profile);
    skcms_SetTransferFunction(&profile, &fn);
    skcms_SetXYZD50(&profile, &toXYZD50);
    return SkEncodedInfo::ICCProfile::Make(profile);
}

SkEncodedInfo CreateEncodedInfo(const rust_png::Reader& reader) {
    rust_png::ColorType rust_color = reader.output_color_type();
    SkEncodedInfo::Color sk_color = ToColor(rust_color);

    std::unique_ptr<SkEncodedInfo::ICCProfile> profile = CreateColorProfile(reader);
    if (!SkPngCodecBase::isCompatibleColorProfileAndType(profile.get(), sk_color)) {
        profile = nullptr;
    }

    return SkEncodedInfo::Make(reader.width(),
                               reader.height(),
                               sk_color,
                               ToAlpha(rust_color),
                               reader.output_bits_per_component(),
                               std::move(profile));
}

SkCodec::Result ToSkCodecResult(rust_png::DecodingResult rustResult) {
    switch (rustResult) {
        case rust_png::DecodingResult::Success:
            return SkCodec::kSuccess;
        case rust_png::DecodingResult::FormatError:
            return SkCodec::kErrorInInput;
        case rust_png::DecodingResult::ParameterError:
            return SkCodec::kInvalidParameters;
        case rust_png::DecodingResult::LimitsExceededError:
            return SkCodec::kInternalError;
    }
    SK_ABORT("Unexpected `rust_png::DecodingResult`: %d", static_cast<int>(rustResult));
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

}  // namespace

// static
std::unique_ptr<SkPngRustCodec> SkPngRustCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                               Result* result) {
    SkASSERT(stream);
    SkASSERT(result);

    auto readTraitAdapter = std::make_unique<ReadTraitAdapterForSkStream>(stream.get());
    rust::Box<rust_png::ResultOfReader> resultOfReader =
            rust_png::new_reader(std::move(readTraitAdapter));
    *result = ToSkCodecResult(resultOfReader->err());
    if (*result != kSuccess) {
        return nullptr;
    }
    rust::Box<rust_png::Reader> reader = resultOfReader->unwrap();

    return std::make_unique<SkPngRustCodec>(
            CreateEncodedInfo(*reader), std::move(stream), std::move(reader));
}

SkPngRustCodec::SkPngRustCodec(SkEncodedInfo&& encodedInfo,
                               std::unique_ptr<SkStream> stream,
                               rust::Box<rust_png::Reader> reader)
        : SkPngCodecBase(std::move(encodedInfo), std::move(stream)), fReader(std::move(reader)) {}

SkPngRustCodec::~SkPngRustCodec() = default;

SkCodec::Result SkPngRustCodec::startDecoding(const SkImageInfo& dstInfo,
                                              void* pixels,
                                              size_t rowBytes,
                                              const Options& options,
                                              DecodingState* decodingState) {
    decodingState->dst = SkSpan(static_cast<uint8_t*>(pixels), rowBytes * dstInfo.height());
    decodingState->dstRowSize = rowBytes;

    // TODO(https://crbug.com/356922876): Expose `png` crate's ability to decode
    // multiple frames.
    if (options.fFrameIndex != 0) {
        return kUnimplemented;
    }

    // TODO(https://crbug.com/362830091): Consider handling `fSubset` (if not
    // for `onGetPixels` then at least for `onStartIncrementalDecode`).
    if (options.fSubset) {
        return kUnimplemented;
    }

    return this->initializeXforms(dstInfo, options);
}

SkCodec::Result SkPngRustCodec::incrementalDecode(DecodingState& decodingState,
                                                  int* rowsDecodedPtr) {
    if (fReader->interlaced()) {
        // TODO(https://crbug.com/356923435): Support incremental decoding of
        // interlaced images.
        return kUnimplemented;
    }
    this->initializeXformParams();

    int rowsDecoded = 0;
    while (true) {
        // TODO(https://crbug.com/357876243): Avoid an unconditional buffer hop
        // through buffer owned by `fReader` (e.g. when we can decode directly
        // into `dst`, because the pixel format received from `fReader` is
        // similar enough to `dstInfo`).
        rust::Slice<const uint8_t> decodedRow;

        Result result = ToSkCodecResult(fReader->next_row(decodedRow));
        if (result != kSuccess) {
            if (result == kIncompleteInput && rowsDecodedPtr) {
                // TODO(https://crbug.com/356923435): Handle `kIncompleteInput` (right
                // now the FFI layer will never return `kIncompleteInput` but we will
                // need to handle it for incremental, row-by-row decoding).
                SkUNREACHABLE;

                *rowsDecodedPtr = rowsDecoded;
            }
            return result;
        }

        if (decodedRow.empty()) {  // This is how FFI layer says "no more rows".
            fIncrementalDecodingState.reset();
            return kSuccess;
        }

        this->applyXformRow(decodingState.dst, decodedRow);
        decodingState.dst = decodingState.dst.subspan(decodingState.dstRowSize);
        rowsDecoded++;
    }
}

SkCodec::Result SkPngRustCodec::decodeInterlacedImage(DecodingState& decodingState) {
    // Decode the whole PNG image into an intermediate buffer.
    //
    // TODO(https://crbug.com/356923435): Handle interlaced images in
    // `incrementalDecode` and stop using (and remove) `next_frame` below.
    std::vector<uint8_t> decodedPixels(fReader->output_buffer_size(), 0x00);
    Result result = ToSkCodecResult(fReader->next_frame(rust::Slice<uint8_t>(decodedPixels)));

    if (result != kSuccess) {
        // TODO(https://crbug.com/356923435): Handle `kIncompleteInput` (right
        // now the FFI layer will never return `kIncompleteInput` but we will
        // need to handle it for incremental, row-by-row decoding).
        SkASSERT_RELEASE(result != kIncompleteInput);

        return result;
    }

    // Convert the `decodedPixels` into the `dstInfo` format.
    SkSpan<const uint8_t> src = decodedPixels;
    size_t srcRowSize = this->getEncodedInfoRowSize();
    int height = this->getEncodedInfo().height();
    for (int y = 0; y < height; ++y) {
        this->applyXformRow(decodingState.dst, src);
        decodingState.dst = decodingState.dst.subspan(decodingState.dstRowSize);
        src = src.subspan(srcRowSize);
    }

    return kSuccess;
}

SkCodec::Result SkPngRustCodec::onGetPixels(const SkImageInfo& dstInfo,
                                            void* pixels,
                                            size_t rowBytes,
                                            const Options& options,
                                            int* rowsDecoded) {
    DecodingState decodingState;
    Result result = this->startDecoding(dstInfo, pixels, rowBytes, options, &decodingState);
    if (result != kSuccess) {
        return result;
    }

    // TODO(https://crbug.com/356923435): Handle interlaced images in
    // `incrementalDecode` and then: remove the `if` statement below
    // (and remove `SkPngRustCodec::decodeInterlacedImage` as well as
    // `ffi::Reader::next_frame`).
    if (fReader->interlaced()) {
        return this->decodeInterlacedImage(decodingState);
    }
    return this->incrementalDecode(decodingState, rowsDecoded);
}

SkCodec::Result SkPngRustCodec::onStartIncrementalDecode(const SkImageInfo& dstInfo,
                                                         void* pixels,
                                                         size_t rowBytes,
                                                         const Options& options) {
    DecodingState decodingState;
    Result result = this->startDecoding(dstInfo, pixels, rowBytes, options, &decodingState);
    if (result != kSuccess) {
        return result;
    }

    SkASSERT(!fIncrementalDecodingState.has_value());
    fIncrementalDecodingState = decodingState;
    return kSuccess;
}

SkCodec::Result SkPngRustCodec::onIncrementalDecode(int* rowsDecoded) {
    SkASSERT(fIncrementalDecodingState.has_value());
    return this->incrementalDecode(*fIncrementalDecodingState, rowsDecoded);
}

std::optional<SkSpan<const SkPngCodecBase::PaletteColorEntry>> SkPngRustCodec::onTryGetPlteChunk() {
    if (fReader->output_color_type() != rust_png::ColorType::Indexed) {
        return std::nullopt;
    }

    // We shouldn't get here because we always use
    // `png::Transformations::EXPAND`.
    //
    // TODO(https://crbug.com/356882657): Handle pLTE and tRNS inside
    // `SkPngRustCodec` rather than via `png::Transformations::EXPAND`.
    SkUNREACHABLE;
}

std::optional<SkSpan<const uint8_t>> SkPngRustCodec::onTryGetTrnsChunk() {
    if (fReader->output_color_type() != rust_png::ColorType::Indexed) {
        return std::nullopt;
    }

    // We shouldn't get here because we always use
    // `png::Transformations::EXPAND`.
    //
    // TODO(https://crbug.com/356882657): Handle pLTE and tRNS inside
    // `SkPngRustCodec` rather than via `png::Transformations::EXPAND`.
    SkUNREACHABLE;
}
