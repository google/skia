/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkJpegXmp.h"

#include "include/private/SkGainmapInfo.h"
#include "include/utils/SkParse.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkJpegPriv.h"
#include "src/core/SkMD5.h"
#include "src/xml/SkDOM.h"

SkJpegXmp::SkJpegXmp() = default;

////////////////////////////////////////////////////////////////////////////////////////////////////
// XMP JPEG extraction helper functions

constexpr size_t kGuidAsciiSize = 32;

/*
 * Extract standard XMP metadata.
 *
 * See XMP Specification Part 3: Storage in files, Section 1.1.3: JPEG.
 */
static sk_sp<SkData> read_xmp_standard(const std::vector<sk_sp<SkData>>& decoderApp1Params) {
    constexpr size_t kSigSize = sizeof(kXMPStandardSig);
    // Iterate through the image's segments.
    for (const auto& params : decoderApp1Params) {
        // Skip segments that don't have the right marker, signature, or are too small.
        if (params->size() <= kSigSize) {
            continue;
        }
        if (memcmp(params->bytes(), kXMPStandardSig, kSigSize) != 0) {
            continue;
        }
        return SkData::MakeWithoutCopy(params->bytes() + kSigSize, params->size() - kSigSize);
    }
    return nullptr;
}

/*
 * Extract and validate extended XMP metadata.
 *
 * See XMP Specification Part 3: Storage in files, Section 1.1.3.1: Extended XMP in JPEG:
 * Each chunk is written into the JPEG file within a separate APP1 marker segment. Each ExtendedXMP
 * marker segment contains:
 *   - A null-terminated signature string
 *   - A 128-bit GUID stored as a 32-byte ASCII hex string, capital A-F, no null termination. The
 *     GUID is a 128-bit MD5 digest of the full ExtendedXMP serialization.
 *   - The full length of the ExtendedXMP serialization as a 32-bit unsigned integer.
 *   - The offset of this portion as a 32-bit unsigned integer.
 *   - The portion of the ExtendedXMP
 */
static sk_sp<SkData> read_xmp_extended(const std::vector<sk_sp<SkData>>& decoderApp1Params,
                                       const char* guidAscii) {
    constexpr size_t kSigSize = sizeof(kXMPExtendedSig);
    constexpr size_t kFullLengthSize = 4;
    constexpr size_t kOffsetSize = 4;
    constexpr size_t kHeaderSize = kSigSize + kGuidAsciiSize + kFullLengthSize + kOffsetSize;

    // Validate the provided ASCII guid.
    SkMD5::Digest guidAsDigest;
    if (strlen(guidAscii) != kGuidAsciiSize) {
        SkCodecPrintf("Invalid ASCII GUID size.\n");
        return nullptr;
    }
    for (size_t i = 0; i < kGuidAsciiSize; ++i) {
        uint8_t digit = 0;
        if (guidAscii[i] >= '0' && guidAscii[i] <= '9') {
            digit = guidAscii[i] - '0';
        } else if (guidAscii[i] >= 'A' && guidAscii[i] <= 'F') {
            digit = guidAscii[i] - 'A' + 10;
        } else {
            SkCodecPrintf("GUID is not upper-case hex.\n");
            return nullptr;
        }
        if (i % 2 == 0) {
            guidAsDigest.data[i / 2] = 16 * digit;
        } else {
            guidAsDigest.data[i / 2] += digit;
        }
    }

    // Iterate through the image's segments.
    uint32_t fullLength = 0;
    using Part = std::tuple<uint32_t, sk_sp<SkData>>;
    std::vector<Part> parts;
    for (const auto& params : decoderApp1Params) {
        // Skip segments that don't have the right marker, signature, or are too small.
        if (params->size() <= kHeaderSize) {
            continue;
        }
        if (memcmp(params->bytes(), kXMPExtendedSig, kSigSize) != 0) {
            continue;
        }

        // Ignore parts that do not match the expected GUID.
        const uint8_t* partGuidAscii = params->bytes() + kSigSize;
        if (memcmp(guidAscii, partGuidAscii, kGuidAsciiSize) != 0) {
            SkCodecPrintf("Ignoring unexpected GUID.\n");
            continue;
        }

        // Read the full length and the offset for this part.
        uint32_t partFullLength = 0;
        uint32_t partOffset = 0;
        const uint8_t* partFullLengthBytes = params->bytes() + kSigSize + kGuidAsciiSize;
        const uint8_t* partOffsetBytes =
                params->bytes() + kSigSize + kGuidAsciiSize + kFullLengthSize;
        for (size_t i = 0; i < 4; ++i) {
            partFullLength *= 256;
            partOffset *= 256;
            partFullLength += partFullLengthBytes[i];
            partOffset += partOffsetBytes[i];
        }

        // If this is the first part, set our global full length size.
        if (parts.empty()) {
            fullLength = partFullLength;
        }

        // Ensure all parts agree on the full length.
        if (partFullLength != fullLength) {
            SkCodecPrintf("Multiple parts had different total lengths.\n");
            return nullptr;
        }

        // Add it to the list.
        auto partData = SkData::MakeWithoutCopy(params->bytes() + kHeaderSize,
                                                params->size() - kHeaderSize);
        parts.push_back({partOffset, partData});
    }
    if (parts.empty() || fullLength == 0) {
        return nullptr;
    }

    // Sort the list of parts by offset.
    std::sort(parts.begin(), parts.end(), [](const Part& a, const Part& b) {
        return std::get<0>(a) < std::get<0>(b);
    });

    // Stitch the parts together. Fail if we find that they are not contiguous.
    auto xmpExtendedData = SkData::MakeUninitialized(fullLength);
    uint8_t* xmpExtendedBase = reinterpret_cast<uint8_t*>(xmpExtendedData->writable_data());
    uint8_t* xmpExtendedCurrent = xmpExtendedBase;
    SkMD5 md5;
    for (const auto& part : parts) {
        uint32_t currentOffset = static_cast<uint32_t>(xmpExtendedCurrent - xmpExtendedBase);
        uint32_t partOffset = std::get<0>(part);
        const sk_sp<SkData>& partData = std::get<1>(part);
        // Make sure the data is contiguous and doesn't overflow the buffer.
        if (partOffset != currentOffset) {
            SkCodecPrintf("XMP extension parts not contiguous\n");
            return nullptr;
        }
        if (partData->size() > fullLength - currentOffset) {
            SkCodecPrintf("XMP extension parts overflow\n");
            return nullptr;
        }
        memcpy(xmpExtendedCurrent, partData->data(), partData->size());
        xmpExtendedCurrent += partData->size();
    }
    // Make sure we wrote the full buffer.
    if (static_cast<uint32_t>(xmpExtendedCurrent - xmpExtendedBase) != fullLength) {
        SkCodecPrintf("XMP extension did not match full length.\n");
        return nullptr;
    }

    // Make sure the MD5 hash of the extended data matched the GUID.
    md5.write(xmpExtendedData->data(), xmpExtendedData->size());
    if (md5.finish() != guidAsDigest) {
        SkCodecPrintf("XMP extension did not hash to GUID.\n");
        return nullptr;
    }

    return xmpExtendedData;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// XMP parsing helper functions

/*
 * Helper function to read a 1 or 3 floats and write them into an SkColor4f.
 */
static void find_per_channel_attr(const SkDOM* dom,
                                  const SkDOM::Node* node,
                                  const char* attr,
                                  SkColor4f* outColor) {
    SkScalar values[3] = {0.f, 0.f, 0.f};
    if (dom->findScalars(node, attr, values, 3)) {
        *outColor = {values[0], values[1], values[2], 1.f};
    } else if (dom->findScalars(node, attr, values, 1)) {
        *outColor = {values[0], values[0], values[0], 1.f};
    }
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
static const char* get_unique_child_text(const SkDOM& dom,
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

// Helper function that builds on get_unique_child_text, returning true if the unique child with
// childName has inner text that matches an expected text.
static bool unique_child_text_matches(const SkDOM& dom,
                                      const SkDOM::Node* node,
                                      const char* childName,
                                      const char* expectedText) {
    const char* text = get_unique_child_text(dom, node, childName);
    if (text && !strcmp(text, expectedText)) {
        return true;
    }
    return false;
}

// Helper function that builds on get_unique_child_text, returning true if the unique child with
// childName has inner text that matches an expected integer.
static bool unique_child_text_matches(const SkDOM& dom,
                                      const SkDOM::Node* node,
                                      const char* childName,
                                      int32_t expectedValue) {
    const char* text = get_unique_child_text(dom, node, childName);
    int32_t actualValue = 0;
    if (text && SkParse::FindS32(text, &actualValue)) {
        return actualValue == expectedValue;
    }
    return false;
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
const SkDOM::Node* find_namespace_uri_match(const SkDOM& dom,
                                            const char* namespaces[],
                                            const char* uris[],
                                            size_t count) {
    const SkDOM::Node* root = dom.getRootNode();
    if (!root) {
        return nullptr;
    }

    // Ensure that the root node identifies itself as XMP metadata.
    const char* rootName = dom.getName(root);
    if (!rootName || strcmp(rootName, "x:xmpmeta") != 0) {
        return nullptr;
    }

    //  Iterate the children with name rdf:RDF.
    const char* kRdf = "rdf:RDF";
    for (const auto* rdf = dom.getFirstChild(root, kRdf); rdf;
         rdf = dom.getNextSibling(rdf, kRdf)) {
        // Iterate the children with name rdf::Description.
        const char* kDesc = "rdf:Description";
        for (const auto* desc = dom.getFirstChild(rdf, kDesc); desc;
             desc = dom.getNextSibling(desc, kDesc)) {
            // See if this node has the requested namespace-URI pairs as attributes.
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

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkJpegXmp

std::unique_ptr<SkJpegXmp> SkJpegXmp::Make(const std::vector<sk_sp<SkData>>& decoderApp1Params) {
    auto xmpStandard = read_xmp_standard(decoderApp1Params);
    if (!xmpStandard) {
        return nullptr;
    }

    std::unique_ptr<SkJpegXmp> xmp(new SkJpegXmp);
    auto xmpStandardStream = SkMemoryStream::Make(xmpStandard);
    if (!xmp->fStandardDOM.build(*xmpStandardStream)) {
        SkCodecPrintf("Failed to parse XMP standard metadata.\n");
        return nullptr;
    }

    // See if there is a note indicating extended XMP. If we encounter any errors in retrieving
    // the extended XMP, return just the standard XMP.
    const char* namespaces[1] = {"xmlns:xmpNote"};
    const char* uris[1] = {"http://ns.adobe.com/xmp/note/"};
    const auto* extendedNode = find_namespace_uri_match(xmp->fStandardDOM, namespaces, uris, 1);
    if (!extendedNode) {
        return xmp;
    }

    // Extract the GUID (the MD5 hash) of the extended metadata.
    const char* extendedGuid = xmp->fStandardDOM.findAttr(extendedNode, "xmpNote:HasExtendedXMP");
    if (!extendedGuid) {
        return xmp;
    }

    // Extract and validate the extended metadata from the JPEG structure.
    auto xmpExtended = read_xmp_extended(decoderApp1Params, extendedGuid);
    if (!xmpExtended) {
        SkCodecPrintf("Extended XMP was indicated but failed to read or validate.\n");
        return xmp;
    }

    // Parse the extended metadata.
    auto xmpExtendedStream = SkMemoryStream::Make(xmpExtended);
    if (xmp->fExtendedDOM.build(*xmpExtendedStream)) {
        SkCodecPrintf("Failed to parse extended XMP metadata.\n");
        return xmp;
    }

    return xmp;
}

bool SkJpegXmp::findNamespaceUriMatch(const char* namespaces[],
                                      const char* uris[],
                                      size_t count,
                                      const SkDOM** outDom,
                                      const SkDOM::Node** outNode) const {
    // See XMP Specification Part 3: Storage in files, Section 1.1.3.1: Extended XMP in JPEG:
    // A JPEG reader must recompose the StandardXMP and ExtendedXMP into a single data model tree
    // containing all of the XMP for the JPEG file, and remove the xmpNote:HasExtendedXMP property.
    // This code does not do that. Instead, it maintains the two separate trees and searches them
    // sequentially.
    *outNode = find_namespace_uri_match(fStandardDOM, namespaces, uris, count);
    if (*outNode) {
        *outDom = &fStandardDOM;
        return true;
    }
    *outNode = find_namespace_uri_match(fExtendedDOM, namespaces, uris, count);
    if (*outNode) {
        *outDom = &fExtendedDOM;
        return true;
    }
    *outDom = nullptr;
    return false;
}

bool SkJpegXmp::getGainmapInfoJpegR(SkGainmapInfo* outInfo,
                                    size_t* outOffset,
                                    size_t* outSize) const {
    // Find a node that matches the requested namespaces and URIs.
    const char* namespaces[2] = {"xmlns:GContainer", "xmlns:RecoveryMap"};
    const char* uris[2] = {"http://ns.google.com/photos/1.0/container/",
                           "http://ns.google.com/photos/1.0/recoverymap/"};
    const SkDOM* dom = nullptr;
    const SkDOM::Node* node = nullptr;
    if (!findNamespaceUriMatch(namespaces, uris, 1, &dom, &node)) {
        return false;
    }

    // The node must have a GContainer:Version child that specifies version 1.
    if (!unique_child_text_matches(*dom, node, "GContainer:Version", 1)) {
        SkCodecPrintf("GContainer:Version is absent or not 1");
        return false;
    }

    // The node must have a GContainer:Directory.
    const auto* directory = dom->getFirstChild(node, "GContainer:Directory");
    if (!directory) {
        SkCodecPrintf("Missing GContainer:Directory");
        return false;
    }

    // That GContainer:Directory must have a sequence of  items.
    const auto* seq = dom->getFirstChild(directory, "rdf:Seq");
    if (!seq) {
        SkCodecPrintf("Missing rdf:Seq");
        return false;
    }

    // Iterate through the items in the GContainer:Directory's sequence. Keep a running sum of the
    // GContainer::ItemLength of all items that appear before the RecoveryMap.
    bool isFirstItem = true;
    size_t itemLengthSum = 0;
    SkGainmapInfo::Type type = SkGainmapInfo::Type::kUnknown;
    SkScalar rangeScalingFactor = 1.f;
    for (const auto* li = dom->getFirstChild(seq, "rdf:li"); li;
         li = dom->getNextSibling(li, "rdf:li")) {
        // Each list item must contain a GContainer item.
        const auto* item = dom->getFirstChild(li, "GContainer:Item");
        if (!item) {
            SkCodecPrintf("List item does not have GContainer:Item.\n");
            return false;
        }
        // An ItemSemantic is required for every GContainer item.
        const char* itemSemantic = dom->findAttr(item, "GContainer:ItemSemantic");
        if (!itemSemantic) {
            SkCodecPrintf("GContainer item is missing ItemSemantic.\n");
            return false;
        }
        // An ItemMime is required for every GContainer item.
        const char* itemMime = dom->findAttr(item, "GContainer:ItemMime");
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
            if (!dom->hasAttr(item, "RecoveryMap:Version", "1")) {
                SkCodecPrintf("RecoveryMap:Version is not 1.");
                return false;
            }
            // The TransferFunction is required for the Primary.
            int32_t transferFunction = 0;
            if (!dom->findS32(item, "RecoveryMap:TransferFunction", &transferFunction)) {
                SkCodecPrintf("RecoveryMap:TransferFunction is absent.");
                return false;
            }
            switch (transferFunction) {
                case 0:
                    type = SkGainmapInfo::Type::kJpegR_Linear;
                    break;
                case 1:
                    type = SkGainmapInfo::Type::kJpegR_HLG;
                    break;
                case 2:
                    type = SkGainmapInfo::Type::kJpegR_PQ;
                    break;
                default:
                    SkCodecPrintf("RecoveryMap:TransferFunction is out of range.");
                    return false;
            }
            // The RangeScalingFactor is required for the Primary.
            if (!dom->findScalars(item, "RecoveryMap:RangeScalingFactor", &rangeScalingFactor, 1)) {
                SkCodecPrintf("RecoveryMap:RangeScalingFactor is absent.");
                return false;
            }
        } else {
            // An ItemLength is required for all non-Primary GContainter items.
            int32_t itemLength = 0;
            if (!dom->findS32(item, "GContainer:ItemLength", &itemLength)) {
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

            // Populate the SkGainmapInfo
            const float kRatioMax = rangeScalingFactor;
            const float kRatioMin = 1.f / rangeScalingFactor;
            outInfo->fGainmapRatioMin = {kRatioMin, kRatioMin, kRatioMin, 1.f};
            outInfo->fGainmapRatioMax = {kRatioMax, kRatioMax, kRatioMax, 1.f};
            outInfo->fGainmapGamma = {1.f, 1.f, 1.f, 1.f};
            outInfo->fEpsilonSdr = {0.f, 0.f, 0.f, 1.f};
            outInfo->fEpsilonHdr = {0.f, 0.f, 0.f, 1.f};
            outInfo->fDisplayRatioSdr = 1.f;
            outInfo->fDisplayRatioHdr = rangeScalingFactor;
            outInfo->fBaseImageType = SkGainmapInfo::BaseImageType::kSDR;
            outInfo->fType = type;

            // Populate the location in the file at which to find the gainmap image.
            *outOffset = itemLengthSum;
            *outSize = itemLength;
            return true;
        }
    }
    return false;
}

// Return true if the specified XMP metadata identifies this image as an HDR gainmap.
bool SkJpegXmp::getGainmapInfoHDRGainMap(SkGainmapInfo* info) const {
    // Find a node that matches the requested namespaces and URIs.
    const char* namespaces[2] = {"xmlns:apdi", "xmlns:HDRGainMap"};
    const char* uris[2] = {"http://ns.apple.com/pixeldatainfo/1.0/",
                           "http://ns.apple.com/HDRGainMap/1.0/"};
    const SkDOM* dom = nullptr;
    const SkDOM::Node* node = nullptr;
    if (!findNamespaceUriMatch(namespaces, uris, 2, &dom, &node)) {
        return false;
    }
    if (!unique_child_text_matches(
                *dom, node, "apdi:AuxiliaryImageType", "urn:com:apple:photo:2020:aux:hdrgainmap")) {
        SkCodecPrintf("Did not find auxiliary image type.\n");
        return false;
    }
    if (!unique_child_text_matches(*dom, node, "HDRGainMap:HDRGainMapVersion", 65536)) {
        SkCodecPrintf("HDRGainMapVersion absent or not 65536.\n");
        return false;
    }

    // This node will often have StoredFormat and NativeFormat children that have inner text that
    // specifies the integer 'L008' (also known as kCVPixelFormatType_OneComponent8).
    const float kRatioMax = sk_float_exp(1.f);
    info->fGainmapRatioMin = {1.f, 1.f, 1.f, 1.f};
    info->fGainmapRatioMax = {kRatioMax, kRatioMax, kRatioMax, 1.f};
    info->fGainmapGamma = {1.f, 1.f, 1.f, 1.f};
    info->fEpsilonSdr = {0.f, 0.f, 0.f, 1.f};
    info->fEpsilonHdr = {0.f, 0.f, 0.f, 1.f};
    info->fDisplayRatioSdr = 1.f;
    info->fDisplayRatioHdr = kRatioMax;
    info->fBaseImageType = SkGainmapInfo::BaseImageType::kSDR;
    info->fType = SkGainmapInfo::Type::kMultiPicture;
    return true;
}

bool SkJpegXmp::getGainmapInfoHDRGM(SkGainmapInfo* outGainmapInfo) const {
    // Find a node that matches the requested namespace and URI.
    const char* namespaces[1] = {"xmlns:hdrgm"};
    const char* uris[1] = {"http://ns.adobe.com/hdr-gain-map/1.0/"};
    const SkDOM* dom = nullptr;
    const SkDOM::Node* node = nullptr;
    if (!findNamespaceUriMatch(namespaces, uris, 1, &dom, &node)) {
        return false;
    }

    // Initialize the parameters to their defaults.
    SkColor4f gainMapMin = {1.f, 1.f, 1.f, 1.f};
    SkColor4f gainMapMax = {2.f, 2.f, 2.f, 1.f};
    SkColor4f gamma = {1.f, 1.f, 1.f, 1.f};
    SkColor4f offsetSdr = {1.f / 64.f, 1.f / 64.f, 1.f / 64.f, 0.f};
    SkColor4f offsetHdr = {1.f / 64.f, 1.f / 64.f, 1.f / 64.f, 0.f};
    SkScalar hdrCapacityMin = 1.f;
    SkScalar hdrCapacityMax = 2.f;

    // Read all parameters that are present.
    const char* baseRendition = dom->findAttr(node, "hdrgm:BaseRendition");
    find_per_channel_attr(dom, node, "hdrgm:GainMapMin", &gainMapMin);
    find_per_channel_attr(dom, node, "hdrgm:GainMapMax", &gainMapMax);
    find_per_channel_attr(dom, node, "hdrgm:Gamma", &gamma);
    find_per_channel_attr(dom, node, "hdrgm:OffsetSDR", &offsetSdr);
    find_per_channel_attr(dom, node, "hdrgm:OffsetHDR", &offsetHdr);
    dom->findScalar(node, "hdrgm:HDRCapacityMin", &hdrCapacityMin);
    dom->findScalar(node, "hdrgm:HDRCapacityMax", &hdrCapacityMax);

    // Translate all parameters to SkGainmapInfo's expected format.
    const float kLog2 = sk_float_log(2.f);
    outGainmapInfo->fGainmapRatioMin = {sk_float_exp(gainMapMin.fR * kLog2),
                                        sk_float_exp(gainMapMin.fG * kLog2),
                                        sk_float_exp(gainMapMin.fB * kLog2),
                                        1.f};
    outGainmapInfo->fGainmapRatioMax = {sk_float_exp(gainMapMax.fR * kLog2),
                                        sk_float_exp(gainMapMax.fG * kLog2),
                                        sk_float_exp(gainMapMax.fB * kLog2),
                                        1.f};
    outGainmapInfo->fGainmapGamma = gamma;
    outGainmapInfo->fEpsilonSdr = offsetSdr;
    outGainmapInfo->fEpsilonHdr = offsetHdr;
    outGainmapInfo->fDisplayRatioSdr = sk_float_exp(hdrCapacityMin * kLog2);
    outGainmapInfo->fDisplayRatioHdr = sk_float_exp(hdrCapacityMax * kLog2);
    if (baseRendition && !strcmp(baseRendition, "HDR")) {
        outGainmapInfo->fBaseImageType = SkGainmapInfo::BaseImageType::kHDR;
    } else {
        outGainmapInfo->fBaseImageType = SkGainmapInfo::BaseImageType::kSDR;
    }
    outGainmapInfo->fType = SkGainmapInfo::Type::kHDRGM;
    return true;
}
