/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/encode/SkPngRustEncoderImpl.h"

#include <limits>
#include <memory>
#include <optional>
#include <utility>

#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngRustEncoder.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkAssert.h"
#include "rust/png/FFI.rs.h"
#include "rust/common/SpanUtils.h"
#include "src/base/SkSafeMath.h"
#include "src/encode/SkImageEncoderFns.h"
#include "src/encode/SkImageEncoderPriv.h"
#include "third_party/rust/cxx/v1/cxx.h"

#ifdef __clang__
#pragma clang diagnostic error "-Wconversion"
#endif

namespace {

rust_png::Compression ToCompression(SkPngRustEncoder::CompressionLevel level) {
    switch (level) {
        case SkPngRustEncoder::CompressionLevel::kLow:
            return rust_png::Compression::Level1WithUpFilter;
        case SkPngRustEncoder::CompressionLevel::kMedium:
#ifdef SK_RUST_PNG_MAP_MEDIUM_COMPRESSION_LEVEL_TO_FDEFLATE_FAST
            // TODO(https://crbug.com/406072770): Consider using `Fast` instead
            // of `Balanced` compression here.  See the bug for details.
            return rust_png::Compression::Fast;
#else
            return rust_png::Compression::Balanced;
#endif
        case SkPngRustEncoder::CompressionLevel::kHigh:
            return rust_png::Compression::High;
    }
    SkUNREACHABLE;
}

rust::Slice<const uint8_t> getDataTableEntry(const SkDataTable& table, int index) {
    SkASSERT_RELEASE((0 <= index) && (index < table.count()));

    size_t size = 0;
    const uint8_t* entry = table.atT<uint8_t>(index, &size);
    while (size > 0 && entry[size - 1] == 0) {
        // Ignore trailing NUL characters - these are *not* part of Rust `&str`.
        size--;
    }

    return rust::Slice<const uint8_t>(entry, size);
}

rust_png::EncodingResult EncodeComments(rust_png::Writer& writer,
                                        const sk_sp<SkDataTable>& comments) {
    if (comments != nullptr) {
        if (comments->count() % 2 != 0) {
            return rust_png::EncodingResult::ParameterError;
        }

        for (int i = 0; i < comments->count() / 2; ++i) {
            rust::Slice<const uint8_t> keyword = getDataTableEntry(*comments, 2 * i);
            rust::Slice<const uint8_t> text = getDataTableEntry(*comments, 2 * i + 1);
            rust_png::EncodingResult result = writer.write_text_chunk(keyword, text);
            if (result != rust_png::EncodingResult::Success) {
                return result;
            }
        }
    }

    return rust_png::EncodingResult::Success;
}

// This helper class adapts `SkWStream` to expose the API required by Rust FFI
// (i.e. the `WriteTrait` API).
class WriteTraitAdapterForSkWStream final : public rust_png::WriteTrait {
public:
    // SAFETY: The caller needs to guarantee that `stream` will be alive for
    // as long as `WriteTraitAdapterForSkWStream`.
    explicit WriteTraitAdapterForSkWStream(SkWStream* stream) : fStream(stream) {
        SkASSERT_RELEASE(fStream);
    }

    ~WriteTraitAdapterForSkWStream() override = default;

    // Non-copyable and non-movable.
    WriteTraitAdapterForSkWStream(const WriteTraitAdapterForSkWStream&) = delete;
    WriteTraitAdapterForSkWStream& operator=(const WriteTraitAdapterForSkWStream&) = delete;
    WriteTraitAdapterForSkWStream(WriteTraitAdapterForSkWStream&&) = delete;
    WriteTraitAdapterForSkWStream& operator=(WriteTraitAdapterForSkWStream&&) = delete;

    // Implementation of the `std::io::Read::read` method.  See `RustTrait`'s
    // doc comments and
    // https://doc.rust-lang.org/nightly/std/io/trait.Read.html#tymethod.read
    // for guidance on the desired implementation and behavior of this method.
    bool write(rust::Slice<const uint8_t> buffer) override {
        SkSpan<const uint8_t> span = ToSkSpan(buffer);
        return fStream->write(span.data(), span.size());
    }

    void flush() override { fStream->flush(); }

private:
    SkWStream* fStream = nullptr;  // Non-owning pointer.
};

}  // namespace

// static
std::unique_ptr<SkEncoder> SkPngRustEncoderImpl::Make(SkWStream* dst,
                                                      const SkPixmap& src,
                                                      const SkPngRustEncoder::Options& options) {
    if (!SkPixmapIsValid(src)) {
        return nullptr;
    }

    std::optional<TargetInfo> maybeTargetInfo = SkPngEncoderBase::getTargetInfo(src.info());
    if (!maybeTargetInfo.has_value()) {
        return nullptr;
    }
    const SkEncodedInfo& dstInfo = maybeTargetInfo->fDstInfo;
    const std::optional<SkImageInfo>& maybeDstRowInfo = maybeTargetInfo->fDstRowInfo;

    SkSafeMath safe;
    uint32_t width = safe.castTo<uint32_t>(dstInfo.width());
    uint32_t height = safe.castTo<uint32_t>(dstInfo.height());
    if (!safe.ok()) {
        return nullptr;
    }

    sk_sp<SkData> encodedProfile;
    rust::Slice<const uint8_t> encodedProfileSlice;
    if (const SkColorSpace* colorSpace = src.colorSpace(); colorSpace && !colorSpace->isSRGB()) {
        encodedProfile = icc_from_color_space(colorSpace);
        if (encodedProfile) {
            encodedProfileSlice =
                    rust::Slice<const uint8_t>(encodedProfile->bytes(), encodedProfile->size());
        }
    }

    rust_png::ColorType rustEncoderColorType;
    ExtraRowTransform extraRowTransform = kNone_ExtraRowTransform;
    switch (dstInfo.color()) {
        case SkEncodedInfo::kRGB_Color:
            rustEncoderColorType = rust_png::ColorType::Rgb;
            break;
        case SkEncodedInfo::kRGBA_Color:
            rustEncoderColorType = rust_png::ColorType::Rgba;
            if (maybeDstRowInfo) {
                if (maybeDstRowInfo->isOpaque()) {
                    rustEncoderColorType = rust_png::ColorType::Rgb;
                    if (maybeDstRowInfo->colorType() == kR16G16B16A16_unorm_SkColorType) {
                        extraRowTransform = kRgba16leToRgb16be_ExtraRowTransform;
                    } else {
                        SkASSERT_RELEASE(maybeDstRowInfo->colorType() == kRGB_888x_SkColorType);
                        extraRowTransform = kRgba8ToRgb8_ExtraRowTransform;
                    }
                } else if (maybeDstRowInfo->colorType() == kR16G16B16A16_unorm_SkColorType) {
                    extraRowTransform = kRgba16leToRgba16be_ExtraRowTransform;
                }
            }
            break;
        case SkEncodedInfo::kGray_Color:
            rustEncoderColorType = rust_png::ColorType::Grayscale;
            break;
        case SkEncodedInfo::kGrayAlpha_Color:
            rustEncoderColorType = rust_png::ColorType::GrayscaleAlpha;
            break;
        default:
            SkUNREACHABLE;
    }

    auto writeTraitAdapter = std::make_unique<WriteTraitAdapterForSkWStream>(dst);
    rust::Box<rust_png::ResultOfWriter> resultOfWriter =
            rust_png::new_writer(std::move(writeTraitAdapter),
                                 width,
                                 height,
                                 rustEncoderColorType,
                                 dstInfo.bitsPerComponent(),
                                 ToCompression(options.fCompressionLevel),
                                 encodedProfileSlice);
    if (resultOfWriter->err() != rust_png::EncodingResult::Success) {
        return nullptr;
    }
    rust::Box<rust_png::Writer> writer = resultOfWriter->unwrap();

    if (EncodeComments(*writer, options.fComments) != rust_png::EncodingResult::Success) {
        return nullptr;
    }

    rust::Box<rust_png::ResultOfStreamWriter> resultOfStreamWriter =
            rust_png::convert_writer_into_stream_writer(std::move(writer));
    if (resultOfStreamWriter->err() != rust_png::EncodingResult::Success) {
        return nullptr;
    }
    rust::Box<rust_png::StreamWriter> stream_writer = resultOfStreamWriter->unwrap();

    return std::make_unique<SkPngRustEncoderImpl>(
            std::move(*maybeTargetInfo), src, std::move(stream_writer), extraRowTransform);
}

SkPngRustEncoderImpl::SkPngRustEncoderImpl(TargetInfo targetInfo,
                                           const SkPixmap& src,
                                           rust::Box<rust_png::StreamWriter> stream_writer,
                                           ExtraRowTransform extraRowTransform)
        : SkPngEncoderBase(std::move(targetInfo), src)
        , fStreamWriter(std::move(stream_writer))
        , fExtraRowTransform(extraRowTransform) {}

SkPngRustEncoderImpl::~SkPngRustEncoderImpl() = default;

bool SkPngRustEncoderImpl::onEncodeRow(SkSpan<const uint8_t> row) {
    rust::Slice<const uint8_t> rustRow;
    if (this->fExtraRowTransform != kNone_ExtraRowTransform) {
        skcms_PixelFormat srcFmt, dstFmt;
        size_t srcRowBytes = this->targetInfo().fDstRowInfo->minRowBytes();
        size_t dstRowBytes;
        switch (this->fExtraRowTransform) {
            case kRgba8ToRgb8_ExtraRowTransform:
                srcFmt = skcms_PixelFormat_RGBA_8888;
                dstFmt = skcms_PixelFormat_RGB_888;
                dstRowBytes = srcRowBytes - (srcRowBytes / 4);
                break;
            case kRgba16leToRgba16be_ExtraRowTransform:
                srcFmt = skcms_PixelFormat_RGBA_16161616LE;
                dstFmt = skcms_PixelFormat_RGBA_16161616BE;
                dstRowBytes = srcRowBytes;
                break;
            case kRgba16leToRgb16be_ExtraRowTransform:
                srcFmt = skcms_PixelFormat_RGBA_16161616LE;
                dstFmt = skcms_PixelFormat_RGB_161616BE;
                dstRowBytes = srcRowBytes - (srcRowBytes / 4);
                break;
            default:
                SkUNREACHABLE;
        }

        fExtraRowBuffer.resize(dstRowBytes, 0x00);
        SkSafeMath safe;
        size_t width = safe.castTo<size_t>(this->targetInfo().fDstRowInfo->width());
        if (!safe.ok()) {
            return false;
        }

        bool success = skcms_Transform(
                row.data(), srcFmt, skcms_AlphaFormat_Unpremul, nullptr,
                fExtraRowBuffer.data(), dstFmt, skcms_AlphaFormat_Unpremul, nullptr,
                width);
        if (!success) {
            return false;
        }

        rustRow = rust::Slice<const uint8_t>(fExtraRowBuffer);
    } else {
        SkASSERT(this->fExtraRowTransform == kNone_ExtraRowTransform);
        rustRow = rust::Slice<const uint8_t>(row);
    }
    return fStreamWriter->write(rustRow) == rust_png::EncodingResult::Success;
}

bool SkPngRustEncoderImpl::onFinishEncoding() {
    return rust_png::finish_encoding(std::move(fStreamWriter)) == rust_png::EncodingResult::Success;
}
