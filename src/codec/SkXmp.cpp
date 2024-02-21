/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkXmp.h"

#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/private/SkGainmapInfo.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/utils/SkParse.h"
#include "src/codec/SkCodecPriv.h"
#include "src/xml/SkDOM.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <utility>

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

// Perform get_attr and parse the result as a bool.
static bool get_attr_bool(const SkDOM* dom,
                          const SkDOM::Node* node,
                          const std::string& prefix,
                          const std::string& key,
                          bool* outValue) {
    const char* attr = get_attr(dom, node, prefix, key);
    if (!attr) {
        return false;
    }
    switch (SkParse::FindList(attr, "False,True")) {
        case 0:
            *outValue = false;
            return true;
        case 1:
            *outValue = true;
            return true;
        default:
            break;
    }
    return false;
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
static bool get_attr_float3_as_list(const SkDOM* dom,
                                    const SkDOM::Node* node,
                                    const std::string& prefix,
                                    const std::string& key,
                                    SkColor4f* outValue) {
    const auto name = prefix + ":" + key;

    // Fail if there are multiple children with childName.
    if (dom->countChildren(node, name.c_str()) != 1) {
        return false;
    }
    // Find the child.
    const auto* child = dom->getFirstChild(node, name.c_str());
    if (!child) {
        return false;
    }

    // Search for the rdf:Seq child.
    const auto* seq = dom->getFirstChild(child, "rdf:Seq");
    if (!seq) {
        return false;
    }

    size_t count = 0;
    SkScalar values[3] = {0.f, 0.f, 0.f};
    for (const auto* liNode = dom->getFirstChild(seq, "rdf:li"); liNode;
         liNode = dom->getNextSibling(liNode, "rdf:li")) {
        if (count > 2) {
            SkCodecPrintf("Too many items in list.\n");
            return false;
        }
        if (dom->countChildren(liNode) != 1) {
            SkCodecPrintf("Item can only have one child.\n");
            return false;
        }
        const auto* liTextNode = dom->getFirstChild(liNode);
        if (dom->getType(liTextNode) != SkDOM::kText_Type) {
            SkCodecPrintf("Item's only child must be text.\n");
            return false;
        }
        const char* liText = dom->getName(liTextNode);
        if (!liText) {
            SkCodecPrintf("Failed to get item's text.\n");
            return false;
        }
        if (!SkParse::FindScalar(liText, values + count)) {
            SkCodecPrintf("Failed to parse item's text to float.\n");
            return false;
        }
        count += 1;
    }
    if (count < 3) {
        SkCodecPrintf("List didn't have enough items.\n");
        return false;
    }
    *outValue = {values[0], values[1], values[2], 1.f};
    return true;
}

static bool get_attr_float3(const SkDOM* dom,
                            const SkDOM::Node* node,
                            const std::string& prefix,
                            const std::string& key,
                            SkColor4f* outValue) {
    if (get_attr_float3_as_list(dom, node, prefix, key, outValue)) {
        return true;
    }
    SkScalar value = -1.0;
    if (get_attr_float(dom, node, prefix, key, &value)) {
        *outValue = {value, value, value, 1.f};
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

// See SkXmp::findUriNamespaces. This function has the same behavior, but only searches
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
// SkXmpImpl

class SkXmpImpl final : public SkXmp {
public:
    SkXmpImpl() = default;

    bool getGainmapInfoHDRGM(SkGainmapInfo* info) const override;
    bool getGainmapInfoHDRGainMap(SkGainmapInfo* info) const override;
    bool getContainerGainmapLocation(size_t* offset, size_t* size) const override;
    const char* getExtendedXmpGuid() const override;
    // Parse the given xmp data and store it into either the standard (main) DOM or the extended
    // DOM. Returns true on successful parsing.
    bool parseDom(sk_sp<SkData> xmpData, bool extended);

private:
    bool findUriNamespaces(size_t count,
                           const char* uris[],
                           const char* outNamespaces[],
                           const SkDOM** outDom,
                           const SkDOM::Node** outNode) const;

    SkDOM fStandardDOM;
    SkDOM fExtendedDOM;
};

const char* SkXmpImpl::getExtendedXmpGuid() const {
    const char* namespaces[1] = {nullptr};
    const char* uris[1] = {"http://ns.adobe.com/xmp/note/"};
    const auto* extendedNode = find_uri_namespaces(fStandardDOM, 1, uris, namespaces);
    if (!extendedNode) {
        return nullptr;
    }
    const auto xmpNotePrefix = get_namespace_prefix(namespaces[0]);
    // Extract the GUID (the MD5 hash) of the extended metadata.
    return get_attr(&fStandardDOM, extendedNode, xmpNotePrefix, "HasExtendedXMP");
}

bool SkXmpImpl::findUriNamespaces(size_t count,
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

bool SkXmpImpl::getContainerGainmapLocation(size_t* outOffset, size_t* outSize) const {
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
    // Item:Length of all items that appear before the GainMap.
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
            if (strcmp(itemSemantic, "GainMap") != 0) {
                offset += length;
                continue;
            }
            // The recovery map must have mime type image/jpeg in this implementation.
            if (strcmp(itemMime, "image/jpeg") != 0) {
                SkCodecPrintf("GainMap does not report that it is image/jpeg.\n");
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
bool SkXmpImpl::getGainmapInfoHDRGainMap(SkGainmapInfo* info) const {
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
    return true;
}

bool SkXmpImpl::getGainmapInfoHDRGM(SkGainmapInfo* outGainmapInfo) const {
    // Find a node that matches the requested namespace and URI.
    const char* namespaces[1] = {nullptr};
    const char* uris[1] = {"http://ns.adobe.com/hdr-gain-map/1.0/"};
    const SkDOM* dom = nullptr;
    const SkDOM::Node* node = nullptr;
    if (!findUriNamespaces(1, uris, namespaces, &dom, &node)) {
        return false;
    }
    const char* hdrgmPrefix = get_namespace_prefix(namespaces[0]);

    // Require that hdrgm:Version="1.0" be present.
    const char* version = get_attr(dom, node, hdrgmPrefix, "Version");
    if (!version) {
        SkCodecPrintf("Version attribute is absent.\n");
        return false;
    }
    if (strcmp(version, "1.0") != 0) {
        SkCodecPrintf("Version is \"%s\", not \"1.0\".\n", version);
        return false;
    }

    // Initialize the parameters to their defaults.
    bool baseRenditionIsHDR = false;
    SkColor4f gainMapMin = {0.f, 0.f, 0.f, 1.f};  // log2 value
    SkColor4f gainMapMax = {1.f, 1.f, 1.f, 1.f};  // log2 value
    SkColor4f gamma = {1.f, 1.f, 1.f, 1.f};
    SkColor4f offsetSdr = {1.f / 64.f, 1.f / 64.f, 1.f / 64.f, 0.f};
    SkColor4f offsetHdr = {1.f / 64.f, 1.f / 64.f, 1.f / 64.f, 0.f};
    SkScalar hdrCapacityMin = 0.f;  // log2 value
    SkScalar hdrCapacityMax = 1.f;  // log2 value

    // Read all parameters that are present.
    get_attr_bool(dom, node, hdrgmPrefix, "BaseRenditionIsHDR", &baseRenditionIsHDR);
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
    outGainmapInfo->fGainmapGamma = {1.f / gamma.fR, 1.f / gamma.fG, 1.f / gamma.fB, 1.f};
    outGainmapInfo->fEpsilonSdr = offsetSdr;
    outGainmapInfo->fEpsilonHdr = offsetHdr;
    outGainmapInfo->fDisplayRatioSdr = sk_float_exp(hdrCapacityMin * kLog2);
    outGainmapInfo->fDisplayRatioHdr = sk_float_exp(hdrCapacityMax * kLog2);
    if (baseRenditionIsHDR) {
        outGainmapInfo->fBaseImageType = SkGainmapInfo::BaseImageType::kHDR;
    } else {
        outGainmapInfo->fBaseImageType = SkGainmapInfo::BaseImageType::kSDR;
    }
    return true;
}

bool SkXmpImpl::parseDom(sk_sp<SkData> xmpData, bool extended) {
    SkDOM* dom = extended ? &fExtendedDOM : &fStandardDOM;
    auto xmpdStream = SkMemoryStream::Make(std::move(xmpData));
    if (!dom->build(*xmpdStream)) {
        SkCodecPrintf("Failed to parse XMP %s metadata.\n", extended ? "extended" : "standard");
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkXmp

std::unique_ptr<SkXmp> SkXmp::Make(sk_sp<SkData> xmpData) {
    std::unique_ptr<SkXmpImpl> xmp(new SkXmpImpl);
    if (!xmp->parseDom(std::move(xmpData), /*extended=*/false)) {
        return nullptr;
    }
    return xmp;
}

std::unique_ptr<SkXmp> SkXmp::Make(sk_sp<SkData> xmpStandard, sk_sp<SkData> xmpExtended) {
    std::unique_ptr<SkXmpImpl> xmp(new SkXmpImpl);
    if (!xmp->parseDom(std::move(xmpStandard), /*extended=*/false)) {
        return nullptr;
    }
    // Try to parse extended xmp but ignore the return value: if parsing fails, we'll still return
    // the standard xmp.
    (void)xmp->parseDom(std::move(xmpExtended), /*extended=*/true);
    return xmp;
}
