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
#include "src/codec/SkJpegConstants.h"
#include "src/core/SkMD5.h"
#include "src/xml/SkDOM.h"

#include <string>

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

const char* kXmlnsPrefix = "xmlns:";
const size_t kXmlnsPrefixLength = 6;

static const char* get_namespace_prefix(const char* name) {
    if (strlen(name) <= kXmlnsPrefixLength) {
        return nullptr;
    }
    return name + kXmlnsPrefixLength;
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
                                         const std::string& childName) {
    // Fail if there are multiple children with childName.
    if (dom.countChildren(node, childName.c_str()) != 1) {
        return nullptr;
    }
    const auto* child = dom.getFirstChild(node, childName.c_str());
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

/*
 * Given a node, find a child node of the specified type.
 *
 * If there exists a child node with name |prefix| + ":" + |type|, then return that child.
 *
 * If there exists a child node with name "rdf:type" that has attribute "rdf:resource" with value
 * of |type|, then if there also exists a child node with name "rdf:value" with attribute
 * "rdf:parseType" of "Resource", then return that child node with name "rdf:value". See Example
 * 3 in section 7.9.2.5: RDF Typed Nodes.
 * TODO(ccameron): This should also accept a URI for the type.
 */
static const SkDOM::Node* get_typed_child(const SkDOM* dom,
                                          const SkDOM::Node* node,
                                          const std::string& prefix,
                                          const std::string& type) {
    const auto name = prefix + std::string(":") + type;
    const SkDOM::Node* child = dom->getFirstChild(node, name.c_str());
    if (child) {
        return child;
    }

    const SkDOM::Node* typeChild = dom->getFirstChild(node, "rdf:type");
    if (!typeChild) {
        return nullptr;
    }
    const char* typeChildResource = dom->findAttr(typeChild, "rdf:resource");
    if (!typeChildResource || typeChildResource != type) {
        return nullptr;
    }

    const SkDOM::Node* valueChild = dom->getFirstChild(node, "rdf:value");
    if (!valueChild) {
        return nullptr;
    }
    const char* valueChildParseType = dom->findAttr(valueChild, "rdf:parseType");
    if (!valueChildParseType || strcmp(valueChildParseType, "Resource") != 0) {
        return nullptr;
    }
    return valueChild;
}

/*
 * Given a node, return its value for the specified attribute.
 *
 * This will first look for an attribute with the name |prefix| + ":" + |key|, and return the value
 * for that attribute.
 *
 * This will then look for a child node of name |prefix| + ":" + |key|, and return the field value
 * for that child.
 */
static const char* get_attr(const SkDOM* dom,
                            const SkDOM::Node* node,
                            const std::string& prefix,
                            const std::string& key) {
    const auto name = prefix + ":" + key;
    const char* attr = dom->findAttr(node, name.c_str());
    if (attr) {
        return attr;
    }
    return get_unique_child_text(*dom, node, name);
}

// Perform get_attr and parse the result as an int32_t.
static bool get_attr_int32(const SkDOM* dom,
                           const SkDOM::Node* node,
                           const std::string& prefix,
                           const std::string& key,
                           int32_t* value) {
    const char* attr = get_attr(dom, node, prefix, key);
    if (!attr) {
        return false;
    }
    if (!SkParse::FindS32(attr, value)) {
        return false;
    }
    return true;
}

// Perform get_attr and parse the result as a float.
static bool get_attr_float(const SkDOM* dom,
                           const SkDOM::Node* node,
                           const std::string& prefix,
                           const std::string& key,
                           float* outValue) {
    const char* attr = get_attr(dom, node, prefix, key);
    if (!attr) {
        return false;
    }
    SkScalar value = 0.f;
    if (SkParse::FindScalar(attr, &value)) {
        *outValue = value;
        return true;
    }
    return false;
}

// Perform get_attr and parse the result as three comma-separated floats. Return the result as an
// SkColor4f with the alpha component set to 1.
static bool get_attr_float3(const SkDOM* dom,
                            const SkDOM::Node* node,
                            const std::string& prefix,
                            const std::string& key,
                            SkColor4f* outValue) {
    const char* attr = get_attr(dom, node, prefix, key);
    if (!attr) {
        return false;
    }
    SkScalar values[3] = {0.f, 0.f, 0.f};
    if (SkParse::FindScalars(attr, values, 3)) {
        *outValue = {values[0], values[1], values[2], 1.f};
        return true;
    }
    if (SkParse::FindScalars(attr, values, 1)) {
        *outValue = {values[0], values[0], values[0], 1.f};
        return true;
    }
    return false;
}

static void find_uri_namespaces(const SkDOM& dom,
                                const SkDOM::Node* node,
                                size_t count,
                                const char* uris[],
                                const char* outNamespaces[]) {
    // Search all attributes for xmlns:NAMESPACEi="URIi".
    for (const auto* attr = dom.getFirstAttr(node); attr; attr = dom.getNextAttr(node, attr)) {
        const char* attrName = dom.getAttrName(node, attr);
        const char* attrValue = dom.getAttrValue(node, attr);
        if (!attrName || !attrValue) {
            continue;
        }
        // Make sure the name starts with "xmlns:".
        if (strlen(attrName) <= kXmlnsPrefixLength) {
            continue;
        }
        if (memcmp(attrName, kXmlnsPrefix, kXmlnsPrefixLength) != 0) {
            continue;
        }
        // Search for a requested URI that matches.
        for (size_t i = 0; i < count; ++i) {
            if (strcmp(attrValue, uris[i]) != 0) {
                continue;
            }
            outNamespaces[i] = attrName;
        }
    }
}

// See SkJpegXmp::findUriNamespaces. This function has the same behavior, but only searches
// a single SkDOM.
static const SkDOM::Node* find_uri_namespaces(const SkDOM& dom,
                                              size_t count,
                                              const char* uris[],
                                              const char* outNamespaces[]) {
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
        std::vector<const char*> rdfNamespaces(count, nullptr);
        find_uri_namespaces(dom, rdf, count, uris, rdfNamespaces.data());

        // Iterate the children with name rdf::Description.
        const char* kDesc = "rdf:Description";
        for (const auto* desc = dom.getFirstChild(rdf, kDesc); desc;
             desc = dom.getNextSibling(desc, kDesc)) {
            std::vector<const char*> descNamespaces = rdfNamespaces;
            find_uri_namespaces(dom, desc, count, uris, descNamespaces.data());

            // If we have a match for all the requested URIs, return.
            bool foundAllUris = true;
            for (size_t i = 0; i < count; ++i) {
                if (!descNamespaces[i]) {
                    foundAllUris = false;
                    break;
                }
            }
            if (foundAllUris) {
                for (size_t i = 0; i < count; ++i) {
                    outNamespaces[i] = descNamespaces[i];
                }
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
    const char* namespaces[1] = {nullptr};
    const char* uris[1] = {"http://ns.adobe.com/xmp/note/"};
    const auto* extendedNode = find_uri_namespaces(xmp->fStandardDOM, 1, uris, namespaces);
    if (!extendedNode) {
        return xmp;
    }
    const auto xmpNotePrefix = get_namespace_prefix(namespaces[0]);

    // Extract the GUID (the MD5 hash) of the extended metadata.
    const char* extendedGuid =
            get_attr(&xmp->fStandardDOM, extendedNode, xmpNotePrefix, "HasExtendedXMP");
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

bool SkJpegXmp::findUriNamespaces(size_t count,
                                  const char* uris[],
                                  const char* outNamespaces[],
                                  const SkDOM** outDom,
                                  const SkDOM::Node** outNode) const {
    // See XMP Specification Part 3: Storage in files, Section 1.1.3.1: Extended XMP in JPEG:
    // A JPEG reader must recompose the StandardXMP and ExtendedXMP into a single data model tree
    // containing all of the XMP for the JPEG file, and remove the xmpNote:HasExtendedXMP property.
    // This code does not do that. Instead, it maintains the two separate trees and searches them
    // sequentially.
    *outNode = find_uri_namespaces(fStandardDOM, count, uris, outNamespaces);
    if (*outNode) {
        *outDom = &fStandardDOM;
        return true;
    }
    *outNode = find_uri_namespaces(fExtendedDOM, count, uris, outNamespaces);
    if (*outNode) {
        *outDom = &fExtendedDOM;
        return true;
    }
    *outDom = nullptr;
    return false;
}

bool SkJpegXmp::getContainerGainmapLocation(size_t* outOffset, size_t* outSize) const {
    // Find a node that matches the requested namespaces and URIs.
    const char* namespaces[2] = {nullptr, nullptr};
    const char* uris[2] = {"http://ns.google.com/photos/1.0/container/",
                           "http://ns.google.com/photos/1.0/container/item/"};
    const SkDOM* dom = nullptr;
    const SkDOM::Node* node = nullptr;
    if (!findUriNamespaces(2, uris, namespaces, &dom, &node)) {
        return false;
    }
    const char* containerPrefix = get_namespace_prefix(namespaces[0]);
    const char* itemPrefix = get_namespace_prefix(namespaces[1]);

    // The node must have a Container:Directory child.
    const auto* directory = get_typed_child(dom, node, containerPrefix, "Directory");
    if (!directory) {
        SkCodecPrintf("Missing Container Directory");
        return false;
    }

    // That Container:Directory must have a sequence of  items.
    const auto* seq = dom->getFirstChild(directory, "rdf:Seq");
    if (!seq) {
        SkCodecPrintf("Missing rdf:Seq");
        return false;
    }

    // Iterate through the items in the Container:Directory's sequence. Keep a running sum of the
    // Item:Length of all items that appear before the RecoveryMap.
    bool isFirstItem = true;
    size_t offset = 0;
    for (const auto* li = dom->getFirstChild(seq, "rdf:li"); li;
         li = dom->getNextSibling(li, "rdf:li")) {
        // Each list item must contain a Container:Item.
        const auto* item = get_typed_child(dom, li, containerPrefix, "Item");
        if (!item) {
            SkCodecPrintf("List item does not have container Item.\n");
            return false;
        }
        // A Semantic is required for every item.
        const char* itemSemantic = get_attr(dom, item, itemPrefix, "Semantic");
        if (!itemSemantic) {
            SkCodecPrintf("Item is missing Semantic.\n");
            return false;
        }
        // A Mime is required for every item.
        const char* itemMime = get_attr(dom, item, itemPrefix, "Mime");
        if (!itemMime) {
            SkCodecPrintf("Item is missing Mime.\n");
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
            // The first media item can contain a Padding attribute, which specifies additional
            // padding between the end of the encoded primary image and the beginning of the next
            // media item. Only the first media item can contain a Padding attribute.
            int32_t padding = 0;
            if (get_attr_int32(dom, item, itemPrefix, "Padding", &padding)) {
                if (padding < 0) {
                    SkCodecPrintf("Item padding must be non-negative.");
                    return false;
                }
                offset += padding;
            }
        } else {
            // A Length is required for all non-Primary items.
            int32_t length = 0;
            if (!get_attr_int32(dom, item, itemPrefix, "Length", &length)) {
                SkCodecPrintf("Item length is absent.");
                return false;
            }
            if (length < 0) {
                SkCodecPrintf("Item length must be non-negative.");
                return false;
            }
            // If this is not the recovery map, then read past it.
            if (strcmp(itemSemantic, "RecoveryMap") != 0) {
                offset += length;
                continue;
            }
            // The recovery map must have mime type image/jpeg in this implementation.
            if (strcmp(itemMime, "image/jpeg") != 0) {
                SkCodecPrintf("RecoveryMap does not report that it is image/jpeg.\n");
                return false;
            }

            // Populate the location in the file at which to find the gainmap image.
            *outOffset = offset;
            *outSize = length;
            return true;
        }
    }
    return false;
}

// Return true if the specified XMP metadata identifies this image as an HDR gainmap.
bool SkJpegXmp::getGainmapInfoHDRGainMap(SkGainmapInfo* info) const {
    // Find a node that matches the requested namespaces and URIs.
    const char* namespaces[2] = {nullptr, nullptr};
    const char* uris[2] = {"http://ns.apple.com/pixeldatainfo/1.0/",
                           "http://ns.apple.com/HDRGainMap/1.0/"};
    const SkDOM* dom = nullptr;
    const SkDOM::Node* node = nullptr;
    if (!findUriNamespaces(2, uris, namespaces, &dom, &node)) {
        return false;
    }
    const char* adpiPrefix = get_namespace_prefix(namespaces[0]);
    const char* hdrGainMapPrefix = get_namespace_prefix(namespaces[1]);

    const char* auxiliaryImageType = get_attr(dom, node, adpiPrefix, "AuxiliaryImageType");
    if (!auxiliaryImageType) {
        SkCodecPrintf("Did not find AuxiliaryImageType.\n");
        return false;
    }
    if (strcmp(auxiliaryImageType, "urn:com:apple:photo:2020:aux:hdrgainmap") != 0) {
        SkCodecPrintf("AuxiliaryImageType was not HDR gain map.\n");
        return false;
    }

    int32_t version = 0;
    if (!get_attr_int32(dom, node, hdrGainMapPrefix, "HDRGainMapVersion", &version)) {
        SkCodecPrintf("Did not find HDRGainMapVersion.\n");
        return false;
    }
    if (version != 65536) {
        SkCodecPrintf("HDRGainMapVersion was not 65536.\n");
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
    const char* namespaces[1] = {nullptr};
    const char* uris[1] = {"http://ns.adobe.com/hdr-gain-map/1.0/"};
    const SkDOM* dom = nullptr;
    const SkDOM::Node* node = nullptr;
    if (!findUriNamespaces(1, uris, namespaces, &dom, &node)) {
        return false;
    }
    const char* hdrgmPrefix = get_namespace_prefix(namespaces[0]);

    // Initialize the parameters to their defaults.
    SkColor4f gainMapMin = {1.f, 1.f, 1.f, 1.f};
    SkColor4f gainMapMax = {2.f, 2.f, 2.f, 1.f};
    SkColor4f gamma = {1.f, 1.f, 1.f, 1.f};
    SkColor4f offsetSdr = {1.f / 64.f, 1.f / 64.f, 1.f / 64.f, 0.f};
    SkColor4f offsetHdr = {1.f / 64.f, 1.f / 64.f, 1.f / 64.f, 0.f};
    SkScalar hdrCapacityMin = 1.f;
    SkScalar hdrCapacityMax = 2.f;

    // Read all parameters that are present.
    const char* baseRendition = get_attr(dom, node, hdrgmPrefix, "BaseRendition");
    get_attr_float3(dom, node, hdrgmPrefix, "GainMapMin", &gainMapMin);
    get_attr_float3(dom, node, hdrgmPrefix, "GainMapMax", &gainMapMax);
    get_attr_float3(dom, node, hdrgmPrefix, "Gamma", &gamma);
    get_attr_float3(dom, node, hdrgmPrefix, "OffsetSDR", &offsetSdr);
    get_attr_float3(dom, node, hdrgmPrefix, "OffsetHDR", &offsetHdr);
    get_attr_float(dom, node, hdrgmPrefix, "HDRCapacityMin", &hdrCapacityMin);
    get_attr_float(dom, node, hdrgmPrefix, "HDRCapacityMax", &hdrCapacityMax);

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
