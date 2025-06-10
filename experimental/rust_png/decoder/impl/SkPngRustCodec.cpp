/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_png/decoder/impl/SkPngRustCodec.h"

#include <limits>
#include <memory>
#include <utility>

#include "experimental/rust_png/ffi/FFI.rs.h"
#include "experimental/rust_png/ffi/UtilsForFFI.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkStream.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkSafe32.h"
#include "include/private/base/SkTemplates.h"
#include "modules/skcms/skcms.h"
#include "src/base/SkAutoMalloc.h"
#include "src/base/SkSafeMath.h"
#include "src/codec/SkFrameHolder.h"
#include "src/codec/SkParseEncodedOrigin.h"
#include "src/codec/SkSwizzler.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "third_party/rust/cxx/v1/cxx.h"

#ifdef __clang__
#pragma clang diagnostic error "-Wconversion"
#endif

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
        case rust_png::ColorType::Indexed:
            return SkEncodedInfo::kPalette_Color;
    }
    SK_ABORT("Unexpected `rust_png::ColorType`: %d", static_cast<int>(colorType));
}

SkEncodedInfo::Alpha ToAlpha(rust_png::ColorType colorType, const rust_png::Reader& reader) {
    switch (colorType) {
        case rust_png::ColorType::Grayscale:
        case rust_png::ColorType::Rgb:
            return SkEncodedInfo::kOpaque_Alpha;
        case rust_png::ColorType::GrayscaleAlpha:
        case rust_png::ColorType::Rgba:
            return SkEncodedInfo::kUnpremul_Alpha;
        case rust_png::ColorType::Indexed:
            if (reader.has_trns_chunk()) {
                return SkEncodedInfo::kUnpremul_Alpha;
            } else {
                return SkEncodedInfo::kOpaque_Alpha;
            }
    }
    SK_ABORT("Unexpected `rust_png::ColorType`: %d", static_cast<int>(colorType));
}

SkCodecAnimation::DisposalMethod ToDisposalMethod(rust_png::DisposeOp op) {
    switch (op) {
        case rust_png::DisposeOp::None:
            return SkCodecAnimation::DisposalMethod::kKeep;
        case rust_png::DisposeOp::Background:
            return SkCodecAnimation::DisposalMethod::kRestoreBGColor;
        case rust_png::DisposeOp::Previous:
            return SkCodecAnimation::DisposalMethod::kRestorePrevious;
    }
    SK_ABORT("Unexpected `rust_png::DisposeOp`: %d", static_cast<int>(op));
}

SkCodecAnimation::Blend ToBlend(rust_png::BlendOp op) {
    switch (op) {
        case rust_png::BlendOp::Source:
            return SkCodecAnimation::Blend::kSrc;
        case rust_png::BlendOp::Over:
            return SkCodecAnimation::Blend::kSrcOver;
    }
    SK_ABORT("Unexpected `rust_png::BlendOp`: %d", static_cast<int>(op));
}

std::unique_ptr<SkEncodedInfo::ICCProfile> CreateColorProfile(const rust_png::Reader& reader) {
    // NOTE: This method is based on `read_color_profile` in
    // `src/codec/SkPngCodec.cpp` but has been refactored to use Rust inputs
    // instead of `libpng`.

    // Considering the `cICP` chunk first, because the spec at
    // https://www.w3.org/TR/png-3/#cICP-chunk says: "This chunk, if understood
    // by the decoder, is the highest-precedence color chunk."
    uint8_t cicpPrimariesId = 0;
    uint8_t cicpTransferId = 0;
    uint8_t cicpMatrixId = 0;
    bool cicpIsFullRange = false;
    if (reader.try_get_cicp_chunk(cicpPrimariesId, cicpTransferId, cicpMatrixId, cicpIsFullRange)) {
        // https://www.w3.org/TR/png-3/#cICP-chunk says "RGB is currently the
        // only supported color model in PNG, and as such Matrix Coefficients
        // shall be set to 0."
        //
        // According to SkColorSpace::MakeCICP narrow range images are rare and
        // therefore not supported.
        if (cicpMatrixId == 0 && cicpIsFullRange) {
            sk_sp<SkColorSpace> colorSpace =
                    SkColorSpace::MakeCICP(static_cast<SkNamedPrimaries::CicpId>(cicpPrimariesId),
                                           static_cast<SkNamedTransferFn::CicpId>(cicpTransferId));
            if (colorSpace) {
                skcms_ICCProfile colorProfile;
                skcms_Init(&colorProfile);
                colorSpace->toProfile(&colorProfile);
                return SkEncodedInfo::ICCProfile::Make(colorProfile);
            }
        }
    }

    if (reader.has_iccp_chunk()) {
        // `SkData::MakeWithCopy` is resilient against 0-sized inputs, so
        // no need to check `rust_slice.empty()` here.
        rust::Slice<const uint8_t> rust_slice = reader.get_iccp_chunk();
        sk_sp<SkData> owned_data = SkData::MakeWithCopy(rust_slice.data(), rust_slice.size());
        std::unique_ptr<SkEncodedInfo::ICCProfile> parsed_data =
                SkEncodedInfo::ICCProfile::Make(std::move(owned_data));
        if (parsed_data) {
            return parsed_data;
        }
    }

    if (reader.is_srgb()) {
        // TODO(https://crbug.com/362304558): Consider the intent field from the
        // `sRGB` chunk.
        return nullptr;
    }

    // Next, check for presence of `gAMA` and `cHRM` chunks.
    float gamma = 0.0;
    const bool got_gamma = reader.try_get_gama(gamma);
    if (!got_gamma) {
        // We ignore whether `chRM` is present or not.
        //
        // This preserves the behavior decided in Chromium's 83587041dc5f1428c09
        // (https://codereview.chromium.org/2469473002).  The PNG spec states
        // that cHRM is valid even without gAMA but we cannot apply the cHRM
        // without guessing a gAMA.  Color correction is not a guessing game,
        // so we match the behavior of Safari and Firefox instead (compat).
        return nullptr;
    }
    float rx = 0.0;
    float ry = 0.0;
    float gx = 0.0;
    float gy = 0.0;
    float bx = 0.0;
    float by = 0.0;
    float wx = 0.0;
    float wy = 0.0;
    const bool got_chrm = reader.try_get_chrm(wx, wy, rx, ry, gx, gy, bx, by);
    if (!got_chrm) {
        // If there is no `cHRM` chunk then check if `gamma` is neutral (in PNG
        // / `SkNamedTransferFn::k2Dot2` sense).  `kPngGammaThreshold` mimics
        // `PNG_GAMMA_THRESHOLD_FIXED` from `libpng`.
        constexpr float kPngGammaThreshold = 0.05f;
        constexpr float kMinNeutralValue = 1.0f - kPngGammaThreshold;
        constexpr float kMaxNeutralValue = 1.0f + kPngGammaThreshold;
        float tmp = gamma * 2.2f;
        bool is_neutral = kMinNeutralValue < tmp && tmp < kMaxNeutralValue;
        if (is_neutral) {
            // Don't construct a custom color profile if the only encoded color
            // space information is a "neutral" gamma.  This is primarily needed
            // for correctness (see // https://crbug.com/388025081), but may
            // also help with performance (using a slightly more direct
            // `SkSwizzler` instead of `skcms_Transform`).
            return nullptr;
        }
    }

    // Construct a color profile based on `cHRM` and `gAMA` chunks.
    skcms_Matrix3x3 toXYZD50;
    if (got_chrm) {
        if (!skcms_PrimariesToXYZD50(rx, ry, gx, gy, bx, by, wx, wy, &toXYZD50)) {
            return nullptr;
        }
    } else {
        // `blink::PNGImageDecoder` returns a null color profile when `gAMA` is
        // present without `cHRM`.  We fall back to the sRGB profile instead
        // because we do gamma correction via `skcms_Transform` (rather than
        // relying on `libpng` gamma correction as the legacy Blink decoder does
        // in this scenario).
        toXYZD50 = skcms_sRGB_profile()->toXYZD50;
    }

    skcms_TransferFunction fn;
    fn.a = 1.0f;
    fn.b = fn.c = fn.d = fn.e = fn.f = 0.0f;
    fn.g = 1.0f / gamma;

    skcms_ICCProfile profile;
    skcms_Init(&profile);
    skcms_SetTransferFunction(&profile, &fn);
    skcms_SetXYZD50(&profile, &toXYZD50);
    return SkEncodedInfo::ICCProfile::Make(profile);
}

SkEncodedInfo CreateEncodedInfo(const rust_png::Reader& reader) {
    rust_png::ColorType rustColor = reader.output_color_type();
    SkEncodedInfo::Color skColor = ToColor(rustColor);

    std::unique_ptr<SkEncodedInfo::ICCProfile> profile = CreateColorProfile(reader);
    if (!SkPngCodecBase::isCompatibleColorProfileAndType(profile.get(), skColor)) {
        profile = nullptr;
    }

    static_assert(sizeof(int) >= sizeof(int32_t), "Is it ok to use Sk64_pin_to_s32 below?");
    return SkEncodedInfo::Make(Sk64_pin_to_s32(reader.width()),
                               Sk64_pin_to_s32(reader.height()),
                               skColor,
                               ToAlpha(rustColor, reader),
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
        case rust_png::DecodingResult::OtherIoError:
        case rust_png::DecodingResult::LimitsExceededError:
            return SkCodec::kInternalError;
        case rust_png::DecodingResult::IncompleteInput:
            return SkCodec::kIncompleteInput;
    }
    SK_ABORT("Unexpected `rust_png::DecodingResult`: %d", static_cast<int>(rustResult));
}

// This helper class adapts `SkStream` to expose the API required by Rust FFI
// (i.e. the `ReadAndSeekTraits` API).
class ReadAndSeekTraitsAdapterForSkStream final : public rust_png::ReadAndSeekTraits {
public:
    // SAFETY: The caller needs to guarantee that `stream` will be alive for
    // as long as `ReadAndSeekTraitsAdapterForSkStream`.
    explicit ReadAndSeekTraitsAdapterForSkStream(SkStream* stream) : fStream(stream) {
        SkASSERT(fStream);
    }

    ~ReadAndSeekTraitsAdapterForSkStream() override = default;

    // Non-copyable and non-movable (we want a stable `this` pointer, because we
    // will be passing a `ReadAndSeekTraits*` pointer over the FFI boundary and
    // retaining it inside `png::Reader`).
    ReadAndSeekTraitsAdapterForSkStream(const ReadAndSeekTraitsAdapterForSkStream&) = delete;
    ReadAndSeekTraitsAdapterForSkStream& operator=(const ReadAndSeekTraitsAdapterForSkStream&) =
            delete;
    ReadAndSeekTraitsAdapterForSkStream(ReadAndSeekTraitsAdapterForSkStream&&) = delete;
    ReadAndSeekTraitsAdapterForSkStream& operator=(ReadAndSeekTraitsAdapterForSkStream&&) = delete;

    // Implementation of the `std::io::Read::read` method.  See Rust trait's
    // doc comments at
    // https://doc.rust-lang.org/nightly/std/io/trait.Read.html#tymethod.read
    // for guidance on the desired implementation and behavior of this method.
    size_t read(rust::Slice<uint8_t> buffer) override {
        SkSpan<uint8_t> span = ToSkSpan(buffer);
        return fStream->read(span.data(), span.size());
    }

    // Implementation of the `std::io::Seek::seek` method.  See Rust trait`'s
    // doc comments at
    // https://doc.rust-lang.org/beta/std/io/trait.Seek.html#tymethod.seek
    // for guidance on the desired implementation and behavior of these methods.
    bool seek_from_start(uint64_t requestedPos, uint64_t& finalPos) override {
        SkSafeMath safe;
        size_t pos = safe.castTo<size_t>(requestedPos);
        if (!safe.ok()) {
            return false;
        }

        if (!fStream->seek(pos)) {
            return false;
        }
        SkASSERT(!fStream->hasPosition() || fStream->getPosition() == requestedPos);

        // Assigning `size_t` to `uint64_t` doesn't need to go through
        // `SkSafeMath`, because `uint64_t` is never smaller than `size_t`.
        static_assert(sizeof(uint64_t) >= sizeof(size_t));
        finalPos = requestedPos;

        return true;
    }
    bool seek_from_end(int64_t requestedOffset, uint64_t& finalPos) override {
        if (!fStream->hasLength()) {
            return false;
        }
        size_t length = fStream->getLength();

        SkSafeMath safe;
        uint64_t endPos = safe.castTo<uint64_t>(length);
        if (requestedOffset > 0) {
            // IIUC `SkStream` doesn't support reading beyond the current
            // length.
            return false;
        }
        if (requestedOffset == std::numeric_limits<int64_t>::min()) {
            // `-requestedOffset` below wouldn't work.
            return false;
        }
        uint64_t offset = safe.castTo<uint64_t>(-requestedOffset);
        if (!safe.ok()) {
            return false;
        }
        if (offset > endPos) {
            // `endPos - offset` below wouldn't work.
            return false;
        }

        return this->seek_from_start(endPos - offset, finalPos);
    }
    bool seek_relative(int64_t requestedOffset, uint64_t& finalPos) override {
        if (!fStream->hasPosition()) {
            return false;
        }

        SkSafeMath safe;
        long offset = safe.castTo<long>(requestedOffset);
        if (!safe.ok()) {
            return false;
        }

        if (!fStream->move(offset)) {
            return false;
        }

        finalPos = safe.castTo<uint64_t>(fStream->getPosition());
        if (!safe.ok()) {
            return false;
        }
        return true;
    }

private:
    SkStream* fStream = nullptr;  // Non-owning pointer.
};

void blendRow(SkSpan<uint8_t> dstRow,
              SkSpan<const uint8_t> srcRow,
              SkColorType color,
              SkAlphaType alpha) {
    SkASSERT(dstRow.size() >= srcRow.size());
    SkRasterPipeline_<256> p;

    SkRasterPipelineContexts::MemoryCtx dstCtx = {dstRow.data(), 0};
    p.appendLoadDst(color, &dstCtx);
    if (kUnpremul_SkAlphaType == alpha) {
        p.append(SkRasterPipelineOp::premul_dst);
    }

    SkRasterPipelineContexts::MemoryCtx srcCtx = {
        const_cast<void*>(static_cast<const void*>(srcRow.data())),
        0,
    };
    p.appendLoad(color, &srcCtx);
    if (kUnpremul_SkAlphaType == alpha) {
        p.append(SkRasterPipelineOp::premul);
    }

    p.append(SkRasterPipelineOp::srcover);

    if (kUnpremul_SkAlphaType == alpha) {
        p.append(SkRasterPipelineOp::unpremul);
    }
    p.appendStore(color, &dstCtx);

    SkSafeMath safe;
    size_t bpp = safe.castTo<size_t>(SkColorTypeBytesPerPixel(color));
    SkASSERT(safe.ok());

    size_t width = srcRow.size() / bpp;
    p.run(0, 0, width, 1);
}

void blendAllRows(SkSpan<uint8_t> dstFrame,
                  SkSpan<const uint8_t> srcFrame,
                  size_t rowSize,
                  size_t rowStride,
                  SkColorType color,
                  SkAlphaType alpha) {
    while (srcFrame.size() >= rowSize) {
        blendRow(dstFrame, srcFrame.first(rowSize), color, alpha);

        dstFrame = dstFrame.subspan(rowStride);
        srcFrame = srcFrame.subspan(rowStride);
    }
}

SkEncodedOrigin GetEncodedOrigin(const rust_png::Reader& reader) {
    if (reader.has_exif_chunk()) {
        rust::Slice<const uint8_t> rust_slice = reader.get_exif_chunk();
        SkEncodedOrigin origin;
        if (SkParseEncodedOrigin(rust_slice.data(), rust_slice.size(), &origin)) {
            return origin;
        }
    }

    return kTopLeft_SkEncodedOrigin;
}

}  // namespace

// static
std::unique_ptr<SkPngRustCodec> SkPngRustCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                               Result* result) {
    SkASSERT(stream);
    SkASSERT(result);

    auto inputAdapter = std::make_unique<ReadAndSeekTraitsAdapterForSkStream>(stream.get());
    rust::Box<rust_png::ResultOfReader> resultOfReader =
            rust_png::new_reader(std::move(inputAdapter));
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
        : SkPngCodecBase(std::move(encodedInfo),
                         // TODO(https://crbug.com/370522089): If/when `SkCodec` can
                         // avoid unnecessary rewinding, then stop "hiding" our stream
                         // from it.
                         /* stream = */ nullptr,
                         GetEncodedOrigin(*reader))
        , fReader(std::move(reader))
        , fPrivStream(std::move(stream))
        , fFrameHolder(encodedInfo.width(), encodedInfo.height()) {
    SkASSERT(fPrivStream);

    bool idatIsNotPartOfAnimation = fReader->has_actl_chunk() && !fReader->has_fctl_chunk();
    fFrameAtCurrentStreamPosition = idatIsNotPartOfAnimation ? -1 : 0;
    fStreamIsPositionedAtStartOfFrameData = true;
    if (!idatIsNotPartOfAnimation) {
        // This `appendNewFrame` call should always succeed because:
        // * `fFrameHolder.size()` is 0 at this point
        // * Width and height are already capped when calling `SkEncodedInfo::Make`
        // * `!fReader->has_fctl_chunk()` means that we don't need to worry
        //   about validating other frame metadata.
        Result result = fFrameHolder.appendNewFrame(*fReader, this->getEncodedInfo());
        SkASSERT(result == kSuccess);
    }
}

SkPngRustCodec::~SkPngRustCodec() = default;

SkCodec::Result SkPngRustCodec::readToStartOfNextFrame() {
    SkASSERT(fFrameAtCurrentStreamPosition < this->getRawFrameCount());
    Result result = ToSkCodecResult(fReader->next_frame_info());
    if (result != kSuccess) {
        fStreamIsPositionedAtStartOfFrameData = false;
        return result;
    }

    fStreamIsPositionedAtStartOfFrameData = true;
    fFrameAtCurrentStreamPosition++;
    if (fFrameAtCurrentStreamPosition == fFrameHolder.size()) {
        result = fFrameHolder.appendNewFrame(*fReader, this->getEncodedInfo());
    }

    return result;
}

SkCodec::Result SkPngRustCodec::seekToStartOfFrame(int index) {
    // Callers of this `private` method should provide a valid `index`.
    //
    // `index == fFrameHolder.size()` means that we are seeking to the next
    // frame (i.e. to the first frame for which an `fcTL` chunk wasn't parsed
    // yet).
    SkASSERT((0 <= index) && (index <= fFrameHolder.size()));

    // TODO(https://crbug.com/371060427): Improve runtime performance by seeking
    // directly to the right offset in the stream, rather than calling `rewind`
    // here and moving one-frame-at-a-time via `readToStartOfNextFrame` below.
    if ((index < fFrameAtCurrentStreamPosition) ||
        (index == fFrameAtCurrentStreamPosition && !fStreamIsPositionedAtStartOfFrameData)) {
        if (!fPrivStream->rewind()) {
            return kCouldNotRewind;
        }

        auto inputAdapter =
                std::make_unique<ReadAndSeekTraitsAdapterForSkStream>(fPrivStream.get());
        rust::Box<rust_png::ResultOfReader> resultOfReader =
                rust_png::new_reader(std::move(inputAdapter));

        // `SkPngRustCodec` constructor must have run before, and the
        // constructor got a successfully created reader - we therefore also
        // expect success here.
        SkASSERT(kSuccess == ToSkCodecResult(resultOfReader->err()));
        fReader = resultOfReader->unwrap();

        bool idatIsNotPartOfAnimation = fReader->has_actl_chunk() && !fReader->has_fctl_chunk();
        fFrameAtCurrentStreamPosition = idatIsNotPartOfAnimation ? -1 : 0;
        fStreamIsPositionedAtStartOfFrameData = true;
    }
    while (fFrameAtCurrentStreamPosition < index) {
        Result result = this->readToStartOfNextFrame();
        if (result != kSuccess) {
            return result;
        }
    }

    return kSuccess;
}

int SkPngRustCodec::getRawFrameCount() const {
    if (!fReader->has_actl_chunk()) {
        return 1;
    }

    static_assert(sizeof(int) >= sizeof(int32_t), "Is it ok to use Sk64_pin_to_s32 below?");
    uint32_t num_frames = fReader->get_actl_num_frames();
    return Sk64_pin_to_s32(num_frames);
}

SkCodec::Result SkPngRustCodec::parseAdditionalFrameInfos() {
    while (fFrameHolder.size() < this->getRawFrameCount()) {
        int oldFrameCount = fFrameHolder.size();

        Result result = this->seekToStartOfFrame(fFrameHolder.size());
        if (result != kSuccess) {
            return result;
        }
        SkASSERT(fFrameHolder.size() == (oldFrameCount + 1));
    }
    return kSuccess;
}

SkCodec::Result SkPngRustCodec::startDecoding(const SkImageInfo& dstInfo,
                                              void* pixels,
                                              size_t rowBytes,
                                              const Options& options,
                                              DecodingState* decodingState) {
    // TODO(https://crbug.com/362830091): Consider handling `fSubset`.
    if (options.fSubset) {
        return kUnimplemented;
    }

    if (options.fFrameIndex < 0 || options.fFrameIndex >= fFrameHolder.size()) {
        return kInvalidParameters;
    }
    const SkFrame* frame = fFrameHolder.getFrame(options.fFrameIndex);
    SkASSERT(frame);

    // https://www.w3.org/TR/png-3/#11PLTE says that for color type 3
    // (indexed-color), the PLTE chunk is required.  OTOH, `Codec_InvalidImages`
    // expects that we will succeed in this case and produce *some* output.
    if (this->getEncodedInfo().color() == SkEncodedInfo::kPalette_Color &&
        !fReader->has_plte_chunk()) {
        return kInvalidInput;
    }

    Result result = this->seekToStartOfFrame(options.fFrameIndex);
    if (result != kSuccess) {
        return result;
    }

    result = this->initializeXforms(dstInfo, options, frame->width());
    if (result != kSuccess) {
        return result;
    }

    {
        SkSafeMath safe;
        decodingState->fDstRowStride = rowBytes;

        uint8_t dstBytesPerPixel = safe.castTo<uint8_t>(dstInfo.bytesPerPixel());
        if (dstBytesPerPixel >= 32u) {
            return kInvalidParameters;
        }

        size_t imageHeight = safe.castTo<size_t>(dstInfo.height());
        size_t imageSize = safe.mul(rowBytes, imageHeight);

        size_t xPixelOffset = safe.castTo<size_t>(frame->xOffset());
        size_t xByteOffset = safe.mul(dstBytesPerPixel, xPixelOffset);

        size_t yPixelOffset = safe.castTo<size_t>(frame->yOffset());
        size_t yByteOffset = safe.mul(rowBytes, yPixelOffset);

        size_t frameWidth = safe.castTo<size_t>(frame->width());
        size_t rowSize = safe.mul(dstBytesPerPixel, frameWidth);
        size_t frameHeight = safe.castTo<size_t>(frame->height());
        size_t frameHeightTimesRowStride = safe.mul(frameHeight, rowBytes);
        decodingState->fDstRowSize = rowSize;

        if (!safe.ok()) {
            return kErrorInInput;
        }

        decodingState->fDst = SkSpan(static_cast<uint8_t*>(pixels), imageSize)
                                      .subspan(xByteOffset)
                                      .subspan(yByteOffset);
        if (frameHeightTimesRowStride < decodingState->fDst.size()) {
            decodingState->fDst = decodingState->fDst.first(frameHeightTimesRowStride);
        }

        if (frame->getBlend() == SkCodecAnimation::Blend::kSrcOver) {
            if (fReader->interlaced()) {
                decodingState->fPreblendBuffer.resize(imageSize, 0x00);
            } else {
                decodingState->fPreblendBuffer.resize(rowSize, 0x00);
            }
        }
    }

    return kSuccess;
}

void SkPngRustCodec::expandDecodedInterlacedRow(SkSpan<uint8_t> dstFrame,
                                                SkSpan<const uint8_t> srcRow,
                                                const DecodingState& decodingState) {
    SkASSERT(fReader->interlaced());
    std::vector<uint8_t> decodedInterlacedFullWidthRow;
    std::vector<uint8_t> xformedInterlacedRow;

    // Copy (potentially shorter for initial Adam7 passes) `srcRow` into a
    // full-frame-width `decodedInterlacedFullWidthRow`.  This is needed because
    // `applyXformRow` requires full-width rows as input (can't change
    // `SkSwizzler::fSrcWidth` after `initializeXforms`).
    //
    // TODO(https://crbug.com/399891492): Having `Reader.read_row` API (see
    // https://github.com/image-rs/image-png/pull/493) would help avoid
    // an extra copy here.
    decodedInterlacedFullWidthRow.resize(this->getEncodedRowBytes(), 0x00);
    SkASSERT(decodedInterlacedFullWidthRow.size() >= srcRow.size());
    memcpy(decodedInterlacedFullWidthRow.data(), srcRow.data(), srcRow.size());

    xformedInterlacedRow.resize(decodingState.fDstRowSize, 0x00);
    this->applyXformRow(xformedInterlacedRow, decodedInterlacedFullWidthRow);

    SkSafeMath safe;
    uint8_t dstBytesPerPixel = safe.castTo<uint8_t>(this->dstInfo().bytesPerPixel());
    SkASSERT(safe.ok());               // Checked in `startDecoding`.
    SkASSERT(dstBytesPerPixel < 32u);  // Checked in `startDecoding`.
    fReader->expand_last_interlaced_row(rust::Slice<uint8_t>(dstFrame),
                                        decodingState.fDstRowStride,
                                        rust::Slice<const uint8_t>(xformedInterlacedRow),
                                        dstBytesPerPixel * 8u);
}

SkCodec::Result SkPngRustCodec::incrementalDecode(DecodingState& decodingState,
                                                  int* rowsDecodedPtr) {
    this->initializeXformParams();

    int rowsDecoded = 0;
    bool interlaced = fReader->interlaced();
    while (true) {
        // TODO(https://crbug.com/357876243): Avoid an unconditional buffer hop
        // through buffer owned by `fReader` (e.g. when we can decode directly
        // into `dst`, because the pixel format received from `fReader` is
        // similar enough to `dstInfo`).
        rust::Slice<const uint8_t> decodedRow;

        fStreamIsPositionedAtStartOfFrameData = false;
        Result result = ToSkCodecResult(fReader->next_interlaced_row(decodedRow));
        if (result != kSuccess) {
            if (result == kIncompleteInput && rowsDecodedPtr) {
                *rowsDecodedPtr = rowsDecoded;
            }
            return result;
        }

        if (decodedRow.empty()) {  // This is how FFI layer says "no more rows".
            if (interlaced && !decodingState.fPreblendBuffer.empty()) {
                blendAllRows(decodingState.fDst,
                             decodingState.fPreblendBuffer,
                             decodingState.fDstRowSize,
                             decodingState.fDstRowStride,
                             this->dstInfo().colorType(),
                             this->dstInfo().alphaType());
            }
            if (!interlaced) {
                // All of the original `fDst` should be filled out at this point.
                SkASSERT(decodingState.fDst.empty());
            }

            // `static_cast` is ok, because `startDecoding` already validated `fFrameIndex`.
            fFrameHolder.markFrameAsFullyReceived(static_cast<size_t>(this->options().fFrameIndex));
            return kSuccess;
        }

        if (interlaced) {
            if (decodingState.fPreblendBuffer.empty()) {
                this->expandDecodedInterlacedRow(decodingState.fDst, decodedRow, decodingState);
            } else {
                this->expandDecodedInterlacedRow(
                        decodingState.fPreblendBuffer, decodedRow, decodingState);
            }
            // `rowsDecoded` is not incremented, because full, contiguous rows
            // are not decoded until pass 6 (or 7 depending on how you look) of
            // Adam7 interlacing scheme.
        } else {
            if (decodingState.fPreblendBuffer.empty()) {
                this->applyXformRow(decodingState.fDst, decodedRow);
            } else {
                this->applyXformRow(decodingState.fPreblendBuffer, decodedRow);
                blendRow(decodingState.fDst,
                         decodingState.fPreblendBuffer,
                         this->dstInfo().colorType(),
                         this->dstInfo().alphaType());
            }

            decodingState.fDst = decodingState.fDst.subspan(
                    std::min(decodingState.fDstRowStride, decodingState.fDst.size()));
            rowsDecoded++;
        }
    }
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
    Result result = this->incrementalDecode(*fIncrementalDecodingState, rowsDecoded);
    if (result != kIncompleteInput) {
        // After successfully reading the whole row (`kSuccess`), and after a
        // fatal error (only recoverable error is `kIncompleteInput`) our client
        // 1) should not call `onIncrementalDecode` and 2) can call
        // `onStartIncrementalDecode` (these are transitive call, done
        // indirectly via public API of `SkCodec`).  This means that the
        // incremental decoding state should be discarded at this point.
        fIncrementalDecodingState.reset();
    }
    return result;
}

int SkPngRustCodec::onGetFrameCount() {
    do {
        if (!fCanParseAdditionalFrameInfos || fIncrementalDecodingState.has_value()) {
            break;
        }

        if (fPrivStream->hasLength()) {
            size_t currentLength = fPrivStream->getLength();
            if (fStreamLengthDuringLastCallToParseAdditionalFrameInfos.has_value()) {
                size_t oldLength = *fStreamLengthDuringLastCallToParseAdditionalFrameInfos;
                if (oldLength == currentLength) {
                    // Don't retry `parseAdditionalFrameInfos` if the input
                    // didn't change.
                    break;
                }
                // We expect the input stream's length to be monotonically
                // increasing (even though the code may not yet rely on that
                // expectation).
                SkASSERT(currentLength > oldLength);
            }
            fStreamLengthDuringLastCallToParseAdditionalFrameInfos = currentLength;
        }

        switch (this->parseAdditionalFrameInfos()) {
            case kIncompleteInput:
                fCanParseAdditionalFrameInfos = true;
                break;
            case kSuccess:
                SkASSERT(fFrameHolder.size() == this->getRawFrameCount());
                fCanParseAdditionalFrameInfos = false;
                break;
            default:
                fCanParseAdditionalFrameInfos = false;
                break;
        }
    } while (false);

    return fFrameHolder.size();
}

bool SkPngRustCodec::onGetFrameInfo(int index, FrameInfo* info) const {
    return fFrameHolder.getFrameInfo(index, info);
}

int SkPngRustCodec::onGetRepetitionCount() {
    if (!fReader->has_actl_chunk()) {
        return 0;
    }

    uint32_t numFrames = fReader->get_actl_num_frames();
    if (numFrames <= 1) {
        return 0;
    }

    // APNG spec says that "`num_plays` indicates the number of times that this
    // animation should play; if it is 0, the animation should play
    // indefinitely."
    SkSafeMath safe;
    int numPlays = safe.castTo<int>(fReader->get_actl_num_plays());
    if ((numPlays == 0) || !safe.ok()) {
        return kRepetitionCountInfinite;
    }

    // Subtracting 1, because `SkCodec::onGetRepetitionCount` doc comment says
    // that "This number does not include the first play through of each frame.
    // For example, a repetition count of 4 means that each frame is played 5
    // times and then the animation stops."
    return numPlays - 1;
}

SkCodec::IsAnimated SkPngRustCodec::onIsAnimated() {
    if (fReader->has_actl_chunk() && fReader->get_actl_num_frames() > 1) {
        return IsAnimated::kYes;
    }
    return IsAnimated::kNo;
}

std::optional<SkSpan<const SkPngCodecBase::PaletteColorEntry>> SkPngRustCodec::onTryGetPlteChunk() {
    if (fReader->output_color_type() != rust_png::ColorType::Indexed) {
        return std::nullopt;
    }

    SkASSERT(fReader->has_plte_chunk());  // Checked in `startDecoding`.
    SkSpan<const uint8_t> bytes = ToSkSpan(fReader->get_plte_chunk());

    // Make sure that `bytes.size()` is a multiple of
    // `sizeof(PaletteColorEntry)`.
    constexpr size_t kEntrySize = sizeof(PaletteColorEntry);
    bytes = bytes.first((bytes.size() / kEntrySize) * kEntrySize);

    // Alignment of `PaletteColorEntry` is 1, because its size is 3, and size
    // has to be a multiple of alignment (every element of an array has to be
    // aligned) + alignment is always a power of 2.  And this means that
    // `bytes.data()` is already aligned.
    static_assert(kEntrySize == 3, "");
    static_assert(std::alignment_of<PaletteColorEntry>::value == 1, "");
    static_assert(std::alignment_of<uint8_t>::value == 1, "");
    SkSpan<const PaletteColorEntry> palette = SkSpan(
            reinterpret_cast<const PaletteColorEntry*>(bytes.data()), bytes.size() / kEntrySize);

    return palette;
}

std::optional<SkSpan<const uint8_t>> SkPngRustCodec::onTryGetTrnsChunk() {
    if (fReader->output_color_type() != rust_png::ColorType::Indexed) {
        return std::nullopt;
    }

    if (!fReader->has_trns_chunk()) {
        return std::nullopt;
    }

    return ToSkSpan(fReader->get_trns_chunk());
}

class SkPngRustCodec::FrameHolder::PngFrame final : public SkFrame {
public:
    PngFrame(int id, SkEncodedInfo::Alpha alpha) : SkFrame(id), fReportedAlpha(alpha) {}

    bool isFullyReceived() const { return fFullyReceived; }
    void markAsFullyReceived() { fFullyReceived = true; }

private:
    SkEncodedInfo::Alpha onReportedAlpha() const override { return fReportedAlpha; }

    const SkEncodedInfo::Alpha fReportedAlpha;
    bool fFullyReceived = false;
};

SkPngRustCodec::FrameHolder::FrameHolder(int width, int height) : SkFrameHolder() {
    fScreenWidth = width;
    fScreenHeight = height;
}

const SkFrameHolder* SkPngRustCodec::getFrameHolder() const { return &fFrameHolder; }

// We cannot use the SkCodec implementation since we pass nullptr to the superclass out of
// an abundance of caution w/r to rewinding the stream.
//
// TODO(https://crbug.com/370522089): See if `SkCodec` can be tweaked to avoid
// the need to hide the stream from it.
std::unique_ptr<SkStream> SkPngRustCodec::getEncodedData() const {
    SkASSERT(fPrivStream);
    return fPrivStream->duplicate();
}

SkPngRustCodec::FrameHolder::~FrameHolder() = default;

const SkFrame* SkPngRustCodec::FrameHolder::onGetFrame(int unverifiedIndex) const {
    SkSafeMath safe;
    size_t index = safe.castTo<size_t>(unverifiedIndex);
    if (safe.ok() && (index < fFrames.size())) {
        return &fFrames[index];
    }
    return nullptr;
}

int SkPngRustCodec::FrameHolder::size() const {
    // This invariant is maintained in `appendNewFrame`.
    SkASSERT(SkTFitsIn<int>(fFrames.size()));
    return static_cast<int>(fFrames.size());
}

void SkPngRustCodec::FrameHolder::markFrameAsFullyReceived(size_t index) {
    SkASSERT(index < fFrames.size());
    fFrames[index].markAsFullyReceived();
}

bool SkPngRustCodec::FrameHolder::getFrameInfo(int index, FrameInfo* info) const {
    const SkFrame* frame = this->getFrame(index);
    if (frame && info) {
        bool isFullyReceived = static_cast<const PngFrame*>(frame)->isFullyReceived();
        frame->fillIn(info, isFullyReceived);
    }
    return !!frame;
}

SkCodec::Result SkPngRustCodec::FrameHolder::appendNewFrame(const rust_png::Reader& reader,
                                                            const SkEncodedInfo& info) {
    // Ensure that `this->size()` fits into an `int`.  `+ 1u` is used to account
    // for `push_back` / `emplace_back` below.
    if (!SkTFitsIn<int>(fFrames.size() + 1u)) {
        return kErrorInInput;
    }
    int id = static_cast<int>(fFrames.size());

    if (reader.has_fctl_chunk()) {
        if (!fFrames.empty()) {
            // Having `fcTL` for a new frame means that the previous frame has been
            // fully received (since all of the previous frame's `fdAT` / `IDAT`
            // chunks must have come before the new frame's `fcTL` chunk).
            fFrames.back().markAsFullyReceived();
        }

        PngFrame frame(id, info.alpha());
        SkCodec::Result result = this->setFrameInfoFromCurrentFctlChunk(reader, &frame);
        if (result == SkCodec::kSuccess) {
            fFrames.push_back(std::move(frame));
        }
        return result;
    }

    SkASSERT(!reader.has_actl_chunk());
    SkASSERT(id == 0);
    fFrames.emplace_back(id, info.alpha());
    SkFrame& frame = fFrames.back();
    frame.setXYWH(0, 0, info.width(), info.height());
    frame.setBlend(SkCodecAnimation::Blend::kSrc);
    this->setAlphaAndRequiredFrame(&frame);
    return kSuccess;
}

SkCodec::Result SkPngRustCodec::FrameHolder::setFrameInfoFromCurrentFctlChunk(
        const rust_png::Reader& reader, PngFrame* frame) {
    SkASSERT(reader.has_fctl_chunk());  // Caller should guarantee this
    SkASSERT(frame);

    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t xOffset = 0;
    uint32_t yOffset = 0;
    auto disposeOp = rust_png::DisposeOp::None;
    auto blendOp = rust_png::BlendOp::Source;
    uint32_t durationMs = 0;
    reader.get_fctl_info(width, height, xOffset, yOffset, disposeOp, blendOp, durationMs);

    {
        SkSafeMath safe;
        frame->setXYWH(safe.castTo<int>(xOffset),
                       safe.castTo<int>(yOffset),
                       safe.castTo<int>(width),
                       safe.castTo<int>(height));
        frame->setDuration(safe.castTo<int>(durationMs));
        if (!safe.ok()) {
            return kErrorInInput;
        }
    }

    frame->setDisposalMethod(ToDisposalMethod(disposeOp));

    // https://wiki.mozilla.org/APNG_Specification#.60fcTL.60:_The_Frame_Control_Chunk
    // points out that "for the first frame the two blend modes are functionally
    // equivalent" so we use `BlendOp::Source` because it has better performance
    // characteristics.
    if (frame->frameId() == 0) {
        blendOp = rust_png::BlendOp::Source;
    }
    frame->setBlend(ToBlend(blendOp));

    // Note: `setAlphaAndRequiredFrame` needs to be called last, because it
    // depends on the other properties set above.
    this->setAlphaAndRequiredFrame(frame);
    return kSuccess;
}
