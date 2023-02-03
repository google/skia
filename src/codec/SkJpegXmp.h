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

    // Find an XMP node that has the specified namespace attributes set to the specified URIs. The
    // XMP that this will search for is as follows (NAMESPACEi and URIi being the parameters
    // specified to this function):
    //
    //   <x:xmpmeta ...>
    //     <rdf:RDF ...>
    //       <rdf:Description NAMESPACE0="URI0" NAMESPACE1="URI1" .../>
    //     </rdf:RDF>
    //   </x:xmpmeta>
    //
    // This function will sequentially search the standard XMP, followed by the extended XMP (which
    // is not correct behavior -- it should merge the two XMP trees and search the merged tree).
    bool findNamespaceUriMatch(const char* namespaces[],
                               const char* uris[],
                               size_t count,
                               const SkDOM** outDom,
                               const SkDOM::Node** outNode) const;

private:
    SkJpegXmp();

    // The DOM for the standard XMP.
    SkDOM fStandardDOM;

    // The DOM for the extended XMP. This may be invalid if there is no extended XMP, or the
    // extended XMP failed to parse.
    SkDOM fExtendedDOM;
};

#endif
