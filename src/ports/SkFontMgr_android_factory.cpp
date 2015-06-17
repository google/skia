/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontMgr.h"
#include "SkFontMgr_android.h"

// For test only.
static const char* gTestFontsXml = NULL;
static const char* gTestFallbackFontsXml = NULL;
static const char* gTestBasePath = NULL;

void SkUseTestFontConfigFile(const char* fontsXml, const char* fallbackFontsXml,
                             const char* basePath)
{
    gTestFontsXml = fontsXml;
    gTestFallbackFontsXml = fallbackFontsXml;
    gTestBasePath = basePath;
    SkASSERT(gTestFontsXml);
    SkASSERT(gTestFallbackFontsXml);
    SkASSERT(gTestBasePath);
    SkDEBUGF(("Test BasePath: %s Fonts: %s FallbackFonts: %s\n",
              gTestBasePath, gTestFontsXml, gTestFallbackFontsXml));
}

SkFontMgr* SkFontMgr::Factory() {
    // These globals exist so that Chromium can override the environment.
    // TODO: these globals need to be removed, and Chromium use SkFontMgr_New_Android instead.
    if ((gTestFontsXml || gTestFallbackFontsXml) && gTestBasePath) {
        SkFontMgr_Android_CustomFonts custom = {
            SkFontMgr_Android_CustomFonts::kOnlyCustom,
            gTestBasePath,
            gTestFontsXml,
            gTestFallbackFontsXml
        };
        return SkFontMgr_New_Android(&custom);
    }

    return SkFontMgr_New_Android(NULL);
}
