/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOTTable_name.h"

#include "SkEndian.h"
#include "SkString.h"
#include "SkTSearch.h"
#include "SkUtils.h"

static SkUnichar SkUTF16BE_NextUnichar(const uint16_t** srcPtr) {
    SkASSERT(srcPtr && *srcPtr);

    const uint16_t* src = *srcPtr;
    SkUnichar c = SkEndian_SwapBE16(*src++);

    SkASSERT(!SkUTF16_IsLowSurrogate(c));
    if (SkUTF16_IsHighSurrogate(c)) {
        unsigned c2 = SkEndian_SwapBE16(*src++);
        SkASSERT(SkUTF16_IsLowSurrogate(c2));

        c = (c << 10) + c2 + (0x10000 - (0xD800 << 10) - 0xDC00);
    }
    *srcPtr = src;
    return c;
}

static void SkStringFromUTF16BE(const uint16_t* utf16be, size_t length, SkString& utf8) {
    SkASSERT(utf16be != NULL);

    utf8.reset();
    size_t numberOf16BitValues = length / 2;
    const uint16_t* end = utf16be + numberOf16BitValues;
    while (utf16be < end) {
        utf8.appendUnichar(SkUTF16BE_NextUnichar(&utf16be));
    }
}

/** UnicodeFromMacRoman[macRomanPoint - 0x80] -> unicodeCodePoint.
 *  Derived from http://www.unicode.org/Public/MAPPINGS/VENDORS/APPLE/ROMAN.TXT .
 *  In MacRoman the first 128 code points match ASCII code points.
 *  This maps the second 128 MacRoman code points to unicode code points.
 */
static uint16_t UnicodeFromMacRoman[0x80] = {
    0x00C4, 0x00C5, 0x00C7, 0x00C9, 0x00D1, 0x00D6, 0x00DC, 0x00E1,
    0x00E0, 0x00E2, 0x00E4, 0x00E3, 0x00E5, 0x00E7, 0x00E9, 0x00E8,
    0x00EA, 0x00EB, 0x00ED, 0x00EC, 0x00EE, 0x00EF, 0x00F1, 0x00F3,
    0x00F2, 0x00F4, 0x00F6, 0x00F5, 0x00FA, 0x00F9, 0x00FB, 0x00FC,
    0x2020, 0x00B0, 0x00A2, 0x00A3, 0x00A7, 0x2022, 0x00B6, 0x00DF,
    0x00AE, 0x00A9, 0x2122, 0x00B4, 0x00A8, 0x2260, 0x00C6, 0x00D8,
    0x221E, 0x00B1, 0x2264, 0x2265, 0x00A5, 0x00B5, 0x2202, 0x2211,
    0x220F, 0x03C0, 0x222B, 0x00AA, 0x00BA, 0x03A9, 0x00E6, 0x00F8,
    0x00BF, 0x00A1, 0x00AC, 0x221A, 0x0192, 0x2248, 0x2206, 0x00AB,
    0x00BB, 0x2026, 0x00A0, 0x00C0, 0x00C3, 0x00D5, 0x0152, 0x0153,
    0x2013, 0x2014, 0x201C, 0x201D, 0x2018, 0x2019, 0x00F7, 0x25CA,
    0x00FF, 0x0178, 0x2044, 0x20AC, 0x2039, 0x203A, 0xFB01, 0xFB02,
    0x2021, 0x00B7, 0x201A, 0x201E, 0x2030, 0x00C2, 0x00CA, 0x00C1,
    0x00CB, 0x00C8, 0x00CD, 0x00CE, 0x00CF, 0x00CC, 0x00D3, 0x00D4,
    0xF8FF, 0x00D2, 0x00DA, 0x00DB, 0x00D9, 0x0131, 0x02C6, 0x02DC,
    0x00AF, 0x02D8, 0x02D9, 0x02DA, 0x00B8, 0x02DD, 0x02DB, 0x02C7,
};

static void SkStringFromMacRoman(const uint8_t* macRoman, size_t length, SkString& utf8) {
    utf8.reset();
    for (size_t i = 0; i < length; ++i) {
        utf8.appendUnichar(macRoman[i] < 0x80 ? macRoman[i] : UnicodeFromMacRoman[macRoman[i]]);
    }
}

static struct BCP47FromLanguageId {
    uint16_t languageID;
    const char* bcp47;
}
/** The Mac and Windows values do not conflict, so this is currently one single table. */
BCP47FromLanguageID[] = {
    /** A mapping from Mac Language Designators to BCP 47 codes.
     *  The following list was constructed more or less manually.
     *  Apple now uses BCP 47 (post OSX10.4), so there will be no new entries.
     */
    {0, "en"}, //English
    {1, "fr"}, //French
    {2, "de"}, //German
    {3, "it"}, //Italian
    {4, "nl"}, //Dutch
    {5, "sv"}, //Swedish
    {6, "es"}, //Spanish
    {7, "da"}, //Danish
    {8, "pt"}, //Portuguese
    {9, "nb"}, //Norwegian
    {10, "he"}, //Hebrew
    {11, "ja"}, //Japanese
    {12, "ar"}, //Arabic
    {13, "fi"}, //Finnish
    {14, "el"}, //Greek
    {15, "is"}, //Icelandic
    {16, "mt"}, //Maltese
    {17, "tr"}, //Turkish
    {18, "hr"}, //Croatian
    {19, "zh-Hant"}, //Chinese (Traditional)
    {20, "ur"}, //Urdu
    {21, "hi"}, //Hindi
    {22, "th"}, //Thai
    {23, "ko"}, //Korean
    {24, "lt"}, //Lithuanian
    {25, "pl"}, //Polish
    {26, "hu"}, //Hungarian
    {27, "et"}, //Estonian
    {28, "lv"}, //Latvian
    {29, "se"}, //Sami
    {30, "fo"}, //Faroese
    {31, "fa"}, //Farsi (Persian)
    {32, "ru"}, //Russian
    {33, "zh-Hans"}, //Chinese (Simplified)
    {34, "nl"}, //Dutch
    {35, "ga"}, //Irish(Gaelic)
    {36, "sq"}, //Albanian
    {37, "ro"}, //Romanian
    {38, "cs"}, //Czech
    {39, "sk"}, //Slovak
    {40, "sl"}, //Slovenian
    {41, "yi"}, //Yiddish
    {42, "sr"}, //Serbian
    {43, "mk"}, //Macedonian
    {44, "bg"}, //Bulgarian
    {45, "uk"}, //Ukrainian
    {46, "be"}, //Byelorussian
    {47, "uz"}, //Uzbek
    {48, "kk"}, //Kazakh
    {49, "az-Cyrl"}, //Azerbaijani (Cyrillic)
    {50, "az-Arab"}, //Azerbaijani (Arabic)
    {51, "hy"}, //Armenian
    {52, "ka"}, //Georgian
    {53, "mo"}, //Moldavian
    {54, "ky"}, //Kirghiz
    {55, "tg"}, //Tajiki
    {56, "tk"}, //Turkmen
    {57, "mn-Mong"}, //Mongolian (Traditional)
    {58, "mn-Cyrl"}, //Mongolian (Cyrillic)
    {59, "ps"}, //Pashto
    {60, "ku"}, //Kurdish
    {61, "ks"}, //Kashmiri
    {62, "sd"}, //Sindhi
    {63, "bo"}, //Tibetan
    {64, "ne"}, //Nepali
    {65, "sa"}, //Sanskrit
    {66, "mr"}, //Marathi
    {67, "bn"}, //Bengali
    {68, "as"}, //Assamese
    {69, "gu"}, //Gujarati
    {70, "pa"}, //Punjabi
    {71, "or"}, //Oriya
    {72, "ml"}, //Malayalam
    {73, "kn"}, //Kannada
    {74, "ta"}, //Tamil
    {75, "te"}, //Telugu
    {76, "si"}, //Sinhalese
    {77, "my"}, //Burmese
    {78, "km"}, //Khmer
    {79, "lo"}, //Lao
    {80, "vi"}, //Vietnamese
    {81, "id"}, //Indonesian
    {82, "tl"}, //Tagalog
    {83, "ms-Latn"}, //Malay (Roman)
    {84, "ms-Arab"}, //Malay (Arabic)
    {85, "am"}, //Amharic
    {86, "ti"}, //Tigrinya
    {87, "om"}, //Oromo
    {88, "so"}, //Somali
    {89, "sw"}, //Swahili
    {90, "rw"}, //Kinyarwanda/Ruanda
    {91, "rn"}, //Rundi
    {92, "ny"}, //Nyanja/Chewa
    {93, "mg"}, //Malagasy
    {94, "eo"}, //Esperanto
    {128, "cy"}, //Welsh
    {129, "eu"}, //Basque
    {130, "ca"}, //Catalan
    {131, "la"}, //Latin
    {132, "qu"}, //Quechua
    {133, "gn"}, //Guarani
    {134, "ay"}, //Aymara
    {135, "tt"}, //Tatar
    {136, "ug"}, //Uighur
    {137, "dz"}, //Dzongkha
    {138, "jv-Latn"}, //Javanese (Roman)
    {139, "su-Latn"}, //Sundanese (Roman)
    {140, "gl"}, //Galician
    {141, "af"}, //Afrikaans
    {142, "br"}, //Breton
    {143, "iu"}, //Inuktitut
    {144, "gd"}, //Scottish (Gaelic)
    {145, "gv"}, //Manx (Gaelic)
    {146, "ga"}, //Irish (Gaelic with Lenition)
    {147, "to"}, //Tongan
    {148, "el"}, //Greek (Polytonic) Note: ISO 15924 does not have an equivalent script name.
    {149, "kl"}, //Greenlandic
    {150, "az-Latn"}, //Azerbaijani (Roman)
    {151, "nn"}, //Nynorsk

    /** A mapping from Windows LCID to BCP 47 codes.
     *  This list is the sorted, curated output of tools/win_lcid.cpp.
     *  Note that these are sorted by value for quick binary lookup, and not logically by lsb.
     *  The 'bare' language ids (e.g. 0x0001 for Arabic) are ommitted
     *  as they do not appear as valid language ids in the OpenType specification.
     */
    { 0x0401, "ar-SA" }, //Arabic
    { 0x0402, "bg-BG" }, //Bulgarian
    { 0x0403, "ca-ES" }, //Catalan
    { 0x0404, "zh-TW" }, //Chinese (Traditional)
    { 0x0405, "cs-CZ" }, //Czech
    { 0x0406, "da-DK" }, //Danish
    { 0x0407, "de-DE" }, //German
    { 0x0408, "el-GR" }, //Greek
    { 0x0409, "en-US" }, //English
    { 0x040a, "es-ES_tradnl" }, //Spanish
    { 0x040b, "fi-FI" }, //Finnish
    { 0x040c, "fr-FR" }, //French
    { 0x040d, "he-IL" }, //Hebrew
    { 0x040d, "he" }, //Hebrew
    { 0x040e, "hu-HU" }, //Hungarian
    { 0x040e, "hu" }, //Hungarian
    { 0x040f, "is-IS" }, //Icelandic
    { 0x0410, "it-IT" }, //Italian
    { 0x0411, "ja-JP" }, //Japanese
    { 0x0412, "ko-KR" }, //Korean
    { 0x0413, "nl-NL" }, //Dutch
    { 0x0414, "nb-NO" }, //Norwegian (Bokmål)
    { 0x0415, "pl-PL" }, //Polish
    { 0x0416, "pt-BR" }, //Portuguese
    { 0x0417, "rm-CH" }, //Romansh
    { 0x0418, "ro-RO" }, //Romanian
    { 0x0419, "ru-RU" }, //Russian
    { 0x041a, "hr-HR" }, //Croatian
    { 0x041b, "sk-SK" }, //Slovak
    { 0x041c, "sq-AL" }, //Albanian
    { 0x041d, "sv-SE" }, //Swedish
    { 0x041e, "th-TH" }, //Thai
    { 0x041f, "tr-TR" }, //Turkish
    { 0x0420, "ur-PK" }, //Urdu
    { 0x0421, "id-ID" }, //Indonesian
    { 0x0422, "uk-UA" }, //Ukrainian
    { 0x0423, "be-BY" }, //Belarusian
    { 0x0424, "sl-SI" }, //Slovenian
    { 0x0425, "et-EE" }, //Estonian
    { 0x0426, "lv-LV" }, //Latvian
    { 0x0427, "lt-LT" }, //Lithuanian
    { 0x0428, "tg-Cyrl-TJ" }, //Tajik (Cyrillic)
    { 0x0429, "fa-IR" }, //Persian
    { 0x042a, "vi-VN" }, //Vietnamese
    { 0x042b, "hy-AM" }, //Armenian
    { 0x042c, "az-Latn-AZ" }, //Azeri (Latin)
    { 0x042d, "eu-ES" }, //Basque
    { 0x042e, "hsb-DE" }, //Upper Sorbian
    { 0x042f, "mk-MK" }, //Macedonian (FYROM)
    { 0x0432, "tn-ZA" }, //Setswana
    { 0x0434, "xh-ZA" }, //isiXhosa
    { 0x0435, "zu-ZA" }, //isiZulu
    { 0x0436, "af-ZA" }, //Afrikaans
    { 0x0437, "ka-GE" }, //Georgian
    { 0x0438, "fo-FO" }, //Faroese
    { 0x0439, "hi-IN" }, //Hindi
    { 0x043a, "mt-MT" }, //Maltese
    { 0x043b, "se-NO" }, //Sami (Northern)
    { 0x043e, "ms-MY" }, //Malay
    { 0x043f, "kk-KZ" }, //Kazakh
    { 0x0440, "ky-KG" }, //Kyrgyz
    { 0x0441, "sw-KE" }, //Kiswahili
    { 0x0442, "tk-TM" }, //Turkmen
    { 0x0443, "uz-Latn-UZ" }, //Uzbek (Latin)
    { 0x0443, "uz" }, //Uzbek
    { 0x0444, "tt-RU" }, //Tatar
    { 0x0445, "bn-IN" }, //Bengali
    { 0x0446, "pa-IN" }, //Punjabi
    { 0x0447, "gu-IN" }, //Gujarati
    { 0x0448, "or-IN" }, //Oriya
    { 0x0449, "ta-IN" }, //Tamil
    { 0x044a, "te-IN" }, //Telugu
    { 0x044b, "kn-IN" }, //Kannada
    { 0x044c, "ml-IN" }, //Malayalam
    { 0x044d, "as-IN" }, //Assamese
    { 0x044e, "mr-IN" }, //Marathi
    { 0x044f, "sa-IN" }, //Sanskrit
    { 0x0450, "mn-Cyrl" }, //Mongolian (Cyrillic)
    { 0x0451, "bo-CN" }, //Tibetan
    { 0x0452, "cy-GB" }, //Welsh
    { 0x0453, "km-KH" }, //Khmer
    { 0x0454, "lo-LA" }, //Lao
    { 0x0456, "gl-ES" }, //Galician
    { 0x0457, "kok-IN" }, //Konkani
    { 0x045a, "syr-SY" }, //Syriac
    { 0x045b, "si-LK" }, //Sinhala
    { 0x045d, "iu-Cans-CA" }, //Inuktitut (Syllabics)
    { 0x045e, "am-ET" }, //Amharic
    { 0x0461, "ne-NP" }, //Nepali
    { 0x0462, "fy-NL" }, //Frisian
    { 0x0463, "ps-AF" }, //Pashto
    { 0x0464, "fil-PH" }, //Filipino
    { 0x0465, "dv-MV" }, //Divehi
    { 0x0468, "ha-Latn-NG" }, //Hausa (Latin)
    { 0x046a, "yo-NG" }, //Yoruba
    { 0x046b, "quz-BO" }, //Quechua
    { 0x046c, "nso-ZA" }, //Sesotho sa Leboa
    { 0x046d, "ba-RU" }, //Bashkir
    { 0x046e, "lb-LU" }, //Luxembourgish
    { 0x046f, "kl-GL" }, //Greenlandic
    { 0x0470, "ig-NG" }, //Igbo
    { 0x0478, "ii-CN" }, //Yi
    { 0x047a, "arn-CL" }, //Mapudungun
    { 0x047c, "moh-CA" }, //Mohawk
    { 0x047e, "br-FR" }, //Breton
    { 0x0480, "ug-CN" }, //Uyghur
    { 0x0481, "mi-NZ" }, //Maori
    { 0x0482, "oc-FR" }, //Occitan
    { 0x0483, "co-FR" }, //Corsican
    { 0x0484, "gsw-FR" }, //Alsatian
    { 0x0485, "sah-RU" }, //Yakut
    { 0x0486, "qut-GT" }, //K'iche
    { 0x0487, "rw-RW" }, //Kinyarwanda
    { 0x0488, "wo-SN" }, //Wolof
    { 0x048c, "prs-AF" }, //Dari
    { 0x0491, "gd-GB" }, //Scottish Gaelic
    { 0x0801, "ar-IQ" }, //Arabic
    { 0x0804, "zh-Hans" }, //Chinese (Simplified)
    { 0x0807, "de-CH" }, //German
    { 0x0809, "en-GB" }, //English
    { 0x080a, "es-MX" }, //Spanish
    { 0x080c, "fr-BE" }, //French
    { 0x0810, "it-CH" }, //Italian
    { 0x0813, "nl-BE" }, //Dutch
    { 0x0814, "nn-NO" }, //Norwegian (Nynorsk)
    { 0x0816, "pt-PT" }, //Portuguese
    { 0x081a, "sr-Latn-CS" }, //Serbian (Latin)
    { 0x081d, "sv-FI" }, //Swedish
    { 0x082c, "az-Cyrl-AZ" }, //Azeri (Cyrillic)
    { 0x082e, "dsb-DE" }, //Lower Sorbian
    { 0x082e, "dsb" }, //Lower Sorbian
    { 0x083b, "se-SE" }, //Sami (Northern)
    { 0x083c, "ga-IE" }, //Irish
    { 0x083e, "ms-BN" }, //Malay
    { 0x0843, "uz-Cyrl-UZ" }, //Uzbek (Cyrillic)
    { 0x0845, "bn-BD" }, //Bengali
    { 0x0850, "mn-Mong-CN" }, //Mongolian (Traditional Mongolian)
    { 0x085d, "iu-Latn-CA" }, //Inuktitut (Latin)
    { 0x085f, "tzm-Latn-DZ" }, //Tamazight (Latin)
    { 0x086b, "quz-EC" }, //Quechua
    { 0x0c01, "ar-EG" }, //Arabic
    { 0x0c04, "zh-Hant" }, //Chinese (Traditional)
    { 0x0c07, "de-AT" }, //German
    { 0x0c09, "en-AU" }, //English
    { 0x0c0a, "es-ES" }, //Spanish
    { 0x0c0c, "fr-CA" }, //French
    { 0x0c1a, "sr-Cyrl-CS" }, //Serbian (Cyrillic)
    { 0x0c3b, "se-FI" }, //Sami (Northern)
    { 0x0c6b, "quz-PE" }, //Quechua
    { 0x1001, "ar-LY" }, //Arabic
    { 0x1004, "zh-SG" }, //Chinese (Simplified)
    { 0x1007, "de-LU" }, //German
    { 0x1009, "en-CA" }, //English
    { 0x100a, "es-GT" }, //Spanish
    { 0x100c, "fr-CH" }, //French
    { 0x101a, "hr-BA" }, //Croatian (Latin)
    { 0x103b, "smj-NO" }, //Sami (Lule)
    { 0x1401, "ar-DZ" }, //Arabic
    { 0x1404, "zh-MO" }, //Chinese (Traditional)
    { 0x1407, "de-LI" }, //German
    { 0x1409, "en-NZ" }, //English
    { 0x140a, "es-CR" }, //Spanish
    { 0x140c, "fr-LU" }, //French
    { 0x141a, "bs-Latn-BA" }, //Bosnian (Latin)
    { 0x141a, "bs" }, //Bosnian
    { 0x143b, "smj-SE" }, //Sami (Lule)
    { 0x143b, "smj" }, //Sami (Lule)
    { 0x1801, "ar-MA" }, //Arabic
    { 0x1809, "en-IE" }, //English
    { 0x180a, "es-PA" }, //Spanish
    { 0x180c, "fr-MC" }, //French
    { 0x181a, "sr-Latn-BA" }, //Serbian (Latin)
    { 0x183b, "sma-NO" }, //Sami (Southern)
    { 0x1c01, "ar-TN" }, //Arabic
    { 0x1c09, "en-ZA" }, //English
    { 0x1c0a, "es-DO" }, //Spanish
    { 0x1c1a, "sr-Cyrl-BA" }, //Serbian (Cyrillic)
    { 0x1c3b, "sma-SE" }, //Sami (Southern)
    { 0x1c3b, "sma" }, //Sami (Southern)
    { 0x2001, "ar-OM" }, //Arabic
    { 0x2009, "en-JM" }, //English
    { 0x200a, "es-VE" }, //Spanish
    { 0x201a, "bs-Cyrl-BA" }, //Bosnian (Cyrillic)
    { 0x201a, "bs-Cyrl" }, //Bosnian (Cyrillic)
    { 0x203b, "sms-FI" }, //Sami (Skolt)
    { 0x203b, "sms" }, //Sami (Skolt)
    { 0x2401, "ar-YE" }, //Arabic
    { 0x2409, "en-029" }, //English
    { 0x240a, "es-CO" }, //Spanish
    { 0x241a, "sr-Latn-RS" }, //Serbian (Latin)
    { 0x243b, "smn-FI" }, //Sami (Inari)
    { 0x2801, "ar-SY" }, //Arabic
    { 0x2809, "en-BZ" }, //English
    { 0x280a, "es-PE" }, //Spanish
    { 0x281a, "sr-Cyrl-RS" }, //Serbian (Cyrillic)
    { 0x2c01, "ar-JO" }, //Arabic
    { 0x2c09, "en-TT" }, //English
    { 0x2c0a, "es-AR" }, //Spanish
    { 0x2c1a, "sr-Latn-ME" }, //Serbian (Latin)
    { 0x3001, "ar-LB" }, //Arabic
    { 0x3009, "en-ZW" }, //English
    { 0x300a, "es-EC" }, //Spanish
    { 0x301a, "sr-Cyrl-ME" }, //Serbian (Cyrillic)
    { 0x3401, "ar-KW" }, //Arabic
    { 0x3409, "en-PH" }, //English
    { 0x340a, "es-CL" }, //Spanish
    { 0x3801, "ar-AE" }, //Arabic
    { 0x380a, "es-UY" }, //Spanish
    { 0x3c01, "ar-BH" }, //Arabic
    { 0x3c0a, "es-PY" }, //Spanish
    { 0x4001, "ar-QA" }, //Arabic
    { 0x4009, "en-IN" }, //English
    { 0x400a, "es-BO" }, //Spanish
    { 0x4409, "en-MY" }, //English
    { 0x440a, "es-SV" }, //Spanish
    { 0x4809, "en-SG" }, //English
    { 0x480a, "es-HN" }, //Spanish
    { 0x4c0a, "es-NI" }, //Spanish
    { 0x500a, "es-PR" }, //Spanish
    { 0x540a, "es-US" }, //Spanish
};

namespace {
int BCP47FromLanguageIdCompare(const BCP47FromLanguageId* a, const BCP47FromLanguageId* b) {
    return a->languageID - b->languageID;
}
}

template <typename D, typename S> static D* SkTAfter(S const * const ptr, size_t count = 1) {
    return (D*)(ptr + count);
}

template <typename D, typename S> static D* SkTAddOffset(S const * const ptr, size_t byteOffset) {
    return (D*)((char*)ptr + byteOffset);
}

bool SkOTTableName::Iterator::next(SkOTTableName::Iterator::Record& record) {
    const size_t nameRecordsCount = SkEndian_SwapBE16(fName.count);
    const SkOTTableName::Record* nameRecords = SkTAfter<const SkOTTableName::Record>(&fName);
    const SkOTTableName::Record* nameRecord;

    // Find the next record which matches the requested type.
    do {
        if (fIndex >= nameRecordsCount) {
            return false;
        }

        nameRecord = &nameRecords[fIndex];
        ++fIndex;
    } while (fType != -1 && nameRecord->nameID.fontSpecific != fType);

    const uint16_t stringTableOffset = SkEndian_SwapBE16(fName.stringOffset);
    const char* stringTable = SkTAddOffset<char>(&fName, stringTableOffset);

    // Decode the name into UTF-8.
    const uint16_t nameOffset = SkEndian_SwapBE16(nameRecord->offset);
    const uint16_t nameLength = SkEndian_SwapBE16(nameRecord->length);
    const char* nameString = SkTAddOffset<char>(stringTable, nameOffset);
    switch (nameRecord->platformID.value) {
        case SkOTTableName::Record::PlatformID::Windows:
            SkASSERT(SkOTTableName::Record::EncodingID::Windows::UnicodeBMPUCS2
                  == nameRecord->encodingID.windows.value
                  || SkOTTableName::Record::EncodingID::Windows::UnicodeUCS4
                  == nameRecord->encodingID.windows.value
                  || SkOTTableName::Record::EncodingID::Windows::Symbol
                  == nameRecord->encodingID.windows.value);
        case SkOTTableName::Record::PlatformID::Unicode:
        case SkOTTableName::Record::PlatformID::ISO:
            SkStringFromUTF16BE((const uint16_t*)nameString, nameLength, record.name);
            break;

        case SkOTTableName::Record::PlatformID::Macintosh:
            SkASSERT(SkOTTableName::Record::EncodingID::Macintosh::Roman
                  == nameRecord->encodingID.macintosh.value);
            SkStringFromMacRoman((const uint8_t*)nameString, nameLength, record.name);
            break;

        case SkOTTableName::Record::PlatformID::Custom:
            // These should never appear in a 'name' table.
        default:
            SkASSERT(false);
            record.name.reset();
            break;
    }

    // Determine the language.
    const uint16_t languageID = SkEndian_SwapBE16(nameRecord->languageID.languageTagID);

    // Handle format 1 languages.
    if (SkOTTableName::format_1 == fName.format && languageID >= 0x8000) {
        const uint16_t languageTagRecordIndex = languageID - 0x8000;

        const SkOTTableName::Format1Ext* format1ext =
            SkTAfter<const SkOTTableName::Format1Ext>(nameRecords, nameRecordsCount);

        if (languageTagRecordIndex < format1ext->langTagCount) {
            const SkOTTableName::Format1Ext::LangTagRecord* languageTagRecord =
                SkTAfter<const SkOTTableName::Format1Ext::LangTagRecord>(format1ext);

            uint16_t offset = SkEndian_SwapBE16(languageTagRecord[languageTagRecordIndex].offset);
            uint16_t length = SkEndian_SwapBE16(languageTagRecord[languageTagRecordIndex].length);
            const uint16_t* string = SkTAddOffset<const uint16_t>(stringTable, offset);
            SkStringFromUTF16BE(string, length, record.language);
            return true;
        }
    }

    // Handle format 0 languages, translating them into BCP 47.
    const BCP47FromLanguageId target = { languageID, "" };
    int languageIndex = SkTSearch<BCP47FromLanguageId, BCP47FromLanguageIdCompare>(
        BCP47FromLanguageID, SK_ARRAY_COUNT(BCP47FromLanguageID), target, sizeof(target));
    if (languageIndex >= 0) {
        record.language = BCP47FromLanguageID[languageIndex].bcp47;
        return true;
    }

    // Unknown language, return the BCP 47 code 'und' for 'undetermined'.
    SkASSERT(false);
    record.language = "und";
    return true;
}
