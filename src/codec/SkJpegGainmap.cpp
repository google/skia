/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkJpegGainmap.h"

#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/private/SkGainmapInfo.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/utils/SkParse.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkJpegMultiPicture.h"
#include "src/codec/SkJpegPriv.h"
#include "src/codec/SkJpegSegmentScan.h"
#include "src/codec/SkJpegSourceMgr.h"
#include "src/xml/SkDOM.h"

#include <cstdint>
#include <cstring>
#include <utility>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkDOM and XMP helpers.

/*
 * Build an SkDOM from an SkData. Return true on success and false on failure (including the input
 * data being nullptr).
 */
bool SkDataToSkDOM(sk_sp<const SkData> data, SkDOM* dom) {
    if (!data) {
        return false;
    }
    auto stream = SkMemoryStream::MakeDirect(data->data(), data->size());
    if (!stream) {
        return false;
    }
    return dom->build(*stream) != nullptr;
}

/*
 * Given an SkDOM, verify that the dom is XMP, and find the first rdf:Description node that matches
 * the specified namespaces to the specified URIs. The XML structure that this function matches is
 * as follows (with NAMESPACEi and URIi being the parameters specified to this function):
 *
 *   <x:xmpmeta ...>
 *     <rdf:RDF ...>
 *       <rdf:Description NAMESPACE0="URI0" NAMESPACE1="URI1" .../>
 *     </rdf:RDF>
 *   </x:xmpmeta>
 */
const SkDOM::Node* FindXmpNamespaceUriMatch(const SkDOM& dom,
                                            const char* namespaces[],
                                            const char* uris[],
                                            size_t count) {
    const SkDOM::Node* root = dom.getRootNode();
    if (!root) {
        return nullptr;
    }
    const char* rootName = dom.getName(root);
    if (!rootName || strcmp(rootName, "x:xmpmeta") != 0) {
        return nullptr;
    }

    const char* kRdf = "rdf:RDF";
    for (const auto* rdf = dom.getFirstChild(root, kRdf); rdf;
         rdf = dom.getNextSibling(rdf, kRdf)) {
        const char* kDesc = "rdf:Description";
        for (const auto* desc = dom.getFirstChild(rdf, kDesc); desc;
             desc = dom.getNextSibling(desc, kDesc)) {
            bool allNamespaceURIsMatch = true;
            for (size_t i = 0; i < count; ++i) {
                if (!dom.hasAttr(desc, namespaces[i], uris[i])) {
                    allNamespaceURIsMatch = false;
                    break;
                }
            }
            if (allNamespaceURIsMatch) {
                return desc;
            }
        }
    }
    return nullptr;
}

/*
 * Given a node, see if that node has only one child with the indicated name. If so, see if that
 * child has only a single child of its own, and that child is text. If all of that is the case
 * then return the text, otherwise return nullptr.
 *
 * In the following example, innerText will be returned.
 *    <node><childName>innerText</childName></node>
 *
 * In the following examples, nullptr will be returned (because there are multiple children with
 * childName in the first case, and because the child has children of its own in the second).
 *    <node><childName>innerTextA</childName><childName>innerTextB</childName></node>
 *    <node><childName>innerText<otherGrandChild/></childName></node>
 */
static const char* GetUniqueChildText(const SkDOM& dom,
                                      const SkDOM::Node* node,
                                      const char* childName) {
    // Fail if there are multiple children with childName.
    if (dom.countChildren(node, childName) != 1) {
        return nullptr;
    }
    const auto* child = dom.getFirstChild(node, childName);
    if (!child) {
        return nullptr;
    }
    // Fail if the child has any children besides text.
    if (dom.countChildren(child) != 1) {
        return nullptr;
    }
    const auto* grandChild = dom.getFirstChild(child);
    if (dom.getType(grandChild) != SkDOM::kText_Type) {
        return nullptr;
    }
    // Return the text.
    return dom.getName(grandChild);
}

// Helper function that builds on GetUniqueChildText, returning true if the unique child with
// childName has inner text that matches an expected text.
static bool UniqueChildTextMatches(const SkDOM& dom,
                                   const SkDOM::Node* node,
                                   const char* childName,
                                   const char* expectedText) {
    const char* text = GetUniqueChildText(dom, node, childName);
    if (text && !strcmp(text, expectedText)) {
        return true;
    }
    return false;
}

// Helper function that builds on GetUniqueChildText, returning true if the unique child with
// childName has inner text that matches an expected integer.
static bool UniqueChildTextMatches(const SkDOM& dom,
                                   const SkDOM::Node* node,
                                   const char* childName,
                                   int32_t expectedValue) {
    const char* text = GetUniqueChildText(dom, node, childName);
    int32_t actualValue = 0;
    if (text && SkParse::FindS32(text, &actualValue)) {
        return actualValue == expectedValue;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Multi-PictureFormat Gainmap Functions

// Return true if the specified XMP metadata identifies this image as an HDR gainmap.
static bool XmpIsHDRGainMap(const sk_sp<const SkData>& xmpMetadata) {
    // Parse the XMP.
    SkDOM dom;
    if (!SkDataToSkDOM(xmpMetadata, &dom)) {
        return false;
    }

    // Find a node that matches the requested namespaces and URIs.
    const char* namespaces[2] = {"xmlns:apdi", "xmlns:HDRGainMap"};
    const char* uris[2] = {"http://ns.apple.com/pixeldatainfo/1.0/",
                           "http://ns.apple.com/HDRGainMap/1.0/"};
    const SkDOM::Node* node = FindXmpNamespaceUriMatch(dom, namespaces, uris, 2);
    if (!node) {
        return false;
    }
    if (!UniqueChildTextMatches(
                dom, node, "apdi:AuxiliaryImageType", "urn:com:apple:photo:2020:aux:hdrgainmap")) {
        SkCodecPrintf("Did not find auxiliary image type.\n");
        return false;
    }
    if (!UniqueChildTextMatches(dom, node, "HDRGainMap:HDRGainMapVersion", 65536)) {
        SkCodecPrintf("HDRGainMapVersion absent or not 65536.\n");
        return false;
    }

    // This node will often have StoredFormat and NativeFormat children that have inner text that
    // specifies the integer 'L008' (also known as kCVPixelFormatType_OneComponent8).
    return true;
}

bool SkJpegGetMultiPictureGainmap(sk_sp<const SkData> decoderMpfMetadata,
                                  SkJpegSourceMgr* decoderSource,
                                  SkGainmapInfo* outInfo,
                                  std::unique_ptr<SkStream>* outGainmapImageStream) {
    // The decoder has already scanned for MPF metadata. If it doesn't exist, or it doesn't parse,
    // then early-out.
    if (!decoderMpfMetadata) {
        return false;
    }
    auto mpParams = SkJpegParseMultiPicture(decoderMpfMetadata);
    if (!mpParams) {
        return false;
    }

    // Extract the Multi-Picture image streams in the original decoder stream (we needed the scan to
    // find the offsets of the MP images within the original decoder stream).
    auto mpStreams = SkJpegExtractMultiPictureStreams(mpParams.get(), decoderSource);
    if (!mpStreams) {
        SkCodecPrintf("Failed to extract MP image streams.\n");
        return false;
    }

    // Iterate over the MP image streams.
    for (auto& mpImage : mpStreams->images) {
        if (!mpImage.stream) {
            continue;
        }

        // Create a temporary source manager for this MP image.
        auto mpImageSource = SkJpegSourceMgr::Make(mpImage.stream.get());

        // Search for the XMP metadata in the MP image's scan.
        for (const auto& segment : mpImageSource->getAllSegments()) {
            if (segment.marker != kXMPMarker) {
                continue;
            }
            auto xmpMetadata = mpImageSource->copyParameters(segment, kXMPSig, sizeof(kXMPSig));
            if (!xmpMetadata) {
                continue;
            }

            // If this XMP does not indicate that the image is an HDR gainmap, then continue.
            if (!XmpIsHDRGainMap(xmpMetadata)) {
                continue;
            }

            // This MP image is the gainmap image. Populate its stream and the rendering parameters
            // for its format.
            if (outGainmapImageStream) {
                if (!mpImage.stream->rewind()) {
                    SkCodecPrintf("Failed to rewind gainmap image stream.\n");
                    return false;
                }
                *outGainmapImageStream = std::move(mpImage.stream);
            }
            constexpr float kLogRatioMin = 0.f;
            constexpr float kLogRatioMax = 1.f;
            outInfo->fLogRatioMin = {kLogRatioMin, kLogRatioMin, kLogRatioMin, 1.f};
            outInfo->fLogRatioMax = {kLogRatioMax, kLogRatioMax, kLogRatioMax, 1.f};
            outInfo->fGainmapGamma = {1.f, 1.f, 1.f, 1.f};
            outInfo->fEpsilonSdr = 1 / 128.f;
            outInfo->fEpsilonHdr = 1 / 128.f;
            outInfo->fHdrRatioMin = 1.f;
            outInfo->fHdrRatioMax = sk_float_exp(kLogRatioMax);
            outInfo->fBaseImageType = SkGainmapInfo::BaseImageType::kSDR;
            outInfo->fType = SkGainmapInfo::Type::kMultiPicture;
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// JpegR Gainmap functions

static bool SkJpegGetJpegRGainmapParseXMP(sk_sp<const SkData> xmpMetadata,
                                          size_t* outOffset,
                                          size_t* outSize,
                                          SkGainmapInfo::Type* outType,
                                          float* outRangeScalingFactor) {
    // Parse the XMP.
    SkDOM dom;
    if (!SkDataToSkDOM(xmpMetadata, &dom)) {
        return false;
    }

    // Find a node that matches the requested namespaces and URIs.
    const char* namespaces[2] = {"xmlns:GContainer", "xmlns:RecoveryMap"};
    const char* uris[2] = {"http://ns.google.com/photos/1.0/container/",
                           "http://ns.google.com/photos/1.0/recoverymap/"};
    const SkDOM::Node* node = FindXmpNamespaceUriMatch(dom, namespaces, uris, 2);
    if (!node) {
        return false;
    }

    // The node must have a GContainer:Version child that specifies version 1.
    if (!UniqueChildTextMatches(dom, node, "GContainer:Version", 1)) {
        SkCodecPrintf("GContainer:Version is absent or not 1");
        return false;
    }

    // The node must have a GContainer:Directory.
    const auto* directory = dom.getFirstChild(node, "GContainer:Directory");
    if (!directory) {
        SkCodecPrintf("Missing GContainer:Directory");
        return false;
    }

    // That GContainer:Directory must have a sequence of  items.
    const auto* seq = dom.getFirstChild(directory, "rdf:Seq");
    if (!seq) {
        SkCodecPrintf("Missing rdf:Seq");
        return false;
    }

    // Iterate through the items in the GContainer:Directory's sequence. Keep a running sum of the
    // GContainer::ItemLength of all items that appear before the RecoveryMap.
    bool isFirstItem = true;
    size_t itemLengthSum = 0;
    for (const auto* li = dom.getFirstChild(seq, "rdf:li"); li;
         li = dom.getNextSibling(li, "rdf:li")) {
        // Each list item must contain a GContainer item.
        const auto* item = dom.getFirstChild(li, "GContainer:Item");
        if (!item) {
            SkCodecPrintf("List item does not have GContainer:Item.\n");
            return false;
        }
        // An ItemSemantic is required for every GContainer item.
        const char* itemSemantic = dom.findAttr(item, "GContainer:ItemSemantic");
        if (!itemSemantic) {
            SkCodecPrintf("GContainer item is missing ItemSemantic.\n");
            return false;
        }
        // An ItemMime is required for every GContainer item.
        const char* itemMime = dom.findAttr(item, "GContainer:ItemMime");
        if (!itemMime) {
            SkCodecPrintf("GContainer item is missing ItemMime.\n");
            return false;
        }
        if (isFirstItem) {
            isFirstItem = false;
            // The first item must be Primary.
            if (strcmp(itemSemantic, "Primary") != 0) {
                SkCodecPrintf("First item is not Primary.\n");
                return false;
            }
            // The first item has mime type image/jpeg (we are decoding a jpeg).
            if (strcmp(itemMime, "image/jpeg") != 0) {
                SkCodecPrintf("Primary does not report that it is image/jpeg.\n");
                return false;
            }
            // The Verison of 1 is required for the Primary.
            if (!dom.hasAttr(item, "RecoveryMap:Version", "1")) {
                SkCodecPrintf("RecoveryMap:Version is not 1.");
                return false;
            }
            // The TransferFunction is required for the Primary.
            int32_t transferFunction = 0;
            if (!dom.findS32(item, "RecoveryMap:TransferFunction", &transferFunction)) {
                SkCodecPrintf("RecoveryMap:TransferFunction is absent.");
                return false;
            }
            switch (transferFunction) {
                case 0:
                    *outType = SkGainmapInfo::Type::kJpegR_Linear;
                    break;
                case 1:
                    *outType = SkGainmapInfo::Type::kJpegR_HLG;
                    break;
                case 2:
                    *outType = SkGainmapInfo::Type::kJpegR_PQ;
                    break;
                default:
                    SkCodecPrintf("RecoveryMap:TransferFunction is out of range.");
                    return false;
            }
            // The RangeScalingFactor is required for the Primary.
            SkScalar rangeScalingFactor = 1.f;
            if (!dom.findScalars(item, "RecoveryMap:RangeScalingFactor", &rangeScalingFactor, 1)) {
                SkCodecPrintf("RecoveryMap:RangeScalingFactor is absent.");
                return false;
            }
            *outRangeScalingFactor = rangeScalingFactor;
        } else {
            // An ItemLength is required for all non-Primary GContainter items.
            int32_t itemLength = 0;
            if (!dom.findS32(item, "GContainer:ItemLength", &itemLength)) {
                SkCodecPrintf("GContainer:ItemLength is absent.");
                return false;
            }
            // If this is not the recovery map, then read past it.
            if (strcmp(itemSemantic, "RecoveryMap") != 0) {
                itemLengthSum += itemLength;
                continue;
            }
            // The recovery map must have mime type image/jpeg in this implementation.
            if (strcmp(itemMime, "image/jpeg") != 0) {
                SkCodecPrintf("RecoveryMap does not report that it is image/jpeg.\n");
                return false;
            }

            // This is the recovery map.
            *outOffset = itemLengthSum;
            *outSize = itemLength;
            return true;
        }
    }
    return false;
}

bool SkJpegGetJpegRGainmap(sk_sp<const SkData> xmpMetadata,
                           SkJpegSourceMgr* decoderSource,
                           SkGainmapInfo* outInfo,
                           std::unique_ptr<SkStream>* outGainmapImageStream) {
    // Parse the XMP metadata of the original image, to see if it specifies a RecoveryMap.
    size_t itemOffsetFromEndOfImage = 0;
    size_t itemSize = 0;
    SkGainmapInfo::Type type = SkGainmapInfo::Type::kUnknown;
    float rangeScalingFactor = 1.f;
    if (!SkJpegGetJpegRGainmapParseXMP(
                xmpMetadata, &itemOffsetFromEndOfImage, &itemSize, &type, &rangeScalingFactor)) {
        return false;
    }

    // The offset read from the XMP metadata is relative to the end of the EndOfImage marker in the
    // original decoder stream. Create a full scan of the original decoder stream, so we can find
    // that EndOfImage marker's offset in the decoder stream.
    const std::vector<SkJpegSegment>& segments = decoderSource->getAllSegments();
    if (segments.empty() || segments.back().marker != SkJpegSegmentScanner::kMarkerEndOfImage) {
        SkCodecPrintf("Failed to construct segments through EndOfImage.\n");
        return false;
    }
    const auto& lastSegment = segments.back();
    const size_t endOfImageOffset = lastSegment.offset + SkJpegSegmentScanner::kMarkerCodeSize;
    const size_t itemOffsetFromStartOfImage = endOfImageOffset + itemOffsetFromEndOfImage;

    // Extract the gainmap image's stream.
    auto gainmapImageStream = decoderSource->getSubsetStream(itemOffsetFromStartOfImage, itemSize);
    if (!gainmapImageStream) {
        SkCodecPrintf("Failed to extract gainmap stream.");
        return false;
    }

    // Populate the output parameters for this format.
    if (outGainmapImageStream) {
        if (!gainmapImageStream->rewind()) {
            SkCodecPrintf("Failed to rewind gainmap image stream.");
            return false;
        }
        *outGainmapImageStream = std::move(gainmapImageStream);
    }

    const float kLogRatioMax = sk_float_log(rangeScalingFactor);
    const float kLogRatioMin = -kLogRatioMax;
    outInfo->fLogRatioMin = {kLogRatioMin, kLogRatioMin, kLogRatioMin, 1.f};
    outInfo->fLogRatioMax = {kLogRatioMax, kLogRatioMax, kLogRatioMax, 1.f};
    outInfo->fGainmapGamma = {1.f, 1.f, 1.f, 1.f};
    outInfo->fEpsilonSdr = 0.f;
    outInfo->fEpsilonHdr = 0.f;
    outInfo->fHdrRatioMin = 1.f;
    outInfo->fHdrRatioMax = rangeScalingFactor;
    outInfo->fBaseImageType = SkGainmapInfo::BaseImageType::kSDR;
    outInfo->fType = type;
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// HDRGM support

// Helper function to read a 1 or 3 floats and write them into an SkColor4f.
static void find_per_channel_attr(const SkDOM& dom,
                                  const SkDOM::Node* node,
                                  const char* attr,
                                  SkColor4f* outColor) {
    SkScalar values[3] = {0.f, 0.f, 0.f};
    if (dom.findScalars(node, attr, values, 3)) {
        *outColor = {values[0], values[1], values[2], 1.f};
    } else if (dom.findScalars(node, attr, values, 1)) {
        *outColor = {values[0], values[0], values[0], 1.f};
    }
}

bool SkJpegGetHDRGMGainmapInfo(sk_sp<const SkData> xmpMetadata, SkGainmapInfo* outGainmapInfo) {
    // Parse the XMP.
    SkDOM dom;
    if (!SkDataToSkDOM(xmpMetadata, &dom)) {
        return false;
    }

    // Find a node that matches the requested namespace and URI.
    const char* namespaces[1] = {"xmlns:hdrgm"};
    const char* uris[1] = {"http://ns.adobe.com/hdr-gain-map/1.0/"};
    const SkDOM::Node* node = FindXmpNamespaceUriMatch(dom, namespaces, uris, 1);
    if (!node) {
        return false;
    }

    // Initialize the parameters to their defaults.
    SkColor4f gainMapMin = {0.f, 0.f, 0.f, 1.f};
    SkColor4f gainMapMax = {1.f, 1.f, 1.f, 1.f};
    SkColor4f gamma = {0.f, 0.f, 0.f, 1.f};
    SkColor4f offsetSdr = {1.f / 64.f, 1.f / 64.f, 1.f / 64.f, 0.f};
    SkColor4f offsetHdr = {1.f / 64.f, 1.f / 64.f, 1.f / 64.f, 0.f};
    SkScalar hdrCapacityMin = 0.f;
    SkScalar hdrCapacityMax = 1.f;

    // Read all parameters that are present.
    const char* baseRendition = dom.findAttr(node, "hdrgm:BaseRendition");
    find_per_channel_attr(dom, node, "hdrgm:GainMapMin", &gainMapMin);
    find_per_channel_attr(dom, node, "hdrgm:GainMapMax", &gainMapMax);
    find_per_channel_attr(dom, node, "hdrgm:Gamma", &gamma);
    find_per_channel_attr(dom, node, "hdrgm:OffsetSDR", &offsetSdr);
    find_per_channel_attr(dom, node, "hdrgm:OffsetHDR", &offsetHdr);
    dom.findScalar(node, "hdrgm:HDRCapacityMin", &hdrCapacityMin);
    dom.findScalar(node, "hdrgm:HDRCapacityMax", &hdrCapacityMax);

    // Translate all parameters to SkGainmapInfo's expected format.
    // TODO(ccameron): Move all of SkGainmapInfo to linear space.
    const float kLog2 = sk_float_log(2.f);
    outGainmapInfo->fLogRatioMin = {
            gainMapMin.fR * kLog2, gainMapMin.fG * kLog2, gainMapMin.fB * kLog2, 1.f};
    outGainmapInfo->fLogRatioMax = {
            gainMapMax.fR * kLog2, gainMapMax.fG * kLog2, gainMapMax.fB * kLog2, 1.f};
    outGainmapInfo->fGainmapGamma = gamma;
    // TODO(ccameron): Use SkColor4f for epsilons.
    outGainmapInfo->fEpsilonSdr = (offsetSdr.fR + offsetSdr.fG + offsetSdr.fB) / 3.f;
    outGainmapInfo->fEpsilonHdr = (offsetHdr.fR + offsetHdr.fG + offsetHdr.fB) / 3.f;
    outGainmapInfo->fHdrRatioMin = sk_float_exp(hdrCapacityMin * kLog2);
    outGainmapInfo->fHdrRatioMax = sk_float_exp(hdrCapacityMax * kLog2);
    if (baseRendition && !strcmp(baseRendition, "HDR")) {
        outGainmapInfo->fBaseImageType = SkGainmapInfo::BaseImageType::kHDR;
    } else {
        outGainmapInfo->fBaseImageType = SkGainmapInfo::BaseImageType::kSDR;
    }
    outGainmapInfo->fType = SkGainmapInfo::Type::kHDRGM;
    return true;
}
