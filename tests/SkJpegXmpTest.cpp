/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/private/SkGainmapInfo.h"
#include "src/codec/SkJpegXmp.h"
#include "src/core/SkMD5.h"
#include "tests/Test.h"

#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>

DEF_TEST(SkJpegXmp_standardXmp, r) {
    const char xmpData[] =
            "http://ns.adobe.com/xap/1.0/\0"
            R"(
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

    std::vector<sk_sp<SkData>> app1Params;
    app1Params.push_back(SkData::MakeWithoutCopy(xmpData, sizeof(xmpData) - 1));

    auto xmp = SkJpegMakeXmp(app1Params);
    REPORTER_ASSERT(r, xmp);

    SkGainmapInfo info;
    REPORTER_ASSERT(r, xmp->getGainmapInfoHDRGM(&info));
    REPORTER_ASSERT(r, info.fGainmapRatioMax.fR == 8.f);
    REPORTER_ASSERT(r, info.fDisplayRatioHdr == 16.f);
}

DEF_TEST(SkJpegXmp_defaultValues, r) {
    const char xmpData[] =
            "http://ns.adobe.com/xap/1.0/\0"
            R"(
            <x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 6.0.0">
               <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
                        xmlns:hdrgm="http://ns.adobe.com/hdr-gain-map/1.0/">
                  <rdf:Description rdf:about="" hdrgm:Version="1.0">
                  </rdf:Description>
               </rdf:RDF>
            </x:xmpmeta>)";

    std::vector<sk_sp<SkData>> app1Params;
    app1Params.push_back(SkData::MakeWithoutCopy(xmpData, sizeof(xmpData) - 1));

    auto xmp = SkJpegMakeXmp(app1Params);
    REPORTER_ASSERT(r, xmp);

    SkGainmapInfo info;
    REPORTER_ASSERT(r, xmp->getGainmapInfoHDRGM(&info));
    REPORTER_ASSERT(r, info.fGainmapRatioMin.fR == 1.f);
    REPORTER_ASSERT(r, info.fGainmapRatioMin.fG == 1.f);
    REPORTER_ASSERT(r, info.fGainmapRatioMin.fB == 1.f);
    REPORTER_ASSERT(r, info.fGainmapRatioMax.fR == 2.f);
    REPORTER_ASSERT(r, info.fGainmapRatioMax.fG == 2.f);
    REPORTER_ASSERT(r, info.fGainmapRatioMax.fB == 2.f);
    REPORTER_ASSERT(r, info.fGainmapGamma.fR == 1.f);
    REPORTER_ASSERT(r, info.fGainmapGamma.fG == 1.f);
    REPORTER_ASSERT(r, info.fGainmapGamma.fB == 1.f);
    REPORTER_ASSERT(r, info.fEpsilonSdr.fR == 1.f / 64.f);
    REPORTER_ASSERT(r, info.fEpsilonSdr.fG == 1.f / 64.f);
    REPORTER_ASSERT(r, info.fEpsilonSdr.fB == 1.f / 64.f);
    REPORTER_ASSERT(r, info.fEpsilonHdr.fG == 1.f / 64.f);
    REPORTER_ASSERT(r, info.fEpsilonHdr.fR == 1.f / 64.f);
    REPORTER_ASSERT(r, info.fEpsilonHdr.fB == 1.f / 64.f);
    REPORTER_ASSERT(r, info.fDisplayRatioSdr == 1.f);
    REPORTER_ASSERT(r, info.fDisplayRatioHdr == 2.f);
}

static std::string uint32_to_string(uint32_t v) {
    const char c[4] = {static_cast<char>((v >> 24) & 0xff),
                       static_cast<char>((v >> 16) & 0xff),
                       static_cast<char>((v >> 8) & 0xff),
                       static_cast<char>(v & 0xff)};
    return std::string(c, 4);
}

static std::string standard_xmp_with_header(const SkMD5::Digest& digest, const std::string& data) {
    const std::string guid = digest.toHexString().c_str();
    const std::string dataWithGuid = std::regex_replace(data, std::regex("\\$GUID"), guid);
    return std::string("http://ns.adobe.com/xap/1.0/") + '\0' + dataWithGuid;
}

static std::string extended_xmp_with_header(const SkMD5::Digest& digest,
                                            uint32_t size,
                                            uint32_t offset,
                                            const std::string& data) {
    return std::string("http://ns.adobe.com/xmp/extension/") + '\0' + digest.toHexString().c_str() +
           uint32_to_string(size) + uint32_to_string(offset) + data;
}

DEF_TEST(SkJpegXmp_readExtendedXmp, r) {
    const std::string standardXmpData = R"(
            <x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 6.0.0">
               <rdf:RDF xmlns:xmpNote="http://ns.adobe.com/xmp/note/">
                  <rdf:Description rdf:about="">
                     <xmpNote:HasExtendedXMP>$GUID</xmpNote:HasExtendedXMP>
                  </rdf:Description>
               </rdf:RDF>
            </x:xmpmeta>)";

    const std::string extendedXmpData1 = R"(
        <x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 6.0.0">
            <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
                    xmlns:hdrgm="http://ns.adobe.com/hdr-gain-map/1.0/">
                <rdf:Description rdf:about="">
                     <hdrgm:Version>1.0</hdrgm:Version>
                     <hdrgm:GainMapMax>3</hdrgm:GainMapMax>
                    <hdrgm:HDRCapacityMax>4</hdrgm:HDRCapacityMax>)";
    const std::string extendedXmpData2 = R"(
                </rdf:Description>
            </rdf:RDF>
        </x:xmpmeta>)";

    const uint32_t totalExtendedXmpSize = extendedXmpData1.size() + extendedXmpData2.size();
    SkMD5 md5;
    md5.write(extendedXmpData1.data(), extendedXmpData1.length());
    md5.write(extendedXmpData2.data(), extendedXmpData2.length());
    const SkMD5::Digest digest = md5.finish();

    const std::string standardXmpDataWithHeader = standard_xmp_with_header(digest, standardXmpData);

    const uint32_t offset1 = 0;
    const std::string extendedXmpData1WithHeader =
            extended_xmp_with_header(digest, totalExtendedXmpSize, offset1, extendedXmpData1);

    const uint32_t offset2 = extendedXmpData1.size();
    const std::string extendedXmpData2WithHeader =
            extended_xmp_with_header(digest, totalExtendedXmpSize, offset2, extendedXmpData2);

    std::vector<sk_sp<SkData>> app1Params;
    app1Params.push_back(SkData::MakeWithoutCopy(standardXmpDataWithHeader.data(),
                                                 standardXmpDataWithHeader.length()));
    app1Params.push_back(SkData::MakeWithoutCopy(extendedXmpData1WithHeader.data(),
                                                 extendedXmpData1WithHeader.length()));
    app1Params.push_back(SkData::MakeWithoutCopy(extendedXmpData2WithHeader.data(),
                                                 extendedXmpData2WithHeader.length()));

    auto xmp = SkJpegMakeXmp(app1Params);
    REPORTER_ASSERT(r, xmp);

    SkGainmapInfo info;
    REPORTER_ASSERT(r, xmp->getGainmapInfoHDRGM(&info));
    REPORTER_ASSERT(r, info.fGainmapRatioMax.fR == 8.f);
    REPORTER_ASSERT(r, info.fDisplayRatioHdr == 16.f);
}
