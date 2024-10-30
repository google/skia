/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkDocument.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSurface.h"
#include "include/core/SkStream.h"
#include "include/docs/SkPDFDocument.h"

#if defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)
#include "include/ports/SkFontMgr_fontconfig.h"
#endif

#if defined(SK_FONTMGR_CORETEXT_AVAILABLE)
#include "include/ports/SkFontMgr_mac_ct.h"
#endif

#include <cstdio>

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <name.pdf>", argv[0]);
        return 1;
    }
    SkFILEWStream output(argv[1]);
    if (!output.isValid()) {
        printf("Cannot open output file %s\n", argv[1]);
        return 1;
    }

    SkPDF::Metadata metadata;
    metadata.fTitle = "Test PDF";
    metadata.fAuthor = "Skia Demo Writer";
    metadata.fLang = "eng";
    metadata.fEncodingQuality = 90;

    sk_sp<SkDocument> pdf = SkPDF::MakeDocument(&output, metadata);
    SkCanvas* canvas = pdf->beginPage(100, 50);

#if defined(SK_FONTMGR_FONTCONFIG_AVAILABLE)
    sk_sp<SkFontMgr> mgr = SkFontMgr_New_FontConfig(nullptr);
#endif
#if defined(SK_FONTMGR_CORETEXT_AVAILABLE)
    sk_sp<SkFontMgr> mgr = SkFontMgr_New_CoreText(nullptr);
#endif

    sk_sp<SkTypeface> face = mgr->matchFamilyStyle("Roboto", SkFontStyle());
    if (!face) {
      printf("Cannot open typeface\n");
      return 1;
    }
    SkFont font(face, 14);
    SkPaint paint;
    paint.setColor(SK_ColorGREEN);
    canvas->clear(SK_ColorYELLOW);
    canvas->drawString("Hello world!", 10, 25, font, paint);

    pdf->endPage();
    pdf->close();
    return 0;
}
