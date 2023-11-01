/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontArguments.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/ports/SkFontConfigInterface.h"
#include "src/ports/SkFontConfigInterface_direct.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"

#include <fontconfig/fontconfig.h>
#include <memory>
#include <utility>
#include <vector>

namespace {

FcConfig* build_fontconfig_with_fontfile(const char* fontFilename) {
    FcConfig* config = FcConfigCreate();

    // FontConfig may modify the passed path (make absolute or other).
    FcConfigSetSysRoot(config, reinterpret_cast<const FcChar8*>(GetResourcePath("").c_str()));
    // FontConfig will lexically compare paths against its version of the sysroot.
    SkString fontFilePath(reinterpret_cast<const char*>(FcConfigGetSysRoot(config)));
    fontFilePath += fontFilename;
    FcConfigAppFontAddFile(config, reinterpret_cast<const FcChar8*>(fontFilePath.c_str()));

    FcConfigBuildFonts(config);
    return config;
}

bool fontmgr_understands_ft_named_instance_bits() {
    std::unique_ptr<SkStreamAsset> distortable(GetResourceAsStream("fonts/Distortable.ttf"));
    if (!distortable) {
        return false;
    }

    sk_sp<SkFontMgr> fm = ToolUtils::TestFontMgr();
    SkFontArguments params;
    // The first named variation position in Distortable is 'Thin'.
    params.setCollectionIndex(0x00010000);
    sk_sp<SkTypeface> typeface = fm->makeFromStream(std::move(distortable), params);
    return !!typeface;
}

}  // namespace

DEF_TEST(FontConfigInterface_MatchStyleNamedInstance, reporter) {
    if (!fontmgr_understands_ft_named_instance_bits()) {
        return;
    }

    FcConfig* config = build_fontconfig_with_fontfile("/fonts/NotoSansCJK-VF-subset.otf.ttc");
    sk_sp<SkFontConfigInterfaceDirect> fciDirect(new SkFontConfigInterfaceDirect(config));

    static constexpr const char* family_names[]{"Noto Sans CJK JP",
                                                "Noto Sans CJK HK",
                                                "Noto Sans CJK SC",
                                                "Noto Sans CJK TC",
                                                "Noto Sans CJK KR"};
    static constexpr const struct Test {
        int weight;
        bool highBitsExpectation;
    } tests[] {
        {100, false},
        {300, true },
        {350, true },
        {400, true },
        {500, true },
        {700, true },
        {900, true },
    };

    for (auto&& font_name : family_names) {
        for (auto&& [weight, highBitsExpectation] : tests) {
            SkFontStyle fontStyle(weight, SkFontStyle::kNormal_Width, SkFontStyle::kUpright_Slant);

            SkFontConfigInterface::FontIdentity resultIdentity;
            SkFontStyle resultStyle;
            SkString resultFamily;
            const bool r = fciDirect->matchFamilyName(
                    font_name, fontStyle, &resultIdentity, &resultFamily, &resultStyle);

            REPORTER_ASSERT(reporter, r, "Expecting to find a match result.");
            REPORTER_ASSERT(
                    reporter,
                    (resultIdentity.fTTCIndex >> 16 > 0) == highBitsExpectation,
                    "Expected to have the ttcIndex' upper 16 bits refer to a named instance.");

            // Intentionally go through manually creating the typeface so that SkFontStyle is
            // derived from data inside the font, not from the FcPattern that is the FontConfig
            // match result, see https://crbug.com/skia/12881
            sk_sp<SkTypeface> typeface(fciDirect->makeTypeface(resultIdentity,
                                                               ToolUtils::TestFontMgr()).release());

            if (!typeface) {
                ERRORF(reporter, "Could not instantiate typeface, FcVersion: %d", FcGetVersion());
                return;
            }

            SkString family_from_typeface;
            typeface->getFamilyName(&family_from_typeface);

            REPORTER_ASSERT(reporter,
                            family_from_typeface == SkString(font_name),
                            "Matched font's family name should match the request.");

            SkFontStyle intrinsic_style = typeface->fontStyle();
            REPORTER_ASSERT(reporter,
                            intrinsic_style.weight() == weight,
                            "Matched font's weight should match request.");
            if (intrinsic_style.weight() != weight) {
                ERRORF(reporter,
                       "Matched font had weight: %d, expected %d, family: %s",
                       intrinsic_style.weight(),
                       weight,
                       family_from_typeface.c_str());
            }

            int numAxes = typeface->getVariationDesignPosition(nullptr, 0);
            std::vector<SkFontArguments::VariationPosition::Coordinate> coords;
            coords.resize(numAxes);
            typeface->getVariationDesignPosition(coords.data(), numAxes);

            REPORTER_ASSERT(reporter,
                            coords.size() == 1,
                            "The font must only have one axis, the weight axis.");

            REPORTER_ASSERT(reporter,
                            coords[0].axis == SkSetFourByteTag('w', 'g', 'h', 't'),
                            "The weight axis must be present and configured.");

            REPORTER_ASSERT(reporter,
                            static_cast<int>(coords[0].value) == weight,
                            "The weight axis must match the weight from the request.");
        }
  }

}
