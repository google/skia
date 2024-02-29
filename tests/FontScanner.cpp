/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkAutoMalloc.h"
#include "src/core/SkFontScanner.h"
#include "src/core/SkTHash.h"
#include "src/core/SkWriteBuffer.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"

#ifdef SK_TYPEFACE_FACTORY_FREETYPE
#include "src/ports/SkTypeface_FreeType.h"
#endif
#ifdef SK_TYPEFACE_FACTORY_FONTATIONS
#include "src/ports/SkFontScanner_fontations.h"
#endif

[[maybe_unused]]
static void FontScanner_VariableFont(skiatest::Reporter* reporter,
                                     SkFontScanner* scanner,
                                     bool defaultInstanceOnly) {
    SkString name = GetResourcePath("fonts/Variable.ttf");

    std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(name.c_str());
    if (!stream) {
         REPORTER_ASSERT(reporter, false, "Cannot open the font file %s\n", name.c_str());
    }

    int numFaces;
    if (!scanner->scanFile(stream.get(), &numFaces)) {
         REPORTER_ASSERT(reporter, false, "Cannot scanFile\n");
    }
    REPORTER_ASSERT(reporter, numFaces == 1);

    skia_private::THashSet<SkFontStyle> uniqueStyles;
    for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex) {
        int numInstances;
        if (!scanner->scanFace(stream.get(), faceIndex, &numInstances)) {
            REPORTER_ASSERT(reporter, false, "Cannot scanFace\n");
            continue;
        }
        if (defaultInstanceOnly) {
            REPORTER_ASSERT(reporter, numInstances == 1);
            continue;
        }
        REPORTER_ASSERT(reporter, numInstances == 5);
        for (int instanceIndex = 1; instanceIndex <= numInstances; ++instanceIndex) {
            bool isFixedPitch;
            SkString realname;
            SkFontStyle style = SkFontStyle(); // avoid uninitialized warning
            if (!scanner->scanInstance(stream.get(),
                                        faceIndex,
                                        instanceIndex,
                                        &realname,
                                        &style,
                                        &isFixedPitch,
                                        nullptr)) {
                REPORTER_ASSERT(reporter,
                                false,
                                "Cannot scanInstance %s %d\n",
                                name.c_str(),
                                faceIndex);
                continue;
            } else {
                if (uniqueStyles.find(style) == nullptr) {
                    uniqueStyles.add(style);
                } else {
                    REPORTER_ASSERT(
                        reporter,
                        false,
                        "Font: %s (%d %d %d)\n",
                        realname.c_str(), style.weight(), style.width(), style.slant());
                }
            }
        }
        REPORTER_ASSERT(reporter, uniqueStyles.count() == numInstances);
    }
}

#ifdef SK_TYPEFACE_FACTORY_FREETYPE
DEF_TEST(FontScanner_FreeType_VariableFont, reporter) {
    SkFontScanner_FreeType free_type;
    FontScanner_VariableFont(reporter, &free_type, /*defaultInstanceOnly=*/ false);
}
#endif

#ifdef SK_TYPEFACE_FACTORY_FONTATIONS
DEF_TEST(FontScanner_Fontations_VariableFont, reporter) {
    SkFontScanner_Fontations fontations;
    FontScanner_VariableFont(reporter, &fontations,/*defaultInstanceOnly=*/ true);
}
#endif
