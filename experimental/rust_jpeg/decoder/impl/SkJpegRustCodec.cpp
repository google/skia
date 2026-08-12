/*
 * Copyright 2026 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_jpeg/decoder/impl/SkJpegRustCodec.h"

#include <algorithm>

#include "include/codec/SkCodecAnimation.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/private/SkAssert.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/SkGainmapInfo.h"
#include "include/private/SkJpegMetadataDecoder.h"
#include "include/private/SkTemplates.h"
#include "rust/common/SkStreamAdapter.h"
#include "rust/common/SpanUtils.h"
#include "src/codec/SkSwizzler.h"
#include "src/core/SkSafeMath.h"
#include "src/core/SkStreamPriv.h"

#include "include/private/SkExif.h"

#ifdef SK_CODEC_DECODES_JPEG_GAINMAPS
#include "include/core/SkColorSpace.h"
#include "include/private/SkXmp.h"
#include "modules/skcms/skcms.h"
#include "src/codec/SkJpegConstants.h"
#include "src/codec/SkJpegXmp.h"
#endif  // SK_CODEC_DECODES_JPEG_GAINMAPS

static constexpr SkColorType kXformSrcColorType = kRGBA_8888_SkColorType;

// Static assertions to validate that Rust JpegColor/JpegAlpha enum values
// match the corresponding SkEncodedInfo enum values.
static_assert(static_cast<int>(rust_jpeg::JpegColor::RGB) ==
              static_cast<int>(SkEncodedInfo::kRGB_Color),
              "JpegColor::RGB must match SkEncodedInfo::kRGB_Color");
static_assert(static_cast<int>(rust_jpeg::JpegColor::Grayscale) ==
              static_cast<int>(SkEncodedInfo::kGray_Color),
              "JpegColor::Grayscale must match SkEncodedInfo::kGray_Color");
static_assert(static_cast<int>(rust_jpeg::JpegColor::InvertedCMYK) ==
              static_cast<int>(SkEncodedInfo::kInvertedCMYK_Color),
              "JpegColor::InvertedCMYK must match SkEncodedInfo::kInvertedCMYK_Color");

static_assert(static_cast<int>(rust_jpeg::JpegAlpha::Opaque) ==
              static_cast<int>(SkEncodedInfo::kOpaque_Alpha),
              "JpegAlpha::Opaque must match SkEncodedInfo::kOpaque_Alpha");

namespace {

SkCodec::Result MapDecodingResult(rust_jpeg::DecodingResult rustResult) {
    switch (rustResult) {
        case rust_jpeg::DecodingResult::Success:
            return SkCodec::kSuccess;
        case rust_jpeg::DecodingResult::FormatError:
            return SkCodec::kErrorInInput;
        case rust_jpeg::DecodingResult::ParameterError:
            return SkCodec::kInvalidParameters;
        case rust_jpeg::DecodingResult::UnsupportedFeature:
            return SkCodec::kUnimplemented;
        case rust_jpeg::DecodingResult::IncompleteInput:
            return SkCodec::kIncompleteInput;
        case rust_jpeg::DecodingResult::MemoryError:
            return SkCodec::kInternalError;
        case rust_jpeg::DecodingResult::OtherError:
            return SkCodec::kErrorInInput;
    }
    SK_ABORT("Unexpected `rust_jpeg::DecodingResult`: %d", static_cast<int>(rustResult));
}

}  // namespace

std::unique_ptr<SkJpegRustCodec> SkJpegRustCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                                 Result* result) {
    Result resultStorage;
    if (!result) {
        result = &resultStorage;
    }

    if (!stream) {
        *result = kInvalidInput;
        return nullptr;
    }

    auto inputAdapter = std::make_unique<rust::stream::SkStreamAdapter>(stream.get());
    rust::Box<rust_jpeg::Reader> reader = rust_jpeg::new_reader(std::move(inputAdapter));

    if (!reader->metadata_loaded()) {
        rust_jpeg::DecodingResult metadataResult = reader->read_metadata();

        if (metadataResult != rust_jpeg::DecodingResult::Success) {
            *result = MapDecodingResult(metadataResult);
            return nullptr;
        }

        SkASSERT_RELEASE(reader->metadata_loaded());
    }

    uint32_t width = reader->width();
    uint32_t height = reader->height();
    rust_jpeg::JpegColor rustColor = reader->color();
    rust_jpeg::JpegAlpha rustAlpha = reader->alpha();

    SkEncodedInfo::Color color = static_cast<SkEncodedInfo::Color>(rustColor);
    SkEncodedInfo::Alpha alpha = static_cast<SkEncodedInfo::Alpha>(rustAlpha);
    constexpr int kBitsPerComponent = 8;

    std::unique_ptr<SkCodecs::ColorProfile> colorProfile;
    rust::Vec<uint8_t> profileData = reader->icc_profile();
    if (!profileData.empty()) {
        sk_sp<SkData> iccData = SkData::MakeWithCopy(profileData.data(), profileData.size());
        colorProfile = SkCodecs::ColorProfile::MakeICCProfile(std::move(iccData));
    }
    // Validate ICC profile data space against the image color space,
    // matching the existing C++ JPEG codec's behavior.  Drop profiles
    // whose data space is incompatible.
    if (colorProfile) {
        auto dataSpace = colorProfile->dataSpace();
        switch (color) {
            case SkEncodedInfo::kInvertedCMYK_Color:
                if (dataSpace != SkCodecs::ColorProfile::DataSpace::kCMYK) {
                    colorProfile = nullptr;
                }
                break;
            case SkEncodedInfo::kGray_Color:
                if (dataSpace != SkCodecs::ColorProfile::DataSpace::kGray &&
                    dataSpace != SkCodecs::ColorProfile::DataSpace::kRGB) {
                    colorProfile = nullptr;
                }
                break;
            default:
                if (dataSpace != SkCodecs::ColorProfile::DataSpace::kRGB) {
                    colorProfile = nullptr;
                }
                break;
        }
    }

    // Parse orientation from EXIF data using SkExif::Parse (shared with C++ codec).
    SkEncodedOrigin orientation = kDefault_SkEncodedOrigin;
    {
        rust::Vec<uint8_t> exifBytes = reader->exif_data();
        if (!exifBytes.empty()) {
            auto exifData = SkData::MakeWithCopy(exifBytes.data(), exifBytes.size());
            SkExif::Metadata exif;
            SkExif::Parse(exif, exifData.get());
            if (exif.fOrigin.has_value()) {
                orientation = exif.fOrigin.value();
            }
        }
    }

    SkEncodedInfo encodedInfo = SkEncodedInfo::Make(
        width,
        height,
        color,
        alpha,
        kBitsPerComponent,
        std::move(colorProfile)
    );

    // Guard against overflow: reject images whose raw pixel buffer would
    // exceed size_t range.
    size_t bytesPerPixel;
    switch (color) {
        case SkEncodedInfo::kRGB_Color:
            bytesPerPixel = 3;
            break;
        case SkEncodedInfo::kGray_Color:
            bytesPerPixel = 1;
            break;
        case SkEncodedInfo::kInvertedCMYK_Color:
            bytesPerPixel = 4;
            break;
        default:
            *result = kInvalidInput;
            return nullptr;
    }
    SkSafeMath safe;
    size_t srcRowBytes = safe.mul(safe.castTo<size_t>(width), bytesPerPixel);
    (void)safe.mul(safe.castTo<size_t>(height), srcRowBytes);
    if (!safe.ok()) {
        *result = kInternalError;
        return nullptr;
    }

    *result = kSuccess;
    return std::unique_ptr<SkJpegRustCodec>(new SkJpegRustCodec(
        std::move(encodedInfo),
        std::move(stream),
        std::move(reader),
        orientation
    ));
}

SkJpegRustCodec::SkJpegRustCodec(SkEncodedInfo&& encodedInfo,
                                 std::unique_ptr<SkStream> stream,
                                 rust::Box<rust_jpeg::Reader> reader,
                                 SkEncodedOrigin origin)
    : SkCodec(std::move(encodedInfo), skcms_PixelFormat_RGBA_8888,
              /* stream = */ nullptr, origin)
    , fPrivStream(std::move(stream))
    , fReader(std::move(reader)) {
    SkASSERT_RELEASE(fPrivStream);
}

SkJpegRustCodec::~SkJpegRustCodec() = default;

sk_sp<const SkData> SkJpegRustCodec::getEncodedData() const {
    SkASSERT_RELEASE(fPrivStream);
    sk_sp<const SkData> data = fPrivStream->getData();
    if (data) {
        return data;
    }
    auto dStream = fPrivStream->duplicate();
    if (!dStream || !dStream->hasLength()) {
        return nullptr;
    }
    return SkData::MakeFromStream(dStream.get(), dStream->getLength());
}

bool SkJpegRustCodec::onGetFrameInfo(int index, FrameInfo* info) const {
    if (index != 0) {
        return false;
    }
    if (info) {
        info->fRequiredFrame = SkCodec::kNoFrame;
        info->fDuration = 0;
        info->fFullyReceived = fReader->image_data_loaded();
        info->fAlphaType = this->getInfo().alphaType();
        info->fHasAlphaWithinBounds = false;  // JPEG never has alpha
        info->fDisposalMethod = SkCodecAnimation::DisposalMethod::kKeep;
        info->fBlend = SkCodecAnimation::Blend::kSrc;
        info->fFrameRect = SkIRect::MakeSize(this->dimensions());
    }
    return true;
}

bool SkJpegRustCodec::onGetGainmapInfo(SkGainmapInfo* info,
                                       std::unique_ptr<SkStream>* gainmapImageStream) {
#ifndef SK_CODEC_DECODES_JPEG_GAINMAPS
    return false;
#else
    // Segment scanning and MPF TIFF/IFD parsing happen in Rust. Higher-level
    // gainmap detection (XMP, ISO 21496-1) uses C++ SkJpegMetadataDecoder.
    if (!fReader->segments_scanned()) {
        return false;
    }
    if (!fReader->has_mpf()) {
        return false;
    }

    // Two lambdas needed: CXX exposes Reader and EmbeddedJpegScanner methods
    // with different prefixes in a flat namespace.
    auto buildSegmentsFromReader = [](const rust_jpeg::Reader& reader)
            -> std::vector<SkJpegMetadataDecoder::Segment> {
        std::vector<SkJpegMetadataDecoder::Segment> segs;
        const uint32_t count = reader.segment_count();
        segs.reserve(count);
        for (uint32_t i = 0; i < count; ++i) {
            rust_jpeg::JpegSegmentInfo si = reader.segment_info(i);
            if (si.parameter_length <= 2) continue;
            rust::Vec<uint8_t> pd = reader.segment_data(i);
            if (pd.empty()) continue;
            segs.emplace_back(si.marker, SkData::MakeWithCopy(pd.data(), pd.size()));
        }
        return segs;
    };

    auto buildSegmentsFromEmbedded = [](const rust_jpeg::EmbeddedJpegScanner& scanner)
            -> std::vector<SkJpegMetadataDecoder::Segment> {
        std::vector<SkJpegMetadataDecoder::Segment> segs;
        const uint32_t count = scanner.embedded_segment_count();
        segs.reserve(count);
        for (uint32_t i = 0; i < count; ++i) {
            rust_jpeg::JpegSegmentInfo si = scanner.embedded_segment_info(i);
            if (si.parameter_length <= 2) continue;
            rust::Vec<uint8_t> pd = scanner.embedded_segment_data(i);
            if (pd.empty()) continue;
            segs.emplace_back(si.marker, SkData::MakeWithCopy(pd.data(), pd.size()));
        }
        return segs;
    };

    auto baseMetadata = SkJpegMetadataDecoder::Make(buildSegmentsFromReader(*fReader));
    if (!baseMetadata) {
        return false;
    }

    SkExif::Metadata baseExif;
    SkExif::Parse(baseExif, baseMetadata->getExifMetadata(/*copyData=*/false).get());

    bool isoGainmapPresent = SkGainmapInfo::ParseVersion(
            baseMetadata->getISOGainmapMetadata(/*copyData=*/false).get());

    std::vector<sk_sp<const SkData>> baseApp1Params;
    {
        const uint32_t segCount = fReader->segment_count();
        for (uint32_t i = 0; i < segCount; ++i) {
            rust_jpeg::JpegSegmentInfo si = fReader->segment_info(i);
            if (si.marker == kXMPMarker && si.parameter_length > 2) {
                rust::Vec<uint8_t> pd = fReader->segment_data(i);
                if (!pd.empty()) {
                    baseApp1Params.push_back(SkData::MakeWithCopy(pd.data(), pd.size()));
                }
            }
        }
    }
    auto baseXmp = SkJpegMakeXmp(baseApp1Params);

    bool adobeGainmapPresent = baseXmp && baseXmp->getGainmapInfoAdobe(nullptr);

    sk_sp<const SkData> encodedData = this->getEncodedData();
    if (!encodedData) {
        return false;
    }

    const uint32_t mpfSegOff = fReader->mpf_segment_offset();
    const uint32_t mpfCount = fReader->mpf_image_count();

    // MPF entries are zero-based. Entry 0 is the primary image, so inspect
    // secondary images in [1, mpfCount).
    for (uint32_t mpIdx = 1; mpIdx < mpfCount; ++mpIdx) {
        rust_jpeg::MpfImageEntry entry = fReader->mpf_image_entry(mpIdx);

        // 2 (marker) + 2 (param length) + 4 (MPF signature).
        constexpr size_t kMpfHeaderSize = 8;
        SkSafeMath safe;
        size_t imageOffset = safe.add(safe.add(static_cast<size_t>(mpfSegOff), kMpfHeaderSize),
                                      static_cast<size_t>(entry.data_offset));
        size_t imageSize = entry.size;
        size_t imageEnd = safe.add(imageOffset, imageSize);
        if (!safe.ok() || imageEnd > encodedData->size()) {
            continue;
        }

        rust::Slice<const uint8_t> embSlice(
                encodedData->bytes() + imageOffset, imageSize);
        auto embScanner = rust_jpeg::scan_embedded_jpeg(embSlice);
        if (embScanner->embedded_had_error()) {
            continue;
        }

        auto embMetadata = SkJpegMetadataDecoder::Make(buildSegmentsFromEmbedded(*embScanner));
        if (!embMetadata) {
            continue;
        }

        SkGainmapInfo gainmapInfo;
        bool foundGainmap = false;

        if (isoGainmapPresent) {
            foundGainmap = SkGainmapInfo::Parse(
                    embMetadata->getISOGainmapMetadata(/*copyData=*/false).get(), gainmapInfo);
            if (foundGainmap && gainmapInfo.fGainmapMathColorSpace) {
                // Use the gain map image's ICC color space for gainmap math.
                sk_sp<SkColorSpace> imageColorSpace;
                auto iccData = embMetadata->getICCProfileData(/*copyData=*/false);
                skcms_ICCProfile iccProfile;
                if (iccData && skcms_Parse(iccData->data(), iccData->size(), &iccProfile)) {
                    imageColorSpace = SkColorSpace::Make(iccProfile);
                }
                gainmapInfo.fGainmapMathColorSpace = std::move(imageColorSpace);
            }
        }

        if (!foundGainmap) {
            std::vector<sk_sp<const SkData>> embApp1Params;
            const uint32_t embSegCount = embScanner->embedded_segment_count();
            for (uint32_t j = 0; j < embSegCount; ++j) {
                rust_jpeg::JpegSegmentInfo si = embScanner->embedded_segment_info(j);
                if (si.marker == kXMPMarker && si.parameter_length > 2) {
                    rust::Vec<uint8_t> pd = embScanner->embedded_segment_data(j);
                    if (!pd.empty()) {
                        embApp1Params.push_back(
                                SkData::MakeWithCopy(pd.data(), pd.size()));
                    }
                }
            }
            auto embXmp = SkJpegMakeXmp(embApp1Params);

            if (embXmp) {
                if (adobeGainmapPresent) {
                    foundGainmap = embXmp->getGainmapInfoAdobe(&gainmapInfo);
                }
                if (!foundGainmap && baseExif.fHdrHeadroom.has_value()) {
                    foundGainmap = embXmp->getGainmapInfoApple(
                            baseExif.fHdrHeadroom.value(), &gainmapInfo);
                }
            }
        }

        if (foundGainmap) {
            *info = gainmapInfo;
            *gainmapImageStream = SkMemoryStream::Make(
                    SkData::MakeWithCopy(encodedData->bytes() + imageOffset, imageSize));
            return true;
        }
    }

    return false;
#endif  // SK_CODEC_DECODES_JPEG_GAINMAPS
}

bool SkJpegRustCodec::onGetGainmapCodec(SkGainmapInfo* info,
                                        std::unique_ptr<SkCodec>* gainmapCodec) {
    std::unique_ptr<SkStream> stream;
    if (!this->onGetGainmapInfo(info, &stream)) {
        return false;
    }
    if (gainmapCodec) {
        Result result;
        *gainmapCodec = SkCodec::MakeFromStream(std::move(stream), &result);
        if (!*gainmapCodec) {
            return false;
        }
    }
    return true;
}

bool SkJpegRustCodec::onRewind() {
    fIncrementalDecodingState.reset();

    if (!fPrivStream->rewind()) {
        return false;
    }

    auto inputAdapter = std::make_unique<rust::stream::SkStreamAdapter>(fPrivStream.get());
    fReader = rust_jpeg::new_reader(std::move(inputAdapter));

    if (!fReader->metadata_loaded()) {
        rust_jpeg::DecodingResult metadataResult = fReader->read_metadata();
        if (metadataResult != rust_jpeg::DecodingResult::Success) {
            return false;
        }
    }

    return true;
}

SkCodec::Result SkJpegRustCodec::initializeSwizzler(const SkImageInfo& dstInfo,
                                                    const Options& opts) {
    SkImageInfo swizzlerInfo = dstInfo;
    SkCodec::Options swizzlerOptions = opts;

    if (this->xformOnDecode()) {
        fXformBuffer.reset(new uint32_t[dstInfo.width()]);
        swizzlerInfo = swizzlerInfo.makeColorType(kXformSrcColorType);
        if (kPremul_SkAlphaType == dstInfo.alphaType()) {
            swizzlerInfo = swizzlerInfo.makeAlphaType(kUnpremul_SkAlphaType);
        }
        swizzlerOptions.fZeroInitialized = kNo_ZeroInitialized;
    }

    fSwizzler = SkSwizzler::Make(this->getEncodedInfo(), nullptr, swizzlerInfo, swizzlerOptions);
    if (!fSwizzler) {
        return kInvalidConversion;
    }
    return kSuccess;
}

void SkJpegRustCodec::swizzleRow(const uint8_t* srcRow, void* dstRow) {
    if (this->xformOnDecode()) {
        const int32_t width = fSwizzler->swizzleWidth();
        fSwizzler->swizzle(fXformBuffer.get(), srcRow);
        this->applyColorXform(dstRow, fXformBuffer.get(), width);
    } else {
        fSwizzler->swizzle(dstRow, srcRow);
    }
}

SkCodec::Result SkJpegRustCodec::onGetPixels(const SkImageInfo& info,
                                             void* dst,
                                             size_t dstRowStride,
                                             const Options& options,
                                             int* rowsDecoded) {
    fReader->reset_decode_state();

    Result result = this->initializeSwizzler(info, options);
    if (result != kSuccess) {
        return result;
    }

    int decodedRows = 0;
    result = this->performFullDecode(info, dst, dstRowStride, &decodedRows);
    if (rowsDecoded) {
        *rowsDecoded = decodedRows;
    }
    return result;
}

SkCodec::Result SkJpegRustCodec::performFullDecode(const SkImageInfo& dstInfo,
                                                   void* dst,
                                                   size_t dstRowStride,
                                                   int* rowsDecoded) {
    rust_jpeg::DecodingResult readResult = fReader->read_image_data();
    if (readResult != rust_jpeg::DecodingResult::Success) {
        return MapDecodingResult(readResult);
    }

    const size_t srcRowBytes = fReader->row_bytes();

    rust::Slice<const uint8_t> imageData;
    rust_jpeg::DecodedRowsInfo rowsInfo = fReader->get_next_rows(imageData);

    if (imageData.empty()) {
        return kErrorInInput;
    }

    SkSpan<const uint8_t> srcImage(imageData.data(), imageData.size());
    SkSafeMath safe;
    size_t totalDstSize = safe.mul(safe.castTo<size_t>(dstInfo.height()), dstRowStride);
    if (!safe.ok()) {
        return kInternalError;
    }
    SkSpan<uint8_t> dstImage(static_cast<uint8_t*>(dst), totalDstSize);
    const uint32_t rowCount = rowsInfo.row_count;

    for (uint32_t y = 0; y < rowCount; ++y) {
        SkSpan<const uint8_t> srcRow = srcImage.subspan(y * srcRowBytes, srcRowBytes);
        SkSpan<uint8_t> dstRow = dstImage.subspan(y * dstRowStride, dstRowStride);
        this->swizzleRow(srcRow.data(), dstRow.data());
    }

    if (rowsDecoded) {
        *rowsDecoded = static_cast<int>(rowCount);
    }

    return (rowCount == static_cast<uint32_t>(this->dimensions().height())) ? kSuccess
                                                                            : kIncompleteInput;
}

SkCodec::Result SkJpegRustCodec::onStartIncrementalDecode(const SkImageInfo& dstInfo,
                                                          void* dst,
                                                          size_t dstRowBytes,
                                                          const Options& options) {
    if (dstRowBytes < dstInfo.minRowBytes()) {
        return kInvalidParameters;
    }

    if (options.fSubset) {
        return kInvalidParameters;
    }

    Result result = this->initializeSwizzler(dstInfo, options);
    if (result != kSuccess) {
        return result;
    }

    fReader->reset_decode_state();

    DecodingState state;

    const int32_t height = dstInfo.height();
    SkSafeMath safe;
    size_t totalDstSize = safe.mul(safe.castTo<size_t>(height), dstRowBytes);
    if (!safe.ok()) {
        return kInternalError;
    }

    state.fDst = SkSpan(static_cast<uint8_t*>(dst), totalDstSize);
    state.fDstRowStride = dstRowBytes;

    fIncrementalDecodingState = std::move(state);

    return kSuccess;
}

SkCodec::Result SkJpegRustCodec::onIncrementalDecode(int* rowsDecoded) {
    if (!fIncrementalDecodingState.has_value()) {
        return kInvalidParameters;
    }

    Result result = this->incrementalDecode(*fIncrementalDecodingState, rowsDecoded);

    if (result != kIncompleteInput) {
        fIncrementalDecodingState.reset();
    }

    return result;
}

SkCodec::Result SkJpegRustCodec::incrementalDecode(DecodingState& state, int* rowsDecodedPtr) {
    const size_t srcRowBytes = fReader->row_bytes();

    rust_jpeg::DecodingResult readResult = fReader->read_incremental_image_data();

    if (readResult != rust_jpeg::DecodingResult::Success &&
        readResult != rust_jpeg::DecodingResult::IncompleteInput) {
        return MapDecodingResult(readResult);
    }

    rust::Slice<const uint8_t> imageData;
    rust_jpeg::DecodedRowsInfo rowsInfo = fReader->get_next_rows(imageData);
    const uint32_t dstRowStart = rowsInfo.dst_row_start;
    const uint32_t rowCount = rowsInfo.row_count;
    if (rowsInfo.is_preview) {
        // Progressive previews replace the initialized frame from row zero.
        // SkCodec permits rowsDecoded to include initialized but unfinished rows.
        if (dstRowStart != 0) {
            return kErrorInInput;
        }
    }

    uint32_t copiedRows = 0;
    if (rowCount > 0) {
        const uint32_t height = static_cast<uint32_t>(this->dimensions().height());
        if (srcRowBytes == 0 || imageData.empty() || dstRowStart >= height ||
            rowCount > height - dstRowStart) {
            return kErrorInInput;
        }

        SkSafeMath safe;
        const size_t requiredSrcBytes = safe.mul(safe.castTo<size_t>(rowCount), srcRowBytes);
        if (!safe.ok() || imageData.size() < requiredSrcBytes) {
            return kErrorInInput;
        }

        SkSpan<const uint8_t> srcImage(imageData.data(), imageData.size());

        for (uint32_t i = 0; i < rowCount; ++i) {
            const uint32_t dstY = dstRowStart + i;
            const size_t srcOffset = safe.mul(safe.castTo<size_t>(i), srcRowBytes);
            const size_t dstOffset = safe.mul(safe.castTo<size_t>(dstY), state.fDstRowStride);
            if (!safe.ok()) {
                return kInternalError;
            }
            SkSpan<const uint8_t> srcRow = srcImage.subspan(srcOffset, srcRowBytes);
            SkSpan<uint8_t> dstRow = state.fDst.subspan(dstOffset, state.fDstRowStride);
            this->swizzleRow(srcRow.data(), dstRow.data());
            copiedRows++;
        }
        state.fTotalRowsInitialized = std::max(
            state.fTotalRowsInitialized, static_cast<int>(dstRowStart + copiedRows));
    }

    if (rowsDecodedPtr) {
        *rowsDecodedPtr = state.fTotalRowsInitialized;
    }

    return readResult == rust_jpeg::DecodingResult::Success &&
               state.fTotalRowsInitialized >= this->dimensions().height()
           ? kSuccess
           : kIncompleteInput;
}
