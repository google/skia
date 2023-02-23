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

static sk_sp<SkData> encode_to_data(const SkPixmap& pm,
                                    const SkJpegEncoder::Options& options,
                                    SkData* xmpMetadata) {
    SkJpegEncoder::Options optionsWithXmp = options;
    optionsWithXmp.xmpMetadata = xmpMetadata;
    SkDynamicMemoryWStream encodeStream;
    auto encoder = SkJpegEncoder::Make(&encodeStream, pm, optionsWithXmp);
    if (!encoder || !encoder->encodeRows(pm.height())) {
        return nullptr;
    }
    return encodeStream.detachAsData();
}

static sk_sp<SkData> get_mpf_segment(const SkJpegMultiPictureParameters& mpParams) {
    SkDynamicMemoryWStream s;
    auto segmentParameters = mpParams.serialize();
    const size_t mpParameterLength = kJpegSegmentParameterLengthSize + segmentParameters->size();
    s.write8(0xFF);
    s.write8(kMpfMarker);
    s.write8(mpParameterLength / 256);
    s.write8(mpParameterLength % 256);
    s.write(segmentParameters->data(), segmentParameters->size());
    return s.detachAsData();
}

bool SkJpegGainmapEncoder::EncodeHDRGM(SkWStream* dst,
                                       const SkPixmap& base,
                                       const SkJpegEncoder::Options& baseOptions,
                                       const SkPixmap& gainmap,
                                       const SkJpegEncoder::Options& gainmapOptions,
                                       const SkGainmapInfo& gainmapInfo) {
    // Encode the gainmap image with the HDRGM XMP metadata.
    sk_sp<SkData> gainmapData;
    {
        // We will include the HDRGM XMP metadata in the gainmap image.
        auto hdrgmXmp = get_hdrgm_xmp_data(gainmapInfo);
        gainmapData = encode_to_data(gainmap, gainmapOptions, hdrgmXmp.get());
        if (!gainmapData) {
            SkCodecPrintf("Failed to encode gainmap image.\n");
            return false;
        }
    }

    // Encode the base image with the Container XMP metadata.
    sk_sp<SkData> baseData;
    {
        auto containerXmp = get_gcontainer_xmp_data(static_cast<int32_t>(gainmapData->size()));
        baseData = encode_to_data(base, baseOptions, containerXmp.get());
        if (!baseData) {
            SkCodecPrintf("Failed to encode base image.\n");
            return false;
        }
    }

    // Combine them into an MPF.
    const SkData* images[] = {
            baseData.get(),
            gainmapData.get(),
    };
    return MakeMPF(dst, images, 2);
}

bool SkJpegGainmapEncoder::MakeMPF(SkWStream* dst, const SkData** images, size_t imageCount) {
    if (imageCount < 1) {
        return true;
    }

    // Create a scan of the primary image.
    SkJpegSegmentScanner primaryScan;
    primaryScan.onBytes(images[0]->data(), images[0]->size());
    if (!primaryScan.isDone()) {
        SkCodecPrintf("Failed to scan encoded primary image header.\n");
        return false;
    }

    // Copy the primary image up to its StartOfScan, then insert the MPF segment, then copy the rest
    // of the primary image, and all other images.
    size_t bytesRead = 0;
    size_t bytesWritten = 0;
    for (const auto& segment : primaryScan.getSegments()) {
        // Write all ECD before this segment.
        {
            size_t ecdBytesToWrite = segment.offset - bytesRead;
            if (!dst->write(images[0]->bytes() + bytesRead, ecdBytesToWrite)) {
                SkCodecPrintf("Failed to write entropy coded data.\n");
                return false;
            }
            bytesWritten += ecdBytesToWrite;
            bytesRead = segment.offset;
        }

        // If this isn't a StartOfScan, write just the segment.
        if (segment.marker != kJpegMarkerStartOfScan) {
            const size_t bytesToWrite = kJpegMarkerCodeSize + segment.parameterLength;
            if (!dst->write(images[0]->bytes() + bytesRead, bytesToWrite)) {
                SkCodecPrintf("Failed to copy segment.\n");
                return false;
            }
            bytesWritten += bytesToWrite;
            bytesRead += bytesToWrite;
            continue;
        }

        // We're now at the StartOfScan.
        const size_t bytesRemaining = images[0]->size() - bytesRead;

        // Compute the MPF offsets for the images.
        SkJpegMultiPictureParameters mpParams;
        {
            mpParams.images.resize(imageCount);
            const size_t mpSegmentSize = kJpegMarkerCodeSize + kJpegSegmentParameterLengthSize +
                                         mpParams.serialize()->size();
            mpParams.images[0].size =
                    static_cast<uint32_t>(bytesWritten + mpSegmentSize + bytesRemaining);
            uint32_t offset =
                    static_cast<uint32_t>(bytesRemaining + mpSegmentSize - kJpegMarkerCodeSize -
                                          kJpegSegmentParameterLengthSize - sizeof(kMpfSig));
            for (size_t i = 0; i < imageCount; ++i) {
                mpParams.images[i].dataOffset = offset;
                mpParams.images[i].size = static_cast<uint32_t>(images[i]->size());
                offset += mpParams.images[i].size;
            }
        }

        // Write the MPF segment.
        auto mpfSegment = get_mpf_segment(mpParams);
        if (!dst->write(mpfSegment->data(), mpfSegment->size())) {
            SkCodecPrintf("Failed to write MPF segment.\n");
            return false;
        }

        // Write the rest of the primary file.
        if (!dst->write(images[0]->bytes() + bytesRead, bytesRemaining)) {
            SkCodecPrintf("Failed to write remainder of primary image.\n");
            return false;
        }
        bytesRead += bytesRemaining;
        SkASSERT(bytesRead == images[0]->size());
        break;
    }

    // Write the remaining files.
    for (size_t i = 1; i < imageCount; ++i) {
        if (!dst->write(images[i]->data(), images[i]->size())) {
            SkCodecPrintf("Failed to write auxiliary image.\n");
        }
    }
    return true;
}

#endif  // SK_ENCODE_JPEG
