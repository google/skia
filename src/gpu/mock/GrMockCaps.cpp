/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mock/GrMockCaps.h"

#if GR_TEST_UTILS
std::vector<GrCaps::TestFormatColorTypeCombination> GrMockCaps::getTestingCombinations() const {
    // TODO: need to add compressed formats to this list
    std::vector<GrCaps::TestFormatColorTypeCombination> combos = {
        { GrColorType::kAlpha_8,         GrBackendFormat::MakeMock(GrColorType::kAlpha_8)        },
        { GrColorType::kBGR_565,         GrBackendFormat::MakeMock(GrColorType::kBGR_565)        },
        { GrColorType::kABGR_4444,       GrBackendFormat::MakeMock(GrColorType::kABGR_4444)      },
        { GrColorType::kRGBA_8888,       GrBackendFormat::MakeMock(GrColorType::kRGBA_8888)      },
        { GrColorType::kRGBA_8888_SRGB,  GrBackendFormat::MakeMock(GrColorType::kRGBA_8888_SRGB) },
        { GrColorType::kRGB_888x,        GrBackendFormat::MakeMock(GrColorType::kRGB_888x)       },
        { GrColorType::kRG_88,           GrBackendFormat::MakeMock(GrColorType::kRG_88)          },
        { GrColorType::kBGRA_8888,       GrBackendFormat::MakeMock(GrColorType::kBGRA_8888)      },
        { GrColorType::kRGBA_1010102,    GrBackendFormat::MakeMock(GrColorType::kRGBA_1010102)   },
        { GrColorType::kGray_8,          GrBackendFormat::MakeMock(GrColorType::kGray_8)         },
        { GrColorType::kAlpha_F16,       GrBackendFormat::MakeMock(GrColorType::kAlpha_F16)      },
        { GrColorType::kRGBA_F16,        GrBackendFormat::MakeMock(GrColorType::kRGBA_F16)       },
        { GrColorType::kRGBA_F16_Clamped,GrBackendFormat::MakeMock(GrColorType::kRGBA_F16_Clamped)},
        { GrColorType::kRGBA_F32,        GrBackendFormat::MakeMock(GrColorType::kRGBA_F32)       },
        { GrColorType::kAlpha_16,        GrBackendFormat::MakeMock(GrColorType::kAlpha_16)       },
        { GrColorType::kRG_1616,         GrBackendFormat::MakeMock(GrColorType::kRG_1616)        },
        { GrColorType::kRGBA_16161616,   GrBackendFormat::MakeMock(GrColorType::kRGBA_16161616)  },
        { GrColorType::kRG_F16,          GrBackendFormat::MakeMock(GrColorType::kRG_F16)         },
    };

#ifdef SK_DEBUG
    for (auto combo : combos) {
        SkASSERT(this->onAreColorTypeAndFormatCompatible(combo.fColorType, combo.fFormat));
    }
#endif

    return combos;
}
#endif
