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
                                     SkFontScanner* scanner) {
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

        REPORTER_ASSERT(reporter, numInstances == 5);
        // Not including the default instance
        for (int instanceIndex = 1; instanceIndex <= numInstances; ++instanceIndex) {
            bool isFixedPitch;
            SkString realName;
            SkFontStyle style = SkFontStyle(); // avoid uninitialized warning
            if (!scanner->scanInstance(stream.get(),
                                       faceIndex,
                                       instanceIndex,
                                       &realName,
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
                if (instanceIndex == 0) {
                    // Do not add it to the set
                } else if (uniqueStyles.find(style) == nullptr) {
                    uniqueStyles.add(style);
                } else {
                    REPORTER_ASSERT(
                        reporter,
                        false,
                        "Font: %s (%d %d %d)\n",
                            realName.c_str(), style.weight(), style.width(), style.slant());
                }
            }
        }
        REPORTER_ASSERT(reporter, uniqueStyles.count() == numInstances);
    }
}

[[maybe_unused]]
static void FontScanner_NamedInstances1(skiatest::Reporter* reporter, SkFontScanner* scanner) {
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
        REPORTER_ASSERT(reporter, numInstances == 5);
        // Not including the default instance (most time it will be listed anyway)
        for (int instanceIndex = 1; instanceIndex <= numInstances; ++instanceIndex) {
            bool isFixedPitch;
            SkString realName;
            SkFontStyle style = SkFontStyle(); // avoid uninitialized warning
            SkFontScanner::AxisDefinitions axes;
            if (!scanner->scanInstance(stream.get(),
                                       faceIndex,
                                       instanceIndex,
                                       &realName,
                                       &style,
                                       &isFixedPitch,
                                       &axes)) {
                REPORTER_ASSERT(reporter,
                                false,
                                "Cannot scanInstance %s %d\n",
                                name.c_str(),
                                faceIndex);
                continue;
            } else {
                if (uniqueStyles.find(style) == nullptr) {
                    uniqueStyles.add(style);
                    REPORTER_ASSERT(reporter, axes.size() == 2);
                    if (instanceIndex == 5) {
                        SkFourByteTag weight = SkSetFourByteTag('w', 'g', 'h', 't');
                        SkFourByteTag width = SkSetFourByteTag('w', 'd', 't', 'h');
                        REPORTER_ASSERT(reporter, axes[0].fTag == weight);
                        REPORTER_ASSERT(reporter, axes[0].fDefault == 400.0f);
                        REPORTER_ASSERT(reporter, axes[0].fMinimum == 100.0f);
                        REPORTER_ASSERT(reporter, axes[0].fMaximum == 900.0f);
                        REPORTER_ASSERT(reporter, axes[1].fTag == width);
                        REPORTER_ASSERT(reporter, axes[1].fDefault == 100.0f);
                        REPORTER_ASSERT(reporter, axes[1].fMinimum == 050.0f);
                        REPORTER_ASSERT(reporter, axes[1].fMaximum == 200.0f);
                    }
                } else {
                    REPORTER_ASSERT(reporter,
                                    false,
                                    "Font #%d: %s (%d %d %d)\n",
                                    instanceIndex,
                                    realName.c_str(),
                                    style.weight(),
                                    style.width(),
                                    style.slant());
                }
            }
        }
    }
}

[[maybe_unused]]
static void FontScanner_NamedInstances2(skiatest::Reporter* reporter, SkFontScanner* scanner) {
    SkString name = GetResourcePath("fonts/VaryAlongQuads.ttf");

    std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(name.c_str());
    if (!stream) {
        REPORTER_ASSERT(reporter, false, "Cannot open the font file %s\n", name.c_str());
    }

    int numFaces;
    if (!scanner->scanFile(stream.get(), &numFaces)) {
        REPORTER_ASSERT(reporter, false, "Cannot scanFile\n");
    }
    REPORTER_ASSERT(reporter, numFaces == 1);

    for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex) {
        int numInstances;
        if (!scanner->scanFace(stream.get(), faceIndex, &numInstances)) {
            REPORTER_ASSERT(reporter, false, "Cannot scanFace\n");
            continue;
        }
        REPORTER_ASSERT(reporter, numInstances == 3);
        // Not including the default instance (most time it will be listed anyway)
        for (int instanceIndex = 1; instanceIndex <= numInstances; ++instanceIndex) {
            bool isFixedPitch;
            SkString realName;
            SkFontStyle style = SkFontStyle(); // avoid uninitialized warning
            SkFontScanner::AxisDefinitions axes;
            if (!scanner->scanInstance(stream.get(),
                                       faceIndex,
                                       instanceIndex,
                                       &realName,
                                       &style,
                                       &isFixedPitch,
                                       &axes)) {
                REPORTER_ASSERT(reporter,
                                false,
                                "Cannot scanInstance %s %d\n",
                                name.c_str(),
                                faceIndex);
                continue;
            }
            REPORTER_ASSERT(reporter, axes.size() == 2);
            SkFourByteTag weight = SkSetFourByteTag('w', 'g', 'h', 't');
            for (auto i = 0; i < axes.size(); ++i) {
                const auto& axis = axes[i];
                REPORTER_ASSERT(reporter, (instanceIndex != 1) || (style.weight() == 100.0f));
                REPORTER_ASSERT(reporter, (instanceIndex != 2) || (style.weight() == 400.0f));
                REPORTER_ASSERT(reporter, (instanceIndex != 3) || (style.weight() == 900.0f));
                REPORTER_ASSERT(reporter, axis.fTag == weight);
                REPORTER_ASSERT(reporter, axis.fDefault == 400.0f);
                REPORTER_ASSERT(reporter, axis.fMinimum == 100.0f);
                REPORTER_ASSERT(reporter, axis.fMaximum == 900.0f);
            }
        }
    }
}

[[maybe_unused]]
static void FontScanner_FontCollection(skiatest::Reporter* reporter, SkFontScanner* scanner) {
    SkString name = GetResourcePath("fonts/test.ttc");

    std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(name.c_str());
    if (!stream) {
        REPORTER_ASSERT(reporter, false, "Cannot open the font file %s\n", name.c_str());
    }

    int numFaces;
    if (!scanner->scanFile(stream.get(), &numFaces)) {
        REPORTER_ASSERT(reporter, false, "Cannot scanFile\n");
    }
    REPORTER_ASSERT(reporter, numFaces == 2);

    for (int faceIndex = 0; faceIndex < numFaces; ++faceIndex) {
        int numInstances;
        if (!scanner->scanFace(stream.get(), faceIndex, &numInstances)) {
            REPORTER_ASSERT(reporter, false, "Cannot scanFace\n");
            continue;
        }
        REPORTER_ASSERT(reporter, numInstances == 0);
        const auto defaultInstance = 0;
        bool isFixedPitch;
        SkString realName;
        SkFontStyle style = SkFontStyle(); // avoid uninitialized warning
        SkFontScanner::AxisDefinitions axes;
        if (!scanner->scanInstance(stream.get(),
                                   faceIndex,
                                   defaultInstance,
                                   &realName,
                                   &style,
                                   &isFixedPitch,
                                   &axes)) {
            REPORTER_ASSERT(reporter,
                            false,
                            "Cannot scanInstance %s %d\n",
                            name.c_str(),
                            faceIndex);
            continue;
        }
        REPORTER_ASSERT(reporter, axes.size() == 0);
        REPORTER_ASSERT(reporter, (faceIndex != 0) || (style.weight() == 400.0f));
        REPORTER_ASSERT(reporter, (faceIndex != 1) || (style.weight() == 700.0f));
    }
}


#ifdef SK_TYPEFACE_FACTORY_FREETYPE
DEF_TEST(FontScanner_FreeType_VariableFont, reporter) {
    SkFontScanner_FreeType free_type;
    FontScanner_VariableFont(reporter, &free_type);
}

DEF_TEST(FontScanner_FreeType__NamedInstances1, reporter) {
    SkFontScanner_FreeType free_type;
    FontScanner_NamedInstances1(reporter, &free_type);
}

DEF_TEST(FontScanner_FreeType__NamedInstances2, reporter) {
    SkFontScanner_FreeType free_type;
    FontScanner_NamedInstances2(reporter, &free_type);
}

DEF_TEST(FontScanner_FreeType_FontCollection, reporter) {
    SkFontScanner_FreeType free_type;
    FontScanner_FontCollection(reporter, &free_type);
}
#endif

#ifdef SK_TYPEFACE_FACTORY_FONTATIONS
DEF_TEST(FontScanner_Fontations_VariableFont, reporter) {
    SkFontScanner_Fontations fontations;
    FontScanner_VariableFont(reporter, &fontations);
}

DEF_TEST(FontScanner_Fontations_NamedInstances1, reporter) {
    SkFontScanner_Fontations fontations;
    FontScanner_NamedInstances1(reporter, &fontations);
}

DEF_TEST(FontScanner_Fontations_NamedInstances2, reporter) {
    SkFontScanner_Fontations fontations;
    FontScanner_NamedInstances2(reporter, &fontations);
}

DEF_TEST(FontScanner_Fontations_FontCollection, reporter) {
    SkFontScanner_Fontations fontations;
    FontScanner_FontCollection(reporter, &fontations);
}
#endif
