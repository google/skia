/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkJpegGainmapEncoder.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkStream.h"
#include "include/encode/SkEncoder.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/private/SkGainmapInfo.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkJpegConstants.h"
#include "src/codec/SkJpegMultiPicture.h"
#include "src/codec/SkJpegPriv.h"
#include "src/codec/SkJpegSegmentScan.h"
#include "src/codec/SkTiffUtility.h"
#include "src/core/SkStreamPriv.h"
#include "src/encode/SkJpegEncoderImpl.h"

#include <vector>

static bool is_single_channel(SkColor4f c) { return c.fR == c.fG && c.fG == c.fB; };

////////////////////////////////////////////////////////////////////////////////////////////////////
// HDRGM encoding

// Generate the XMP metadata for an HDRGM file.
sk_sp<SkData> get_gainmap_image_xmp_metadata(const SkGainmapInfo& gainmapInfo) {
    SkDynamicMemoryWStream s;
    const float kLog2 = std::log(2.f);
    const SkColor4f gainMapMin = {std::log(gainmapInfo.fGainmapRatioMin.fR) / kLog2,
                                  std::log(gainmapInfo.fGainmapRatioMin.fG) / kLog2,
                                  std::log(gainmapInfo.fGainmapRatioMin.fB) / kLog2,
                                  1.f};
    const SkColor4f gainMapMax = {std::log(gainmapInfo.fGainmapRatioMax.fR) / kLog2,
                                  std::log(gainmapInfo.fGainmapRatioMax.fG) / kLog2,
                                  std::log(gainmapInfo.fGainmapRatioMax.fB) / kLog2,
                                  1.f};
    const SkColor4f gamma = {1.f / gainmapInfo.fGainmapGamma.fR,
                             1.f / gainmapInfo.fGainmapGamma.fG,
                             1.f / gainmapInfo.fGainmapGamma.fB,
                             1.f};
    // Write a scalar attribute.
    auto write_scalar_attr = [&s](const char* attrib, SkScalar value) {
        s.writeText("        ");
        s.writeText(attrib);
        s.writeText("=\"");
        s.writeScalarAsText(value);
        s.writeText("\"\n");
    };

    // Write a scalar attribute only if all channels of |value| are equal (otherwise, write
    // nothing).
    auto maybe_write_scalar_attr = [&write_scalar_attr](const char* attrib, SkColor4f value) {
        if (!is_single_channel(value)) {
            return;
        }
        write_scalar_attr(attrib, value.fR);
    };

    // Write a float3 attribute as a list ony if not all channels of |value| are equal (otherwise,
    // write nothing).
    auto maybe_write_float3_attr = [&s](const char* attrib, SkColor4f value) {
        if (is_single_channel(value)) {
            return;
        }
        s.writeText("      <");
        s.writeText(attrib);
        s.writeText(">\n");
        s.writeText("        <rdf:Seq>\n");
        s.writeText("          <rdf:li>");
        s.writeScalarAsText(value.fR);
        s.writeText("</rdf:li>\n");
        s.writeText("          <rdf:li>");
        s.writeScalarAsText(value.fG);
        s.writeText("</rdf:li>\n");
        s.writeText("          <rdf:li>");
        s.writeScalarAsText(value.fB);
        s.writeText("</rdf:li>\n");
        s.writeText("        </rdf:Seq>\n");
        s.writeText("      </");
        s.writeText(attrib);
        s.writeText(">\n");
    };

    s.writeText(
            "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"XMP Core 5.5.0\">\n"
            "  <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
            "    <rdf:Description rdf:about=\"\"\n"
            "        xmlns:hdrgm=\"http://ns.adobe.com/hdr-gain-map/1.0/\"\n"
            "        hdrgm:Version=\"1.0\"\n");
    maybe_write_scalar_attr("hdrgm:GainMapMin", gainMapMin);
    maybe_write_scalar_attr("hdrgm:GainMapMax", gainMapMax);
    maybe_write_scalar_attr("hdrgm:Gamma", gamma);
    maybe_write_scalar_attr("hdrgm:OffsetSDR", gainmapInfo.fEpsilonSdr);
    maybe_write_scalar_attr("hdrgm:OffsetHDR", gainmapInfo.fEpsilonHdr);
    write_scalar_attr("hdrgm:HDRCapacityMin", std::log(gainmapInfo.fDisplayRatioSdr) / kLog2);
    write_scalar_attr("hdrgm:HDRCapacityMax", std::log(gainmapInfo.fDisplayRatioHdr) / kLog2);
    switch (gainmapInfo.fBaseImageType) {
        case SkGainmapInfo::BaseImageType::kSDR:
            s.writeText("        hdrgm:BaseRenditionIsHDR=\"False\">\n");
            break;
        case SkGainmapInfo::BaseImageType::kHDR:
            s.writeText("        hdrgm:BaseRenditionIsHDR=\"True\">\n");
            break;
    }

    // Write any of the vector parameters that cannot be represented as scalars (and thus cannot
    // be written inline as above).
    maybe_write_float3_attr("hdrgm:GainMapMin", gainMapMin);
    maybe_write_float3_attr("hdrgm:GainMapMax", gainMapMax);
    maybe_write_float3_attr("hdrgm:Gamma", gamma);
    maybe_write_float3_attr("hdrgm:OffsetSDR", gainmapInfo.fEpsilonSdr);
    maybe_write_float3_attr("hdrgm:OffsetHDR", gainmapInfo.fEpsilonHdr);
    s.writeText(
            "    </rdf:Description>\n"
            "  </rdf:RDF>\n"
            "</x:xmpmeta>");
    return s.detachAsData();
}

// Generate the GContainer metadata for an image with a JPEG gainmap.
static sk_sp<SkData> get_base_image_xmp_metadata(size_t gainmapItemLength) {
    SkDynamicMemoryWStream s;
    s.writeText(
            "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"Adobe XMP Core 5.1.2\">\n"
            "  <rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
            "    <rdf:Description\n"
            "        xmlns:Container=\"http://ns.google.com/photos/1.0/container/\"\n"
            "        xmlns:Item=\"http://ns.google.com/photos/1.0/container/item/\"\n"
            "        xmlns:hdrgm=\"http://ns.adobe.com/hdr-gain-map/1.0/\"\n"
            "        hdrgm:Version=\"1.0\">\n"
            "      <Container:Directory>\n"
            "        <rdf:Seq>\n"
            "          <rdf:li rdf:parseType=\"Resource\">\n"
            "            <Container:Item\n"
            "             Item:Semantic=\"Primary\"\n"
            "             Item:Mime=\"image/jpeg\"/>\n"
            "          </rdf:li>\n"
            "          <rdf:li rdf:parseType=\"Resource\">\n"
            "            <Container:Item\n"
            "             Item:Semantic=\"GainMap\"\n"
            "             Item:Mime=\"image/jpeg\"\n"
            "             Item:Length=\"");
    s.writeDecAsText(gainmapItemLength);
    s.writeText(
            "\"/>\n"
            "          </rdf:li>\n"
            "        </rdf:Seq>\n"
            "      </Container:Directory>\n"
            "    </rdf:Description>\n"
            "  </rdf:RDF>\n"
            "</x:xmpmeta>\n");
    return s.detachAsData();
}

static sk_sp<SkData> encode_to_data(const SkPixmap& pm,
                                    const SkJpegEncoder::Options& options,
                                    const SkJpegMetadataEncoder::SegmentList& metadataSegments) {
    SkDynamicMemoryWStream encodeStream;
    auto encoder = SkJpegEncoderImpl::MakeRGB(&encodeStream, pm, options, metadataSegments);
    if (!encoder || !encoder->encodeRows(pm.height())) {
        return nullptr;
    }
    return encodeStream.detachAsData();
}

static sk_sp<SkData> get_exif_params() {
    SkDynamicMemoryWStream s;

    s.write(kExifSig, sizeof(kExifSig));
    s.write8(0);

    s.write(SkTiff::kEndianBig, sizeof(SkTiff::kEndianBig));
    SkWStreamWriteU32BE(&s, 8);  // Offset of index IFD

    // Write the index IFD.
    {
        constexpr uint16_t kIndexIfdNumberOfTags = 1;
        SkWStreamWriteU16BE(&s, kIndexIfdNumberOfTags);

        constexpr uint16_t kSubIFDOffsetTag = 0x8769;
        constexpr uint32_t kSubIfdCount = 1;
        constexpr uint32_t kSubIfdOffset = 26;
        SkWStreamWriteU16BE(&s, kSubIFDOffsetTag);
        SkWStreamWriteU16BE(&s, SkTiff::kTypeUnsignedLong);
        SkWStreamWriteU32BE(&s, kSubIfdCount);
        SkWStreamWriteU32BE(&s, kSubIfdOffset);

        constexpr uint32_t kIndexIfdNextIfdOffset = 0;
        SkWStreamWriteU32BE(&s, kIndexIfdNextIfdOffset);
    }

    // Write the sub-IFD.
    {
        constexpr uint16_t kSubIfdNumberOfTags = 1;
        SkWStreamWriteU16BE(&s, kSubIfdNumberOfTags);

        constexpr uint16_t kVersionTag = 0x9000;
        constexpr uint32_t kVersionCount = 4;
        constexpr uint8_t kVersion[kVersionCount] = {'0', '2', '3', '2'};
        SkWStreamWriteU16BE(&s, kVersionTag);
        SkWStreamWriteU16BE(&s, SkTiff::kTypeUndefined);
        SkWStreamWriteU32BE(&s, kVersionCount);
        s.write(kVersion, sizeof(kVersion));

        constexpr uint32_t kSubIfdNextIfdOffset = 0;
        SkWStreamWriteU32BE(&s, kSubIfdNextIfdOffset);
    }

    return s.detachAsData();
}

static sk_sp<SkData> get_mpf_segment(const SkJpegMultiPictureParameters& mpParams,
                                     size_t imageNumber) {
    SkDynamicMemoryWStream s;
    auto segmentParameters = mpParams.serialize(static_cast<uint32_t>(imageNumber));
    const size_t mpParameterLength = kJpegSegmentParameterLengthSize + segmentParameters->size();
    s.write8(0xFF);
    s.write8(kMpfMarker);
    s.write8(mpParameterLength / 256);
    s.write8(mpParameterLength % 256);
    s.write(segmentParameters->data(), segmentParameters->size());
    return s.detachAsData();
}

static sk_sp<SkData> get_iso_gainmap_segment_params(sk_sp<SkData> data) {
    SkDynamicMemoryWStream s;
    s.write(kISOGainmapSig, sizeof(kISOGainmapSig));
    s.write(data->data(), data->size());
    return s.detachAsData();
}

bool SkJpegGainmapEncoder::EncodeHDRGM(SkWStream* dst,
                                       const SkPixmap& base,
                                       const SkJpegEncoder::Options& baseOptions,
                                       const SkPixmap& gainmap,
                                       const SkJpegEncoder::Options& gainmapOptions,
                                       const SkGainmapInfo& gainmapInfo) {
    bool includeUltraHDRv1 = gainmapInfo.isUltraHDRv1Compatible();

    // All images will have the same minimial Exif metadata.
    auto exif_params = get_exif_params();

    // Encode the gainmap image.
    sk_sp<SkData> gainmapData;
    {
        SkJpegMetadataEncoder::SegmentList metadataSegments;

        // Start with Exif metadata.
        metadataSegments.emplace_back(kExifMarker, exif_params);

        // MPF segment will be inserted after this.

        // Add XMP metadata.
        if (includeUltraHDRv1) {
            SkJpegMetadataEncoder::AppendXMPStandard(
                    metadataSegments, get_gainmap_image_xmp_metadata(gainmapInfo).get());
        }

        // Include the ICC profile of the alternate color space, if it is used.
        if (gainmapInfo.fGainmapMathColorSpace) {
            SkJpegMetadataEncoder::AppendICC(
                    metadataSegments, gainmapOptions, gainmapInfo.fGainmapMathColorSpace.get());
        }

        // Add the ISO 21946-1 metadata.
        metadataSegments.emplace_back(kISOGainmapMarker,
                                      get_iso_gainmap_segment_params(gainmapInfo.serialize()));

        // Encode the gainmap image.
        gainmapData = encode_to_data(gainmap, gainmapOptions, metadataSegments);
        if (!gainmapData) {
            SkCodecPrintf("Failed to encode gainmap image.\n");
            return false;
        }
    }

    // Encode the base image.
    sk_sp<SkData> baseData;
    {
        SkJpegMetadataEncoder::SegmentList metadataSegments;

        // Start with Exif metadata.
        metadataSegments.emplace_back(kExifMarker, exif_params);

        // MPF segment will be inserted after this.

        // Include XMP.
        if (includeUltraHDRv1) {
            // Add to the gainmap image size the size of the MPF segment for image 1 of a 2-image
            // file.
            SkJpegMultiPictureParameters mpParams(2);
            size_t gainmapImageSize = gainmapData->size() + get_mpf_segment(mpParams, 1)->size();
            SkJpegMetadataEncoder::AppendXMPStandard(
                    metadataSegments,
                    get_base_image_xmp_metadata(static_cast<int32_t>(gainmapImageSize)).get());
        }

        // Include ICC profile metadata.
        SkJpegMetadataEncoder::AppendICC(metadataSegments, baseOptions, base.colorSpace());

        // Include the ISO 21946-1 version metadata.
        metadataSegments.emplace_back(
                kISOGainmapMarker,
                get_iso_gainmap_segment_params(SkGainmapInfo::SerializeVersion()));

        // Encode the base image.
        baseData = encode_to_data(base, baseOptions, metadataSegments);
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

// Compute the offset into |image| at which the MP segment should be inserted. Return 0 on failure.
static size_t mp_segment_offset(const SkData* image) {
    // Scan the image until StartOfScan marker.
    SkJpegSegmentScanner scan(kJpegMarkerStartOfScan);
    scan.onBytes(image->data(), image->size());
    if (!scan.isDone()) {
        SkCodecPrintf("Failed to scan image header.\n");
        return 0;
    }
    const auto& segments = scan.getSegments();

    // According to CIPA DC-007 section 5.1, "Basic MP File Structure", "The MP Extensions are
    // specified in the APP2 marker segment which follows immediately after the Exif Attributes in
    // the APP1 marker segment except as specified in section 7". In practice, this is rarely
    // obeyed, and further, makes the file dangerous for use by less robust editors (see
    // b/355642172). Instead, place the MP segment just before the StartOfScan marker.

    // If there is no Exif segment, then insert the MPF segment just before the StartOfScan.
    return segments.back().offset;
}

bool SkJpegGainmapEncoder::MakeMPF(SkWStream* dst, const SkData** images, size_t imageCount) {
    if (imageCount < 1) {
        return true;
    }

    // The offset into each image at which the MP segment will be written.
    std::vector<size_t> mpSegmentOffsets(imageCount);

    // Populate the MP parameters (image sizes and offsets).
    SkJpegMultiPictureParameters mpParams(imageCount);
    size_t cumulativeSize = 0;
    for (size_t i = 0; i < imageCount; ++i) {
        // Compute the offset into the each image where we will write the MP parameters.
        mpSegmentOffsets[i] = mp_segment_offset(images[i]);
        if (!mpSegmentOffsets[i]) {
            return false;
        }

        // Add the size of the MPF segment to image size. Note that the contents of
        // get_mpf_segment() are incorrect (because we don't have the right offset values), but
        // the size is correct.
        const size_t imageSize = images[i]->size() + get_mpf_segment(mpParams, i)->size();
        mpParams.images[i].dataOffset = SkJpegMultiPictureParameters::GetImageDataOffset(
                cumulativeSize, mpSegmentOffsets[0]);
        mpParams.images[i].size = static_cast<uint32_t>(imageSize);
        cumulativeSize += imageSize;
    }

    // Write the images.
    for (size_t i = 0; i < imageCount; ++i) {
        // Write up to the MP segment.
        if (!dst->write(images[i]->bytes(), mpSegmentOffsets[i])) {
            SkCodecPrintf("Failed to write image header.\n");
            return false;
        }

        // Write the MP segment.
        auto mpfSegment = get_mpf_segment(mpParams, i);
        if (!dst->write(mpfSegment->data(), mpfSegment->size())) {
            SkCodecPrintf("Failed to write MPF segment.\n");
            return false;
        }

        // Write the rest of the image.
        if (!dst->write(images[i]->bytes() + mpSegmentOffsets[i],
                        images[i]->size() - mpSegmentOffsets[i])) {
            SkCodecPrintf("Failed to write image body.\n");
            return false;
        }
    }

    return true;
}
