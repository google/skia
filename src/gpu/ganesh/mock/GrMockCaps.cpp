/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/mock/GrMockCaps.h"

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
            return requestCount > kMaxSampleCnt ? 0 : GrNextPow2(requestCount);
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
    SkTextureCompressionType compression = format.asMockCompressionType();
    SkASSERT(compression == SkTextureCompressionType::kNone);
#endif
    auto ct = format.asMockColorType();
    return (uint64_t)ct;
}

#if defined(GR_TEST_UTILS)
std::vector<GrTest::TestFormatColorTypeCombination> GrMockCaps::getTestingCombinations() const {
    // TODO: need to add compressed formats to this list
    std::vector<GrTest::TestFormatColorTypeCombination> combos = {
        { GrColorType::kAlpha_8,        GrBackendFormat::MakeMock(GrColorType::kAlpha_8,
                                                                  SkTextureCompressionType::kNone)},
        { GrColorType::kBGR_565,        GrBackendFormat::MakeMock(GrColorType::kBGR_565,
                                                                  SkTextureCompressionType::kNone)},
        { GrColorType::kRGB_565,        GrBackendFormat::MakeMock(GrColorType::kRGB_565,
                                                                  SkTextureCompressionType::kNone)},
        { GrColorType::kABGR_4444,      GrBackendFormat::MakeMock(GrColorType::kABGR_4444,
                                                                  SkTextureCompressionType::kNone)},
        { GrColorType::kRGBA_8888,      GrBackendFormat::MakeMock(GrColorType::kRGBA_8888,
                                                                  SkTextureCompressionType::kNone)},
        { GrColorType::kRGBA_8888_SRGB, GrBackendFormat::MakeMock(GrColorType::kRGBA_8888_SRGB,
                                                                  SkTextureCompressionType::kNone)},
        { GrColorType::kRGB_888x,       GrBackendFormat::MakeMock(GrColorType::kRGB_888x,
                                                                  SkTextureCompressionType::kNone)},
        { GrColorType::kRG_88,          GrBackendFormat::MakeMock(GrColorType::kRG_88,
                                                                  SkTextureCompressionType::kNone)},
        { GrColorType::kBGRA_8888,      GrBackendFormat::MakeMock(GrColorType::kBGRA_8888,
                                                                  SkTextureCompressionType::kNone)},
        { GrColorType::kRGBA_1010102,   GrBackendFormat::MakeMock(GrColorType::kRGBA_1010102,
                                                                  SkTextureCompressionType::kNone)},
        { GrColorType::kBGRA_1010102,   GrBackendFormat::MakeMock(GrColorType::kBGRA_1010102,
                                                                  SkTextureCompressionType::kNone)},
        { GrColorType::kGray_8,         GrBackendFormat::MakeMock(GrColorType::kGray_8,
                                                                  SkTextureCompressionType::kNone)},
        { GrColorType::kAlpha_F16,      GrBackendFormat::MakeMock(GrColorType::kAlpha_F16,
                                                                  SkTextureCompressionType::kNone)},
        { GrColorType::kRGBA_F16,       GrBackendFormat::MakeMock(GrColorType::kRGBA_F16,
                                                                  SkTextureCompressionType::kNone)},
        { GrColorType::kRGBA_F16_Clamped,GrBackendFormat::MakeMock(GrColorType::kRGBA_F16_Clamped,
                                                                  SkTextureCompressionType::kNone)},
        { GrColorType::kAlpha_16,       GrBackendFormat::MakeMock(GrColorType::kAlpha_16,
                                                                  SkTextureCompressionType::kNone)},
        { GrColorType::kRG_1616,        GrBackendFormat::MakeMock(GrColorType::kRG_1616,
                                                                  SkTextureCompressionType::kNone)},
        { GrColorType::kRGBA_16161616,  GrBackendFormat::MakeMock(GrColorType::kRGBA_16161616,
                                                                  SkTextureCompressionType::kNone)},
        { GrColorType::kRG_F16,         GrBackendFormat::MakeMock(GrColorType::kRG_F16,
                                                                  SkTextureCompressionType::kNone)},
        // For these two compressed image formats the color type will effectively be RGB_888x
        { GrColorType::kRGB_888x,       GrBackendFormat::MakeMock(GrColorType::kUnknown,
                                                    SkTextureCompressionType::kETC2_RGB8_UNORM)},
        { GrColorType::kRGB_888x,       GrBackendFormat::MakeMock(GrColorType::kUnknown,
                                                    SkTextureCompressionType::kBC1_RGB8_UNORM)},
        { GrColorType::kRGBA_8888,      GrBackendFormat::MakeMock(GrColorType::kUnknown,
                                                    SkTextureCompressionType::kBC1_RGBA8_UNORM)},
    };

#ifdef SK_DEBUG
    for (const GrTest::TestFormatColorTypeCombination& combo : combos) {
        SkASSERT(this->onAreColorTypeAndFormatCompatible(combo.fColorType, combo.fFormat));
    }
#endif

    return combos;
}

#endif
