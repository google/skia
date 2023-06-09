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

static void append_uint32(uint32_t v, std::vector<char>* c) {
    for (int i = 0; i < 4; ++i) {
        c->push_back(static_cast<char>((v >> ((3 - i) * 8)) & 0xff));
    }
}

static std::string digest_to_hex_string(const SkMD5::Digest& digest) {
    std::stringstream ss;
    for (int i = 0; i < 16; ++i) {
        ss << std::uppercase << std::setfill('0') << std::setw(2) << std::right << std::hex
           << (int)digest.data[i];
    }
    return ss.str();
}

static std::string standard_xmp_with_header(const SkMD5::Digest& digest, const std::string& data) {
    const std::string sig = "http://ns.adobe.com/xap/1.0/";
    std::vector<char> c(sig.begin(), sig.end());
    c.push_back('\0');
    const std::string guid = digest_to_hex_string(digest);
    const std::string dataWithGuid = std::regex_replace(data, std::regex("\\$GUID"), guid);
    c.insert(c.end(), dataWithGuid.begin(), dataWithGuid.end());
    return std::string(c.data(), c.size());
}

static std::string extended_xmp_with_header(const SkMD5::Digest& digest,
                                            uint32_t size,
                                            uint32_t offset,
                                            const std::string& data) {
    const std::string sig = "http://ns.adobe.com/xmp/extension/";
    std::vector<char> c(sig.begin(), sig.end());
    c.push_back('\0');
    const std::string guid = digest_to_hex_string(digest);
    c.insert(c.end(), guid.begin(), guid.end());
    append_uint32(size, &c);
    append_uint32(offset, &c);
    c.insert(c.end(), data.begin(), data.end());
    return std::string(c.data(), c.size());
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
