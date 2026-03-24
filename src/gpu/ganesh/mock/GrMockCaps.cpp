/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/mock/GrMockCaps.h"

#include "include/gpu/ganesh/mock/GrMockBackendSurface.h"
#include "include/private/base/SkDebug.h"
#include "src/base/SkMathPriv.h"
#include "src/gpu/ganesh/GrProgramDesc.h"
#include "src/gpu/ganesh/TestFormatColorTypeCombination.h"

int GrMockCaps::getRenderTargetSampleCount(int requestCount, GrColorType ct) const {
    requestCount = std::max(requestCount, 1);

    switch (fOptions.fConfigOptions[(int)ct].fRenderability) {
        case GrMockOptions::ConfigOptions::Renderability::kNo:
            return 0;
        case GrMockOptions::ConfigOptions::Renderability::kNonMSAA:
            return requestCount > 1 ? 0 : 1;
        case GrMockOptions::ConfigOptions::Renderability::kMSAA:
            return requestCount > kMaxSampleCnt ? 0 : SkNextPow2(requestCount);
    }
    return 0;
}

GrProgramDesc GrMockCaps::makeDesc(GrRenderTarget* /* rt */,
                                   const GrProgramInfo& programInfo,
                                   ProgramDescOverrideFlags overrideFlags) const {
    SkASSERT(overrideFlags == ProgramDescOverrideFlags::kNone);
    GrProgramDesc desc;
    GrProgramDesc::Build(&desc, programInfo, *this);
    return desc;
}

uint64_t GrMockCaps::computeFormatKey(const GrBackendFormat& format) const {
#ifdef SK_DEBUG
    SkTextureCompressionType compression = GrBackendFormats::AsMockCompressionType(format);
    SkASSERT(compression == SkTextureCompressionType::kNone);
#endif
    auto ct = GrBackendFormats::AsMockColorType(format);
    return (uint64_t)ct;
}

#if defined(GPU_TEST_UTILS)
std::vector<GrTest::TestFormatColorTypeCombination> GrMockCaps::getTestingCombinations() const {
    using GCT = GrColorType;
    using TCT = SkTextureCompressionType;
    // TODO: need to add compressed formats to this list
    std::vector<GrTest::TestFormatColorTypeCombination> combos = {
        { GCT::kAlpha_8,          GrBackendFormats::MakeMockColorType(GCT::kAlpha_8)},
        { GCT::kBGR_565,          GrBackendFormats::MakeMockColorType(GCT::kBGR_565)},
        { GCT::kRGB_565,          GrBackendFormats::MakeMockColorType(GCT::kRGB_565)},
        { GCT::kABGR_4444,        GrBackendFormats::MakeMockColorType(GCT::kABGR_4444)},
        { GCT::kRGBA_8888,        GrBackendFormats::MakeMockColorType(GCT::kRGBA_8888)},
        { GCT::kRGBA_8888_SRGB,   GrBackendFormats::MakeMockColorType(GCT::kRGBA_8888_SRGB)},
        { GCT::kRGB_888x,         GrBackendFormats::MakeMockColorType(GCT::kRGB_888x)},
        { GCT::kRG_88,            GrBackendFormats::MakeMockColorType(GCT::kRG_88)},
        { GCT::kBGRA_8888,        GrBackendFormats::MakeMockColorType(GCT::kBGRA_8888)},
        { GCT::kRGBA_1010102,     GrBackendFormats::MakeMockColorType(GCT::kRGBA_1010102)},
        { GCT::kBGRA_1010102,     GrBackendFormats::MakeMockColorType(GCT::kBGRA_1010102)},
        { GCT::kGray_8,           GrBackendFormats::MakeMockColorType(GCT::kGray_8)},
        { GCT::kAlpha_F16,        GrBackendFormats::MakeMockColorType(GCT::kAlpha_F16)},
        { GCT::kRGBA_F16,         GrBackendFormats::MakeMockColorType(GCT::kRGBA_F16)},
        { GCT::kRGBA_F16_Clamped, GrBackendFormats::MakeMockColorType(GCT::kRGBA_F16_Clamped)},
        { GCT::kRGB_F16F16F16x,   GrBackendFormats::MakeMockColorType(GCT::kRGB_F16F16F16x)},
        { GCT::kAlpha_16,         GrBackendFormats::MakeMockColorType(GCT::kAlpha_16)},
        { GCT::kRG_1616,          GrBackendFormats::MakeMockColorType(GCT::kRG_1616)},
        { GCT::kRGBA_16161616,    GrBackendFormats::MakeMockColorType(GCT::kRGBA_16161616)},
        { GCT::kR_F16,            GrBackendFormats::MakeMockColorType(GCT::kR_F16)},
        { GCT::kRG_F16,           GrBackendFormats::MakeMockColorType(GCT::kRG_F16)},
        // For these two compressed image formats the color type will effectively be RGB_888x
        { GCT::kRGB_888x,         GrBackendFormats::MakeMockCompressionType(TCT::kETC2_RGB8_UNORM)},
        { GCT::kRGB_888x,         GrBackendFormats::MakeMockCompressionType(TCT::kBC1_RGB8_UNORM)},
        { GCT::kRGBA_8888,        GrBackendFormats::MakeMockCompressionType(TCT::kBC1_RGBA8_UNORM)},
    };

#ifdef SK_DEBUG
    for (const GrTest::TestFormatColorTypeCombination& combo : combos) {
        SkASSERT(this->onAreColorTypeAndFormatCompatible(combo.fColorType, combo.fFormat));
    }
#endif

    return combos;
}

#endif
