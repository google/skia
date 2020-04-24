/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontMgr.h"
#include "include/core/SkTypeface.h"
#include "src/sfnt/SkOTTable_name.h"
#include "tests/Test.h"
#include "tools/flags/CommandLineFlags.h"

#include <stddef.h>

namespace {

template <size_t R, size_t D> struct Format0NameTable {
    SkOTTableName header;
    SkOTTableName::Record nameRecord[R];
    char data[D];
};

template <size_t R, size_t L, size_t D> struct Format1NameTable {
    SkOTTableName header;
    SkOTTableName::Record nameRecord[R];
    struct {
        SkOTTableName::Format1Ext header;
        SkOTTableName::Format1Ext::LangTagRecord langTagRecord[L];
    } format1ext;
    char data[D];
};

typedef Format0NameTable<1, 9> SimpleFormat0NameTable;
constexpr SimpleFormat0NameTable simpleFormat0NameTable = {
    /*header*/ {
        /*format*/ SkOTTableName::format_0,
        /*count*/ SkTEndianSwap16<1>::value,
        /*stringOffset*/ SkTEndianSwap16<offsetof(SimpleFormat0NameTable, data)>::value,
    },
    /*nameRecord[]*/ {
        /*Record*/ {
            /*platformID*/ { SkOTTableName::Record::PlatformID::Windows },
            /*encodingID*/ { SkOTTableName::Record::EncodingID::Windows::UnicodeBMPUCS2 },
            /*languageID*/ { SkOTTableName::Record::LanguageID::Windows::English_UnitedStates },
            /*nameID*/ { SkOTTableName::Record::NameID::Predefined::FontFamilyName },
            /*length*/ SkTEndianSwap16<8>::value,
            /*offset*/ SkTEndianSwap16<0>::value,
        }
    },
    /*data*/ "\x0" "T" "\x0" "e" "\x0" "s" "\x0" "t",
};

typedef Format1NameTable<1, 1, 19> SimpleFormat1NameTable;
constexpr SimpleFormat1NameTable simpleFormat1NameTable = {
    /*header*/ {
        /*format*/ SkOTTableName::format_1,
        /*count*/ SkTEndianSwap16<1>::value,
        /*stringOffset*/ SkTEndianSwap16<offsetof(SimpleFormat1NameTable, data)>::value,
    },
    /*nameRecord[]*/ {
        /*Record*/ {
            /*platformID*/ { SkOTTableName::Record::PlatformID::Windows },
            /*encodingID*/ { SkOTTableName::Record::EncodingID::Windows::UnicodeBMPUCS2 },
            /*languageID*/ { SkTEndianSwap16<0x8000 + 0>::value },
            /*nameID*/ { SkOTTableName::Record::NameID::Predefined::FontFamilyName },
            /*length*/ SkTEndianSwap16<8>::value,
            /*offset*/ SkTEndianSwap16<0>::value,
        }
    },
    /*format1ext*/ {
        /*header*/ {
            /*langTagCount*/ SkTEndianSwap16<1>::value,
        },
        /*langTagRecord[]*/ {
            /*LangTagRecord*/ {
                /*length*/ SkTEndianSwap16<10>::value,
                /*offset*/ SkTEndianSwap16<8>::value,
            },
        },
    },
    /*data*/ "\x0" "T" "\x0" "e" "\x0" "s" "\x0" "t"
             "\x0" "e" "\x0" "n" "\x0" "-" "\x0" "U" "\x0" "S",
};

struct FontNamesTest {
    const uint8_t* data;
    size_t size;
    SkOTTableName::Record::NameID nameID;
    size_t nameCount;
    struct {
        const char* name;
        const char* language;
    } names[10];

} tests[] = {
    {
        reinterpret_cast<const uint8_t*>(&simpleFormat0NameTable),
        sizeof(simpleFormat0NameTable),
        { SkOTTableName::Record::NameID::Predefined::FontFamilyName },
        1,
        {
            { "Test", "en-US" },
        },
    },
    {
        reinterpret_cast<const uint8_t*>(&simpleFormat1NameTable),
        sizeof(simpleFormat1NameTable),
        { SkOTTableName::Record::NameID::Predefined::FontFamilyName },
        1,
        {
            { "Test", "en-US" },
        },
    },
};

static void test_synthetic(skiatest::Reporter* reporter, bool verbose) {
    for (const auto& test : tests) {
        SkOTTableName::Iterator iter(test.data, test.size, test.nameID.predefined.value);
        SkOTTableName::Iterator::Record record;
        size_t nameIndex = 0;
        while (nameIndex < test.nameCount && iter.next(record)) {
            REPORTER_ASSERT(reporter,
                            strcmp(test.names[nameIndex].name, record.name.c_str()) == 0,
                            "Name did not match.");

            REPORTER_ASSERT(reporter,
                            strcmp(test.names[nameIndex].language, record.language.c_str()) == 0,
                            "Language did not match.");

            if (verbose) {
                SkDebugf("%s <%s>\n", record.name.c_str(), record.language.c_str());
            }

            ++nameIndex;
        }

        REPORTER_ASSERT(reporter, nameIndex == test.nameCount, "Fewer names than expected.");

        REPORTER_ASSERT(reporter, !iter.next(record), "More names than expected.");
    }
}

#define MAX_FAMILIES 1000
static void test_systemfonts(skiatest::Reporter* reporter, bool verbose) {
    static const SkFontTableTag nameTag = SkSetFourByteTag('n','a','m','e');

    sk_sp<SkFontMgr> fm(SkFontMgr::RefDefault());
    int count = SkMin32(fm->countFamilies(), MAX_FAMILIES);
    for (int i = 0; i < count; ++i) {
        sk_sp<SkFontStyleSet> set(fm->createStyleSet(i));
        for (int j = 0; j < set->count(); ++j) {
            SkString sname;
            SkFontStyle fs;
            set->getStyle(j, &fs, &sname);

            sk_sp<SkTypeface> typeface(set->createTypeface(j));

            SkString familyName;
            typeface->getFamilyName(&familyName);
            if (verbose) {
                SkDebugf("[%s]\n", familyName.c_str());
            }

            sk_sp<SkTypeface::LocalizedStrings> familyNamesIter(
                typeface->createFamilyNameIterator());
            SkTypeface::LocalizedString familyNameLocalized;
            while (familyNamesIter->next(&familyNameLocalized)) {
                if (verbose) {
                    SkDebugf("(%s) <%s>\n", familyNameLocalized.fString.c_str(),
                                            familyNameLocalized.fLanguage.c_str());
                }
            }

            size_t nameTableSize = typeface->getTableSize(nameTag);
            if (0 == nameTableSize) {
                continue;
            }
            SkAutoTMalloc<uint8_t> nameTableData(nameTableSize);
            size_t copied = typeface->getTableData(nameTag, 0, nameTableSize, nameTableData.get());
            if (copied != nameTableSize) {
                continue;
            }

            SkOTTableName::Iterator::Record record;
            SkOTTableName::Iterator familyNameIter(nameTableData.get(), nameTableSize,
                SkOTTableName::Record::NameID::Predefined::FontFamilyName);
            while (familyNameIter.next(record)) {
                REPORTER_ASSERT(
                        reporter,
                        SkOTTableName::Record::NameID::Predefined::FontFamilyName == record.type,
                        "Requested family name, got something else.");
                if (verbose) {
                    SkDebugf("{%s} <%s>\n", record.name.c_str(), record.language.c_str());
                }
            }

            SkOTTableName::Iterator styleNameIter(nameTableData.get(), nameTableSize,
                SkOTTableName::Record::NameID::Predefined::FontSubfamilyName);
            while (styleNameIter.next(record)) {
                REPORTER_ASSERT(
                        reporter,
                        SkOTTableName::Record::NameID::Predefined::FontSubfamilyName == record.type,
                        "Requested subfamily name, got something else.");
                if (verbose) {
                    SkDebugf("{{%s}} <%s>\n", record.name.c_str(), record.language.c_str());
                }
            }

            if (verbose) {
                SkDebugf("\n");
            }
        }
    }
}

} // namespace

static DEFINE_bool(verboseFontNames, false, "verbose FontNames test.");

DEF_TEST(FontNames, reporter) {
    test_synthetic(reporter, FLAGS_verboseFontNames);
    test_systemfonts(reporter, FLAGS_verboseFontNames);
}
