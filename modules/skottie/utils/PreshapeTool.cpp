/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkFontMgr.h"  // IWYU pragma: keep
#include "include/core/SkGraphics.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFeatures.h"
#include "modules/skottie/utils/TextPreshape.h"
#include "modules/skresources/include/SkResources.h"
#include "modules/skshaper/utils/FactoryHelpers.h"
#include "tools/flags/CommandLineFlags.h"

#if defined(SK_BUILD_FOR_MAC) && defined(SK_FONTMGR_CORETEXT_AVAILABLE)
#include "include/ports/SkFontMgr_mac_ct.h"
#elif defined(SK_BUILD_FOR_ANDROID) && defined(SK_FONTMGR_ANDROID_AVAILABLE)
#include "include/ports/SkFontMgr_android.h"
#include "src/ports/SkTypeface_FreeType.h"
#elif defined(SK_BUILD_FOR_UNIX) && defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)
#include "include/ports/SkFontMgr_fontconfig.h"
#else
#include "include/ports/SkFontMgr_empty.h"
#endif

static DEFINE_string2(input , i, nullptr, "Input .json file.");
static DEFINE_string2(output, o, nullptr, "Output .json file.");

 int main(int argc, char** argv) {
    CommandLineFlags::Parse(argc, argv);
    SkGraphics::Init();

    if (FLAGS_input.isEmpty() || FLAGS_output.isEmpty()) {
        SkDebugf("Missing required 'input' and 'output' args.\n");
        return 1;
    }

    const auto data = SkData::MakeFromFileName(FLAGS_input[0]);
    if (!data) {
        SkDebugf("Could not read file: %s\n", FLAGS_input[0]);
        return 1;
    }

    SkFILEWStream out(FLAGS_output[0]);
    if (!out.isValid()) {
        SkDebugf("Could not write file: %s\n", FLAGS_output[0]);
        return 1;
    }

#if defined(SK_BUILD_FOR_MAC) && defined(SK_FONTMGR_CORETEXT_AVAILABLE)
    sk_sp<SkFontMgr> fontMgr = SkFontMgr_New_CoreText(nullptr);
#elif defined(SK_BUILD_FOR_ANDROID) && defined(SK_FONTMGR_ANDROID_AVAILABLE)
    sk_sp<SkFontMgr> fontMgr = SkFontMgr_New_Android(nullptr, std::make_unique<SkFontScanner_FreeType>());
#elif defined(SK_BUILD_FOR_UNIX) && defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)
    sk_sp<SkFontMgr> fontMgr = SkFontMgr_New_FontConfig(nullptr);
#else
    sk_sp<SkFontMgr> fontMgr = SkFontMgr_New_Custom_Empty();
#endif

    if (!skottie_utils::Preshape(data, &out, fontMgr, SkShapers::BestAvailable(), nullptr)) {
        SkDebugf("Could not preshape: %s\n", FLAGS_input[0]);
        return -1;
    }

    return 0;
 }
