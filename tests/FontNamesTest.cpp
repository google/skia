/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "SkOTTable_name.h"
#include "SkTypeface.h"

#include <stddef.h>

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
SimpleFormat0NameTable simpleFormat0NameTable = {
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
SimpleFormat1NameTable simpleFormat1NameTable = {
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
    SkOTTableName* data;
    SkOTTableName::Record::NameID nameID;
    size_t nameCount;
    struct {
        const char* name;
        const char* language;
    } names[10];

} test[] = {
    {
        (SkOTTableName*)&simpleFormat0NameTable,
        { SkOTTableName::Record::NameID::Predefined::FontFamilyName },
        1,
        {
            { "Test", "en-US" },
        },
    },
    {
        (SkOTTableName*)&simpleFormat1NameTable,
        { SkOTTableName::Record::NameID::Predefined::FontFamilyName },
        1,
        {
            { "Test", "en-US" },
        },
    },
};

static void TestFontNames(skiatest::Reporter* reporter) {
    static const char* interestingFont[] = {
        "Arial",
        "Times New Roman",
        "MS PGothic", // Has Japanese name.
        "Wingdings", // Uses 'Symbol' name encoding.
    };
    static const SkFontTableTag nameTag = SkSetFourByteTag('n','a','m','e');

    for (size_t i = 0; i < SK_ARRAY_COUNT(interestingFont); ++i) {
        SkAutoTUnref<SkTypeface> typeface(SkTypeface::CreateFromName(interestingFont[i],
                                          SkTypeface::kNormal));
        if (NULL == typeface.get()) {
            continue;
        }
        size_t nameTableSize = typeface->getTableSize(nameTag);
        if (0 == nameTableSize) {
            continue;
        }
        uint8_t* nameTableData = new uint8_t[nameTableSize];
        SkAutoTDeleteArray<uint8_t> ada(nameTableData);
        size_t copied = typeface->getTableData(nameTag, 0, nameTableSize, nameTableData);
        if (copied != nameTableSize) {
            continue;
        }

        SkOTTableName::Iterator iter(*((SkOTTableName*)nameTableData),
                                     SkOTTableName::Record::NameID::Predefined::FontFamilyName);
        SkOTTableName::Iterator::Record record;
        while (iter.next(record)) {
            //printf("%s <%s>\n", record.name.c_str(), record.language.c_str());
        }
    }

    for (size_t i = 0; i < SK_ARRAY_COUNT(test); ++i) {
        SkOTTableName::Iterator iter(*test[i].data, test[i].nameID.predefined.value);
        SkOTTableName::Iterator::Record record;
        size_t nameIndex = 0;
        while (nameIndex < test[i].nameCount && iter.next(record)) {
            REPORTER_ASSERT_MESSAGE(reporter,
                strcmp(test[i].names[nameIndex].name, record.name.c_str()) == 0,
                "Name did not match."
            );

            REPORTER_ASSERT_MESSAGE(reporter,
                strcmp(test[i].names[nameIndex].language, record.language.c_str()) == 0,
                "Language did not match."
            );

            //printf("%s <%s>\n", record.name.c_str(), record.language.c_str());

            ++nameIndex;
        }

        REPORTER_ASSERT_MESSAGE(reporter, nameIndex == test[i].nameCount,
                                "Fewer names than expected.");

        REPORTER_ASSERT_MESSAGE(reporter, !iter.next(record),
                                "More names than expected.");
    }
}


#include "TestClassDef.h"
DEFINE_TESTCLASS("FontNames", FontNamesTestClass, TestFontNames)
