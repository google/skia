/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if defined(SK_XML)
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkGainmapInfo.h"
#include "include/private/SkXmp.h"
#include "tests/Test.h"

#include <cstddef>
#include <memory>

DEF_TEST(SkXmp_invalidXml, r) {
    // Invalid truncated xml.
    const char xmpData[] = R"(
            <x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 6.0.0">
               <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
                        xmlns:)";

    sk_sp<SkData> app1Param = SkData::MakeWithoutCopy(xmpData, sizeof(xmpData) - 1);

    auto xmp = SkXmp::Make(app1Param);
    REPORTER_ASSERT(r, xmp == nullptr);
}

DEF_TEST(SkXmp_xmpHdrgmAsFieldValue, r) {
    // Expose HDRM values as fields. Also place the HDRGM namespace in the rdf:RDF node.
    const char xmpData[] = R"(
            <x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 6.0.0">
               <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
                        xmlns:hdrgm="http://ns.adobe.com/hdr-gain-map/1.0/">
                  <rdf:Description rdf:about="">
                     <hdrgm:Version>1.0</hdrgm:Version>
                     <hdrgm:GainMapMax>3</hdrgm:GainMapMax>
                     <hdrgm:HDRCapacityMax>4</hdrgm:HDRCapacityMax>
                  </rdf:Description>
               </rdf:RDF>
            </x:xmpmeta>)";

    sk_sp<SkData> app1Param = SkData::MakeWithoutCopy(xmpData, sizeof(xmpData) - 1);

    auto xmp = SkXmp::Make(app1Param);
    REPORTER_ASSERT(r, xmp);

    SkGainmapInfo info;
    REPORTER_ASSERT(r, xmp->getGainmapInfoHDRGM(&info));
    REPORTER_ASSERT(r, info.fGainmapRatioMax.fR == 8.f);
    REPORTER_ASSERT(r, info.fDisplayRatioHdr == 16.f);
}

DEF_TEST(SkXmp_xmpHdrgmRequiresVersion, r) {
    // Same as the above, except with Version being absent.
    const char xmpData[] = R"(
            <x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 6.0.0">
               <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
                        xmlns:hdrgm="http://ns.adobe.com/hdr-gain-map/1.0/">
                  <rdf:Description rdf:about="">
                     <hdrgm:GainMapMax>3</hdrgm:GainMapMax>
                     <hdrgm:HDRCapacityMax>4</hdrgm:HDRCapacityMax>
                  </rdf:Description>
               </rdf:RDF>
            </x:xmpmeta>)";

    sk_sp<SkData> app1Param = SkData::MakeWithoutCopy(xmpData, sizeof(xmpData) - 1);

    auto xmp = SkXmp::Make(app1Param);
    REPORTER_ASSERT(r, xmp);

    SkGainmapInfo info;
    REPORTER_ASSERT(r, !xmp->getGainmapInfoHDRGM(&info));
}

DEF_TEST(SkXmp_xmpHdrgmAsDescriptionPropertyAttributes, r) {
    // Expose HDRGM values as attributes on an rdf:Description node.
    const char xmpData[] = R"(
            <x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 6.0.0">
               <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
                  <rdf:Description rdf:about=""
                        xmlns:hdrgm="http://ns.adobe.com/hdr-gain-map/1.0/"
                     hdrgm:Version="1.0"
                     hdrgm:GainMapMax="3"
                     hdrgm:HDRCapacityMax="4"/>
               </rdf:RDF>
            </x:xmpmeta>)";

    sk_sp<SkData> app1Param = SkData::MakeWithoutCopy(xmpData, sizeof(xmpData) - 1);

    auto xmp = SkXmp::Make(app1Param);
    REPORTER_ASSERT(r, xmp);

    SkGainmapInfo info;
    REPORTER_ASSERT(r, xmp->getGainmapInfoHDRGM(&info));
    REPORTER_ASSERT(r, info.fGainmapRatioMax.fR == 8.f);
    REPORTER_ASSERT(r, info.fDisplayRatioHdr == 16.f);
}

// Test mixed list and non-list entries.
DEF_TEST(SkXmp_xmpHdrgmList, r) {
    const char xmpData[] = R"(
            <x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 6.0.0">
               <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
                        xmlns:hdrgm="http://ns.adobe.com/hdr-gain-map/1.0/">
                  <rdf:Description rdf:about=""
                     hdrgm:Version="1.0"
                     hdrgm:GainMapMin="2.0"
                     hdrgm:OffsetSDR="0.1">
                     <hdrgm:GainMapMax>
                       <rdf:Seq>
                         <rdf:li>3</rdf:li>
                         <rdf:li>4</rdf:li>
                         <rdf:li>5</rdf:li>
                       </rdf:Seq>
                     </hdrgm:GainMapMax>
                     <hdrgm:Gamma>
                       1.2
                     </hdrgm:Gamma>
                     <hdrgm:OffsetHDR>
                       <rdf:Seq>
                         <rdf:li>
                           0.2
                         </rdf:li>
                         <rdf:li>
                           0.3
                         </rdf:li>
                         <rdf:li>
                           0.4
                         </rdf:li>
                       </rdf:Seq>
                     </hdrgm:OffsetHDR>
                  </rdf:Description>
               </rdf:RDF>
            </x:xmpmeta>)";

    sk_sp<SkData> app1Param = SkData::MakeWithoutCopy(xmpData, sizeof(xmpData) - 1);

    auto xmp = SkXmp::Make(app1Param);
    REPORTER_ASSERT(r, xmp);

    SkGainmapInfo info;
    REPORTER_ASSERT(r, xmp->getGainmapInfoHDRGM(&info));
    REPORTER_ASSERT(r, info.fGainmapRatioMin.fR == 4.f);
    REPORTER_ASSERT(r, info.fGainmapRatioMin.fG == 4.f);
    REPORTER_ASSERT(r, info.fGainmapRatioMin.fB == 4.f);
    REPORTER_ASSERT(r, info.fGainmapRatioMax.fR == 8.f);
    REPORTER_ASSERT(r, info.fGainmapRatioMax.fG == 16.f);
    REPORTER_ASSERT(r, info.fGainmapRatioMax.fB == 32.f);

    REPORTER_ASSERT(r, info.fGainmapGamma.fR == 1.f/1.2f);
    REPORTER_ASSERT(r, info.fGainmapGamma.fG == 1.f/1.2f);
    REPORTER_ASSERT(r, info.fGainmapGamma.fB == 1.f/1.2f);

    REPORTER_ASSERT(r, info.fEpsilonSdr.fR == 0.1f);
    REPORTER_ASSERT(r, info.fEpsilonSdr.fG == 0.1f);
    REPORTER_ASSERT(r, info.fEpsilonSdr.fB == 0.1f);

    REPORTER_ASSERT(r, info.fEpsilonHdr.fR == 0.2f);
    REPORTER_ASSERT(r, info.fEpsilonHdr.fG == 0.3f);
    REPORTER_ASSERT(r, info.fEpsilonHdr.fB == 0.4f);
}

DEF_TEST(SkXmp_xmpContainerTypedNode, r) {
    // Container and Item using a node of type Container:Item.
    const char xmpData[] = R"(
            <x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 5.5.0">
             <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
              <rdf:Description rdf:about=""
                xmlns:Container="http://ns.google.com/photos/1.0/container/"
                xmlns:Item="http://ns.google.com/photos/1.0/container/item/">
               <Container:Directory>
                <rdf:Seq>
                 <rdf:li rdf:parseType="Resource">
                  <Container:Item>
                   <Item:Mime>image/jpeg</Item:Mime>
                   <Item:Semantic>Primary</Item:Semantic>
                  </Container:Item>
                 </rdf:li>
                 <rdf:li rdf:parseType="Resource">
                  <Container:Item
                     Item:Semantic="GainMap"
                     Item:Mime="image/jpeg"
                     Item:Length="49035"/>
                 </rdf:li>
                </rdf:Seq>
               </Container:Directory>
              </rdf:Description>
             </rdf:RDF>
            </x:xmpmeta>)";
    sk_sp<SkData> app1Param = SkData::MakeWithoutCopy(xmpData, sizeof(xmpData) - 1);

    auto xmp = SkXmp::Make(app1Param);
    REPORTER_ASSERT(r, xmp);

    size_t offset = 999;
    size_t size = 999;
    REPORTER_ASSERT(r, xmp->getContainerGainmapLocation(&offset, &size));
    REPORTER_ASSERT(r, size == 49035);
}

DEF_TEST(SkXmp_xmpContainerTypedNodeRdfEquivalent, r) {
    // Container and Item using rdf:value and rdf:type pairs.
    const char xmpData[] = R"(
            <x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 5.5.0">
             <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
              <rdf:Description rdf:about=""
                xmlns:Container="http://ns.google.com/photos/1.0/container/"
                xmlns:Item="http://ns.google.com/photos/1.0/container/item/">
               <Container:Directory>
                <rdf:Seq>
                 <rdf:li rdf:parseType="Resource">
                  <rdf:value rdf:parseType="Resource">
                   <Item:Mime>image/jpeg</Item:Mime>
                   <Item:Semantic>Primary</Item:Semantic>
                  </rdf:value>
                  <rdf:type rdf:resource="Item"/>
                 </rdf:li>
                 <rdf:li rdf:parseType="Resource">
                  <rdf:value rdf:parseType="Resource">
                   <Item:Semantic>GainMap</Item:Semantic>
                   <Item:Mime>image/jpeg</Item:Mime>
                   <Item:Length>49035</Item:Length>
                  </rdf:value>
                  <rdf:type rdf:resource="Item"/>
                 </rdf:li>
                </rdf:Seq>
               </Container:Directory>
              </rdf:Description>
             </rdf:RDF>
            </x:xmpmeta>)";
    sk_sp<SkData> app1Param = SkData::MakeWithoutCopy(xmpData, sizeof(xmpData) - 1);

    auto xmp = SkXmp::Make(app1Param);
    REPORTER_ASSERT(r, xmp);

    size_t offset = 999;
    size_t size = 999;
    REPORTER_ASSERT(r, xmp->getContainerGainmapLocation(&offset, &size));
    REPORTER_ASSERT(r, size == 49035);
}
#endif  // SK_XML
