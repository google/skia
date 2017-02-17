/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkOTTable_OS_2.h"
#include "SkSFNTHeader.h"
#include "SkStream.h"
#include "SkRefCnt.h"
#include "SkTypeface.h"
#include "SkTypefaceCache.h"
#include "Resources.h"
#include "Test.h"

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

    sk_sp<SkTypeface> newTypeface(SkTypeface::MakeFromStream(new SkMemoryStream(sk_ref_sp(data))));
    SkASSERT_RELEASE(newTypeface);

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
    std::unique_ptr<SkStreamAsset> stream(GetResourceAsStream("/fonts/Em.ttf"));
    if (!stream) {
        REPORT_FAILURE(reporter, "/fonts/Em.ttf", SkString("Cannot load resource"));
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

DEF_TEST(Typeface, reporter) {

    sk_sp<SkTypeface> t1(SkTypeface::MakeFromName(nullptr, SkFontStyle()));
    sk_sp<SkTypeface> t2(SkTypeface::MakeDefault(SkTypeface::kNormal));

    REPORTER_ASSERT(reporter, SkTypeface::Equal(t1.get(), t2.get()));
    REPORTER_ASSERT(reporter, SkTypeface::Equal(0, t1.get()));
    REPORTER_ASSERT(reporter, SkTypeface::Equal(0, t2.get()));
    REPORTER_ASSERT(reporter, SkTypeface::Equal(t1.get(), 0));
    REPORTER_ASSERT(reporter, SkTypeface::Equal(t2.get(), 0));

#ifdef SK_BUILD_FOR_ANDROID
    sk_sp<SkTypeface> t3(SkTypeface::MakeFromName("non-existent-font", SkFontStyle()));
    REPORTER_ASSERT(reporter, nullptr == t3);
#endif
}

namespace {

class SkEmptyTypeface : public SkTypeface {
public:
    static sk_sp<SkTypeface> Create() { return sk_sp<SkTypeface>(new SkEmptyTypeface()); }
protected:
    SkEmptyTypeface() : SkTypeface(SkFontStyle(), true) { }

    SkStreamAsset* onOpenStream(int* ttcIndex) const override { return nullptr; }
    SkScalerContext* onCreateScalerContext(const SkScalerContextEffects&,
                                           const SkDescriptor*) const override {
        return nullptr;
    }
    void onFilterRec(SkScalerContextRec*) const override { }
    virtual SkAdvancedTypefaceMetrics* onGetAdvancedTypefaceMetrics(
                                PerGlyphInfo,
                                const uint32_t*, uint32_t) const override { return nullptr; }
    void onGetFontDescriptor(SkFontDescriptor*, bool*) const override { }
    virtual int onCharsToGlyphs(const void* chars, Encoding encoding,
                                uint16_t glyphs[], int glyphCount) const override {
        SK_ABORT("unimplemented");
        return 0;
    }
    int onCountGlyphs() const override { return 0; }
    int onGetUPEM() const override { return 0; }
    void onGetFamilyName(SkString* familyName) const override { familyName->reset(); }
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override {
        SK_ABORT("unimplemented");
        return nullptr;
    }
    int onGetTableTags(SkFontTableTag tags[]) const override { return 0; }
    size_t onGetTableData(SkFontTableTag, size_t, size_t, void*) const override { return 0; }
};

}

static bool count_proc(SkTypeface* face, void* ctx) {
    int* count = static_cast<int*>(ctx);
    *count = *count + 1;
    return false;
}
static int count(skiatest::Reporter* reporter, const SkTypefaceCache& cache) {
    int count = 0;
    SkTypeface* none = cache.findByProcAndRef(count_proc, &count);
    REPORTER_ASSERT(reporter, none == nullptr);
    return count;
}

DEF_TEST(TypefaceCache, reporter) {
    sk_sp<SkTypeface> t1(SkEmptyTypeface::Create());
    {
        SkTypefaceCache cache;
        REPORTER_ASSERT(reporter, count(reporter, cache) == 0);
        {
            sk_sp<SkTypeface> t0(SkEmptyTypeface::Create());
            cache.add(t0.get());
            REPORTER_ASSERT(reporter, count(reporter, cache) == 1);
            cache.add(t1.get());
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
