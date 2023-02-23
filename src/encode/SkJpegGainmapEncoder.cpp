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
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkJpegConstants.h"
#include "src/codec/SkJpegMultiPicture.h"
#include "src/codec/SkJpegPriv.h"
#include "src/codec/SkJpegSegmentScan.h"

#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////
// XMP helpers

void xmp_write_prefix(SkDynamicMemoryWStream& s, const std::string& ns, const std::string& attrib) {
    s.writeText(ns.c_str());
    s.writeText(":");
    s.writeText(attrib.c_str());
    s.writeText("=\"");
}

void xmp_write_suffix(SkDynamicMemoryWStream& s, bool newLine) {
    s.writeText("\"");
    if (newLine) {
        s.writeText("\n");
    }
}

void xmp_write_per_channel_attr(SkDynamicMemoryWStream& s,
                                const std::string& ns,
                                const std::string& attrib,
                                SkScalar r,
                                SkScalar g,
                                SkScalar b,
                                bool newLine = true) {
    xmp_write_prefix(s, ns, attrib);
    if (r == g && r == b) {
        s.writeScalarAsText(r);
    } else {
        s.writeScalarAsText(r);
        s.writeText(",");
        s.writeScalarAsText(g);
        s.writeText(",");
        s.writeScalarAsText(b);
    }
    xmp_write_suffix(s, newLine);
}

void xmp_write_scalar_attr(SkDynamicMemoryWStream& s,
                           const std::string& ns,
                           const std::string& attrib,
                           SkScalar value,
                           bool newLine = true) {
    xmp_write_prefix(s, ns, attrib);
    s.writeScalarAsText(value);
    xmp_write_suffix(s, newLine);
}

void xmp_write_decimal_attr(SkDynamicMemoryWStream& s,
                            const std::string& ns,
                            const std::string& attrib,
                            int32_t value,
                            bool newLine = true) {
    xmp_write_prefix(s, ns, attrib);
    s.writeDecAsText(value);
    xmp_write_suffix(s, newLine);
}

void xmp_write_string_attr(SkDynamicMemoryWStream& s,
                           const std::string& ns,
                           const std::string& attrib,
                           const std::string& value,
                           bool newLine = true) {
    xmp_write_prefix(s, ns, attrib);
    s.writeText(value.c_str());
    xmp_write_suffix(s, newLine);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// JpegR encoding

bool SkJpegGainmapEncoder::EncodeJpegR(SkWStream* dst,
                                       const SkPixmap& base,
                                       const SkJpegEncoder::Options& baseOptions,
                                       const SkPixmap& gainmap,
                                       const SkJpegEncoder::Options& gainmapOptions,
                                       const SkGainmapInfo& gainmapInfo) {
    return EncodeHDRGM(dst, base, baseOptions, gainmap, gainmapOptions, gainmapInfo);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// HDRGM encoding

// Generate the XMP metadata for an HDRGM file.
sk_sp<SkData> get_hdrgm_xmp_data(const SkGainmapInfo& gainmapInfo) {
    const float kLog2 = sk_float_log(2.f);
    SkDynamicMemoryWStream s;
    s.writeText(
            "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"XMP Core 5.5.0\">\n"
            "  <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
            "    <rdf:Description rdf:about=\"\"\n"
            "     xmlns:hdrgm=\"http://ns.adobe.com/hdr-gain-map/1.0/\"\n");
    const std::string hdrgmPrefix = "     hdrgm";
    xmp_write_string_attr(s, hdrgmPrefix, "Version", "1.0");
    xmp_write_per_channel_attr(s,
                               hdrgmPrefix,
                               "GainMapMin",
                               sk_float_log(gainmapInfo.fGainmapRatioMin.fR) / kLog2,
                               sk_float_log(gainmapInfo.fGainmapRatioMin.fG) / kLog2,
                               sk_float_log(gainmapInfo.fGainmapRatioMin.fB) / kLog2);
    xmp_write_per_channel_attr(s,
                               hdrgmPrefix,
                               "GainMapMax",
                               sk_float_log(gainmapInfo.fGainmapRatioMax.fR) / kLog2,
                               sk_float_log(gainmapInfo.fGainmapRatioMax.fG) / kLog2,
                               sk_float_log(gainmapInfo.fGainmapRatioMax.fB) / kLog2);
    xmp_write_per_channel_attr(s,
                               hdrgmPrefix,
                               "Gamma",
                               gainmapInfo.fGainmapGamma.fR,
                               gainmapInfo.fGainmapGamma.fG,
                               gainmapInfo.fGainmapGamma.fB);
    xmp_write_per_channel_attr(s,
                               hdrgmPrefix,
                               "OffsetSDR",
                               gainmapInfo.fEpsilonSdr.fR,
                               gainmapInfo.fEpsilonSdr.fG,
                               gainmapInfo.fEpsilonSdr.fB);
    xmp_write_per_channel_attr(s,
                               hdrgmPrefix,
                               "OffsetHDR",
                               gainmapInfo.fEpsilonHdr.fR,
                               gainmapInfo.fEpsilonHdr.fG,
                               gainmapInfo.fEpsilonHdr.fB);
    xmp_write_scalar_attr(
            s, hdrgmPrefix, "HDRCapacityMin", sk_float_log(gainmapInfo.fDisplayRatioSdr) / kLog2);
    xmp_write_scalar_attr(
            s, hdrgmPrefix, "HDRCapacityMax", sk_float_log(gainmapInfo.fDisplayRatioHdr) / kLog2);
    switch (gainmapInfo.fBaseImageType) {
        case SkGainmapInfo::BaseImageType::kSDR:
            xmp_write_string_attr(s, hdrgmPrefix, "BaseRendition", "SDR", /*newLine=*/false);
            break;
        case SkGainmapInfo::BaseImageType::kHDR:
            xmp_write_string_attr(s, hdrgmPrefix, "BaseRendition", "HDR", /*newLine=*/false);
            break;
    }
    s.writeText(
            "/>\n"
            "  </rdf:RDF>\n"
            "</x:xmpmeta>");
    return s.detachAsData();
}

// Generate the GContainer metadata for an image with a JPEG gainmap.
static sk_sp<SkData> get_gcontainer_xmp_data(size_t gainmapItemLength) {
    SkDynamicMemoryWStream s;
    s.writeText(
            "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"Adobe XMP Core 5.1.2\">\n"
            "  <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
            "    <rdf:Description\n"
            "     xmlns:Container=\"http://ns.google.com/photos/1.0/container/\"\n"
            "     xmlns:Item=\"http://ns.google.com/photos/1.0/container/item/\">\n"
            "      <Container:Directory>\n"
            "        <rdf:Seq>\n"
            "          <rdf:li>\n"
            "            <Container:Item\n"
            "             Item:Semantic=\"Primary\"\n"
            "             Item:Mime=\"image/jpeg\"/>\n"
            "          </rdf:li>\n"
            "          <rdf:li>\n"
            "            <Container:Item\n"
            "             Item:Semantic=\"RecoveryMap\"\n"
            "             Item:Mime=\"image/jpeg\"\n"
            "             ");
    xmp_write_decimal_attr(s, "Item", "Length", gainmapItemLength, /*newLine=*/false);
    s.writeText(
            "/>\n"
            "          </rdf:li>\n"
            "        </rdf:Seq>\n"
            "      </Container:Directory>\n"
            "    </rdf:Description>\n"
            "  </rdf:RDF>\n"
            "</x:xmpmeta>\n");
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

sk_sp<SkData> SkJpegGainmapEncoder::EncodeToData(const SkPixmap& pm,
                                                 const SkJpegEncoder::Options& options,
                                                 SkData* xmpMetadata,
                                                 SkData* mpfSegment) {
    SkJpegEncoder::OptionsPrivate optionsPrivate;
    optionsPrivate.xmpMetadata = xmpMetadata;
    optionsPrivate.mpfSegment = mpfSegment;
    SkDynamicMemoryWStream encodeStream;
    auto encoder = SkJpegEncoder::Make(&encodeStream, pm, options, optionsPrivate);
    if (!encoder || !encoder->encodeRows(pm.height())) {
        return nullptr;
    }
    return encodeStream.detachAsData();
}

bool SkJpegGainmapEncoder::EncodeHDRGM(SkWStream* dst,
                                       const SkPixmap& base,
                                       const SkJpegEncoder::Options& baseOptions,
                                       const SkPixmap& gainmap,
                                       const SkJpegEncoder::Options& gainmapOptions,
                                       const SkGainmapInfo& gainmapInfo) {
    // We will include the HDRGM XMP metadata in the gainmap image.
    auto hdrgmXmp = get_hdrgm_xmp_data(gainmapInfo);

    // Encode the gainmap image.
    auto gainmapData = EncodeToData(gainmap, gainmapOptions, hdrgmXmp.get(), nullptr);
    if (!gainmapData) {
        SkCodecPrintf("Failed to encode gainmap image.\n");
        return false;
    }

    // We will include the GContainer XMP metadata in the base image.
    auto gcontainerXmp = get_gcontainer_xmp_data(static_cast<int32_t>(gainmapData->size()));

    // Build placeholder MPF parameters that we will include in the base image.
    SkJpegMultiPictureParameters mpParams;
    mpParams.images.resize(2);
    auto placeholderMpfData = mpParams.serialize();

    // Encode the base image with the GContainer XMP and MPF.
    sk_sp<SkData> baseData =
            EncodeToData(base, baseOptions, gcontainerXmp.get(), placeholderMpfData.get());
    uint8_t* baseDataBytes = reinterpret_cast<uint8_t*>(baseData->writable_data());
    if (!baseData) {
        SkCodecPrintf("Failed to encode base image.\n");
        return false;
    }

    // Create a segment scan of of the encoded base image and search for our MPF parameters.
    SkJpegSegmentScanner baseScan(kJpegMarkerStartOfScan);
    baseScan.onBytes(baseData->bytes(), baseData->size());
    if (!baseScan.isDone()) {
        SkCodecPrintf("Failed to scan encoded base image header.\n");
        return false;
    }
    for (const auto& segment : baseScan.getSegments()) {
        // See if this segment has an MPF marker and parses as MPF parameters.
        if (segment.marker != kMpfMarker) {
            continue;
        }
        auto segmentParameters = SkJpegSegmentScanner::GetParameters(baseData.get(), segment);
        if (!SkJpegMultiPictureParameters::Make(segmentParameters)) {
            continue;
        }

        // Assert that it is exactly the placeholder data that we wrote.
        SkASSERT(segmentParameters->size() == placeholderMpfData->size());
        SkASSERT(memcmp(segmentParameters->data(),
                        placeholderMpfData->data(),
                        placeholderMpfData->size()) == 0);

        // Compute the real MPF parameters.
        uint32_t mpDataOffsetBase =
                static_cast<uint32_t>(segment.offset +       // The offset of the segment
                                      kJpegMarkerCodeSize +  // Including the marker
                                      kJpegSegmentParameterLengthSize +  // And the size parameter
                                      sizeof(kMpfSig));                  // And the signature
        mpParams.images[0].size = static_cast<uint32_t>(baseData->size());
        mpParams.images[1].dataOffset = mpParams.images[0].size - mpDataOffsetBase;
        mpParams.images[1].size = static_cast<uint32_t>(gainmapData->size());

        // Assert that they serialize to same size.
        auto mpfData = mpParams.serialize();
        SkASSERT(mpfData->size() == placeholderMpfData->size());

        // Overwrite the placeholder parameters in the encoded image.
        memcpy(baseDataBytes + segment.offset + kJpegMarkerCodeSize +
                       kJpegSegmentParameterLengthSize,
               mpfData->bytes(),
               mpfData->size());
        break;
    }

    // Write the concatenated images to the output stream.
    if (!dst->write(baseData->data(), baseData->size())) {
        SkCodecPrintf("Failed to write encoded base image.\n");
        return false;
    }
    if (!dst->write(gainmapData->data(), gainmapData->size())) {
        SkCodecPrintf("Failed to write encoded gainmap image.\n");
        return false;
    }
    return true;
}

#endif  // SK_ENCODE_JPEG
