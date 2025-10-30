/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkPngRustCodec.h"

#include <limits>
#include <memory>
#include <utility>

#include "include/core/SkColorSpace.h"
#include "include/core/SkStream.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/SkHdrMetadata.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkSafe32.h"
#include "include/private/base/SkTemplates.h"
#include "modules/skcms/skcms.h"
#include "rust/png/FFI.rs.h"
#include "rust/common/SpanUtils.h"
#include "src/base/SkAutoMalloc.h"
#include "src/base/SkSafeMath.h"
#include "src/codec/SkFrameHolder.h"
#include "src/codec/SkParseEncodedOrigin.h"
#include "src/codec/SkPngPriv.h"
#include "src/codec/SkSwizzler.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "third_party/rust/cxx/v1/cxx.h"

#ifdef __clang__
#pragma clang diagnostic error "-Wconversion"
#endif

namespace {

SkEncodedInfo::Color ToColor(rust_png::ColorType colorType, const rust_png::Reader& reader) {
    switch (colorType) {
        case rust_png::ColorType::Grayscale:
            return SkEncodedInfo::kGray_Color;
        case rust_png::ColorType::Rgb:
            if (reader.has_sbit_chunk()) {
                SkSpan<const uint8_t> sBit = ToSkSpan(reader.get_sbit_chunk());
                if (sBit.size() == 3) {
                  if (sBit[0] == 5 && sBit[1] == 6 && sBit[2] == 5) {
                      return SkEncodedInfo::k565_Color;
                  }
                }
            }
            return SkEncodedInfo::kRGB_Color;
        case rust_png::ColorType::GrayscaleAlpha:
            if (reader.has_sbit_chunk()) {
                SkSpan<const uint8_t> sBit = ToSkSpan(reader.get_sbit_chunk());
                if (sBit.size() == 2) {
                    if (sBit[0] == kGraySigBit_GrayAlphaIsJustAlpha && sBit[1] == 8) {
                        return SkEncodedInfo::kXAlpha_Color;
                    }
                }
            }
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

SkColorSpacePrimaries ToSkColorSpacePrimaries(const rust_png::ColorSpacePrimaries& p) {
    return SkColorSpacePrimaries({p.fRX, p.fRY, p.fGX, p.fGY, p.fBX, p.fBY, p.fWX, p.fWY});
}

skhdr::MasteringDisplayColorVolume ToSkMDCV(const rust_png::MasteringDisplayColorVolume& mdcv) {
    return skhdr::MasteringDisplayColorVolume({
        ToSkColorSpacePrimaries(mdcv.fDisplayPrimaries),
        mdcv.fMaximumDisplayMasteringLuminance,
        mdcv.fMinimumDisplayMasteringLuminance});
}

skhdr::ContentLightLevelInformation ToSkCLLI(const rust_png::ContentLightLevelInfo& clli) {
    return skhdr::ContentLightLevelInformation({clli.fMaxCLL, clli.fMaxFALL});
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
    float gamma = 0;
    // Unlike libpng, image-rs does not seem to validate the gamma value.
    // So we check explicitly for zero, which is an invalid value encountered in the wild.
    // TODO: upstream?
    if (!reader.try_get_gama(gamma) || gamma <= 0) {
        // We ignore whether `chRM` is present or not.
        //
        // This preserves the behavior decided in Chromium's 83587041dc5f1428c09
        // (https://codereview.chromium.org/2469473002).  The PNG spec states
        // that cHRM is valid even without gAMA but we cannot apply the cHRM
        // without guessing a gAMA.  Color correction is not a guessing game,
        // so we match the behavior of Safari and Firefox instead (compat).
        return nullptr;
    }
    rust_png::ColorSpacePrimaries chrm;
    const bool got_chrm = reader.try_get_chrm(chrm);
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
        if (!ToSkColorSpacePrimaries(chrm).toXYZD50(&toXYZD50)) {
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

// Returns `nullopt` when input errors are encountered.
std::optional<SkEncodedInfo> CreateEncodedInfo(const rust_png::Reader& reader) {
    rust_png::ColorType rustColor = reader.output_color_type();
    SkEncodedInfo::Color skColor = ToColor(rustColor, reader);

    std::unique_ptr<SkEncodedInfo::ICCProfile> profile = CreateColorProfile(reader);
    if (!SkPngCodecBase::isCompatibleColorProfileAndType(profile.get(), skColor)) {
        profile = nullptr;
    }

    skhdr::Metadata hdrMetadata;
    {
        rust_png::MasteringDisplayColorVolume rust_mdcv;
        if (reader.try_get_mdcv_chunk(rust_mdcv)) {
            hdrMetadata.setMasteringDisplayColorVolume(ToSkMDCV(rust_mdcv));
        }
        rust_png::ContentLightLevelInfo rust_clli;
        if (reader.try_get_clli_chunk(rust_clli)) {
            hdrMetadata.setContentLightLevelInformation(ToSkCLLI(rust_clli));
        }
    }

    // Protect against large PNGs. See http://bugzil.la/251381 for more details.
    constexpr uint32_t kMaxPNGSize = 1000000;
    if ((reader.width() > kMaxPNGSize) || (reader.height() > kMaxPNGSize)) {
        return std::nullopt;
    }
    // We checked image dimensions above, so here we can just assert that casts
    // from `uint32_t` to `int` work ok.
    //
    // We don't use a saturating cast, because this could invalidate `fcTL`
    // checks done within the `png` crate.  For example, the new / truncated
    // image width could end up smaller than `fcTL.frameWidth`.
    SkSafeMath safe;
    int width = safe.castTo<int>(reader.width());
    int height = safe.castTo<int>(reader.height());
    SkASSERT_RELEASE(safe.ok());

    return SkEncodedInfo::Make(width,
                               height,
                               skColor,
                               ToAlpha(rustColor, reader),
                               reader.output_bits_per_component(), // bitsPerComponent
                               reader.output_bits_per_component(), // colorDepth
                               std::move(profile),
                               hdrMetadata);
}

SkCodec::Result ToSkCodecResult(rust_png::DecodingResult rustResult) {
    switch (rustResult) {
        case rust_png::DecodingResult::Success:
        case rust_png::DecodingResult::EndOfFrame:
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
        SkASSERT_RELEASE(fStream);
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
        SkASSERT_RELEASE(!fStream->hasPosition() || fStream->getPosition() == requestedPos);

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
    SkASSERT_RELEASE(dstRow.size() >= srcRow.size());
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
    SkASSERT_RELEASE(safe.ok());

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

        srcFrame = srcFrame.subspan(std::min(rowStride, srcFrame.size()));
        dstFrame = dstFrame.subspan(std::min(rowStride, dstFrame.size()));
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

bool IsValidFctlIfAny(const SkEncodedInfo& imageInfo,
                      const rust_png::Reader& reader) {
    // Enforce that if an `fcTL` appears before an `IDAT` chunk, then it has the
    // same dimensions as the ones in the earlier `IHDR` chunk.  This
    // corresponds to the restrictions that the spec at
    // https://www.w3.org/TR/png-3/#fcTL-chunk places on "fcTL chunk
    // corresponding to the default image".
    //
    // Doing this check here is more robust than doing it inside
    // `setFrameInfoFromCurrentFctlChunk`, because the code elsewhere in
    // `SkPngRustCodec` may ignore the `fcTL` chunk (e.g. if
    // `idatIsNotPartOfAnimation` and/or if `acTL` chunk is missing).
    //
    // Reporting a hard error is more robust than trying to ignore the `fcTL`
    // chunk and falling back to decoding a static image, because such a
    // fallback would risk discrepancies between dimensions used in different
    // layers of the software stack (see https://crbug.com/428205250).
    //
    // This check is kind of a defense-in-depth - in the long-term the
    // dimensions should be checked in the Rust `png` crate itself
    // (see https://github.com/image-rs/image-png/pull/614 which hasn't yet been
    // released in a new crates.io version).
    if (reader.has_fctl_chunk()) {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t xOffset = 0;
        uint32_t yOffset = 0;
        auto ignoredDisposeOp = rust_png::DisposeOp::None;
        auto ignoredBlendOp = rust_png::BlendOp::Source;
        uint32_t ignoredDurationMs = 0;
        reader.get_fctl_info(width, height, xOffset, yOffset,
                             ignoredDisposeOp, ignoredBlendOp, ignoredDurationMs);

        SkSafeMath safe;
        int frameWidth = safe.castTo<int>(width);
        int frameHeight = safe.castTo<int>(height);
        if (!safe.ok()) {
            return false;
        }

        if (xOffset != 0 || frameWidth != imageInfo.width() ||
                yOffset != 0 || frameHeight != imageInfo.height()) {
            return false;
        }
    }

    return true;
}

}  // namespace

// static
std::unique_ptr<SkPngRustCodec> SkPngRustCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                               Result* result) {
    SkASSERT_RELEASE(stream);
    SkASSERT_RELEASE(result);

    auto inputAdapter = std::make_unique<ReadAndSeekTraitsAdapterForSkStream>(stream.get());
    rust::Box<rust_png::ResultOfReader> resultOfReader =
            rust_png::new_reader(std::move(inputAdapter));
    *result = ToSkCodecResult(resultOfReader->err());
    if (*result != kSuccess) {
        return nullptr;
    }
    rust::Box<rust_png::Reader> reader = resultOfReader->unwrap();

    std::optional<SkEncodedInfo> maybeImageInfo = CreateEncodedInfo(*reader);
    if (!maybeImageInfo.has_value()) {
        *result = kErrorInInput;
        return nullptr;
    }
    SkEncodedInfo& imageInfo = *maybeImageInfo;

    if (!IsValidFctlIfAny(imageInfo, *reader)) {
        *result = kErrorInInput;
        return nullptr;
    }

    return std::make_unique<SkPngRustCodec>(
            std::move(imageInfo), std::move(stream), std::move(reader));
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
    SkASSERT_RELEASE(fPrivStream);

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
        SkASSERT_RELEASE(result == kSuccess);
    }
}

SkPngRustCodec::~SkPngRustCodec() = default;

SkCodec::Result SkPngRustCodec::readToStartOfNextFrame() {
    SkASSERT_RELEASE(fFrameAtCurrentStreamPosition < this->getRawFrameCount());
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
    SkASSERT_RELEASE((0 <= index) && (index <= fFrameHolder.size()));

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
        SkASSERT_RELEASE(kSuccess == ToSkCodecResult(resultOfReader->err()));
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
        SkASSERT_RELEASE(fFrameHolder.size() == (oldFrameCount + 1));
    }
    return kSuccess;
}

void SkPngRustCodec::getSubsetFromFullImage(SkSpan<const uint8_t> fullImageBuffer,
                                            SkSpan<uint8_t> dst,
                                            size_t dstRowStride,
                                            size_t offset) {
    // This only needs to be used in the case of interlaced images that need a subset,
    // otherwise we can decode row by row.
    SkASSERT_RELEASE(fReader->interlaced());
    SkASSERT_RELEASE(this->options().fSubset);
    SkASSERT_RELEASE(fullImageBuffer.size() >= dst.size());
    // We want the whole row and applyXformRow does the rest, so only offset to correct y value.
    fullImageBuffer = fullImageBuffer.subspan(offset);
    const size_t encodedRowBytes = this->getEncodedRowBytes();

    for (int i = 0; i < this->options().fSubset->height(); ++i) {
        SkSpan<const uint8_t> srcRow = fullImageBuffer.first(encodedRowBytes);
        fullImageBuffer = fullImageBuffer.subspan(encodedRowBytes);

        SkSpan<uint8_t> dstRow = dst.first(dstRowStride);
        dst = dst.subspan(dstRowStride);

        // Copy the source row into the correct position in the destination.
        this->applyXformRow(dstRow, srcRow);
    }
}

SkCodec::Result SkPngRustCodec::startDecoding(const SkImageInfo& dstInfo,
                                              void* pixels,
                                              size_t rowBytes,
                                              const Options& options,
                                              DecodingState* decodingState) {
    // TODO(https://crbug.com/362830091): Consider handling `fSubset` for APNG.
    if (options.fSubset && this->isAnimated() != IsAnimated::kNo) {
        return kUnimplemented;
    }

    if (options.fFrameIndex < 0 || options.fFrameIndex >= fFrameHolder.size()) {
        return kInvalidParameters;
    }
    const SkFrame* frame = fFrameHolder.getFrame(options.fFrameIndex);
    SkASSERT_RELEASE(frame);

    Result result = this->seekToStartOfFrame(options.fFrameIndex);
    if (result != kSuccess) {
        return result;
    }

    // https://www.w3.org/TR/png-3/#11PLTE says that for color type 3
    // (indexed-color), the PLTE chunk is required.  OTOH, `Codec_InvalidImages`
    // expects that we will succeed in this case and produce *some* output.
    //
    // This check needs to happen after `seekToStartOfFrame`, because the act
    // of seeking-and/or-rewinding may reset whether `fReader` has encountered
    // an PLTE chunk yet or not.
    if (this->getEncodedInfo().color() == SkEncodedInfo::kPalette_Color &&
        !fReader->has_plte_chunk()) {
        return kInvalidInput;
    }

    result = this->initializeXforms(dstInfo, options, frame->width());
    if (result != kSuccess) {
        return result;
    }

    {
        DecodingDstInfo& decodingDst = decodingState->fDecodingDstInfo;

        SkSafeMath safe;
        decodingDst.fDstRowStride = rowBytes;

        uint8_t dstBytesPerPixel = safe.castTo<uint8_t>(dstInfo.bytesPerPixel());
        if (dstBytesPerPixel >= 32u) {
            return kInvalidParameters;
        }
        decodingDst.fDstBytesPerPixel = dstBytesPerPixel;

        size_t imageHeight = safe.castTo<size_t>(dstInfo.height());
        size_t imageSize = safe.mul(rowBytes, imageHeight);

        size_t xPixelOffset = safe.castTo<size_t>(frame->xOffset());
        size_t xByteOffsetFrame = safe.mul(dstBytesPerPixel, xPixelOffset);

        size_t yPixelOffset = safe.castTo<size_t>(frame->yOffset());
        size_t yByteOffsetFrame = safe.mul(rowBytes, yPixelOffset);

        if (options.fSubset) {
            decodingState->fYByteOffset = safe.mul(safe.castTo<size_t>(this->getEncodedRowBytes()),
                                                   safe.castTo<size_t>(options.fSubset->top()));
            decodingState->fFirstRow = options.fSubset->top();
            decodingState->fLastRow = options.fSubset->bottom() - 1;
        } else {
            decodingState->fYByteOffset = 0;
            decodingState->fFirstRow = 0;
            decodingState->fLastRow = frame->yOffset() + frame->height() - 1;
        }

        size_t frameWidth = safe.castTo<size_t>(frame->width());
        size_t rowSize = safe.mul(dstBytesPerPixel, frameWidth);
        size_t frameHeight = safe.castTo<size_t>(frame->height());
        size_t frameHeightTimesRowStride = safe.mul(frameHeight, rowBytes);
        decodingDst.fDstRowSize = rowSize;

        size_t encodedImageSize = safe.mul(this->getEncodedRowBytes(),
            safe.castTo<size_t>(this->getEncodedInfo().height()));

        if (!safe.ok()) {
            return kErrorInInput;
        }

        decodingDst.fDst = SkSpan(static_cast<uint8_t*>(pixels), imageSize)
                                .subspan(xByteOffsetFrame)
                                .subspan(yByteOffsetFrame);
        if (frameHeightTimesRowStride < decodingDst.fDst.size()) {
            decodingDst.fDst = decodingDst.fDst.first(frameHeightTimesRowStride);
        }

        if (fReader->interlaced()) {
            // Use fPreblendBuffer to decode whole image untransformed, then truncate/xform later.
            if (options.fSubset) {
                decodingState->fPreblendBuffer.resize(encodedImageSize, 0x00);
            } else if (frame->getBlend() == SkCodecAnimation::Blend::kSrcOver) {
                decodingState->fPreblendBuffer.resize(decodingDst.fDst.size(), 0x00);
            }
        } else if (frame->getBlend() == SkCodecAnimation::Blend::kSrcOver) {
            decodingState->fPreblendBuffer.resize(rowSize, 0x00);
        }
    }

    return kSuccess;
}

void SkPngRustCodec::expandDecodedInterlacedRow(SkSpan<uint8_t> dstFrame,
                                                SkSpan<const uint8_t> srcRow,
                                                const DecodingDstInfo& decodingDst,
                                                bool xFormNeeded) {
    SkASSERT_RELEASE(fReader->interlaced());
    std::vector<uint8_t> decodedInterlacedFullWidthRow;
    std::vector<uint8_t> xformedInterlacedRow;
    const size_t dstRowStride = decodingDst.fDstRowStride;
    if (xFormNeeded) {
        // Copy (potentially shorter for initial Adam7 passes) `srcRow` into a
        // full-frame-width `decodedInterlacedFullWidthRow`.  This is needed because
        // `applyXformRow` requires full-width rows as input (can't change
        // `SkSwizzler::fSrcWidth` after `initializeXforms`).
        decodedInterlacedFullWidthRow.resize(this->getEncodedRowBytes(), 0x00);
        SkASSERT_RELEASE(decodedInterlacedFullWidthRow.size() >= srcRow.size());
        memcpy(decodedInterlacedFullWidthRow.data(), srcRow.data(), srcRow.size());

        xformedInterlacedRow.resize(decodingDst.fDstRowSize, 0x00);
        this->applyXformRow(xformedInterlacedRow, decodedInterlacedFullWidthRow);

    }

    const uint8_t dstBytesPerPixel = decodingDst.fDstBytesPerPixel;
    SkASSERT_RELEASE(dstBytesPerPixel < 32u);  // Checked in `startDecoding`.
    if (xFormNeeded) {
        fReader->expand_last_interlaced_row(rust::Slice<uint8_t>(dstFrame),
                                            dstRowStride,
                                            rust::Slice<const uint8_t>(xformedInterlacedRow),
                                            dstBytesPerPixel * 8u);
    } else {
        fReader->expand_last_interlaced_row(rust::Slice<uint8_t>(dstFrame),
                                            dstRowStride,
                                            rust::Slice<const uint8_t>(srcRow),
                                            dstBytesPerPixel * 8u);
    }
}

// Given the dstInfo and the rust colortype/bits per component, determines if we
// can use rust_png::Reader::read_row to decode directly into dst.
bool SkPngRustCodec::canReadRow() {
    // Check alpha types
    if (this->dstInfo().alphaType() != kUnpremul_SkAlphaType) {
        return false;
    }
    // We use temporary buffer to read the full image for subsets.
    if (this->options().fSubset) {
        return false;
    }

    // Check color types
    rust_png::ColorType color_type = fReader->output_color_type();
    uint8_t bits_per_component =  fReader->output_bits_per_component();
    switch (this->dstInfo().colorType()) {
        case kRGBA_8888_SkColorType:
            if (color_type != rust_png::ColorType::Rgba
                || bits_per_component != 8) {
                return false;
            }
            break;
        case kGray_8_SkColorType:
            if (color_type != rust_png::ColorType::Grayscale
                || bits_per_component != 8) {
                return false;
            }
            break;
        default:
            return false;
    }

    // Check profiles
    if (!!this->getEncodedInfo().profile() != !!this->dstInfo().colorSpace()) {
        return false;
    }
    if (this->getEncodedInfo().profile()) {
        SkASSERT_RELEASE(this->dstInfo().colorSpace());
        skcms_ICCProfile dstProfile;
        this->dstInfo().colorSpace()->toProfile(&dstProfile);
        if (!skcms_ApproximatelyEqualProfiles(this->getEncodedInfo().profile(), &dstProfile)) {
            return false;
        }
    }

    return true;
}

SkCodec::Result SkPngRustCodec::incrementalDecodeXForm(DecodingState& decodingState,
                                                       int* rowsDecodedPtr) {
    SkASSERT_RELEASE(!this->canReadRow());
    this->initializeXformParams();

    int rowsDecoded = 0;
    const bool interlaced = fReader->interlaced();
    const bool subset = this->options().fSubset;
    DecodingDstInfo& decodingDst = decodingState.fDecodingDstInfo;

    int rowNum = 0;
    while (true) {
        rust::Slice<const uint8_t> decodedRow;
        fStreamIsPositionedAtStartOfFrameData = false;
        Result result = ToSkCodecResult(fReader->next_interlaced_row(decodedRow));
        if (result != kSuccess) {
            if (result == kIncompleteInput && rowsDecodedPtr) {
                *rowsDecodedPtr = rowsDecoded;
            }
            return result;
        }

        // This is how FFI layer says "no more rows". We also want to stop reading rows
        // if we are at the end of our subset.
        if (decodedRow.empty() || rowNum > decodingState.fLastRow) {
            if (interlaced && !decodingState.fPreblendBuffer.empty()) {
                if (subset) {
                    this->getSubsetFromFullImage(SkSpan<uint8_t>(decodingState.fPreblendBuffer),
                                                 decodingDst.fDst,
                                                 decodingDst.fDstRowStride,
                                                 decodingState.fYByteOffset);
                } else {
                    blendAllRows(decodingDst.fDst,
                                 decodingState.fPreblendBuffer,
                                 decodingDst.fDstRowSize,
                                 decodingDst.fDstRowStride,
                                 this->dstInfo().colorType(),
                                 this->dstInfo().alphaType());
                }
            }
            if (!interlaced && !subset) {
                // All of the original `fDst` should be filled out at this point.
                SkASSERT_RELEASE(decodingDst.fDst.empty());
            }

            // `static_cast` is ok, because `startDecoding` already validated `fFrameIndex`.
            fFrameHolder.markFrameAsFullyReceived(static_cast<size_t>(this->options().fFrameIndex));
            return kSuccess;
        }

        if (interlaced) {
            if (decodingState.fPreblendBuffer.empty()) {
                this->expandDecodedInterlacedRow(
                    decodingDst.fDst, decodedRow, decodingDst, /*xFormNeeded=*/true);
            } else {
                if (subset) {
                    SkSafeMath safe;
                    uint8_t encodedBytesPerPixel = safe.castTo<uint8_t>(this->getEncodedInfo()
                                                      .makeImageInfo()
                                                      .bytesPerPixel());
                    SkASSERT_RELEASE(safe.ok());               // Checked in `startDecoding`.
                    DecodingDstInfo fullImageDecodingDst =
                        {.fDst = decodingState.fPreblendBuffer,
                         .fDstRowStride = this->getEncodedRowBytes(),
                         .fDstRowSize = this->getEncodedRowBytes(),
                         .fDstBytesPerPixel = encodedBytesPerPixel};
                    this->expandDecodedInterlacedRow(decodingState.fPreblendBuffer,
                                                     decodedRow,
                                                     fullImageDecodingDst,
                                                     /*xFormNeeded=*/false);
                } else {
                  this->expandDecodedInterlacedRow(decodingState.fPreblendBuffer,
                                                   decodedRow,
                                                   decodingDst,
                                                   /*xFormNeeded=*/true);
                }
            }
            // `rowsDecoded` is not incremented, because full, contiguous rows
            // are not decoded until pass 6 (or 7 depending on how you look) of
            // Adam7 interlacing scheme.
        } else {
            if (rowNum++ < decodingState.fFirstRow) {
                continue;
            }
            if (decodingState.fPreblendBuffer.empty()) {
                this->applyXformRow(decodingDst.fDst, decodedRow);
            } else {
                this->applyXformRow(decodingState.fPreblendBuffer, decodedRow);
                blendRow(decodingDst.fDst,
                         decodingState.fPreblendBuffer,
                         this->dstInfo().colorType(),
                         this->dstInfo().alphaType());
            }

            decodingDst.fDst = decodingDst.fDst.subspan(
                    std::min(decodingDst.fDstRowStride, decodingDst.fDst.size()));
            rowsDecoded++;
        }
    }
}

SkCodec::Result SkPngRustCodec::incrementalDecode(DecodingState& decodingState,
                                                  int* rowsDecodedPtr) {
    SkASSERT_RELEASE(this->canReadRow());
    int rowsDecoded = 0;
    const bool interlaced = fReader->interlaced();
    rust::Slice<uint8_t> dstSlice;
    // If we have interlaced rows we have to copy into a temp buffer.
    std::vector<uint8_t> fullWidthRow;
    if (interlaced) {
        fullWidthRow.resize(this->getEncodedRowBytes());
        dstSlice = rust::Slice<uint8_t>(fullWidthRow);
    }
    DecodingDstInfo& decodingDst = decodingState.fDecodingDstInfo;

    while (true) {
        if (!interlaced) {
            dstSlice = decodingState.fPreblendBuffer.empty()
                          ? rust::Slice<uint8_t>(decodingDst.fDst)
                          : rust::Slice<uint8_t>(decodingState.fPreblendBuffer);
        }
        fStreamIsPositionedAtStartOfFrameData = false;

        rust_png::DecodingResult rustResult = fReader->read_row(dstSlice);
        Result result = ToSkCodecResult(rustResult);

        if (result != kSuccess) {
            if (result == kIncompleteInput && rowsDecodedPtr) {
                *rowsDecodedPtr = rowsDecoded;
            }
            return result;
        }

        // No more rows.
        if (rustResult == rust_png::DecodingResult::EndOfFrame) {
            if (interlaced && !decodingState.fPreblendBuffer.empty()) {
                blendAllRows(decodingDst.fDst,
                             decodingState.fPreblendBuffer,
                             decodingDst.fDstRowSize,
                             decodingDst.fDstRowStride,
                             this->dstInfo().colorType(),
                             this->dstInfo().alphaType());
                }

            if (!interlaced) {
                // All of the original `fDst` should be filled out at this point.
                SkASSERT_RELEASE(decodingDst.fDst.empty());
            }
            // `static_cast` is ok, because `startDecoding` already validated `fFrameIndex`.
            fFrameHolder.markFrameAsFullyReceived(static_cast<size_t>(this->options().fFrameIndex));
            return kSuccess;
        }

        // Expand interlaced rows or blend into previous frame if needed.
        if (interlaced) {
            if (decodingState.fPreblendBuffer.empty()) {
                this->expandDecodedInterlacedRow(decodingDst.fDst,
                                                 dstSlice,
                                                 decodingDst,
                                                 /*xFormNeeded=*/false);
            } else {
                this->expandDecodedInterlacedRow(decodingState.fPreblendBuffer,
                                                 dstSlice,
                                                 decodingDst,
                                                 /*xFormNeeded=*/false);
            }
            // `rowsDecoded` is not incremented, because full, contiguous rows
            // are not decoded until pass 6 (or 7 depending on how you look) of
            // Adam7 interlacing scheme.
        } else {
            if (!decodingState.fPreblendBuffer.empty()) {
                blendRow(decodingDst.fDst,
                         decodingState.fPreblendBuffer,
                         this->dstInfo().colorType(),
                         this->dstInfo().alphaType());
            }
            // Increment our pointer to dst memory.
            decodingDst.fDst = decodingDst.fDst.subspan(
                std::min(decodingDst.fDstRowStride, decodingDst.fDst.size()));
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

    if (this->canReadRow()) {
        result = this->incrementalDecode(decodingState, rowsDecoded);
    } else {
        result = this->incrementalDecodeXForm(decodingState, rowsDecoded);
    }
    return result;
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

    // It is okay if `fIncrementalDecodingState` contains state of another,
    // partially decoded frame - in this case we want to clobber
    fIncrementalDecodingState = decodingState;
    return kSuccess;
}

SkCodec::Result SkPngRustCodec::onIncrementalDecode(int* rowsDecoded) {
    if (!fIncrementalDecodingState.has_value()) {
        return kInvalidParameters;
    }

    Result result;
    if (this->canReadRow()) {
        result = this->incrementalDecode(*fIncrementalDecodingState, rowsDecoded);
    } else {
        result = this->incrementalDecodeXForm(*fIncrementalDecodingState, rowsDecoded);
    }
    if (result != kIncompleteInput) {
        // After successfully reading the whole row (`kSuccess`), and after a
        // fatal error (only recoverable error is `kIncompleteInput`) our client
        // should not call `onIncrementalDecode` again.  This means that the
        // incremental decoding state can be discarded at this point.
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
            if (fMaxStreamLengthSeenWhenParsingAdditionalFrameInfos.has_value()) {
                size_t oldLength = *fMaxStreamLengthSeenWhenParsingAdditionalFrameInfos;
                // We use `>=` instead of `==`, because the underlying stream
                // can be "cleared" - see https://crbug.com/431273809#comment4.
                if (oldLength >= currentLength) {
                    // Don't retry `parseAdditionalFrameInfos` if the input
                    // didn't change (or is smaller than last time).
                    break;
                }
            }
            fMaxStreamLengthSeenWhenParsingAdditionalFrameInfos = currentLength;
        }

        switch (this->parseAdditionalFrameInfos()) {
            case kIncompleteInput:
                fCanParseAdditionalFrameInfos = true;
                break;
            case kSuccess:
                SkASSERT_RELEASE(fFrameHolder.size() == this->getRawFrameCount());
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

    SkASSERT_RELEASE(fReader->has_plte_chunk());  // Checked in `startDecoding`.
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
    SkASSERT_RELEASE(fPrivStream);
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
    SkASSERT_RELEASE(SkTFitsIn<int>(fFrames.size()));
    return static_cast<int>(fFrames.size());
}

void SkPngRustCodec::FrameHolder::markFrameAsFullyReceived(size_t index) {
    SkASSERT_RELEASE(index < fFrames.size());
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

    if (reader.has_actl_chunk() && reader.has_fctl_chunk()) {
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

    SkASSERT_RELEASE(!reader.has_actl_chunk() || !reader.has_fctl_chunk());
    SkASSERT_RELEASE(id == 0);
    fFrames.emplace_back(id, info.alpha());
    SkFrame& frame = fFrames.back();
    frame.setXYWH(0, 0, info.width(), info.height());
    frame.setBlend(SkCodecAnimation::Blend::kSrc);
    this->setAlphaAndRequiredFrame(&frame);
    return kSuccess;
}

SkCodec::Result SkPngRustCodec::FrameHolder::setFrameInfoFromCurrentFctlChunk(
        const rust_png::Reader& reader, PngFrame* frame) {
    SkASSERT_RELEASE(reader.has_fctl_chunk());  // Caller should guarantee this
    SkASSERT_RELEASE(frame);

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
