/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/encode/SkPngEncoder.h"
#include "modules/svg/include/SkSVGDOM.h"
#include "modules/skshaper/utils/FactoryHelpers.h"

#if defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)
#include "include/ports/SkFontMgr_fontconfig.h"
#endif

#if defined(SK_FONTMGR_CORETEXT_AVAILABLE)
#include "include/ports/SkFontMgr_mac_ct.h"
#endif

#include <cstdio>

constexpr size_t kWidth =  450;
constexpr size_t kHeight = 120;

// The text in this SVG should be properly shaped if the V and A characters overlap slightly.
const char* svg = "<svg viewBox=\"0 0 150 40\" xmlns=\"http://www.w3.org/2000/svg\">"
  "<style>"
    "text {"
      "font: 13px sans-serif;"
    "}"
  "</style>"
  "<text x=\"10\" y=\"30\">VAVAVAVA</text>"
"</svg>";

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <name.png>", argv[0]);
        return 1;
    }
    SkFILEWStream output(argv[1]);
    if (!output.isValid()) {
        printf("Cannot open output file %s\n", argv[1]);
        return 1;
    }

    // If necessary, clients should use a font manager that would load fonts from the system.
    sk_sp<SkFontMgr> fontMgr;
#if defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)
    fontMgr = SkFontMgr_New_FontConfig(nullptr);
#elif defined(SK_FONTMGR_CORETEXT_AVAILABLE)
    fontMgr = SkFontMgr_New_CoreText(nullptr);
#endif
    if (!fontMgr) {
        printf("No Font Manager configured\n");
        return 1;
    }

    SkMemoryStream svgStream(svg, std::strlen(svg));

    auto svg_dom = SkSVGDOM::Builder()
                           .setFontManager(fontMgr)
                           .setTextShapingFactory(SkShapers::BestAvailable())
                           .make(svgStream);

    if (!svg_dom) {
        printf("Could not parse compiled-in svg\n");
        return 1;
    }

    auto surface = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(kWidth, kHeight));

    svg_dom->setContainerSize(SkSize::Make(kWidth, kHeight));
    svg_dom->render(surface->getCanvas());

    SkPixmap pixmap;
    surface->peekPixels(&pixmap);

    if (!SkPngEncoder::Encode(&output, pixmap, {})) {
        printf("PNG encoding failed.\n");
        return 1;
    }
    printf("Success!\n");
    return 0;
}
