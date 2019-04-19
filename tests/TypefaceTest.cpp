/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkAdvancedTypefaceMetrics.h"
#include "SkData.h"
#include "SkFixed.h"
#include "SkFontDescriptor.h"
#include "SkFontMgr.h"
#include "SkMakeUnique.h"
#include "SkOTTable_OS_2.h"
#include "SkRefCnt.h"
#include "SkSFNTHeader.h"
#include "SkStream.h"
#include "SkTypeface.h"
#include "SkTypefaceCache.h"
#include "Test.h"
#include "TestEmptyTypeface.h"

#include <memory>

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

    sk_sp<SkTypeface> newTypeface(SkTypeface::MakeFromData(sk_ref_sp(data)));
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

DEF_TEST(TypefaceRoundTrip, reporter) {
    sk_sp<SkTypeface> typeface(MakeResourceAsTypeface("fonts/7630.otf"));
    if (!typeface) {
        // Not all SkFontMgr can MakeFromStream().
        return;
    }

    int fontIndex;
    std::unique_ptr<SkStreamAsset> stream = typeface->openStream(&fontIndex);

    sk_sp<SkFontMgr> fm = SkFontMgr::RefDefault();
    sk_sp<SkTypeface> typeface2 = fm->makeFromStream(std::move(stream), fontIndex);
    REPORTER_ASSERT(reporter, typeface2);
}

DEF_TEST(FontDescriptorNegativeVariationSerialize, reporter) {
    SkFontDescriptor desc;
    SkFixed axis = -SK_Fixed1;
    auto font = skstd::make_unique<SkMemoryStream>("a", 1, false);
    desc.setFontData(skstd::make_unique<SkFontData>(std::move(font), 0, &axis, 1));

    SkDynamicMemoryWStream stream;
    desc.serialize(&stream);
    SkFontDescriptor descD;
    SkFontDescriptor::Deserialize(stream.detachAsStream().get(), &descD);
    std::unique_ptr<SkFontData> fontData = descD.detachFontData();
    if (!fontData) {
        REPORT_FAILURE(reporter, "fontData", SkString());
        return;
    }

    if (fontData->getAxisCount() != 1) {
        REPORT_FAILURE(reporter, "fontData->getAxisCount() != 1", SkString());
        return;
    }

    REPORTER_ASSERT(reporter, fontData->getAxis()[0] == -SK_Fixed1);
};

DEF_TEST(TypefaceAxes, reporter) {
    std::unique_ptr<SkStreamAsset> distortable(GetResourceAsStream("fonts/Distortable.ttf"));
    if (!distortable) {
        REPORT_FAILURE(reporter, "distortable", SkString());
        return;
    }
    constexpr int numberOfAxesInDistortable = 1;

    sk_sp<SkFontMgr> fm = SkFontMgr::RefDefault();
    // The position may be over specified. If there are multiple values for a given axis,
    // ensure the last one since that's what css-fonts-4 requires.
    const SkFontArguments::VariationPosition::Coordinate position[] = {
        { SkSetFourByteTag('w','g','h','t'), 1.618033988749895f },
        { SkSetFourByteTag('w','g','h','t'), SK_ScalarSqrt2 },
    };
    SkFontArguments params;
    params.setVariationDesignPosition({position, SK_ARRAY_COUNT(position)});
    // TODO: if axes are set and the back-end doesn't support them, should we create the typeface?
    sk_sp<SkTypeface> typeface = fm->makeFromStream(std::move(distortable), params);

    if (!typeface) {
        // Not all SkFontMgr can makeFromStream().
        return;
    }

    int count = typeface->getVariationDesignPosition(nullptr, 0);
    if (count == -1) {
        return;
    }
    REPORTER_ASSERT(reporter, count == numberOfAxesInDistortable);

    SkFontArguments::VariationPosition::Coordinate positionRead[numberOfAxesInDistortable];
    count = typeface->getVariationDesignPosition(positionRead, SK_ARRAY_COUNT(positionRead));
    REPORTER_ASSERT(reporter, count == SK_ARRAY_COUNT(positionRead));

    REPORTER_ASSERT(reporter, positionRead[0].axis == position[1].axis);

    // Convert to fixed for "almost equal".
    SkFixed fixedRead = SkScalarToFixed(positionRead[0].value);
    SkFixed fixedOriginal = SkScalarToFixed(position[1].value);
    REPORTER_ASSERT(reporter, SkTAbs(fixedRead - fixedOriginal) < 2);
}

DEF_TEST(TypefaceVariationIndex, reporter) {
    std::unique_ptr<SkStreamAsset> distortable(GetResourceAsStream("fonts/Distortable.ttf"));
    if (!distortable) {
        REPORT_FAILURE(reporter, "distortable", SkString());
        return;
    }

    sk_sp<SkFontMgr> fm = SkFontMgr::RefDefault();
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
    count = typeface->getVariationDesignPosition(positionRead, SK_ARRAY_COUNT(positionRead));
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

    sk_sp<SkTypeface> t1(SkTypeface::MakeFromName(nullptr, SkFontStyle()));
    sk_sp<SkTypeface> t2(SkTypeface::MakeDefault());

    REPORTER_ASSERT(reporter, SkTypeface::Equal(t1.get(), t2.get()));
    REPORTER_ASSERT(reporter, SkTypeface::Equal(nullptr, t1.get()));
    REPORTER_ASSERT(reporter, SkTypeface::Equal(nullptr, t2.get()));
    REPORTER_ASSERT(reporter, SkTypeface::Equal(t1.get(), nullptr));
    REPORTER_ASSERT(reporter, SkTypeface::Equal(t2.get(), nullptr));
}

DEF_TEST(TypefaceAxesParameters, reporter) {
    std::unique_ptr<SkStreamAsset> distortable(GetResourceAsStream("fonts/Distortable.ttf"));
    if (!distortable) {
        REPORT_FAILURE(reporter, "distortable", SkString());
        return;
    }
    constexpr int numberOfAxesInDistortable = 1;
    constexpr SkScalar minAxisInDistortable = 0.5;
    constexpr SkScalar defAxisInDistortable = 1;
    constexpr SkScalar maxAxisInDistortable = 2;
    constexpr bool axisIsHiddenInDistortable = false;

    sk_sp<SkFontMgr> fm = SkFontMgr::RefDefault();

    SkFontArguments params;
    sk_sp<SkTypeface> typeface = fm->makeFromStream(std::move(distortable), params);

    if (!typeface) {
        // Not all SkFontMgr can makeFromStream().
        return;
    }

    SkFontParameters::Variation::Axis parameter[numberOfAxesInDistortable];
    int count = typeface->getVariationDesignParameters(parameter, SK_ARRAY_COUNT(parameter));
    if (count == -1) {
        return;
    }

    REPORTER_ASSERT(reporter, count == SK_ARRAY_COUNT(parameter));
    REPORTER_ASSERT(reporter, parameter[0].min == minAxisInDistortable);
    REPORTER_ASSERT(reporter, parameter[0].def == defAxisInDistortable);
    REPORTER_ASSERT(reporter, parameter[0].max == maxAxisInDistortable);
    REPORTER_ASSERT(reporter, parameter[0].tag == SkSetFourByteTag('w','g','h','t'));
    REPORTER_ASSERT(reporter, parameter[0].isHidden() == axisIsHiddenInDistortable);

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

static void check_serialize_behaviors(sk_sp<SkTypeface> tf, bool isLocalData,
                                      skiatest::Reporter* reporter) {
    if (!tf) {
        return;
    }
    auto data0 = tf->serialize(SkTypeface::SerializeBehavior::kDoIncludeData);
    auto data1 = tf->serialize(SkTypeface::SerializeBehavior::kDontIncludeData);
    auto data2 = tf->serialize(SkTypeface::SerializeBehavior::kIncludeDataIfLocal);

    REPORTER_ASSERT(reporter, data0->size() >= data1->size());

    if (isLocalData) {
        REPORTER_ASSERT(reporter, data0->equals(data2.get()));
    } else {
        REPORTER_ASSERT(reporter, data1->equals(data2.get()));
    }
}

DEF_TEST(Typeface_serialize, reporter) {
    check_serialize_behaviors(SkTypeface::MakeDefault(), false, reporter);
    check_serialize_behaviors(SkTypeface::MakeFromStream(
                                         GetResourceAsStream("fonts/Distortable.ttf")),
                              true, reporter);

}

