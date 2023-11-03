/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontArguments.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontParameters.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFixed.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkEndian.h"
#include "src/base/SkUTF.h"
#include "src/core/SkFontDescriptor.h"
#include "src/core/SkFontPriv.h"
#include "src/core/SkTypefaceCache.h"
#include "src/sfnt/SkOTTable_OS_2.h"
#include "src/sfnt/SkOTTable_OS_2_V0.h"
#include "src/sfnt/SkSFNTHeader.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/fonts/TestEmptyTypeface.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>

static void TypefaceStyle_test(skiatest::Reporter* reporter,
                               uint16_t weight, uint16_t width, SkData* data)
{
    sk_sp<SkData> dataCopy;
    if (!data->unique()) {
        dataCopy = SkData::MakeWithCopy(data->data(), data->size());
        data = dataCopy.get();
    }
    SkSFNTHeader* sfntHeader = static_cast<SkSFNTHeader*>(data->writable_data());

    SkSFNTHeader::TableDirectoryEntry* tableEntry =
        SkTAfter<SkSFNTHeader::TableDirectoryEntry>(sfntHeader);
    SkSFNTHeader::TableDirectoryEntry* os2TableEntry = nullptr;
    int numTables = SkEndian_SwapBE16(sfntHeader->numTables);
    for (int tableEntryIndex = 0; tableEntryIndex < numTables; ++tableEntryIndex) {
        if (SkOTTableOS2::TAG == tableEntry[tableEntryIndex].tag) {
            os2TableEntry = tableEntry + tableEntryIndex;
            break;
        }
    }
    SkASSERT_RELEASE(os2TableEntry);

    size_t os2TableOffset = SkEndian_SwapBE32(os2TableEntry->offset);
    SkOTTableOS2_V0* os2Table = SkTAddOffset<SkOTTableOS2_V0>(sfntHeader, os2TableOffset);
    os2Table->usWeightClass.value = SkEndian_SwapBE16(weight);
    using WidthType = SkOTTableOS2_V0::WidthClass::Value;
    os2Table->usWidthClass.value = static_cast<WidthType>(SkEndian_SwapBE16(width));

    sk_sp<SkTypeface> newTypeface(ToolUtils::TestFontMgr()->makeFromData(sk_ref_sp(data)));
    if (!newTypeface) {
        // Not all SkFontMgr can MakeFromStream().
        return;
    }

    SkFontStyle newStyle = newTypeface->fontStyle();

    //printf("%d, %f\n", weight, (newStyle.weight() - (float)0x7FFF) / (float)0x7FFF);
    //printf("%d, %f\n", width , (newStyle.width()  - (float)0x7F)   / (float)0x7F);
    //printf("%d, %d\n", weight, newStyle.weight());
    //printf("%d, %d\n", width , newStyle.width());

    // Some back-ends (CG, GDI, DW) support OS/2 version A which uses 0 - 10 (but all differently).
    REPORTER_ASSERT(reporter,
                    newStyle.weight() == weight ||
                    (weight <=   10 && newStyle.weight() == 100 * weight) ||
                    (weight ==    4 && newStyle.weight() == 350) ||  // GDI weirdness
                    (weight ==    5 && newStyle.weight() == 400) ||  // GDI weirdness
                    (weight ==    0 && newStyle.weight() ==   1) ||  // DW weirdness
                    (weight == 1000 && newStyle.weight() == 999)     // DW weirdness
    );

    // Some back-ends (GDI) don't support width, ensure these always report 'medium'.
    REPORTER_ASSERT(reporter,
                    newStyle.width() == width ||
                    newStyle.width() == 5);
}
DEF_TEST(TypefaceStyle, reporter) {
    std::unique_ptr<SkStreamAsset> stream(GetResourceAsStream("fonts/Em.ttf"));
    if (!stream) {
        REPORT_FAILURE(reporter, "fonts/Em.ttf", SkString("Cannot load resource"));
        return;
    }
    sk_sp<SkData> data(SkData::MakeFromStream(stream.get(), stream->getLength()));

    using SkFS = SkFontStyle;
    for (int weight = SkFS::kInvisible_Weight; weight <= SkFS::kExtraBlack_Weight; ++weight) {
        TypefaceStyle_test(reporter, weight, 5, data.get());
    }
    for (int width = SkFS::kUltraCondensed_Width; width <= SkFS::kUltraExpanded_Width; ++width) {
        TypefaceStyle_test(reporter, 400, width, data.get());
    }
}

DEF_TEST(TypefacePostScriptName, reporter) {
    sk_sp<SkTypeface> typeface(ToolUtils::CreateTypefaceFromResource("fonts/Em.ttf"));
    if (!typeface) {
        // Not all SkFontMgr can MakeFromStream().
        return;
    }

    SkString postScriptName;
    bool hasName = typeface->getPostScriptName(&postScriptName);
    bool hasName2 = typeface->getPostScriptName(nullptr);
    REPORTER_ASSERT(reporter, hasName == hasName2);
    if (hasName) {
        REPORTER_ASSERT(reporter, postScriptName == SkString("Em"));
    }
}

DEF_TEST(TypefaceRoundTrip, reporter) {
    sk_sp<SkTypeface> typeface(ToolUtils::CreateTypefaceFromResource("fonts/7630.otf"));
    if (!typeface) {
        // Not all SkFontMgr can MakeFromStream().
        return;
    }

    int fontIndex;
    std::unique_ptr<SkStreamAsset> stream = typeface->openStream(&fontIndex);

    sk_sp<SkTypeface> typeface2 =
            ToolUtils::TestFontMgr()->makeFromStream(std::move(stream), fontIndex);
    REPORTER_ASSERT(reporter, typeface2);
}

DEF_TEST(FontDescriptorNegativeVariationSerialize, reporter) {
    SkFontDescriptor desc;
    SkFontStyle style(2, 9, SkFontStyle::kOblique_Slant);
    desc.setStyle(style);
    const char postscriptName[] = "postscript";
    desc.setPostscriptName(postscriptName);
    SkFontArguments::VariationPosition::Coordinate* variation = desc.setVariationCoordinates(1);
    variation[0] = { 0, -1.0f };

    SkDynamicMemoryWStream stream;
    desc.serialize(&stream);
    SkFontDescriptor descD;
    SkFontDescriptor::Deserialize(stream.detachAsStream().get(), &descD);

    REPORTER_ASSERT(reporter, descD.getStyle() == style);
    REPORTER_ASSERT(reporter, 0 == strcmp(desc.getPostscriptName(), postscriptName));
    if (descD.getVariationCoordinateCount() != 1) {
        REPORT_FAILURE(reporter, "descD.getVariationCoordinateCount() != 1", SkString());
        return;
    }

    REPORTER_ASSERT(reporter, descD.getVariation()[0].value == -1.0f);
}

DEF_TEST(TypefaceAxes, reporter) {
    using Variation = SkFontArguments::VariationPosition;
    // In DWrite in at least up to 1901 18363.1198 IDWriteFontFace5::GetFontAxisValues and
    // GetFontAxisValueCount along with IDWriteFontResource::GetFontAxisAttributes and
    // GetFontAxisCount (and related) seem to incorrectly collapse multiple axes with the same tag.
    // Since this is a limitation of the underlying implementation, for now allow the test to pass
    // with the axis tag count (as opposed to the axis count). Eventually all implementations should
    // pass this test without 'alsoAcceptedAxisTagCount'.
    auto test = [&](SkTypeface* typeface, const Variation& expected, int alsoAcceptedAxisTagCount) {
        if (!typeface) {
            return;  // Not all SkFontMgr can makeFromStream().
        }

        int actualCount = typeface->getVariationDesignPosition(nullptr, 0);
        if (actualCount == -1) {
            return;  // The number of axes is unknown.
        }
        REPORTER_ASSERT(reporter, actualCount == expected.coordinateCount ||
                                  actualCount == alsoAcceptedAxisTagCount);

        // Variable font conservative bounds don't vary, so ensure they aren't reported.
        REPORTER_ASSERT(reporter, typeface->getBounds().isEmpty());

        std::unique_ptr<Variation::Coordinate[]> actual(new Variation::Coordinate[actualCount]);
        actualCount = typeface->getVariationDesignPosition(actual.get(), actualCount);
        if (actualCount == -1) {
            return;  // The position cannot be determined.
        }
        REPORTER_ASSERT(reporter, actualCount == expected.coordinateCount ||
                                  actualCount == alsoAcceptedAxisTagCount);

        // Every actual must be expected.
        std::unique_ptr<bool[]> expectedUsed(new bool[expected.coordinateCount]());
        for (int actualIdx = 0; actualIdx < actualCount; ++actualIdx) {
            bool actualFound = false;
            for (int expectedIdx = 0; expectedIdx < expected.coordinateCount; ++expectedIdx) {
                if (expectedUsed[expectedIdx]) {
                    continue;
                }

                if (actual[actualIdx].axis != expected.coordinates[expectedIdx].axis) {
                    continue;
                }

                // Convert to fixed for "almost equal".
                SkFixed fixedRead = SkScalarToFixed(actual[actualIdx].value);
                SkFixed fixedOriginal = SkScalarToFixed(expected.coordinates[expectedIdx].value);
                if (!(SkTAbs(fixedRead - fixedOriginal) < 2)) {
                    continue;
                }

                // This actual matched an unused expected.
                actualFound = true;
                expectedUsed[expectedIdx] = true;
                break;
            }
            REPORTER_ASSERT(reporter, actualFound,
                "Actual axis '%c%c%c%c' with value '%f' not expected",
                (actual[actualIdx].axis >> 24) & 0xFF,
                (actual[actualIdx].axis >> 16) & 0xFF,
                (actual[actualIdx].axis >>  8) & 0xFF,
                (actual[actualIdx].axis      ) & 0xFF,
                SkScalarToDouble(actual[actualIdx].value));
        }
    };

    sk_sp<SkFontMgr> fm = ToolUtils::TestFontMgr();

    // Not specifying a position should produce the default.
    {
        std::unique_ptr<SkStreamAsset> variable(GetResourceAsStream("fonts/Variable.ttf"));
        if (!variable) {
            REPORT_FAILURE(reporter, "variable", SkString());
            return;
        }
        const Variation::Coordinate defaultPosition[] = {
            { SkSetFourByteTag('w','g','h','t'), 400.0f },
            { SkSetFourByteTag('w','d','t','h'), 100.0f },
        };
        sk_sp<SkTypeface> typeface = fm->makeFromStream(std::move(variable), 0);
        test(typeface.get(), Variation{&defaultPosition[0], 2}, -1);
    }

    // Multiple axes with the same tag (and min, max, default) works.
    {
        std::unique_ptr<SkStreamAsset> dupTags(GetResourceAsStream("fonts/VaryAlongQuads.ttf"));
        if (!dupTags) {
            REPORT_FAILURE(reporter, "dupTags", SkString());
            return;
        }

        // The position may be over specified. If there are multiple values for a given axis,
        // ensure the last one since that's what css-fonts-4 requires.
        const Variation::Coordinate position[] = {
            { SkSetFourByteTag('w','g','h','t'), 700.0f },
            { SkSetFourByteTag('w','g','h','t'), 600.0f },
            { SkSetFourByteTag('w','g','h','t'), 600.0f },
        };
        SkFontArguments params;
        params.setVariationDesignPosition({position, std::size(position)});
        sk_sp<SkTypeface> typeface = fm->makeFromStream(std::move(dupTags), params);
        test(typeface.get(), Variation{&position[1], 2}, 1);
    }

    // Overspecifying an axis tag value applies the last one in the list.
    {
        std::unique_ptr<SkStreamAsset> distortable(GetResourceAsStream("fonts/Distortable.ttf"));
        if (!distortable) {
            REPORT_FAILURE(reporter, "distortable", SkString());
            return;
        }

        // The position may be over specified. If there are multiple values for a given axis,
        // ensure the last one since that's what css-fonts-4 requires.
        const Variation::Coordinate position[] = {
            { SkSetFourByteTag('w','g','h','t'), 1.618033988749895f },
            { SkSetFourByteTag('w','g','h','t'), SK_ScalarSqrt2 },
        };
        SkFontArguments params;
        params.setVariationDesignPosition({position, std::size(position)});
        sk_sp<SkTypeface> typeface = fm->makeFromStream(std::move(distortable), params);
        test(typeface.get(), Variation{&position[1], 1}, -1);

        if (typeface) {
            // Cloning without specifying any parameters should produce an equivalent variation.
            sk_sp<SkTypeface> clone = typeface->makeClone(SkFontArguments());
            test(clone.get(), Variation{&position[1], 1}, -1);
        }
    }
}

DEF_TEST(TypefaceVariationIndex, reporter) {
    std::unique_ptr<SkStreamAsset> distortable(GetResourceAsStream("fonts/Distortable.ttf"));
    if (!distortable) {
        REPORT_FAILURE(reporter, "distortable", SkString());
        return;
    }

    sk_sp<SkFontMgr> fm = ToolUtils::TestFontMgr();
    SkFontArguments params;
    // The first named variation position in Distortable is 'Thin'.
    params.setCollectionIndex(0x00010000);
    sk_sp<SkTypeface> typeface = fm->makeFromStream(std::move(distortable), params);
    if (!typeface) {
        // FreeType is the only weird thing that supports this, Skia just needs to make sure if it
        // gets one of these things make sense.
        return;
    }

    int count = typeface->getVariationDesignPosition(nullptr, 0);
    if (!(count == 1)) {
        REPORT_FAILURE(reporter, "count == 1", SkString());
        return;
    }

    SkFontArguments::VariationPosition::Coordinate positionRead[1];
    count = typeface->getVariationDesignPosition(positionRead, std::size(positionRead));
    if (count == -1) {
        return;
    }
    if (!(count == 1)) {
        REPORT_FAILURE(reporter, "count == 1", SkString());
        return;
    }
    REPORTER_ASSERT(reporter, positionRead[0].axis == SkSetFourByteTag('w','g','h','t'));
    REPORTER_ASSERT(reporter, positionRead[0].value == 0.5);
}

DEF_TEST(Typeface, reporter) {

    sk_sp<SkTypeface> t1(ToolUtils::CreateTestTypeface(nullptr, SkFontStyle()));
    sk_sp<SkTypeface> t2(ToolUtils::DefaultTypeface());

    REPORTER_ASSERT(reporter, SkTypeface::Equal(t1.get(), t2.get()));
    REPORTER_ASSERT(reporter, SkTypeface::Equal(nullptr, nullptr));

    REPORTER_ASSERT(reporter, !SkTypeface::Equal(nullptr, t1.get()));
    REPORTER_ASSERT(reporter, !SkTypeface::Equal(nullptr, t2.get()));
    REPORTER_ASSERT(reporter, !SkTypeface::Equal(t1.get(), nullptr));
    REPORTER_ASSERT(reporter, !SkTypeface::Equal(t2.get(), nullptr));
}

DEF_TEST(TypefaceAxesParameters, reporter) {
    using Axis = SkFontParameters::Variation::Axis;

    // In DWrite in at least up to 1901 18363.1198 IDWriteFontFace5::GetFontAxisValues and
    // GetFontAxisValueCount along with IDWriteFontResource::GetFontAxisAttributes and
    // GetFontAxisCount (and related) seem to incorrectly collapse multiple axes with the same tag.
    // Since this is a limitation of the underlying implementation, for now allow the test to pass
    // with the axis tag count (as opposed to the axis count). Eventually all implementations should
    // pass this test without 'alsoAcceptedAxisTagCount'.
    auto test = [&](SkTypeface* typeface, const Axis* expected, int expectedCount,
                    int alsoAcceptedAxisTagCount)
    {
        if (!typeface) {
            return;  // Not all SkFontMgr can makeFromStream().
        }

        int actualCount = typeface->getVariationDesignParameters(nullptr, 0);
        if (actualCount == -1) {
            return;  // The number of axes is unknown.
        }
        REPORTER_ASSERT(reporter, actualCount == expectedCount ||
                                  actualCount == alsoAcceptedAxisTagCount);

        std::unique_ptr<Axis[]> actual(new Axis[actualCount]);
        actualCount = typeface->getVariationDesignParameters(actual.get(), actualCount);
        if (actualCount == -1) {
            return;  // The position cannot be determined.
        }
        REPORTER_ASSERT(reporter, actualCount == expectedCount ||
                                  actualCount == alsoAcceptedAxisTagCount);

        // Every actual must be expected.
        std::unique_ptr<bool[]> expectedUsed(new bool[expectedCount]());
        for (int actualIdx = 0; actualIdx < actualCount; ++actualIdx) {
            bool actualFound = false;
            for (int expectedIdx = 0; expectedIdx < expectedCount; ++expectedIdx) {
                if (expectedUsed[expectedIdx]) {
                    continue;
                }

                if (actual[actualIdx].tag != expected[expectedIdx].tag) {
                    continue;
                }

                // Convert to fixed for "almost equal".
                SkFixed fixedActualMin = SkScalarToFixed(actual[actualIdx].min);
                SkFixed fixedExpectedMin = SkScalarToFixed(expected[expectedIdx].min);
                if (!(SkTAbs(fixedActualMin - fixedExpectedMin) < 2)) {
                    continue;
                }

                SkFixed fixedActualMax = SkScalarToFixed(actual[actualIdx].max);
                SkFixed fixedExpectedMax = SkScalarToFixed(expected[expectedIdx].max);
                if (!(SkTAbs(fixedActualMax - fixedExpectedMax) < 2)) {
                    continue;
                }

                SkFixed fixedActualDefault = SkScalarToFixed(actual[actualIdx].def);
                SkFixed fixedExpectedDefault = SkScalarToFixed(expected[expectedIdx].def);
                if (!(SkTAbs(fixedActualDefault - fixedExpectedDefault) < 2)) {
                    continue;
                }

                // This seems silly, but allows MSAN to ensure that isHidden is initialized.
                // In GDI or before macOS 10.12, Win10, or FreeType 2.8.1 API for hidden is missing.
                if (actual[actualIdx].isHidden() &&
                    actual[actualIdx].isHidden() != expected[expectedIdx].isHidden())
                {
                    continue;
                }

                // This actual matched an unused expected.
                actualFound = true;
                expectedUsed[expectedIdx] = true;
                break;
            }
            REPORTER_ASSERT(reporter, actualFound,
                "Actual axis '%c%c%c%c' with min %f max %f default %f hidden %s not expected",
                (actual[actualIdx].tag >> 24) & 0xFF,
                (actual[actualIdx].tag >> 16) & 0xFF,
                (actual[actualIdx].tag >>  8) & 0xFF,
                (actual[actualIdx].tag      ) & 0xFF,
                actual[actualIdx].min,
                actual[actualIdx].def,
                actual[actualIdx].max,
                actual[actualIdx].isHidden() ? "true" : "false");
        }
    };

    sk_sp<SkFontMgr> fm = ToolUtils::TestFontMgr();

    // Two axis OpenType variable font.
    {
        std::unique_ptr<SkStreamAsset> variable(GetResourceAsStream("fonts/Variable.ttf"));
        if (!variable) {
            REPORT_FAILURE(reporter, "variable", SkString());
            return;
        }
        constexpr Axis expected[] = {
            Axis(SkSetFourByteTag('w','g','h','t'), 100.0f, 400.0f, 900.0f, true ),
            Axis(SkSetFourByteTag('w','d','t','h'),  50.0f, 100.0f, 200.0f, false),
        };
        sk_sp<SkTypeface> typeface = fm->makeFromStream(std::move(variable), 0);
        test(typeface.get(), &expected[0], std::size(expected), -1);
    }

    // Multiple axes with the same tag (and min, max, default) works.
    {
        std::unique_ptr<SkStreamAsset> dupTags(GetResourceAsStream("fonts/VaryAlongQuads.ttf"));
        if (!dupTags) {
            REPORT_FAILURE(reporter, "dupTags", SkString());
            return;
        }

        // The position may be over specified. If there are multiple values for a given axis,
        // ensure the last one since that's what css-fonts-4 requires.
        constexpr Axis expected[] = {
            Axis(SkSetFourByteTag('w','g','h','t'), 100.0f, 400.0f, 900.0f, false),
            Axis(SkSetFourByteTag('w','g','h','t'), 100.0f, 400.0f, 900.0f, false),
        };
        sk_sp<SkTypeface> typeface = fm->makeFromStream(std::move(dupTags), 0);
        test(typeface.get(), &expected[0], std::size(expected), 1);
    }

    // Simple single axis GX variable font.
    {
        std::unique_ptr<SkStreamAsset> distortable(GetResourceAsStream("fonts/Distortable.ttf"));
        if (!distortable) {
            REPORT_FAILURE(reporter, "distortable", SkString());
            return;
        }
        constexpr Axis expected[] = {
            Axis(SkSetFourByteTag('w','g','h','t'), 0.5f, 1.0f, 2.0f, true),
        };
        sk_sp<SkTypeface> typeface = fm->makeFromStream(std::move(distortable), 0);
        test(typeface.get(), &expected[0], std::size(expected), -1);
    }
}

static bool count_proc(SkTypeface* face, void* ctx) {
    int* count = static_cast<int*>(ctx);
    *count = *count + 1;
    return false;
}
static int count(skiatest::Reporter* reporter, const SkTypefaceCache& cache) {
    int count = 0;
    sk_sp<SkTypeface> none = cache.findByProcAndRef(count_proc, &count);
    REPORTER_ASSERT(reporter, none == nullptr);
    return count;
}

DEF_TEST(TypefaceCache, reporter) {
    sk_sp<SkTypeface> t1(TestEmptyTypeface::Make());
    {
        SkTypefaceCache cache;
        REPORTER_ASSERT(reporter, count(reporter, cache) == 0);
        {
            sk_sp<SkTypeface> t0(TestEmptyTypeface::Make());
            cache.add(t0);
            REPORTER_ASSERT(reporter, count(reporter, cache) == 1);
            cache.add(t1);
            REPORTER_ASSERT(reporter, count(reporter, cache) == 2);
            cache.purgeAll();
            REPORTER_ASSERT(reporter, count(reporter, cache) == 2);
        }
        REPORTER_ASSERT(reporter, count(reporter, cache) == 2);
        cache.purgeAll();
        REPORTER_ASSERT(reporter, count(reporter, cache) == 1);
    }
    REPORTER_ASSERT(reporter, t1->unique());
}

static void check_serialize_behaviors(sk_sp<SkTypeface> tf, skiatest::Reporter* reporter) {
    if (!tf) {
        return;
    }

    SkFontDescriptor desc;
    bool serialize;
    tf->getFontDescriptor(&desc, &serialize);

    auto data0 = tf->serialize(SkTypeface::SerializeBehavior::kDoIncludeData);
    auto data1 = tf->serialize(SkTypeface::SerializeBehavior::kDontIncludeData);
    auto data2 = tf->serialize(SkTypeface::SerializeBehavior::kIncludeDataIfLocal);

    REPORTER_ASSERT(reporter, data0->size() >= data1->size());

    if (serialize) {
        REPORTER_ASSERT(reporter, data0->equals(data2.get()));
    } else {
        REPORTER_ASSERT(reporter, data1->equals(data2.get()));
    }
}

DEF_TEST(Typeface_serialize, reporter) {
    check_serialize_behaviors(ToolUtils::DefaultTypeface(), reporter);
    check_serialize_behaviors(
            ToolUtils::TestFontMgr()->makeFromStream(GetResourceAsStream("fonts/Distortable.ttf")),
            reporter);
}

DEF_TEST(Typeface_glyph_to_char, reporter) {
    SkFont font(ToolUtils::EmojiTypeface(), 12);
    SkASSERT(font.getTypeface());
    char const * text = ToolUtils::EmojiSampleText();
    size_t const textLen = strlen(text);
    SkString familyName;
    font.getTypeface()->getFamilyName(&familyName);

    size_t const codepointCount = SkUTF::CountUTF8(text, textLen);
    char const * const textEnd = text + textLen;
    std::unique_ptr<SkUnichar[]> originalCodepoints(new SkUnichar[codepointCount]);
    for (size_t i = 0; i < codepointCount; ++i) {
        originalCodepoints[i] = SkUTF::NextUTF8(&text, textEnd);
    }
    std::unique_ptr<SkGlyphID[]> glyphs(new SkGlyphID[codepointCount]);
    font.unicharsToGlyphs(originalCodepoints.get(), codepointCount, glyphs.get());
    if (std::any_of(glyphs.get(), glyphs.get()+codepointCount, [](SkGlyphID g){ return g == 0;})) {
        ERRORF(reporter, "Unexpected typeface \"%s\". Expected full support for emoji_sample_text.",
               familyName.c_str());
        return;
    }

    std::unique_ptr<SkUnichar[]> newCodepoints(new SkUnichar[codepointCount]);
    SkFontPriv::GlyphsToUnichars(font, glyphs.get(), codepointCount, newCodepoints.get());

    for (size_t i = 0; i < codepointCount; ++i) {
        // GDI does not support character to glyph mapping outside BMP.
        if (ToolUtils::FontMgrIsGDI() && 0xFFFF < originalCodepoints[i] && newCodepoints[i] == 0) {
            continue;
        }
        // If two codepoints map to the same glyph then this assert is not valid.
        // However, the emoji test font should never have multiple characters map to the same glyph.
        REPORTER_ASSERT(reporter, originalCodepoints[i] == newCodepoints[i],
                        "name:%s i:%zu original:%d new:%d glyph:%d", familyName.c_str(), i,
                        originalCodepoints[i], newCodepoints[i], glyphs[i]);
    }
}

// This test makes sure the legacy typeface creation does not lose its specified
// style. See https://bugs.chromium.org/p/skia/issues/detail?id=8447 for more
// context.
DEF_TEST(LegacyMakeTypeface, reporter) {
    sk_sp<SkFontMgr> fm = ToolUtils::TestFontMgr();
    sk_sp<SkTypeface> typeface1 = fm->legacyMakeTypeface(nullptr, SkFontStyle::Italic());
    sk_sp<SkTypeface> typeface2 = fm->legacyMakeTypeface(nullptr, SkFontStyle::Bold());
    sk_sp<SkTypeface> typeface3 = fm->legacyMakeTypeface(nullptr, SkFontStyle::BoldItalic());

    REPORTER_ASSERT(reporter, typeface1->isItalic());
    REPORTER_ASSERT(reporter, !typeface1->isBold());
    REPORTER_ASSERT(reporter, !typeface2->isItalic());
    REPORTER_ASSERT(reporter, typeface2->isBold());
    REPORTER_ASSERT(reporter, typeface3->isItalic());
    REPORTER_ASSERT(reporter, typeface3->isBold());
}
