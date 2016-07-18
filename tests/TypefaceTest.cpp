/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRefCnt.h"
#include "SkTypeface.h"
#include "SkTypefaceCache.h"
#include "Test.h"

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
    static sk_sp<SkTypeface> Create(SkFontID id) { return sk_sp<SkTypeface>(new SkEmptyTypeface(id)); }
protected:
    SkEmptyTypeface(SkFontID id) : SkTypeface(SkFontStyle(), id, true) { }

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
    int onCountGlyphs() const override { return 0; };
    int onGetUPEM() const override { return 0; };
    void onGetFamilyName(SkString* familyName) const override { familyName->reset(); }
    SkTypeface::LocalizedStrings* onCreateFamilyNameIterator() const override {
        SK_ABORT("unimplemented");
        return nullptr;
    };
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
    sk_sp<SkTypeface> t1(SkEmptyTypeface::Create(1));
    {
        SkTypefaceCache cache;
        REPORTER_ASSERT(reporter, count(reporter, cache) == 0);
        {
            sk_sp<SkTypeface> t0(SkEmptyTypeface::Create(0));
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
