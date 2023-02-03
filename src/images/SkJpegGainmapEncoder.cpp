/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkJpegGainmapEncoder.h"

#ifdef SK_ENCODE_JPEG

#include "include/core/SkBitmap.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkStream.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/private/SkGainmapInfo.h"
#include "src/codec/SkJpegPriv.h"

#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////
// XMP helpers

void xmp_write_per_channel_attr(
        SkDynamicMemoryWStream& s, const char* attrib, SkScalar r, SkScalar g, SkScalar b) {
    s.writeText(attrib);
    s.writeText("=\"");
    if (r == g && r == b) {
        s.writeScalarAsText(r);
    } else {
        s.writeScalarAsText(r);
        s.writeText(",");
        s.writeScalarAsText(g);
        s.writeText(",");
        s.writeScalarAsText(b);
    }
    s.writeText("\"\n");
}

void xmp_write_scalar_attr(SkDynamicMemoryWStream& s, const char* attrib, SkScalar value) {
    s.writeText(attrib);
    s.writeText("=\"");
    s.writeScalarAsText(value);
    s.writeText("\"\n");
}

void xmp_write_decimal_attr(SkDynamicMemoryWStream& s,
                            const char* attrib,
                            int32_t value,
                            bool newLine = true) {
    s.writeText(attrib);
    s.writeText("=\"");
    s.writeDecAsText(value);
    s.writeText("\"");
    if (newLine) {
        s.writeText("\n");
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// JpegR encoding

static float mix(float a, float b, float amount) { return (b - a) * amount + a; }

static float compute_range_scaling_factor(const SkGainmapInfo& info) {
    // Find the minimum and maximum log-ratio values that can be encoded. We don't want to encode a
    // range any larger than this.
    const float gainmapRatioMaxComponent = std::max(
            {info.fGainmapRatioMax.fR, info.fGainmapRatioMax.fG, info.fGainmapRatioMax.fB});
    const float gainmapRatioMinComponent = std::min(
            {info.fGainmapRatioMin.fR, info.fGainmapRatioMin.fG, info.fGainmapRatioMin.fB});
    const float gainmapRSF = std::max(gainmapRatioMaxComponent, 1.f / gainmapRatioMinComponent);

    // Limit the range to only encode values that could reach the the maximum rendering brightness.
    float displayRSF = info.fDisplayRatioHdr;

    return std::min(gainmapRSF, displayRSF);
}

// Ensure that the specified gainmap can be encoded as a JpegR. If it cannot, transform it so that
// it can.
void make_jpegr_compatible_if_needed(SkGainmapInfo& info, SkBitmap& bitmap) {
    // If fGainmapRatioMax == 1/fGainmapRatioMin and bitmap has a single channel then this is
    // already compatible with JpegR.
    if (info.fGainmapRatioMin.fR * info.fGainmapRatioMax.fR == 1.f &&
        info.fGainmapRatioMin.fG * info.fGainmapRatioMax.fG == 1.f &&
        info.fGainmapRatioMin.fB * info.fGainmapRatioMax.fB == 1.f &&
        bitmap.colorType() == kGray_8_SkColorType) {
        return;
    }

    // If not, transform the gainmap to a JpegR compatible format.
    SkGainmapInfo oldInfo = info;
    SkBitmap oldBitmap = bitmap;
    SkBitmap newBitmap;
    SkImageInfo newBitmapInfo =
            SkImageInfo::Make(oldBitmap.dimensions(), kGray_8_SkColorType, kOpaque_SkAlphaType);
    newBitmap.allocPixels(newBitmapInfo);

    // Compute the new gainmap rangeScalingFactor and its log.
    const float rangeScalingFactor = compute_range_scaling_factor(oldInfo);
    const float newLogRatioMax = sk_float_log(rangeScalingFactor);
    const float newLogRatioMin = -newLogRatioMax;

    // Transform the old gainmap to the new range.
    // TODO(ccameron): This is not remotely performant. Consider using a blit.
    {
        const SkColor4f oldLogRatioMin = {sk_float_log(oldInfo.fGainmapRatioMin.fR),
                                          sk_float_log(oldInfo.fGainmapRatioMin.fG),
                                          sk_float_log(oldInfo.fGainmapRatioMin.fB),
                                          1.f};
        const SkColor4f oldLogRatioMax = {sk_float_log(oldInfo.fGainmapRatioMax.fR),
                                          sk_float_log(oldInfo.fGainmapRatioMax.fG),
                                          sk_float_log(oldInfo.fGainmapRatioMax.fB),
                                          1.f};
        const SkColor4f gainmapGamma = oldInfo.fGainmapGamma;
        const auto& newPixmap = newBitmap.pixmap();
        for (int y = 0; y < oldBitmap.height(); ++y) {
            for (int x = 0; x < oldBitmap.width(); ++x) {
                // Convert the gainmap from its encoded value to oldLogRatio, which is log(HDR/SDR).
                SkColor4f oldG = oldBitmap.getColor4f(x, y);
                SkColor4f oldLogRatio = {
                        mix(oldLogRatioMin.fR,
                            oldLogRatioMax.fR,
                            sk_float_pow(oldG.fR, gainmapGamma.fR)),
                        mix(oldLogRatioMin.fG,
                            oldLogRatioMax.fG,
                            sk_float_pow(oldG.fG, gainmapGamma.fG)),
                        mix(oldLogRatioMin.fB,
                            oldLogRatioMax.fB,
                            sk_float_pow(oldG.fB, gainmapGamma.fB)),
                        1.f,
                };

                // Undo the log, computing HDR/SDR, and take the average of the components of this.
                // TODO(ccameron): This assumes that the primaries of the base image are sRGB.
                float averageLinearRatio = 0.2126f * sk_float_exp(oldLogRatio.fR) +
                                           0.7152f * sk_float_exp(oldLogRatio.fG) +
                                           0.0722f * sk_float_exp(oldLogRatio.fB);

                // Compute log(HDR/SDR) for the average HDR/SDR ratio.
                float newLogRatio = sk_float_log(averageLinearRatio);

                // Convert from log(HDR/SDR) to the JpegR gainmap image encoding.
                float newG = (newLogRatio - newLogRatioMin) / (newLogRatioMax - newLogRatioMin);
                *newPixmap.writable_addr8(x, y) =
                        std::min(std::max(sk_float_round(255.f * newG), 0.f), 255.f);
            }
        }
    }

    // Write the gainmap info for the transformed gainmap.
    SkGainmapInfo newInfo;
    const float gainmapRatioMin = 1.f / rangeScalingFactor;
    const float gainmapRatioMax = rangeScalingFactor;
    newInfo.fGainmapRatioMin = {gainmapRatioMin, gainmapRatioMin, gainmapRatioMin, 1.f};
    newInfo.fGainmapRatioMax = {gainmapRatioMax, gainmapRatioMax, gainmapRatioMax, 1.f};
    newInfo.fGainmapGamma = {1.f, 1.f, 1.f, 1.f};
    newInfo.fEpsilonSdr = {0.f, 0.f, 0.f, 1.f};
    newInfo.fEpsilonHdr = {0.f, 0.f, 0.f, 1.f};
    newInfo.fDisplayRatioSdr = 1.f;
    newInfo.fDisplayRatioHdr = rangeScalingFactor;
    newInfo.fType = SkGainmapInfo::Type::kJpegR_Linear;
    info = newInfo;
    bitmap = newBitmap;
}

// Generate the XMP metadata for a JpegR file.
sk_sp<SkData> get_jpegr_xmp_data(float rangeScalingFactor,
                                 int32_t transferFunction,
                                 int32_t itemLength) {
    SkDynamicMemoryWStream s;
    s.write(kXMPStandardSig, sizeof(kXMPStandardSig));
    s.writeText(
            "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"Adobe XMP Core 5.1.2\">\n"
            "<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
            "<rdf:Description xmlns:GContainer=\"http://ns.google.com/photos/1.0/container/\" "
            "xmlns:RecoveryMap=\"http://ns.google.com/photos/1.0/recoverymap/\">\n"
            "<GContainer:Version>1</GContainer:Version>\n"
            "<GContainer:Directory>\n"
            "<rdf:Seq>\n"
            "<rdf:li>\n"
            "<GContainer:Item GContainer:ItemSemantic=\"Primary\"\n"
            "GContainer:ItemMime=\"image/jpeg\"\n");
    xmp_write_decimal_attr(s, "RecoveryMap:Version", 1);
    xmp_write_scalar_attr(s, "RecoveryMap:RangeScalingFactor", rangeScalingFactor);
    xmp_write_decimal_attr(s, "RecoveryMap:TransferFunction", transferFunction, /*newLine=*/false);
    s.writeText("/>\n");
    s.writeText(
            "</rdf:li>\n"
            "<rdf:li>\n"
            "<GContainer:Item GContainer:ItemSemantic=\"RecoveryMap\"\n"
            "GContainer:ItemMime=\"image/jpeg\"\n");
    xmp_write_decimal_attr(s, "GContainer:ItemLength", itemLength, /*newLine=*/false);
    s.writeText("/>\n");
    s.writeText(
            "</rdf:li>\n"
            "</rdf:Seq>\n"
            "</GContainer:Directory>\n"
            "</rdf:Description>\n"
            "</rdf:RDF>\n"
            "</x:xmpmeta>\n");
    return s.detachAsData();
}

bool SkJpegGainmapEncoder::EncodeJpegR(SkWStream* dst,
                                       const SkPixmap& base,
                                       const SkJpegEncoder::Options& baseOptions,
                                       const SkPixmap& gainmap,
                                       const SkJpegEncoder::Options& gainmapOptions,
                                       const SkGainmapInfo& gainmapInfo) {
    // Transform the gainmap to be compatible with JpegR, if needed.
    SkBitmap gainmapJpegR;
    gainmapJpegR.installPixels(gainmap);
    SkGainmapInfo gainmapInfoJpegR = gainmapInfo;
    make_jpegr_compatible_if_needed(gainmapInfoJpegR, gainmapJpegR);

    // Encode the gainmap as a Jpeg.
    SkDynamicMemoryWStream gainmapEncodeStream;
    if (!SkJpegEncoder::Encode(&gainmapEncodeStream, gainmapJpegR.pixmap(), gainmapOptions)) {
        return false;
    }
    sk_sp<SkData> gainmapEncoded = gainmapEncodeStream.detachAsData();

    // Compute the XMP metadata.
    sk_sp<SkData> xmpMetadata =
            get_jpegr_xmp_data(gainmapInfoJpegR.fDisplayRatioHdr, 0, gainmapEncoded->size());

    // Send this to the base image encoder.
    uint8_t segmentMarker = kXMPMarker;
    SkData* segmentData = xmpMetadata.get();
    auto encoder = SkJpegEncoder::Make(
            dst, base, baseOptions, 1, &segmentMarker, &segmentData, gainmapEncoded.get());
    return encoder.get() && encoder->encodeRows(base.height());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// HDRGM encoding

// Generate the XMP metadata for an HDRGM file.
sk_sp<SkData> get_hdrgm_xmp_data(const SkGainmapInfo& gainmapInfo) {
    const float kLog2 = sk_float_log(2.f);
    SkDynamicMemoryWStream s;
    s.write(kXMPStandardSig, sizeof(kXMPStandardSig));
    s.writeText(
            "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"XMP Core 5.5.0\">\n"
            "<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
            "<rdf:Description rdf:about=\"\"\n"
            "xmlns:hdrgm=\"http://ns.adobe.com/hdr-gain-map/1.0/\"\n"
            "hdrgm:Version=\"1.0\"\n");
    xmp_write_per_channel_attr(s,
                               "hdrgm:GainMapMin",
                               sk_float_log(gainmapInfo.fGainmapRatioMin.fR) / kLog2,
                               sk_float_log(gainmapInfo.fGainmapRatioMin.fG) / kLog2,
                               sk_float_log(gainmapInfo.fGainmapRatioMin.fB) / kLog2);
    xmp_write_per_channel_attr(s,
                               "hdrgm:GainMapMax",
                               sk_float_log(gainmapInfo.fGainmapRatioMax.fR) / kLog2,
                               sk_float_log(gainmapInfo.fGainmapRatioMax.fG) / kLog2,
                               sk_float_log(gainmapInfo.fGainmapRatioMax.fB) / kLog2);
    xmp_write_per_channel_attr(s,
                               "hdrgm:Gamma",
                               gainmapInfo.fGainmapGamma.fR,
                               gainmapInfo.fGainmapGamma.fG,
                               gainmapInfo.fGainmapGamma.fB);
    xmp_write_per_channel_attr(s,
                               "hdrgm:OffsetSDR",
                               gainmapInfo.fEpsilonSdr.fR,
                               gainmapInfo.fEpsilonSdr.fG,
                               gainmapInfo.fEpsilonSdr.fB);
    xmp_write_per_channel_attr(s,
                               "hdrgm:OffsetHDR",
                               gainmapInfo.fEpsilonHdr.fR,
                               gainmapInfo.fEpsilonHdr.fG,
                               gainmapInfo.fEpsilonHdr.fB);
    xmp_write_scalar_attr(
            s, "hdrgm:HDRCapacityMin", sk_float_log(gainmapInfo.fDisplayRatioSdr) / kLog2);
    xmp_write_scalar_attr(
            s, "hdrgm:HDRCapacityMax", sk_float_log(gainmapInfo.fDisplayRatioHdr) / kLog2);
    s.writeText("hdrgm:BaseRendition=\"");
    switch (gainmapInfo.fBaseImageType) {
        case SkGainmapInfo::BaseImageType::kSDR:
            s.writeText("SDR");
            break;
        case SkGainmapInfo::BaseImageType::kHDR:
            s.writeText("HDR");
            break;
    }
    s.writeText(
            "\"/>\n"
            "</rdf:RDF>\n"
            "</x:xmpmeta>");
    return s.detachAsData();
}

// Split an SkData into segments.
std::vector<sk_sp<SkData>> get_hdrgm_image_segments(sk_sp<SkData> image,
                                                    size_t segmentMaxDataSize) {
    // Compute the total size of the header to a gainmap image segment (not including the 2 bytes
    // for the segment size, which the encoder is responsible for writing).
    constexpr size_t kGainmapHeaderSize = sizeof(kGainmapSig) + 2 * kGainmapMarkerIndexSize;

    // Compute the payload size for each segment.
    const size_t kGainmapPayloadSize = segmentMaxDataSize - kGainmapHeaderSize;

    // Compute the number of segments we'll need.
    const size_t segmentCount = (image->size() + kGainmapPayloadSize - 1) / kGainmapPayloadSize;
    std::vector<sk_sp<SkData>> result;
    result.reserve(segmentCount);

    // Move |imageData| through |image| until it hits |imageDataEnd|.
    const uint8_t* imageData = image->bytes();
    const uint8_t* imageDataEnd = image->bytes() + image->size();
    while (imageData < imageDataEnd) {
        SkDynamicMemoryWStream segmentStream;

        // Write the signature.
        segmentStream.write(kGainmapSig, sizeof(kGainmapSig));

        // Write the segment index as big-endian.
        size_t segmentIndex = result.size() + 1;
        uint8_t segmentIndexBytes[2] = {
                static_cast<uint8_t>(segmentIndex / 256u),
                static_cast<uint8_t>(segmentIndex % 256u),
        };
        segmentStream.write(segmentIndexBytes, sizeof(segmentIndexBytes));

        // Write the segment count as big-endian.
        uint8_t segmentCountBytes[2] = {
                static_cast<uint8_t>(segmentCount / 256u),
                static_cast<uint8_t>(segmentCount % 256u),
        };
        segmentStream.write(segmentCountBytes, sizeof(segmentCountBytes));

        // Verify that our header size math is correct.
        SkASSERT(segmentStream.bytesWritten() == kGainmapHeaderSize);

        // Write the rest of the segment.
        size_t bytesToWrite =
                std::min(imageDataEnd - imageData, static_cast<intptr_t>(kGainmapPayloadSize));
        segmentStream.write(imageData, bytesToWrite);
        imageData += bytesToWrite;

        // Verify that our data size math is correct.
        if (segmentIndex == segmentCount) {
            SkASSERT(segmentStream.bytesWritten() <= segmentMaxDataSize);
        } else {
            SkASSERT(segmentStream.bytesWritten() == segmentMaxDataSize);
        }
        result.push_back(segmentStream.detachAsData());
    }

    // Verify that our segment count math was correct.
    SkASSERT(imageData == imageDataEnd);
    SkASSERT(result.size() == segmentCount);
    return result;
}

bool SkJpegGainmapEncoder::EncodeHDRGM(SkWStream* dst,
                                       const SkPixmap& base,
                                       const SkJpegEncoder::Options& baseOptions,
                                       const SkPixmap& gainmap,
                                       const SkJpegEncoder::Options& gainmapOptions,
                                       const SkGainmapInfo& gainmapInfo) {
    // Encode the gainmap as a Jpeg, and split it into segments.
    SkDynamicMemoryWStream gainmapEncodeStream;
    if (!SkJpegEncoder::Encode(&gainmapEncodeStream, gainmap, gainmapOptions)) {
        return false;
    }
    std::vector<sk_sp<SkData>> gainmapSegments = get_hdrgm_image_segments(
            gainmapEncodeStream.detachAsData(), SkJpegEncoder::kSegmentDataMaxSize);

    // Compute the XMP metadata.
    sk_sp<SkData> xmpMetadata = get_hdrgm_xmp_data(gainmapInfo);

    // Merge these into the list of segments to send to the encoder.
    std::vector<uint8_t> segmentMarker;
    std::vector<SkData*> segmentData;
    segmentMarker.push_back(kXMPMarker);
    segmentData.push_back(xmpMetadata.get());
    for (auto& gainmapSegment : gainmapSegments) {
        segmentMarker.push_back(kGainmapMarker);
        segmentData.push_back(gainmapSegment.get());
    }
    SkASSERT(segmentMarker.size() == segmentData.size());

    // Send this to the base image encoder.
    auto encoder = SkJpegEncoder::Make(dst,
                                       base,
                                       baseOptions,
                                       segmentMarker.size(),
                                       segmentMarker.data(),
                                       segmentData.data(),
                                       nullptr);
    return encoder.get() && encoder->encodeRows(base.height());
}

#endif  // SK_ENCODE_JPEG
