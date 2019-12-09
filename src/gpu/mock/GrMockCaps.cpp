/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mock/GrMockCaps.h"

#include "src/gpu/GrProgramDesc.h"

GrProgramDesc GrMockCaps::makeDesc(const GrRenderTarget* rt,
                                   const GrProgramInfo& programInfo) const {
    GrProgramDesc desc;
    SkDEBUGCODE(bool result =) GrProgramDesc::Build(&desc, rt, programInfo, *this);
    SkASSERT(result == desc.isValid());
    return desc;
}

#if GR_TEST_UTILS
std::vector<GrCaps::TestFormatColorTypeCombination> GrMockCaps::getTestingCombinations() const {
    // TODO: need to add compressed formats to this list
    std::vector<GrCaps::TestFormatColorTypeCombination> combos = {
        { GrColorType::kAlpha_8,         GrBackendFormat::MakeMock(GrColorType::kAlpha_8, SkImage::kNone_CompressionType)        },
        { GrColorType::kBGR_565,         GrBackendFormat::MakeMock(GrColorType::kBGR_565, SkImage::kNone_CompressionType)        },
        { GrColorType::kABGR_4444,       GrBackendFormat::MakeMock(GrColorType::kABGR_4444, SkImage::kNone_CompressionType)      },
        { GrColorType::kRGBA_8888,       GrBackendFormat::MakeMock(GrColorType::kRGBA_8888, SkImage::kNone_CompressionType)      },
        { GrColorType::kRGBA_8888_SRGB,  GrBackendFormat::MakeMock(GrColorType::kRGBA_8888_SRGB, SkImage::kNone_CompressionType) },
        { GrColorType::kRGB_888x,        GrBackendFormat::MakeMock(GrColorType::kRGB_888x, SkImage::kNone_CompressionType)       },
        { GrColorType::kRG_88,           GrBackendFormat::MakeMock(GrColorType::kRG_88, SkImage::kNone_CompressionType)          },
        { GrColorType::kBGRA_8888,       GrBackendFormat::MakeMock(GrColorType::kBGRA_8888, SkImage::kNone_CompressionType)      },
        { GrColorType::kRGBA_1010102,    GrBackendFormat::MakeMock(GrColorType::kRGBA_1010102, SkImage::kNone_CompressionType)   },
        { GrColorType::kGray_8,          GrBackendFormat::MakeMock(GrColorType::kGray_8, SkImage::kNone_CompressionType)         },
        { GrColorType::kAlpha_F16,       GrBackendFormat::MakeMock(GrColorType::kAlpha_F16, SkImage::kNone_CompressionType)      },
        { GrColorType::kRGBA_F16,        GrBackendFormat::MakeMock(GrColorType::kRGBA_F16, SkImage::kNone_CompressionType)       },
        { GrColorType::kRGBA_F16_Clamped,GrBackendFormat::MakeMock(GrColorType::kRGBA_F16_Clamped, SkImage::kNone_CompressionType)},
        { GrColorType::kAlpha_16,        GrBackendFormat::MakeMock(GrColorType::kAlpha_16, SkImage::kNone_CompressionType)       },
        { GrColorType::kRG_1616,         GrBackendFormat::MakeMock(GrColorType::kRG_1616, SkImage::kNone_CompressionType)        },
        { GrColorType::kRGBA_16161616,   GrBackendFormat::MakeMock(GrColorType::kRGBA_16161616, SkImage::kNone_CompressionType)  },
        { GrColorType::kRG_F16,          GrBackendFormat::MakeMock(GrColorType::kRG_F16, SkImage::kNone_CompressionType)         },
    };

#ifdef SK_DEBUG
    for (auto combo : combos) {
        SkASSERT(this->onAreColorTypeAndFormatCompatible(combo.fColorType, combo.fFormat));
    }
#endif

    return combos;
}
#endif
