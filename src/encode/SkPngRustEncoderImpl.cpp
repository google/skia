/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/encode/SkPngRustEncoderImpl.h"

#include <array>
#include <limits>
#include <memory>
#include <optional>
#include <utility>

#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngRustEncoder.h"
#include "include/private/SkAssert.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/SkGainmapInfo.h"
#include "rust/common/SpanUtils.h"
#include "rust/png/FFI.rs.h"
#include "src/codec/SkPngPriv.h"
#include "src/core/SkSafeMath.h"
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

#ifdef SK_CODEC_USES_PNG_WITH_RUST_FOR_ANDROID
std::vector<uint8_t> GetSbitData(SkColorType colorType) {
    switch (colorType) {
        case kRGBA_F16Norm_SkColorType:
        case kRGBA_F16_SkColorType:
        case kRGBA_F32_SkColorType:
            return {16, 16, 16, 16};
        case kRGB_F16F16F16x_SkColorType:
            return {16, 16, 16};
        case kGray_8_SkColorType:
            return {8};
        case kRGB_888x_SkColorType:
            return {8, 8, 8};
        case kARGB_4444_SkColorType:
            return {4, 4, 4, 4};
        case kRGB_565_SkColorType:
            return {5, 6, 5};
        case kAlpha_8_SkColorType:
            return {kGraySigBit_GrayAlphaIsJustAlpha, 8};
        case kRGBA_1010102_SkColorType:
        case kBGRA_1010102_SkColorType:
            return {10, 10, 10, 2};
        case kBGR_101010x_XR_SkColorType:
        case kRGB_101010x_SkColorType:
        case kBGR_101010x_SkColorType:
            return {10, 10, 10};
        case kBGRA_10101010_XR_SkColorType:
            return {10, 10, 10, 10};
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            return {8, 8, 8, 8};
        default:
            return {};
    }
}

size_t ExpectedSbitSize(rust_png::ColorType colorType) {
    switch (colorType) {
        case rust_png::ColorType::Grayscale:
            return 1;
        case rust_png::ColorType::Rgb:
            return 3;
        case rust_png::ColorType::Indexed:
            return 3;
        case rust_png::ColorType::GrayscaleAlpha:
            return 2;
        case rust_png::ColorType::Rgba:
            return 4;
    }
    return 0;
}

bool WriteChunk(rust_png::Writer& writer, std::array<uint8_t, 4> name, SkSpan<const uint8_t> data) {
    auto slice = rust::Slice<const uint8_t>(data.data(), data.size());
    return writer.write_chunk(name, slice) == rust_png::EncodingResult::Success;
}

bool WriteSbitChunk(rust_png::Writer& writer,
                    SkColorType colorType,
                    rust_png::ColorType rustEncoderColorType) {
    std::vector<uint8_t> sbitData = GetSbitData(colorType);
    if (sbitData.empty()) {
        return true;
    }

    size_t expectedSize = ExpectedSbitSize(rustEncoderColorType);
    if (expectedSize == 0) {
        return WriteChunk(writer, {'s', 'B', 'I', 'T'}, sbitData);
    }

    if (sbitData.size() < expectedSize) {
        return true;
    }

    if (sbitData.size() > expectedSize) {
        sbitData.resize(expectedSize);
    }

    return WriteChunk(writer, {'s', 'B', 'I', 'T'}, sbitData);
}

bool WriteGainmapChunks(rust_png::Writer& writer, const SkPngRustEncoder::Options& options) {
    if (!options.fGainmapInfo) {
        return true;
    }

    if (!options.fGainmap) {
        // Encode gainmap info only (options.fGainmapInfo is true, options.fGainmap is null)
        sk_sp<SkData> data = options.fGainmapInfo->serialize();
        return WriteChunk(writer, {'g', 'm', 'A', 'P'}, data->byteSpan());
    }

    // Encode gainmap pixels (options.fGainmapInfo is true, options.fGainmap is non-null)

    // When we encode the gainmap, we need to remove the gainmap from its
    // own encoding options, so that we don't recurse.
    auto modifiedOptions = options;
    modifiedOptions.fGainmap = nullptr;

    auto gainmapInfo = *(options.fGainmapInfo);
    auto gainmapPixels = *(options.fGainmap);
    auto targetInfo = SkPngEncoderBase::getTargetInfo(gainmapPixels.info());

    if (targetInfo && targetInfo->fDstInfo.color() != SkEncodedInfo::kGray_Color &&
        targetInfo->fDstInfo.color() != SkEncodedInfo::kGrayAlpha_Color) {
        // Encode the alternate image colorspace directly in the gainmap profile,
        // since the ISO gainmap payload does not contain the actual alternative
        // image primaries.
        const auto& gainmapColorSpace = options.fGainmapInfo->fGainmapMathColorSpace;
        gainmapPixels.setColorSpace(gainmapColorSpace);
    } else {
        // Scrub the gainmap colorspace, since grayscale PNGs don't support
        // RGB ICC profiles
        gainmapInfo.fGainmapMathColorSpace = nullptr;
        modifiedOptions.fGainmapInfo = &gainmapInfo;
    }

    sk_sp<SkData> gainmapData = SkPngRustEncoder::Encode(gainmapPixels, modifiedOptions);
    if (!gainmapData) {
        return false;
    }

    sk_sp<SkData> gainmapVersion = SkGainmapInfo::SerializeVersion();
    if (!WriteChunk(writer, {'g', 'm', 'A', 'P'}, gainmapVersion->byteSpan())) {
        return false;
    }

    if (!WriteChunk(writer, {'g', 'd', 'A', 'T'}, gainmapData->byteSpan())) {
        return false;
    }
    return true;
}
#else
inline bool WriteSbitChunk(rust_png::Writer&, SkColorType, rust_png::ColorType) { return true; }
inline bool WriteGainmapChunks(rust_png::Writer&, const SkPngRustEncoder::Options&) { return true; }
#endif

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

    if (!WriteSbitChunk(*writer, src.colorType(), rustEncoderColorType)) {
        return nullptr;
    }

    if (!WriteGainmapChunks(*writer, options)) {
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
