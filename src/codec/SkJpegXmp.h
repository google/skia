/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJpegXmp_codec_DEFINED
#define SkJpegXmp_codec_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/xml/SkDOM.h"

class SkData;
struct SkGainmapInfo;

#include <memory>
#include <vector>

/*
 * A structure to manage JPEG XMP metadata.
 */
class SkJpegXmp {
public:
    // Find and parse all XMP metadata, given a list of all APP1 segment parameters.
    static std::unique_ptr<SkJpegXmp> Make(const std::vector<sk_sp<SkData>>& decoderApp1Params);

    // Extract HDRGM gainmap parameters.
    bool getGainmapInfoHDRGM(SkGainmapInfo* info) const;

    // Extract HDRGainMap gainmap parameters.
    bool getGainmapInfoHDRGainMap(SkGainmapInfo* info) const;

    // If this includes GContainer metadata and the GContainer contains an item with semantic
    // RecoveryMap and Mime of image/jpeg, then return true, and populate |offset| and |size| with
    // that item's offset (from the end of the primary JPEG image's EndOfImage), and the size of
    // the gainmap.
    bool getContainerGainmapLocation(size_t* offset, size_t* size) const;

private:
    SkJpegXmp();

    // Find an XMP node that assigns namespaces to the specified URIs. The XMP that this will search
    // for is as follows. URIi is the input parameters in |uris|, and NAMESPACEi is the output
    // written to |outNamespaces|. The output NAMESPACEi strings will always start with the prefix
    // "xmlns:".
    //
    //   <x:xmpmeta ...>
    //     <rdf:RDF ...>
    //       <rdf:Description NAMESPACE0="URI0" NAMESPACE1="URI1" .../>
    //     </rdf:RDF>
    //   </x:xmpmeta>
    //
    // This function will sequentially search the standard XMP, followed by the extended XMP (which
    // is not correct behavior -- it should merge the two XMP trees and search the merged tree).
    bool findUriNamespaces(size_t count,
                           const char* uris[],
                           const char* outNamespaces[],
                           const SkDOM** outDom,
                           const SkDOM::Node** outNode) const;

    // The DOM for the standard XMP.
    SkDOM fStandardDOM;

    // The DOM for the extended XMP. This may be invalid if there is no extended XMP, or the
    // extended XMP failed to parse.
    SkDOM fExtendedDOM;
};

#endif
