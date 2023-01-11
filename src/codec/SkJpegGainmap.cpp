/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkJpegGainmap.h"

#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "include/private/SkFloatingPoint.h"
#include "include/private/SkGainmapInfo.h"
#include "include/utils/SkParse.h"
#include "src/codec/SkCodecPriv.h"
#include "src/codec/SkJpegMultiPicture.h"
#include "src/codec/SkJpegPriv.h"
#include "src/codec/SkJpegSegmentScan.h"
#include "src/xml/SkDOM.h"

#include <cstdint>
#include <cstring>
#include <utility>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkStream helpers.

/*
 * Class that will will rewind an SkStream, and then restore it to its original position when it
 * goes out of scope. If the SkStream is not seekable, then the stream will not be altered at all,
 * and will return false from canRestore.
 */

class ScopedSkStreamRestorer {
public:
    ScopedSkStreamRestorer(SkStream* stream)
            : fStream(stream), fPosition(stream->hasPosition() ? stream->getPosition() : 0) {
        if (canRestore()) {
            if (!fStream->rewind()) {
                SkCodecPrintf("Failed to rewind decoder stream.\n");
            }
        }
    }
    ~ScopedSkStreamRestorer() {
        if (canRestore()) {
            if (!fStream->seek(fPosition)) {
                SkCodecPrintf("Failed to restore decoder stream.\n");
            }
        }
    }
    bool canRestore() const { return fStream->hasPosition(); }

private:
    SkStream* const fStream;
    const size_t fPosition;
};

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
                                  SkStream* decoderStream,
                                  SkGainmapInfo* outInfo,
                                  std::unique_ptr<SkStream>* outGainmapImageStream) {
    // The decoder has already scanned for MPF metadata. If it doesn't exist, or it doesn't parse,
    // then early-out.
    if (!decoderMpfMetadata || !SkJpegParseMultiPicture(decoderMpfMetadata)) {
        return false;
    }

    // The implementation of Multi-Picture images requires a seekable stream. Save the position so
    // that it can be restored before returning.
    ScopedSkStreamRestorer streamRestorer(decoderStream);
    if (!streamRestorer.canRestore()) {
        SkCodecPrintf("Multi-Picture gainmap extraction requires a seekable stream.\n");
        return false;
    }

    // Scan the original decoder stream.
    auto scan = SkJpegSegmentScan::Create(decoderStream, SkJpegSegmentScan::Options());
    if (!scan) {
        SkCodecPrintf("Failed to scan decoder stream.\n");
        return false;
    }

    // Extract the Multi-Picture image streams in the original decoder stream (we needed the scan to
    // find the offsets of the MP images within the original decoder stream).
    auto mpStreams = SkJpegExtractMultiPictureStreams(scan.get());
    if (!mpStreams) {
        SkCodecPrintf("Failed to extract MP image streams.\n");
        return false;
    }

    // Iterate over the MP image streams.
    for (auto& mpImage : mpStreams->images) {
        if (!mpImage.stream) {
            continue;
        }

        // Create a scan of this MP image.
        auto mpImageScan =
                SkJpegSegmentScan::Create(mpImage.stream.get(), SkJpegSegmentScan::Options());
        if (!mpImageScan) {
            SkCodecPrintf("Failed to can MP image.\n");
            continue;
        }

        // Search for the XMP metadata in the MP image's scan.
        for (const auto& segment : mpImageScan->segments()) {
            if (segment.marker != kXMPMarker) {
                continue;
            }
            auto xmpMetadata = mpImageScan->copyParameters(segment, kXMPSig, sizeof(kXMPSig));
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
            return true;
        }
    }
    return false;
}
