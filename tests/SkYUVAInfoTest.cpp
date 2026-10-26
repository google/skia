/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColor.h"
#include "include/core/SkYUVAInfo.h"
#include "src/core/SkYUVAInfoLocation.h" // IWYU pragma: keep
#include "tests/Test.h"

#if defined(SK_ENABLE_YUVA_PACKED_422)
#include "include/codec/SkEncodedOrigin.h"
#endif

using PlaneConfig = SkYUVAInfo::PlaneConfig;
using Subsampling = SkYUVAInfo::Subsampling;

#if defined(SK_ENABLE_YUVA_PACKED_422)
DEF_TEST(SkYUVAInfoPackedYUYV, reporter) {
    // The packed 4:2:2 config is only compatible with horizontal 2:1
    // subsampling.
    for (Subsampling s : {Subsampling::kUnknown, Subsampling::k444, Subsampling::k422,
                          Subsampling::k420, Subsampling::k440, Subsampling::k411,
                          Subsampling::k410}) {
        SkYUVAInfo info({1920, 1080}, PlaneConfig::kYUYV, s, kRec601_SkYUVColorSpace);
        REPORTER_ASSERT(reporter, info.isValid() == (s == Subsampling::k422));
    }

    SkYUVAInfo info({1920, 1080}, PlaneConfig::kYUYV, Subsampling::k422,
                    kRec601_SkYUVColorSpace);
    REPORTER_ASSERT(reporter, info.isValid());
    REPORTER_ASSERT(reporter, info.numPlanes() == 1);
    REPORTER_ASSERT(reporter, !info.hasAlpha());
    REPORTER_ASSERT(reporter, info.numChannelsInPlane(0) == 4);
    REPORTER_ASSERT(reporter, info.numChannelsInPlane(1) == 0);

    // The plane has one 4-channel texel per 2x1 pixel block, so its horizontal texel dimension is
    // half the image's.
    SkISize planeDimensions[SkYUVAInfo::kMaxPlanes];
    REPORTER_ASSERT(reporter, info.planeDimensions(planeDimensions) == 1);
    REPORTER_ASSERT(reporter, planeDimensions[0] == SkISize::Make(960, 1080));

    auto [ssx, ssy] = info.planeSubsamplingFactors(0);
    REPORTER_ASSERT(reporter, ssx == 2 && ssy == 1);

    for (int planeIdx : {-1, info.numPlanes(), info.numPlanes() + 1}) {
        auto [invalidX, invalidY] = info.planeSubsamplingFactors(planeIdx);
        REPORTER_ASSERT(reporter, invalidX == 0 && invalidY == 0);
    }

    // Odd image widths are not representable (the last pixel would be
    // unpaired) and are rejected by the constructor.
    SkYUVAInfo odd({1919, 1080}, PlaneConfig::kYUYV, Subsampling::k422,
                   kRec601_SkYUVColorSpace);
    REPORTER_ASSERT(reporter, !odd.isValid());

    // Origins that swap width and height store the plane rotated 90 degrees, so the plane's memory
    // width is half the image height.
    SkYUVAInfo swappedOk({1919, 1080}, PlaneConfig::kYUYV, Subsampling::k422,
                         kRec601_SkYUVColorSpace, kLeftBottom_SkEncodedOrigin);
    REPORTER_ASSERT(reporter, swappedOk.isValid());
    REPORTER_ASSERT(reporter, swappedOk.planeDimensions(planeDimensions) == 1);
    REPORTER_ASSERT(reporter, planeDimensions[0] == SkISize::Make(540, 1919));

    SkYUVAInfo swappedOdd({1080, 1919}, PlaneConfig::kYUYV, Subsampling::k422,
                          kRec601_SkYUVColorSpace, kLeftBottom_SkEncodedOrigin);
    REPORTER_ASSERT(reporter, !swappedOdd.isValid());

    // Channel locations record the even pixel's channels: Y is channel 0 (R), U is channel 1 (G)
    // and V is channel 3 (A) of the pair's texel; the odd pixel shares U and V and takes its Y from
    // channel 2 (B) at sampling time. There is no alpha.
    const uint32_t rgbaFlags = kRGBA_SkColorChannelFlags;
    auto locations = info.toYUVALocations(&rgbaFlags);
    REPORTER_ASSERT(reporter, locations[SkYUVAInfo::YUVAChannels::kY].fPlane == 0);
    REPORTER_ASSERT(reporter, locations[SkYUVAInfo::YUVAChannels::kY].fChannel ==
                                 SkColorChannel::kR);
    REPORTER_ASSERT(reporter, locations[SkYUVAInfo::YUVAChannels::kU].fPlane == 0);
    REPORTER_ASSERT(reporter, locations[SkYUVAInfo::YUVAChannels::kU].fChannel ==
                                 SkColorChannel::kG);
    REPORTER_ASSERT(reporter, locations[SkYUVAInfo::YUVAChannels::kV].fPlane == 0);
    REPORTER_ASSERT(reporter, locations[SkYUVAInfo::YUVAChannels::kV].fChannel ==
                                 SkColorChannel::kA);
    REPORTER_ASSERT(reporter, locations[SkYUVAInfo::YUVAChannels::kA].fPlane < 0);
}
#endif  // SK_ENABLE_YUVA_PACKED_422

DEF_TEST(SkYUVAInfoSinglePlaneConfigs, reporter) {
    // Y410 is modeled as kUYVA with k444 subsampling: plane channel order is
    // U, Y, V, A in R, G, B, A.
    SkYUVAInfo y410({1920, 1080}, PlaneConfig::kUYVA, Subsampling::k444,
                    kRec709_SkYUVColorSpace);
    REPORTER_ASSERT(reporter, y410.isValid());
    REPORTER_ASSERT(reporter, y410.numPlanes() == 1);
    REPORTER_ASSERT(reporter, y410.hasAlpha());
    REPORTER_ASSERT(reporter, y410.numChannelsInPlane(0) == 4);

    for (int planeIdx : {-1, y410.numPlanes(), y410.numPlanes() + 1}) {
        auto [invalidX, invalidY] = y410.planeSubsamplingFactors(planeIdx);
        REPORTER_ASSERT(reporter, invalidX == 0 && invalidY == 0);
    }

    const uint32_t rgbaFlags = kRGBA_SkColorChannelFlags;
    auto locations = y410.toYUVALocations(&rgbaFlags);
    REPORTER_ASSERT(reporter, locations[SkYUVAInfo::YUVAChannels::kY].fChannel ==
                                 SkColorChannel::kG);
    REPORTER_ASSERT(reporter, locations[SkYUVAInfo::YUVAChannels::kU].fChannel ==
                                 SkColorChannel::kR);
    REPORTER_ASSERT(reporter, locations[SkYUVAInfo::YUVAChannels::kV].fChannel ==
                                 SkColorChannel::kB);
    REPORTER_ASSERT(reporter, locations[SkYUVAInfo::YUVAChannels::kA].fChannel ==
                                 SkColorChannel::kA);
}
