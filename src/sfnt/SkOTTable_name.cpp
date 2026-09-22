/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sfnt/SkOTTable_name.h"

#include "src/core/SkEndian.h"
#include "src/core/SkStringUtils.h"
#include "src/core/SkTSearch.h"
#include "src/core/SkUTF.h"

#include <array>

static SkUnichar next_unichar_UTF16BE(const uint8_t** srcPtr, size_t* length) {
    SkASSERT(srcPtr && *srcPtr && length);
    SkASSERT(*length > 0);

    uint16_t leading;
    if (*length < sizeof(leading)) {
        *length = 0;
        return 0xFFFD;
    }
    memcpy(&leading, *srcPtr, sizeof(leading));
    *srcPtr += sizeof(leading);
    *length -= sizeof(leading);
    SkUnichar c = SkEndian_SwapBE16(leading);

    if (SkUTF::IsTrailingSurrogateUTF16(c)) {
        return 0xFFFD;
    }
    if (SkUTF::IsLeadingSurrogateUTF16(c)) {
        uint16_t trailing;
        if (*length < sizeof(trailing)) {
            *length = 0;
            return 0xFFFD;
        }
        memcpy(&trailing, *srcPtr, sizeof(trailing));
        SkUnichar c2 = SkEndian_SwapBE16(trailing);
        if (!SkUTF::IsTrailingSurrogateUTF16(c2)) {
            return 0xFFFD;
        }
        *srcPtr += sizeof(trailing);
        *length -= sizeof(trailing);

        c = (c << 10) + c2 + (0x10000 - (0xD800 << 10) - 0xDC00);
    }
    return c;
}

static void SkString_from_UTF16BE(const uint8_t* utf16be, size_t length, SkString& utf8) {
    // Note that utf16be may not be 2-byte aligned.
    SkASSERT(utf16be != nullptr);

    utf8.reset();
    while (length) {
        utf8.appendUnichar(next_unichar_UTF16BE(&utf16be, &length));
    }
}

/** UnicodeFromMacRoman[macRomanPoint - 0x80] -> unicodeCodePoint.
 *  Derived from http://www.unicode.org/Public/MAPPINGS/VENDORS/APPLE/ROMAN.TXT .
 *  In MacRoman the first 128 code points match ASCII code points.
 *  This maps the second 128 MacRoman code points to unicode code points.
 */
static constexpr std::array<uint16_t, 0x80> UnicodeFromMacRoman = {
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
        utf8.appendUnichar(macRoman[i] < 0x80 ? macRoman[i]
                                              : UnicodeFromMacRoman[macRoman[i] - 0x80]);
    }
}

struct BCP47FromLanguageId {
    uint16_t languageID;
    const char* bcp47;
};
static constexpr auto BCP47FromLanguageID = std::to_array<BCP47FromLanguageId>({
        /** A mapping from Mac Language Designators to BCP 47 codes.
         *  The following list was constructed more or less manually.
         *  Apple now uses BCP 47 (post OSX10.4), so there will be no new entries.
         */
        BCP47FromLanguageId{     0,           "en"}, //  English
        BCP47FromLanguageId{     1,           "fr"}, //  French
        BCP47FromLanguageId{     2,           "de"}, //  German
        BCP47FromLanguageId{     3,           "it"}, //  Italian
        BCP47FromLanguageId{     4,           "nl"}, //  Dutch
        BCP47FromLanguageId{     5,           "sv"}, //  Swedish
        BCP47FromLanguageId{     6,           "es"}, //  Spanish
        BCP47FromLanguageId{     7,           "da"}, //  Danish
        BCP47FromLanguageId{     8,           "pt"}, //  Portuguese
        BCP47FromLanguageId{     9,           "nb"}, //  Norwegian
        BCP47FromLanguageId{    10,           "he"}, //  Hebrew
        BCP47FromLanguageId{    11,           "ja"}, //  Japanese
        BCP47FromLanguageId{    12,           "ar"}, //  Arabic
        BCP47FromLanguageId{    13,           "fi"}, //  Finnish
        BCP47FromLanguageId{    14,           "el"}, //  Greek
        BCP47FromLanguageId{    15,           "is"}, //  Icelandic
        BCP47FromLanguageId{    16,           "mt"}, //  Maltese
        BCP47FromLanguageId{    17,           "tr"}, //  Turkish
        BCP47FromLanguageId{    18,           "hr"}, //  Croatian
        BCP47FromLanguageId{    19,      "zh-Hant"}, //  Chinese (Traditional)
        BCP47FromLanguageId{    20,           "ur"}, //  Urdu
        BCP47FromLanguageId{    21,           "hi"}, //  Hindi
        BCP47FromLanguageId{    22,           "th"}, //  Thai
        BCP47FromLanguageId{    23,           "ko"}, //  Korean
        BCP47FromLanguageId{    24,           "lt"}, //  Lithuanian
        BCP47FromLanguageId{    25,           "pl"}, //  Polish
        BCP47FromLanguageId{    26,           "hu"}, //  Hungarian
        BCP47FromLanguageId{    27,           "et"}, //  Estonian
        BCP47FromLanguageId{    28,           "lv"}, //  Latvian
        BCP47FromLanguageId{    29,           "se"}, //  Sami
        BCP47FromLanguageId{    30,           "fo"}, //  Faroese
        BCP47FromLanguageId{    31,           "fa"}, //  Farsi (Persian)
        BCP47FromLanguageId{    32,           "ru"}, //  Russian
        BCP47FromLanguageId{    33,      "zh-Hans"}, //  Chinese (Simplified)
        BCP47FromLanguageId{    34,           "nl"}, //  Dutch
        BCP47FromLanguageId{    35,           "ga"}, //  Irish(Gaelic)
        BCP47FromLanguageId{    36,           "sq"}, //  Albanian
        BCP47FromLanguageId{    37,           "ro"}, //  Romanian
        BCP47FromLanguageId{    38,           "cs"}, //  Czech
        BCP47FromLanguageId{    39,           "sk"}, //  Slovak
        BCP47FromLanguageId{    40,           "sl"}, //  Slovenian
        BCP47FromLanguageId{    41,           "yi"}, //  Yiddish
        BCP47FromLanguageId{    42,           "sr"}, //  Serbian
        BCP47FromLanguageId{    43,           "mk"}, //  Macedonian
        BCP47FromLanguageId{    44,           "bg"}, //  Bulgarian
        BCP47FromLanguageId{    45,           "uk"}, //  Ukrainian
        BCP47FromLanguageId{    46,           "be"}, //  Byelorussian
        BCP47FromLanguageId{    47,           "uz"}, //  Uzbek
        BCP47FromLanguageId{    48,           "kk"}, //  Kazakh
        BCP47FromLanguageId{    49,      "az-Cyrl"}, //  Azerbaijani (Cyrillic)
        BCP47FromLanguageId{    50,      "az-Arab"}, //  Azerbaijani (Arabic)
        BCP47FromLanguageId{    51,           "hy"}, //  Armenian
        BCP47FromLanguageId{    52,           "ka"}, //  Georgian
        BCP47FromLanguageId{    53,           "mo"}, //  Moldavian
        BCP47FromLanguageId{    54,           "ky"}, //  Kirghiz
        BCP47FromLanguageId{    55,           "tg"}, //  Tajiki
        BCP47FromLanguageId{    56,           "tk"}, //  Turkmen
        BCP47FromLanguageId{    57,      "mn-Mong"}, //  Mongolian (Traditional)
        BCP47FromLanguageId{    58,      "mn-Cyrl"}, //  Mongolian (Cyrillic)
        BCP47FromLanguageId{    59,           "ps"}, //  Pashto
        BCP47FromLanguageId{    60,           "ku"}, //  Kurdish
        BCP47FromLanguageId{    61,           "ks"}, //  Kashmiri
        BCP47FromLanguageId{    62,           "sd"}, //  Sindhi
        BCP47FromLanguageId{    63,           "bo"}, //  Tibetan
        BCP47FromLanguageId{    64,           "ne"}, //  Nepali
        BCP47FromLanguageId{    65,           "sa"}, //  Sanskrit
        BCP47FromLanguageId{    66,           "mr"}, //  Marathi
        BCP47FromLanguageId{    67,           "bn"}, //  Bengali
        BCP47FromLanguageId{    68,           "as"}, //  Assamese
        BCP47FromLanguageId{    69,           "gu"}, //  Gujarati
        BCP47FromLanguageId{    70,           "pa"}, //  Punjabi
        BCP47FromLanguageId{    71,           "or"}, //  Oriya
        BCP47FromLanguageId{    72,           "ml"}, //  Malayalam
        BCP47FromLanguageId{    73,           "kn"}, //  Kannada
        BCP47FromLanguageId{    74,           "ta"}, //  Tamil
        BCP47FromLanguageId{    75,           "te"}, //  Telugu
        BCP47FromLanguageId{    76,           "si"}, //  Sinhalese
        BCP47FromLanguageId{    77,           "my"}, //  Burmese
        BCP47FromLanguageId{    78,           "km"}, //  Khmer
        BCP47FromLanguageId{    79,           "lo"}, //  Lao
        BCP47FromLanguageId{    80,           "vi"}, //  Vietnamese
        BCP47FromLanguageId{    81,           "id"}, //  Indonesian
        BCP47FromLanguageId{    82,           "tl"}, //  Tagalog
        BCP47FromLanguageId{    83,      "ms-Latn"}, //  Malay (Roman)
        BCP47FromLanguageId{    84,      "ms-Arab"}, //  Malay (Arabic)
        BCP47FromLanguageId{    85,           "am"}, //  Amharic
        BCP47FromLanguageId{    86,           "ti"}, //  Tigrinya
        BCP47FromLanguageId{    87,           "om"}, //  Oromo
        BCP47FromLanguageId{    88,           "so"}, //  Somali
        BCP47FromLanguageId{    89,           "sw"}, //  Swahili
        BCP47FromLanguageId{    90,           "rw"}, //  Kinyarwanda/Ruanda
        BCP47FromLanguageId{    91,           "rn"}, //  Rundi
        BCP47FromLanguageId{    92,           "ny"}, //  Nyanja/Chewa
        BCP47FromLanguageId{    93,           "mg"}, //  Malagasy
        BCP47FromLanguageId{    94,           "eo"}, //  Esperanto
        BCP47FromLanguageId{   128,           "cy"}, //  Welsh
        BCP47FromLanguageId{   129,           "eu"}, //  Basque
        BCP47FromLanguageId{   130,           "ca"}, //  Catalan
        BCP47FromLanguageId{   131,           "la"}, //  Latin
        BCP47FromLanguageId{   132,           "qu"}, //  Quechua
        BCP47FromLanguageId{   133,           "gn"}, //  Guarani
        BCP47FromLanguageId{   134,           "ay"}, //  Aymara
        BCP47FromLanguageId{   135,           "tt"}, //  Tatar
        BCP47FromLanguageId{   136,           "ug"}, //  Uighur
        BCP47FromLanguageId{   137,           "dz"}, //  Dzongkha
        BCP47FromLanguageId{   138,      "jv-Latn"}, //  Javanese (Roman)
        BCP47FromLanguageId{   139,      "su-Latn"}, //  Sundanese (Roman)
        BCP47FromLanguageId{   140,           "gl"}, //  Galician
        BCP47FromLanguageId{   141,           "af"}, //  Afrikaans
        BCP47FromLanguageId{   142,           "br"}, //  Breton
        BCP47FromLanguageId{   143,           "iu"}, //  Inuktitut
        BCP47FromLanguageId{   144,           "gd"}, //  Scottish (Gaelic)
        BCP47FromLanguageId{   145,           "gv"}, //  Manx (Gaelic)
        BCP47FromLanguageId{   146,           "ga"}, //  Irish (Gaelic with Lenition)
        BCP47FromLanguageId{   147,           "to"}, //  Tongan
        BCP47FromLanguageId{   148,           "el"}, //  Greek (Polytonic) Note: ISO 15924 does not
                                                     //  have an equivalent script name.
        BCP47FromLanguageId{   149,           "kl"}, //  Greenlandic
        BCP47FromLanguageId{   150,      "az-Latn"}, //  Azerbaijani (Roman)
        BCP47FromLanguageId{   151,           "nn"}, //  Nynorsk

        /** A mapping from Windows LCID to BCP 47 codes.
         *  This list is the sorted, curated output of tools/win_lcid.cpp.
         *  Note that these are sorted by value for quick binary lookup, and not logically by lsb.
         *  The 'bare' language ids (e.g. 0x0001 for Arabic) are ommitted
         *  as they do not appear as valid language ids in the OpenType specification.
         */
        BCP47FromLanguageId{0x0401,        "ar-SA"}, //  Arabic
        BCP47FromLanguageId{0x0402,        "bg-BG"}, //  Bulgarian
        BCP47FromLanguageId{0x0403,        "ca-ES"}, //  Catalan
        BCP47FromLanguageId{0x0404,        "zh-TW"}, //  Chinese (Traditional)
        BCP47FromLanguageId{0x0405,        "cs-CZ"}, //  Czech
        BCP47FromLanguageId{0x0406,        "da-DK"}, //  Danish
        BCP47FromLanguageId{0x0407,        "de-DE"}, //  German
        BCP47FromLanguageId{0x0408,        "el-GR"}, //  Greek
        BCP47FromLanguageId{0x0409,        "en-US"}, //  English
        BCP47FromLanguageId{0x040a, "es-ES_tradnl"}, //  Spanish
        BCP47FromLanguageId{0x040b,        "fi-FI"}, //  Finnish
        BCP47FromLanguageId{0x040c,        "fr-FR"}, //  French
        BCP47FromLanguageId{0x040d,        "he-IL"}, //  Hebrew
        BCP47FromLanguageId{0x040d,           "he"}, //  Hebrew
        BCP47FromLanguageId{0x040e,        "hu-HU"}, //  Hungarian
        BCP47FromLanguageId{0x040e,           "hu"}, //  Hungarian
        BCP47FromLanguageId{0x040f,        "is-IS"}, //  Icelandic
        BCP47FromLanguageId{0x0410,        "it-IT"}, //  Italian
        BCP47FromLanguageId{0x0411,        "ja-JP"}, //  Japanese
        BCP47FromLanguageId{0x0412,        "ko-KR"}, //  Korean
        BCP47FromLanguageId{0x0413,        "nl-NL"}, //  Dutch
        BCP47FromLanguageId{0x0414,        "nb-NO"}, //  Norwegian (Bokmål)
        BCP47FromLanguageId{0x0415,        "pl-PL"}, //  Polish
        BCP47FromLanguageId{0x0416,        "pt-BR"}, //  Portuguese
        BCP47FromLanguageId{0x0417,        "rm-CH"}, //  Romansh
        BCP47FromLanguageId{0x0418,        "ro-RO"}, //  Romanian
        BCP47FromLanguageId{0x0419,        "ru-RU"}, //  Russian
        BCP47FromLanguageId{0x041a,        "hr-HR"}, //  Croatian
        BCP47FromLanguageId{0x041b,        "sk-SK"}, //  Slovak
        BCP47FromLanguageId{0x041c,        "sq-AL"}, //  Albanian
        BCP47FromLanguageId{0x041d,        "sv-SE"}, //  Swedish
        BCP47FromLanguageId{0x041e,        "th-TH"}, //  Thai
        BCP47FromLanguageId{0x041f,        "tr-TR"}, //  Turkish
        BCP47FromLanguageId{0x0420,        "ur-PK"}, //  Urdu
        BCP47FromLanguageId{0x0421,        "id-ID"}, //  Indonesian
        BCP47FromLanguageId{0x0422,        "uk-UA"}, //  Ukrainian
        BCP47FromLanguageId{0x0423,        "be-BY"}, //  Belarusian
        BCP47FromLanguageId{0x0424,        "sl-SI"}, //  Slovenian
        BCP47FromLanguageId{0x0425,        "et-EE"}, //  Estonian
        BCP47FromLanguageId{0x0426,        "lv-LV"}, //  Latvian
        BCP47FromLanguageId{0x0427,        "lt-LT"}, //  Lithuanian
        BCP47FromLanguageId{0x0428,   "tg-Cyrl-TJ"}, //  Tajik (Cyrillic)
        BCP47FromLanguageId{0x0429,        "fa-IR"}, //  Persian
        BCP47FromLanguageId{0x042a,        "vi-VN"}, //  Vietnamese
        BCP47FromLanguageId{0x042b,        "hy-AM"}, //  Armenian
        BCP47FromLanguageId{0x042c,   "az-Latn-AZ"}, //  Azeri (Latin)
        BCP47FromLanguageId{0x042d,        "eu-ES"}, //  Basque
        BCP47FromLanguageId{0x042e,       "hsb-DE"}, //  Upper Sorbian
        BCP47FromLanguageId{0x042f,        "mk-MK"}, //  Macedonian (FYROM)
        BCP47FromLanguageId{0x0432,        "tn-ZA"}, //  Setswana
        BCP47FromLanguageId{0x0434,        "xh-ZA"}, //  isiXhosa
        BCP47FromLanguageId{0x0435,        "zu-ZA"}, //  isiZulu
        BCP47FromLanguageId{0x0436,        "af-ZA"}, //  Afrikaans
        BCP47FromLanguageId{0x0437,        "ka-GE"}, //  Georgian
        BCP47FromLanguageId{0x0438,        "fo-FO"}, //  Faroese
        BCP47FromLanguageId{0x0439,        "hi-IN"}, //  Hindi
        BCP47FromLanguageId{0x043a,        "mt-MT"}, //  Maltese
        BCP47FromLanguageId{0x043b,        "se-NO"}, //  Sami (Northern)
        BCP47FromLanguageId{0x043e,        "ms-MY"}, //  Malay
        BCP47FromLanguageId{0x043f,        "kk-KZ"}, //  Kazakh
        BCP47FromLanguageId{0x0440,        "ky-KG"}, //  Kyrgyz
        BCP47FromLanguageId{0x0441,        "sw-KE"}, //  Kiswahili
        BCP47FromLanguageId{0x0442,        "tk-TM"}, //  Turkmen
        BCP47FromLanguageId{0x0443,   "uz-Latn-UZ"}, //  Uzbek (Latin)
        BCP47FromLanguageId{0x0443,           "uz"}, //  Uzbek
        BCP47FromLanguageId{0x0444,        "tt-RU"}, //  Tatar
        BCP47FromLanguageId{0x0445,        "bn-IN"}, //  Bengali
        BCP47FromLanguageId{0x0446,        "pa-IN"}, //  Punjabi
        BCP47FromLanguageId{0x0447,        "gu-IN"}, //  Gujarati
        BCP47FromLanguageId{0x0448,        "or-IN"}, //  Oriya
        BCP47FromLanguageId{0x0449,        "ta-IN"}, //  Tamil
        BCP47FromLanguageId{0x044a,        "te-IN"}, //  Telugu
        BCP47FromLanguageId{0x044b,        "kn-IN"}, //  Kannada
        BCP47FromLanguageId{0x044c,        "ml-IN"}, //  Malayalam
        BCP47FromLanguageId{0x044d,        "as-IN"}, //  Assamese
        BCP47FromLanguageId{0x044e,        "mr-IN"}, //  Marathi
        BCP47FromLanguageId{0x044f,        "sa-IN"}, //  Sanskrit
        BCP47FromLanguageId{0x0450,      "mn-Cyrl"}, //  Mongolian (Cyrillic)
        BCP47FromLanguageId{0x0451,        "bo-CN"}, //  Tibetan
        BCP47FromLanguageId{0x0452,        "cy-GB"}, //  Welsh
        BCP47FromLanguageId{0x0453,        "km-KH"}, //  Khmer
        BCP47FromLanguageId{0x0454,        "lo-LA"}, //  Lao
        BCP47FromLanguageId{0x0456,        "gl-ES"}, //  Galician
        BCP47FromLanguageId{0x0457,       "kok-IN"}, //  Konkani
        BCP47FromLanguageId{0x045a,       "syr-SY"}, //  Syriac
        BCP47FromLanguageId{0x045b,        "si-LK"}, //  Sinhala
        BCP47FromLanguageId{0x045d,   "iu-Cans-CA"}, //  Inuktitut (Syllabics)
        BCP47FromLanguageId{0x045e,        "am-ET"}, //  Amharic
        BCP47FromLanguageId{0x0461,        "ne-NP"}, //  Nepali
        BCP47FromLanguageId{0x0462,        "fy-NL"}, //  Frisian
        BCP47FromLanguageId{0x0463,        "ps-AF"}, //  Pashto
        BCP47FromLanguageId{0x0464,       "fil-PH"}, //  Filipino
        BCP47FromLanguageId{0x0465,        "dv-MV"}, //  Divehi
        BCP47FromLanguageId{0x0468,   "ha-Latn-NG"}, //  Hausa (Latin)
        BCP47FromLanguageId{0x046a,        "yo-NG"}, //  Yoruba
        BCP47FromLanguageId{0x046b,       "quz-BO"}, //  Quechua
        BCP47FromLanguageId{0x046c,       "nso-ZA"}, //  Sesotho sa Leboa
        BCP47FromLanguageId{0x046d,        "ba-RU"}, //  Bashkir
        BCP47FromLanguageId{0x046e,        "lb-LU"}, //  Luxembourgish
        BCP47FromLanguageId{0x046f,        "kl-GL"}, //  Greenlandic
        BCP47FromLanguageId{0x0470,        "ig-NG"}, //  Igbo
        BCP47FromLanguageId{0x0478,        "ii-CN"}, //  Yi
        BCP47FromLanguageId{0x047a,       "arn-CL"}, //  Mapudungun
        BCP47FromLanguageId{0x047c,       "moh-CA"}, //  Mohawk
        BCP47FromLanguageId{0x047e,        "br-FR"}, //  Breton
        BCP47FromLanguageId{0x0480,        "ug-CN"}, //  Uyghur
        BCP47FromLanguageId{0x0481,        "mi-NZ"}, //  Maori
        BCP47FromLanguageId{0x0482,        "oc-FR"}, //  Occitan
        BCP47FromLanguageId{0x0483,        "co-FR"}, //  Corsican
        BCP47FromLanguageId{0x0484,       "gsw-FR"}, //  Alsatian
        BCP47FromLanguageId{0x0485,       "sah-RU"}, //  Yakut
        BCP47FromLanguageId{0x0486,       "qut-GT"}, //  K'iche
        BCP47FromLanguageId{0x0487,        "rw-RW"}, //  Kinyarwanda
        BCP47FromLanguageId{0x0488,        "wo-SN"}, //  Wolof
        BCP47FromLanguageId{0x048c,       "prs-AF"}, //  Dari
        BCP47FromLanguageId{0x0491,        "gd-GB"}, //  Scottish Gaelic
        BCP47FromLanguageId{0x0801,        "ar-IQ"}, //  Arabic
        BCP47FromLanguageId{0x0804,      "zh-Hans"}, //  Chinese (Simplified)
        BCP47FromLanguageId{0x0807,        "de-CH"}, //  German
        BCP47FromLanguageId{0x0809,        "en-GB"}, //  English
        BCP47FromLanguageId{0x080a,        "es-MX"}, //  Spanish
        BCP47FromLanguageId{0x080c,        "fr-BE"}, //  French
        BCP47FromLanguageId{0x0810,        "it-CH"}, //  Italian
        BCP47FromLanguageId{0x0813,        "nl-BE"}, //  Dutch
        BCP47FromLanguageId{0x0814,        "nn-NO"}, //  Norwegian (Nynorsk)
        BCP47FromLanguageId{0x0816,        "pt-PT"}, //  Portuguese
        BCP47FromLanguageId{0x081a,   "sr-Latn-CS"}, //  Serbian (Latin)
        BCP47FromLanguageId{0x081d,        "sv-FI"}, //  Swedish
        BCP47FromLanguageId{0x082c,   "az-Cyrl-AZ"}, //  Azeri (Cyrillic)
        BCP47FromLanguageId{0x082e,       "dsb-DE"}, //  Lower Sorbian
        BCP47FromLanguageId{0x082e,          "dsb"}, //  Lower Sorbian
        BCP47FromLanguageId{0x083b,        "se-SE"}, //  Sami (Northern)
        BCP47FromLanguageId{0x083c,        "ga-IE"}, //  Irish
        BCP47FromLanguageId{0x083e,        "ms-BN"}, //  Malay
        BCP47FromLanguageId{0x0843,   "uz-Cyrl-UZ"}, //  Uzbek (Cyrillic)
        BCP47FromLanguageId{0x0845,        "bn-BD"}, //  Bengali
        BCP47FromLanguageId{0x0850,   "mn-Mong-CN"}, //  Mongolian (Traditional Mongolian)
        BCP47FromLanguageId{0x085d,   "iu-Latn-CA"}, //  Inuktitut (Latin)
        BCP47FromLanguageId{0x085f,  "tzm-Latn-DZ"}, //  Tamazight (Latin)
        BCP47FromLanguageId{0x086b,       "quz-EC"}, //  Quechua
        BCP47FromLanguageId{0x0c01,        "ar-EG"}, //  Arabic
        BCP47FromLanguageId{0x0c04,      "zh-Hant"}, //  Chinese (Traditional)
        BCP47FromLanguageId{0x0c07,        "de-AT"}, //  German
        BCP47FromLanguageId{0x0c09,        "en-AU"}, //  English
        BCP47FromLanguageId{0x0c0a,        "es-ES"}, //  Spanish
        BCP47FromLanguageId{0x0c0c,        "fr-CA"}, //  French
        BCP47FromLanguageId{0x0c1a,   "sr-Cyrl-CS"}, //  Serbian (Cyrillic)
        BCP47FromLanguageId{0x0c3b,        "se-FI"}, //  Sami (Northern)
        BCP47FromLanguageId{0x0c6b,       "quz-PE"}, //  Quechua
        BCP47FromLanguageId{0x1001,        "ar-LY"}, //  Arabic
        BCP47FromLanguageId{0x1004,        "zh-SG"}, //  Chinese (Simplified)
        BCP47FromLanguageId{0x1007,        "de-LU"}, //  German
        BCP47FromLanguageId{0x1009,        "en-CA"}, //  English
        BCP47FromLanguageId{0x100a,        "es-GT"}, //  Spanish
        BCP47FromLanguageId{0x100c,        "fr-CH"}, //  French
        BCP47FromLanguageId{0x101a,        "hr-BA"}, //  Croatian (Latin)
        BCP47FromLanguageId{0x103b,       "smj-NO"}, //  Sami (Lule)
        BCP47FromLanguageId{0x1401,        "ar-DZ"}, //  Arabic
        BCP47FromLanguageId{0x1404,        "zh-MO"}, //  Chinese (Traditional)
        BCP47FromLanguageId{0x1407,        "de-LI"}, //  German
        BCP47FromLanguageId{0x1409,        "en-NZ"}, //  English
        BCP47FromLanguageId{0x140a,        "es-CR"}, //  Spanish
        BCP47FromLanguageId{0x140c,        "fr-LU"}, //  French
        BCP47FromLanguageId{0x141a,   "bs-Latn-BA"}, //  Bosnian (Latin)
        BCP47FromLanguageId{0x141a,           "bs"}, //  Bosnian
        BCP47FromLanguageId{0x143b,       "smj-SE"}, //  Sami (Lule)
        BCP47FromLanguageId{0x143b,          "smj"}, //  Sami (Lule)
        BCP47FromLanguageId{0x1801,        "ar-MA"}, //  Arabic
        BCP47FromLanguageId{0x1809,        "en-IE"}, //  English
        BCP47FromLanguageId{0x180a,        "es-PA"}, //  Spanish
        BCP47FromLanguageId{0x180c,        "fr-MC"}, //  French
        BCP47FromLanguageId{0x181a,   "sr-Latn-BA"}, //  Serbian (Latin)
        BCP47FromLanguageId{0x183b,       "sma-NO"}, //  Sami (Southern)
        BCP47FromLanguageId{0x1c01,        "ar-TN"}, //  Arabic
        BCP47FromLanguageId{0x1c09,        "en-ZA"}, //  English
        BCP47FromLanguageId{0x1c0a,        "es-DO"}, //  Spanish
        BCP47FromLanguageId{0x1c1a,   "sr-Cyrl-BA"}, //  Serbian (Cyrillic)
        BCP47FromLanguageId{0x1c3b,       "sma-SE"}, //  Sami (Southern)
        BCP47FromLanguageId{0x1c3b,          "sma"}, //  Sami (Southern)
        BCP47FromLanguageId{0x2001,        "ar-OM"}, //  Arabic
        BCP47FromLanguageId{0x2009,        "en-JM"}, //  English
        BCP47FromLanguageId{0x200a,        "es-VE"}, //  Spanish
        BCP47FromLanguageId{0x201a,   "bs-Cyrl-BA"}, //  Bosnian (Cyrillic)
        BCP47FromLanguageId{0x201a,      "bs-Cyrl"}, //  Bosnian (Cyrillic)
        BCP47FromLanguageId{0x203b,       "sms-FI"}, //  Sami (Skolt)
        BCP47FromLanguageId{0x203b,          "sms"}, //  Sami (Skolt)
        BCP47FromLanguageId{0x2401,        "ar-YE"}, //  Arabic
        BCP47FromLanguageId{0x2409,       "en-029"}, //  English
        BCP47FromLanguageId{0x240a,        "es-CO"}, //  Spanish
        BCP47FromLanguageId{0x241a,   "sr-Latn-RS"}, //  Serbian (Latin)
        BCP47FromLanguageId{0x243b,       "smn-FI"}, //  Sami (Inari)
        BCP47FromLanguageId{0x2801,        "ar-SY"}, //  Arabic
        BCP47FromLanguageId{0x2809,        "en-BZ"}, //  English
        BCP47FromLanguageId{0x280a,        "es-PE"}, //  Spanish
        BCP47FromLanguageId{0x281a,   "sr-Cyrl-RS"}, //  Serbian (Cyrillic)
        BCP47FromLanguageId{0x2c01,        "ar-JO"}, //  Arabic
        BCP47FromLanguageId{0x2c09,        "en-TT"}, //  English
        BCP47FromLanguageId{0x2c0a,        "es-AR"}, //  Spanish
        BCP47FromLanguageId{0x2c1a,   "sr-Latn-ME"}, //  Serbian (Latin)
        BCP47FromLanguageId{0x3001,        "ar-LB"}, //  Arabic
        BCP47FromLanguageId{0x3009,        "en-ZW"}, //  English
        BCP47FromLanguageId{0x300a,        "es-EC"}, //  Spanish
        BCP47FromLanguageId{0x301a,   "sr-Cyrl-ME"}, //  Serbian (Cyrillic)
        BCP47FromLanguageId{0x3401,        "ar-KW"}, //  Arabic
        BCP47FromLanguageId{0x3409,        "en-PH"}, //  English
        BCP47FromLanguageId{0x340a,        "es-CL"}, //  Spanish
        BCP47FromLanguageId{0x3801,        "ar-AE"}, //  Arabic
        BCP47FromLanguageId{0x380a,        "es-UY"}, //  Spanish
        BCP47FromLanguageId{0x3c01,        "ar-BH"}, //  Arabic
        BCP47FromLanguageId{0x3c0a,        "es-PY"}, //  Spanish
        BCP47FromLanguageId{0x4001,        "ar-QA"}, //  Arabic
        BCP47FromLanguageId{0x4009,        "en-IN"}, //  English
        BCP47FromLanguageId{0x400a,        "es-BO"}, //  Spanish
        BCP47FromLanguageId{0x4409,        "en-MY"}, //  English
        BCP47FromLanguageId{0x440a,        "es-SV"}, //  Spanish
        BCP47FromLanguageId{0x4809,        "en-SG"}, //  English
        BCP47FromLanguageId{0x480a,        "es-HN"}, //  Spanish
        BCP47FromLanguageId{0x4c0a,        "es-NI"}, //  Spanish
        BCP47FromLanguageId{0x500a,        "es-PR"}, //  Spanish
        BCP47FromLanguageId{0x540a,        "es-US"}, //  Spanish
});

namespace {
bool BCP47FromLanguageIdLess(const BCP47FromLanguageId& a, const BCP47FromLanguageId& b) {
    return a.languageID < b.languageID;
}
}  // namespace

bool SkOTTableName::Iterator::next(SkOTTableName::Iterator::Record& record) {
    SkOTTableName nameTable;
    if (fNameTableSize < sizeof(nameTable)) {
        return false;
    }
    memcpy(&nameTable, fNameTable, sizeof(nameTable));

    const uint8_t* nameRecords = fNameTable + sizeof(nameTable);
    const size_t nameRecordsSize = fNameTableSize - sizeof(nameTable);

    const size_t stringTableOffset = SkEndian_SwapBE16(nameTable.stringOffset);
    if (fNameTableSize < stringTableOffset) {
        return false;
    }
    const uint8_t* stringTable = fNameTable + stringTableOffset;
    const size_t stringTableSize = fNameTableSize - stringTableOffset;

    // Find the next record which matches the requested type.
    SkOTTableName::Record nameRecord;
    const size_t nameRecordsCount = SkEndian_SwapBE16(nameTable.count);
    const size_t nameRecordsMax = std::min(nameRecordsCount, nameRecordsSize / sizeof(nameRecord));
    do {
        if (fIndex >= nameRecordsMax) {
            return false;
        }

        memcpy(&nameRecord, nameRecords + sizeof(nameRecord)*fIndex, sizeof(nameRecord));
        ++fIndex;
    } while (fType != -1 && nameRecord.nameID.fontSpecific != fType);

    record.type = nameRecord.nameID.fontSpecific;

    // Decode the name into UTF-8.
    const size_t nameOffset = SkEndian_SwapBE16(nameRecord.offset);
    const size_t nameLength = SkEndian_SwapBE16(nameRecord.length);
    if (stringTableSize < nameOffset + nameLength) {
        return false; // continue?
    }
    const uint8_t* nameString = stringTable + nameOffset;
    switch (nameRecord.platformID.value) {
        case SkOTTableName::Record::PlatformID::Windows:
            if (SkOTTableName::Record::EncodingID::Windows::UnicodeBMPUCS2
                   != nameRecord.encodingID.windows.value
                && SkOTTableName::Record::EncodingID::Windows::UnicodeUCS4
                   != nameRecord.encodingID.windows.value
                && SkOTTableName::Record::EncodingID::Windows::Symbol
                   != nameRecord.encodingID.windows.value)
            {
                record.name.reset();
                break; // continue?
            }
            [[fallthrough]];
        case SkOTTableName::Record::PlatformID::Unicode:
        case SkOTTableName::Record::PlatformID::ISO:
            SkString_from_UTF16BE(nameString, nameLength, record.name);
            break;

        case SkOTTableName::Record::PlatformID::Macintosh:
            // TODO: need better decoding, especially on Mac.
            if (SkOTTableName::Record::EncodingID::Macintosh::Roman
                != nameRecord.encodingID.macintosh.value)
            {
                record.name.reset();
                break;  // continue?
            }
            SkStringFromMacRoman(nameString, nameLength, record.name);
            break;

        case SkOTTableName::Record::PlatformID::Custom:
            // These should never appear in a 'name' table.
        default:
            SkASSERT(false);
            record.name.reset();
            break;  // continue?
    }

    // Determine the language.
    const uint16_t languageID = SkEndian_SwapBE16(nameRecord.languageID.languageTagID);

    // Handle format 1 languages.
    if (SkOTTableName::format_1 == nameTable.format && languageID >= 0x8000) {
        const uint16_t languageTagRecordIndex = languageID - 0x8000;

        if (nameRecordsSize < sizeof(nameRecord)*nameRecordsCount) {
            return false; //"und" or break?
        }
        const uint8_t* format1extData = nameRecords + sizeof(nameRecord)*nameRecordsCount;
        size_t format1extSize = nameRecordsSize - sizeof(nameRecord)*nameRecordsCount;
        SkOTTableName::Format1Ext format1ext;
        if (format1extSize < sizeof(format1ext)) {
            return false; // "und" or break?
        }
        memcpy(&format1ext, format1extData, sizeof(format1ext));

        const uint8_t* languageTagRecords = format1extData + sizeof(format1ext);
        size_t languageTagRecordsSize = format1extSize - sizeof(format1ext);
        if (languageTagRecordIndex < SkEndian_SwapBE16(format1ext.langTagCount)) {
            SkOTTableName::Format1Ext::LangTagRecord languageTagRecord;
            if (languageTagRecordsSize < sizeof(languageTagRecord)*(languageTagRecordIndex+1)) {
                return false; // "und"?
            }
            const uint8_t* languageTagData = languageTagRecords
                                           + sizeof(languageTagRecord)*languageTagRecordIndex;
            memcpy(&languageTagRecord, languageTagData, sizeof(languageTagRecord));

            uint16_t languageOffset = SkEndian_SwapBE16(languageTagRecord.offset);
            uint16_t languageLength = SkEndian_SwapBE16(languageTagRecord.length);

            if (fNameTableSize < stringTableOffset + languageOffset + languageLength) {
                return false; // "und"?
            }
            const uint8_t* languageString = stringTable + languageOffset;
            SkString_from_UTF16BE(languageString, languageLength, record.language);
            return true;
        }
    }

    // Handle format 0 languages, translating them into BCP 47.
    const BCP47FromLanguageId target = { languageID, "" };
    int languageIndex = SkTSearch<BCP47FromLanguageId, BCP47FromLanguageIdLess>(
            BCP47FromLanguageID.data(), std::size(BCP47FromLanguageID), target, sizeof(target));
    if (languageIndex >= 0) {
        record.language = BCP47FromLanguageID[languageIndex].bcp47;
        return true;
    }

    // Unknown language, return the BCP 47 code 'und' for 'undetermined'.
    record.language = "und";
    return true;
}
