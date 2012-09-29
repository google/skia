/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_name_DEFINED
#define SkOTTable_name_DEFINED

#include "SkEndian.h"
#include "SkOTTableTypes.h"
#include "SkTypedEnum.h"

#pragma pack(push, 1)

struct SkOTTableName {
    static const SK_OT_CHAR TAG0 = 'n';
    static const SK_OT_CHAR TAG1 = 'a';
    static const SK_OT_CHAR TAG2 = 'm';
    static const SK_OT_CHAR TAG3 = 'e';
    static const SK_OT_ULONG TAG = SkOTTableTAG<SkOTTableName>::value;

    SK_OT_USHORT format;
    static const SK_OT_USHORT format_0 = SkTEndian_SwapBE16(0);
    static const SK_OT_USHORT format_1 = SkTEndian_SwapBE16(1);
    SK_OT_USHORT count;
    SK_OT_USHORT stringOffset; //Offset to start of storage area (from start of table).
    //SkOTTableNameRecord nameRecord[count];
};
struct SkOTTableNameF1 {
    SK_OT_USHORT langTagCount;
    //SkOTTableNameLangTagRecord langTagRecord[langTagCount];
};

struct SkOTTableNameLangTagRecord {
    SK_OT_USHORT length;
    SK_OT_USHORT offset; //From start of storage area.
    //The string is always UTF-16BE from IETF specification BCP 47.
};

struct SkOTTableNameRecord {
    //The platform ID specifies how to interpret the encoding and language ID.
    struct PlatformID {
        SK_TYPED_ENUM(Value, SK_OT_USHORT,
            ((Unicode, SkTEndian_SwapBE16(0)))
            ((Macintosh, SkTEndian_SwapBE16(1)))
            ((ISO, SkTEndian_SwapBE16(2))) //deprecated, use Unicode instead
            ((Windows, SkTEndian_SwapBE16(3)))
            ((Custom, SkTEndian_SwapBE16(4)))
            SK_SEQ_END,
        (value)SK_SEQ_END)
    } platformID;
    union EncodingID {
        //Always UTF-16BE
        struct Unicode {
            SK_TYPED_ENUM(Value, SK_OT_USHORT,
                ((Unicode10, SkTEndian_SwapBE16(0)))
                ((Unicode11, SkTEndian_SwapBE16(1)))
                ((ISO10646, SkTEndian_SwapBE16(2))) //deprecated, use Unicode11
                ((Unicode20BMP, SkTEndian_SwapBE16(3)))
                ((Unicode20, SkTEndian_SwapBE16(4)))
                ((UnicodeVariationSequences, SkTEndian_SwapBE16(5)))
                ((UnicodeFull, SkTEndian_SwapBE16(6)))
                SK_SEQ_END,
            (value)SK_SEQ_END)
        } unicode;
        struct Macintosh {
            SK_TYPED_ENUM(Value, SK_OT_USHORT,
                ((Roman, SkTEndian_SwapBE16(0)))
                ((Japanese, SkTEndian_SwapBE16(1)))
                ((ChineseTraditional, SkTEndian_SwapBE16(2)))
                ((Korean, SkTEndian_SwapBE16(3)))
                ((Arabic, SkTEndian_SwapBE16(4)))
                ((Hebrew, SkTEndian_SwapBE16(5)))
                ((Greek, SkTEndian_SwapBE16(6)))
                ((Russian, SkTEndian_SwapBE16(7)))
                ((RSymbol, SkTEndian_SwapBE16(8)))
                ((Devanagari, SkTEndian_SwapBE16(9)))
                ((Gurmukhi, SkTEndian_SwapBE16(10)))
                ((Gujarati, SkTEndian_SwapBE16(11)))
                ((Oriya, SkTEndian_SwapBE16(12)))
                ((Bengali, SkTEndian_SwapBE16(13)))
                ((Tamil, SkTEndian_SwapBE16(14)))
                ((Telugu, SkTEndian_SwapBE16(15)))
                ((Kannada, SkTEndian_SwapBE16(16)))
                ((Malayalam, SkTEndian_SwapBE16(17)))
                ((Sinhalese, SkTEndian_SwapBE16(18)))
                ((Burmese, SkTEndian_SwapBE16(19)))
                ((Khmer, SkTEndian_SwapBE16(20)))
                ((Thai, SkTEndian_SwapBE16(21)))
                ((Laotian, SkTEndian_SwapBE16(22)))
                ((Georgian, SkTEndian_SwapBE16(23)))
                ((Armenian, SkTEndian_SwapBE16(24)))
                ((ChineseSimplified, SkTEndian_SwapBE16(25)))
                ((Tibetan, SkTEndian_SwapBE16(26)))
                ((Mongolian, SkTEndian_SwapBE16(27)))
                ((Geez, SkTEndian_SwapBE16(28)))
                ((Slavic, SkTEndian_SwapBE16(29)))
                ((Vietnamese, SkTEndian_SwapBE16(30)))
                ((Sindhi, SkTEndian_SwapBE16(31)))
                ((Uninterpreted, SkTEndian_SwapBE16(32)))
                SK_SEQ_END,
            (value)SK_SEQ_END)
        } macintosh;
        //deprecated, use Unicode instead
        struct ISO {
            SK_TYPED_ENUM(Value, SK_OT_USHORT,
                ((ASCII7, SkTEndian_SwapBE16(0)))
                ((ISO10646, SkTEndian_SwapBE16(1)))
                ((ISO88591, SkTEndian_SwapBE16(2)))
                SK_SEQ_END,
            (value)SK_SEQ_END)
        } iso;
        struct Windows {
            SK_TYPED_ENUM(Value, SK_OT_USHORT,
                ((Symbol, SkTEndian_SwapBE16(0)))
                ((UnicodeBMPUCS2, SkTEndian_SwapBE16(1))) //Windows default
                ((ShiftJIS, SkTEndian_SwapBE16(2)))
                ((PRC, SkTEndian_SwapBE16(3)))
                ((Big5, SkTEndian_SwapBE16(4)))
                ((Wansung, SkTEndian_SwapBE16(5)))
                ((Johab, SkTEndian_SwapBE16(6)))
                ((UnicodeUCS4, SkTEndian_SwapBE16(10)))
                SK_SEQ_END,
            (value)SK_SEQ_END)
        } windows;
        SK_OT_USHORT custom;
    } encodingID;
    union LanguageID {
        struct Macintosh {
            SK_TYPED_ENUM(Value, SK_OT_USHORT,
                ((English, SkTEndian_SwapBE16(0)))
                ((French, SkTEndian_SwapBE16(1)))
                ((German, SkTEndian_SwapBE16(2)))
                ((Italian, SkTEndian_SwapBE16(3)))
                ((Dutch, SkTEndian_SwapBE16(4)))
                ((Swedish, SkTEndian_SwapBE16(5)))
                ((Spanish, SkTEndian_SwapBE16(6)))
                ((Danish, SkTEndian_SwapBE16(7)))
                ((Portuguese, SkTEndian_SwapBE16(8)))
                ((Norwegian, SkTEndian_SwapBE16(9)))
                ((Hebrew, SkTEndian_SwapBE16(10)))
                ((Japanese, SkTEndian_SwapBE16(11)))
                ((Arabic, SkTEndian_SwapBE16(12)))
                ((Finnish, SkTEndian_SwapBE16(13)))
                ((Greek, SkTEndian_SwapBE16(14)))
                ((Icelandic, SkTEndian_SwapBE16(15)))
                ((Maltese, SkTEndian_SwapBE16(16)))
                ((Turkish, SkTEndian_SwapBE16(17)))
                ((Croatian, SkTEndian_SwapBE16(18)))
                ((ChineseTraditional, SkTEndian_SwapBE16(19)))
                ((Urdu, SkTEndian_SwapBE16(20)))
                ((Hindi, SkTEndian_SwapBE16(21)))
                ((Thai, SkTEndian_SwapBE16(22)))
                ((Korean, SkTEndian_SwapBE16(23)))
                ((Lithuanian, SkTEndian_SwapBE16(24)))
                ((Polish, SkTEndian_SwapBE16(25)))
                ((Hungarian, SkTEndian_SwapBE16(26)))
                ((Estonian, SkTEndian_SwapBE16(27)))
                ((Latvian, SkTEndian_SwapBE16(28)))
                ((Sami, SkTEndian_SwapBE16(29)))
                ((Faroese, SkTEndian_SwapBE16(30)))
                ((Farsi_Persian, SkTEndian_SwapBE16(31)))
                ((Russian, SkTEndian_SwapBE16(32)))
                ((ChineseSimplified, SkTEndian_SwapBE16(33)))
                ((Flemish, SkTEndian_SwapBE16(34)))
                ((IrishGaelic, SkTEndian_SwapBE16(35)))
                ((Albanian, SkTEndian_SwapBE16(36)))
                ((Romanian, SkTEndian_SwapBE16(37)))
                ((Czech, SkTEndian_SwapBE16(38)))
                ((Slovak, SkTEndian_SwapBE16(39)))
                ((Slovenian, SkTEndian_SwapBE16(40)))
                ((Yiddish, SkTEndian_SwapBE16(41)))
                ((Serbian, SkTEndian_SwapBE16(42)))
                ((Macedonian, SkTEndian_SwapBE16(43)))
                ((Bulgarian, SkTEndian_SwapBE16(44)))
                ((Ukrainian, SkTEndian_SwapBE16(45)))
                ((Byelorussian, SkTEndian_SwapBE16(46)))
                ((Uzbek, SkTEndian_SwapBE16(47)))
                ((Kazakh, SkTEndian_SwapBE16(48)))
                ((AzerbaijaniCyrillic, SkTEndian_SwapBE16(49)))
                ((AzerbaijaniArabic, SkTEndian_SwapBE16(50)))
                ((Armenian, SkTEndian_SwapBE16(51)))
                ((Georgian, SkTEndian_SwapBE16(52)))
                ((Moldavian, SkTEndian_SwapBE16(53)))
                ((Kirghiz, SkTEndian_SwapBE16(54)))
                ((Tajiki, SkTEndian_SwapBE16(55)))
                ((Turkmen, SkTEndian_SwapBE16(56)))
                ((MongolianTraditional, SkTEndian_SwapBE16(57)))
                ((MongolianCyrillic, SkTEndian_SwapBE16(58)))
                ((Pashto, SkTEndian_SwapBE16(59)))
                ((Kurdish, SkTEndian_SwapBE16(60)))
                ((Kashmiri, SkTEndian_SwapBE16(61)))
                ((Sindhi, SkTEndian_SwapBE16(62)))
                ((Tibetan, SkTEndian_SwapBE16(63)))
                ((Nepali, SkTEndian_SwapBE16(64)))
                ((Sanskrit, SkTEndian_SwapBE16(65)))
                ((Marathi, SkTEndian_SwapBE16(66)))
                ((Bengali, SkTEndian_SwapBE16(67)))
                ((Assamese, SkTEndian_SwapBE16(68)))
                ((Gujarati, SkTEndian_SwapBE16(69)))
                ((Punjabi, SkTEndian_SwapBE16(70)))
                ((Oriya, SkTEndian_SwapBE16(71)))
                ((Malayalam, SkTEndian_SwapBE16(72)))
                ((Kannada, SkTEndian_SwapBE16(73)))
                ((Tamil, SkTEndian_SwapBE16(74)))
                ((Telugu, SkTEndian_SwapBE16(75)))
                ((Sinhalese, SkTEndian_SwapBE16(76)))
                ((Burmese, SkTEndian_SwapBE16(77)))
                ((Khmer, SkTEndian_SwapBE16(78)))
                ((Lao, SkTEndian_SwapBE16(79)))
                ((Vietnamese, SkTEndian_SwapBE16(80)))
                ((Indonesian, SkTEndian_SwapBE16(81)))
                ((Tagalong, SkTEndian_SwapBE16(82)))
                ((MalayRoman, SkTEndian_SwapBE16(83)))
                ((MalayArabic, SkTEndian_SwapBE16(84)))
                ((Amharic, SkTEndian_SwapBE16(85)))
                ((Tigrinya, SkTEndian_SwapBE16(86)))
                ((Galla, SkTEndian_SwapBE16(87)))
                ((Somali, SkTEndian_SwapBE16(88)))
                ((Swahili, SkTEndian_SwapBE16(89)))
                ((Kinyarwanda_Ruanda, SkTEndian_SwapBE16(90)))
                ((Rundi, SkTEndian_SwapBE16(91)))
                ((Nyanja_Chewa, SkTEndian_SwapBE16(92)))
                ((Malagasy, SkTEndian_SwapBE16(93)))
                ((Esperanto, SkTEndian_SwapBE16(94)))
                ((Welsh, SkTEndian_SwapBE16(128)))
                ((Basque, SkTEndian_SwapBE16(129)))
                ((Catalan, SkTEndian_SwapBE16(130)))
                ((Latin, SkTEndian_SwapBE16(131)))
                ((Quenchua, SkTEndian_SwapBE16(132)))
                ((Guarani, SkTEndian_SwapBE16(133)))
                ((Aymara, SkTEndian_SwapBE16(134)))
                ((Tatar, SkTEndian_SwapBE16(135)))
                ((Uighur, SkTEndian_SwapBE16(136)))
                ((Dzongkha, SkTEndian_SwapBE16(137)))
                ((JavaneseRoman, SkTEndian_SwapBE16(138)))
                ((SundaneseRoman, SkTEndian_SwapBE16(139)))
                ((Galician, SkTEndian_SwapBE16(140)))
                ((Afrikaans, SkTEndian_SwapBE16(141)))
                ((Breton, SkTEndian_SwapBE16(142)))
                ((Inuktitut, SkTEndian_SwapBE16(143)))
                ((ScottishGaelic, SkTEndian_SwapBE16(144)))
                ((ManxGaelic, SkTEndian_SwapBE16(145)))
                ((IrishGaelicWithLenition, SkTEndian_SwapBE16(146)))
                ((Tongan, SkTEndian_SwapBE16(147)))
                ((GreekPolytonic, SkTEndian_SwapBE16(148)))
                ((Greenlandic, SkTEndian_SwapBE16(149)))
                ((AzerbaijaniRoman, SkTEndian_SwapBE16(150)))
                SK_SEQ_END,
            (value)SK_SEQ_END)
        } macintosh;
        struct Windows {
            SK_TYPED_ENUM(Value, SK_OT_USHORT,
                ((Afrikaans_SouthAfrica, SkTEndian_SwapBE16(0x0436)))
                ((Albanian_Albania, SkTEndian_SwapBE16(0x041C)))
                ((Alsatian_France, SkTEndian_SwapBE16(0x0484)))
                ((Amharic_Ethiopia, SkTEndian_SwapBE16(0x045E)))
                ((Arabic_Algeria, SkTEndian_SwapBE16(0x1401)))
                ((Arabic_Bahrain, SkTEndian_SwapBE16(0x3C01)))
                ((Arabic_Egypt, SkTEndian_SwapBE16(0x0C01)))
                ((Arabic_Iraq, SkTEndian_SwapBE16(0x0801)))
                ((Arabic_Jordan, SkTEndian_SwapBE16(0x2C01)))
                ((Arabic_Kuwait, SkTEndian_SwapBE16(0x3401)))
                ((Arabic_Lebanon, SkTEndian_SwapBE16(0x3001)))
                ((Arabic_Libya, SkTEndian_SwapBE16(0x1001)))
                ((Arabic_Morocco, SkTEndian_SwapBE16(0x1801)))
                ((Arabic_Oman, SkTEndian_SwapBE16(0x2001)))
                ((Arabic_Qatar, SkTEndian_SwapBE16(0x4001)))
                ((Arabic_SaudiArabia, SkTEndian_SwapBE16(0x0401)))
                ((Arabic_Syria, SkTEndian_SwapBE16(0x2801)))
                ((Arabic_Tunisia, SkTEndian_SwapBE16(0x1C01)))
                ((Arabic_UAE, SkTEndian_SwapBE16(0x3801)))
                ((Arabic_Yemen, SkTEndian_SwapBE16(0x2401)))
                ((Armenian_Armenia, SkTEndian_SwapBE16(0x042B)))
                ((Assamese_India, SkTEndian_SwapBE16(0x044D)))
                ((AzeriCyrillic_Azerbaijan, SkTEndian_SwapBE16(0x082C)))
                ((AzeriLatin_Azerbaijan, SkTEndian_SwapBE16(0x042C)))
                ((Bashkir_Russia, SkTEndian_SwapBE16(0x046D)))
                ((Basque_Basque, SkTEndian_SwapBE16(0x042D)))
                ((Belarusian_Belarus, SkTEndian_SwapBE16(0x0423)))
                ((Bengali_Bangladesh, SkTEndian_SwapBE16(0x0845)))
                ((Bengali_India, SkTEndian_SwapBE16(0x0445)))
                ((BosnianCyrillic_BosniaAndHerzegovina, SkTEndian_SwapBE16(0x201A)))
                ((BosnianLatin_BosniaAndHerzegovina, SkTEndian_SwapBE16(0x141A)))
                ((Breton_France, SkTEndian_SwapBE16(0x047E)))
                ((Bulgarian_Bulgaria, SkTEndian_SwapBE16(0x0402)))
                ((Catalan_Catalan, SkTEndian_SwapBE16(0x0403)))
                ((Chinese_HongKongSAR, SkTEndian_SwapBE16(0x0C04)))
                ((Chinese_MacaoSAR, SkTEndian_SwapBE16(0x1404)))
                ((Chinese_PeoplesRepublicOfChina, SkTEndian_SwapBE16(0x0804)))
                ((Chinese_Singapore, SkTEndian_SwapBE16(0x1004)))
                ((Chinese_Taiwan, SkTEndian_SwapBE16(0x0404)))
                ((Corsican_France, SkTEndian_SwapBE16(0x0483)))
                ((Croatian_Croatia, SkTEndian_SwapBE16(0x041A)))
                ((CroatianLatin_BosniaAndHerzegovina, SkTEndian_SwapBE16(0x101A)))
                ((Czech_CzechRepublic, SkTEndian_SwapBE16(0x0405)))
                ((Danish_Denmark, SkTEndian_SwapBE16(0x0406)))
                ((Dari_Afghanistan, SkTEndian_SwapBE16(0x048C)))
                ((Divehi_Maldives, SkTEndian_SwapBE16(0x0465)))
                ((Dutch_Belgium, SkTEndian_SwapBE16(0x0813)))
                ((Dutch_Netherlands, SkTEndian_SwapBE16(0x0413)))
                ((English_Australia, SkTEndian_SwapBE16(0x0C09)))
                ((English_Belize, SkTEndian_SwapBE16(0x2809)))
                ((English_Canada, SkTEndian_SwapBE16(0x1009)))
                ((English_Caribbean, SkTEndian_SwapBE16(0x2409)))
                ((English_India, SkTEndian_SwapBE16(0x4009)))
                ((English_Ireland, SkTEndian_SwapBE16(0x1809)))
                ((English_Jamaica, SkTEndian_SwapBE16(0x2009)))
                ((English_Malaysia, SkTEndian_SwapBE16(0x4409)))
                ((English_NewZealand, SkTEndian_SwapBE16(0x1409)))
                ((English_RepublicOfThePhilippines, SkTEndian_SwapBE16(0x3409)))
                ((English_Singapore, SkTEndian_SwapBE16(0x4809)))
                ((English_SouthAfrica, SkTEndian_SwapBE16(0x1C09)))
                ((English_TrinidadAndTobago, SkTEndian_SwapBE16(0x2C09)))
                ((English_UnitedKingdom, SkTEndian_SwapBE16(0x0809)))
                ((English_UnitedStates, SkTEndian_SwapBE16(0x0409)))
                ((English_Zimbabwe, SkTEndian_SwapBE16(0x3009)))
                ((Estonian_Estonia, SkTEndian_SwapBE16(0x0425)))
                ((Faroese_FaroeIslands, SkTEndian_SwapBE16(0x0438)))
                ((Filipino_Philippines, SkTEndian_SwapBE16(0x0464)))
                ((Finnish_Finland, SkTEndian_SwapBE16(0x040B)))
                ((French_Belgium, SkTEndian_SwapBE16(0x080C)))
                ((French_Canada, SkTEndian_SwapBE16(0x0C0C)))
                ((French_France, SkTEndian_SwapBE16(0x040C)))
                ((French_Luxembourg, SkTEndian_SwapBE16(0x140c)))
                ((French_PrincipalityOfMonoco, SkTEndian_SwapBE16(0x180C)))
                ((French_Switzerland, SkTEndian_SwapBE16(0x100C)))
                ((Frisian_Netherlands, SkTEndian_SwapBE16(0x0462)))
                ((Galician_Galician, SkTEndian_SwapBE16(0x0456)))
                ((Georgian_Georgia, SkTEndian_SwapBE16(0x0437)))
                ((German_Austria, SkTEndian_SwapBE16(0x0C07)))
                ((German_Germany, SkTEndian_SwapBE16(0x0407)))
                ((German_Liechtenstein, SkTEndian_SwapBE16(0x1407)))
                ((German_Luxembourg, SkTEndian_SwapBE16(0x1007)))
                ((German_Switzerland, SkTEndian_SwapBE16(0x0807)))
                ((Greek_Greece, SkTEndian_SwapBE16(0x0408)))
                ((Greenlandic_Greenland, SkTEndian_SwapBE16(0x046F)))
                ((Gujarati_India, SkTEndian_SwapBE16(0x0447)))
                ((HausaLatin_Nigeria, SkTEndian_SwapBE16(0x0468)))
                ((Hebrew_Israel, SkTEndian_SwapBE16(0x040D)))
                ((Hindi_India, SkTEndian_SwapBE16(0x0439)))
                ((Hungarian_Hungary, SkTEndian_SwapBE16(0x040E)))
                ((Icelandic_Iceland, SkTEndian_SwapBE16(0x040F)))
                ((Igbo_Nigeria, SkTEndian_SwapBE16(0x0470)))
                ((Indonesian_Indonesia, SkTEndian_SwapBE16(0x0421)))
                ((Inuktitut_Canada, SkTEndian_SwapBE16(0x045D)))
                ((InuktitutLatin_Canada, SkTEndian_SwapBE16(0x085D)))
                ((Irish_Ireland, SkTEndian_SwapBE16(0x083C)))
                ((isiXhosa_SouthAfrica, SkTEndian_SwapBE16(0x0434)))
                ((isiZulu_SouthAfrica, SkTEndian_SwapBE16(0x0435)))
                ((Italian_Italy, SkTEndian_SwapBE16(0x0410)))
                ((Italian_Switzerland, SkTEndian_SwapBE16(0x0810)))
                ((Japanese_Japan, SkTEndian_SwapBE16(0x0411)))
                ((Kannada_India, SkTEndian_SwapBE16(0x044B)))
                ((Kazakh_Kazakhstan, SkTEndian_SwapBE16(0x043F)))
                ((Khmer_Cambodia, SkTEndian_SwapBE16(0x0453)))
                ((Kiche_Guatemala, SkTEndian_SwapBE16(0x0486)))
                ((Kinyarwanda_Rwanda, SkTEndian_SwapBE16(0x0487)))
                ((Kiswahili_Kenya, SkTEndian_SwapBE16(0x0441)))
                ((Konkani_India, SkTEndian_SwapBE16(0x0457)))
                ((Korean_Korea, SkTEndian_SwapBE16(0x0412)))
                ((Kyrgyz_Kyrgyzstan, SkTEndian_SwapBE16(0x0440)))
                ((Lao_LaoPDR, SkTEndian_SwapBE16(0x0454)))
                ((Latvian_Latvia, SkTEndian_SwapBE16(0x0426)))
                ((Lithuanian_Lithuania, SkTEndian_SwapBE16(0x0427)))
                ((LowerSorbian_Germany, SkTEndian_SwapBE16(0x082E)))
                ((Luxembourgish_Luxembourg, SkTEndian_SwapBE16(0x046E)))
                ((MacedonianFYROM_FormerYugoslavRepublicOfMacedonia, SkTEndian_SwapBE16(0x042F)))
                ((Malay_BruneiDarussalam, SkTEndian_SwapBE16(0x083E)))
                ((Malay_Malaysia, SkTEndian_SwapBE16(0x043E)))
                ((Malayalam_India, SkTEndian_SwapBE16(0x044C)))
                ((Maltese_Malta, SkTEndian_SwapBE16(0x043A)))
                ((Maori_NewZealand, SkTEndian_SwapBE16(0x0481)))
                ((Mapudungun_Chile, SkTEndian_SwapBE16(0x047A)))
                ((Marathi_India, SkTEndian_SwapBE16(0x044E)))
                ((Mohawk_Mohawk, SkTEndian_SwapBE16(0x047C)))
                ((MongolianCyrillic_Mongolia, SkTEndian_SwapBE16(0x0450)))
                ((MongolianTraditional_PeoplesRepublicOfChina, SkTEndian_SwapBE16(0x0850)))
                ((Nepali_Nepal, SkTEndian_SwapBE16(0x0461)))
                ((NorwegianBokmal_Norway, SkTEndian_SwapBE16(0x0414)))
                ((NorwegianNynorsk_Norway, SkTEndian_SwapBE16(0x0814)))
                ((Occitan_France, SkTEndian_SwapBE16(0x0482)))
                ((Odia_India, SkTEndian_SwapBE16(0x0448)))
                ((Pashto_Afghanistan, SkTEndian_SwapBE16(0x0463)))
                ((Polish_Poland, SkTEndian_SwapBE16(0x0415)))
                ((Portuguese_Brazil, SkTEndian_SwapBE16(0x0416)))
                ((Portuguese_Portugal, SkTEndian_SwapBE16(0x0816)))
                ((Punjabi_India, SkTEndian_SwapBE16(0x0446)))
                ((Quechua_Bolivia, SkTEndian_SwapBE16(0x046B)))
                ((Quechua_Ecuador, SkTEndian_SwapBE16(0x086B)))
                ((Quechua_Peru, SkTEndian_SwapBE16(0x0C6B)))
                ((Romanian_Romania, SkTEndian_SwapBE16(0x0418)))
                ((Romansh_Switzerland, SkTEndian_SwapBE16(0x0417)))
                ((Russian_Russia, SkTEndian_SwapBE16(0x0419)))
                ((SamiInari_Finland, SkTEndian_SwapBE16(0x243B)))
                ((SamiLule_Norway, SkTEndian_SwapBE16(0x103B)))
                ((SamiLule_Sweden, SkTEndian_SwapBE16(0x143B)))
                ((SamiNorthern_Finland, SkTEndian_SwapBE16(0x0C3B)))
                ((SamiNorthern_Norway, SkTEndian_SwapBE16(0x043B)))
                ((SamiNorthern_Sweden, SkTEndian_SwapBE16(0x083B)))
                ((SamiSkolt_Finland, SkTEndian_SwapBE16(0x203B)))
                ((SamiSouthern_Norway, SkTEndian_SwapBE16(0x183B)))
                ((SamiSouthern_Sweden, SkTEndian_SwapBE16(0x1C3B)))
                ((Sanskrit_India, SkTEndian_SwapBE16(0x044F)))
                ((SerbianCyrillic_BosniaAndHerzegovina, SkTEndian_SwapBE16(0x1C1A)))
                ((SerbianCyrillic_Serbia, SkTEndian_SwapBE16(0x0C1A)))
                ((SerbianLatin_BosniaAndHerzegovina, SkTEndian_SwapBE16(0x181A)))
                ((SerbianLatin_Serbia, SkTEndian_SwapBE16(0x081A)))
                ((SesothoSaLeboa_SouthAfrica, SkTEndian_SwapBE16(0x046C)))
                ((Setswana_SouthAfrica, SkTEndian_SwapBE16(0x0432)))
                ((Sinhala_SriLanka, SkTEndian_SwapBE16(0x045B)))
                ((Slovak_Slovakia, SkTEndian_SwapBE16(0x041B)))
                ((Slovenian_Slovenia, SkTEndian_SwapBE16(0x0424)))
                ((Spanish_Argentina, SkTEndian_SwapBE16(0x2C0A)))
                ((Spanish_Bolivia, SkTEndian_SwapBE16(0x400A)))
                ((Spanish_Chile, SkTEndian_SwapBE16(0x340A)))
                ((Spanish_Colombia, SkTEndian_SwapBE16(0x240A)))
                ((Spanish_CostaRica, SkTEndian_SwapBE16(0x140A)))
                ((Spanish_DominicanRepublic, SkTEndian_SwapBE16(0x1C0A)))
                ((Spanish_Ecuador, SkTEndian_SwapBE16(0x300A)))
                ((Spanish_ElSalvador, SkTEndian_SwapBE16(0x440A)))
                ((Spanish_Guatemala, SkTEndian_SwapBE16(0x100A)))
                ((Spanish_Honduras, SkTEndian_SwapBE16(0x480A)))
                ((Spanish_Mexico, SkTEndian_SwapBE16(0x080A)))
                ((Spanish_Nicaragua, SkTEndian_SwapBE16(0x4C0A)))
                ((Spanish_Panama, SkTEndian_SwapBE16(0x180A)))
                ((Spanish_Paraguay, SkTEndian_SwapBE16(0x3C0A)))
                ((Spanish_Peru, SkTEndian_SwapBE16(0x280A)))
                ((Spanish_PuertoRico, SkTEndian_SwapBE16(0x500A)))
                ((SpanishModernSort_Spain, SkTEndian_SwapBE16(0x0C0A)))
                ((SpanishTraditionalSort_Spain, SkTEndian_SwapBE16(0x040A)))
                ((Spanish_UnitedStates, SkTEndian_SwapBE16(0x540A)))
                ((Spanish_Uruguay, SkTEndian_SwapBE16(0x380A)))
                ((Spanish_Venezuela, SkTEndian_SwapBE16(0x200A)))
                ((Sweden_Finland, SkTEndian_SwapBE16(0x081D)))
                ((Swedish_Sweden, SkTEndian_SwapBE16(0x041D)))
                ((Syriac_Syria, SkTEndian_SwapBE16(0x045A)))
                ((TajikCyrillic_Tajikistan, SkTEndian_SwapBE16(0x0428)))
                ((TamazightLatin_Algeria, SkTEndian_SwapBE16(0x085F)))
                ((Tamil_India, SkTEndian_SwapBE16(0x0449)))
                ((Tatar_Russia, SkTEndian_SwapBE16(0x0444)))
                ((Telugu_India, SkTEndian_SwapBE16(0x044A)))
                ((Thai_Thailand, SkTEndian_SwapBE16(0x041E)))
                ((Tibetan_PRC, SkTEndian_SwapBE16(0x0451)))
                ((Turkish_Turkey, SkTEndian_SwapBE16(0x041F)))
                ((Turkmen_Turkmenistan, SkTEndian_SwapBE16(0x0442)))
                ((Uighur_PRC, SkTEndian_SwapBE16(0x0480)))
                ((Ukrainian_Ukraine, SkTEndian_SwapBE16(0x0422)))
                ((UpperSorbian_Germany, SkTEndian_SwapBE16(0x042E)))
                ((Urdu_IslamicRepublicOfPakistan, SkTEndian_SwapBE16(0x0420)))
                ((UzbekCyrillic_Uzbekistan, SkTEndian_SwapBE16(0x0843)))
                ((UzbekLatin_Uzbekistan, SkTEndian_SwapBE16(0x0443)))
                ((Vietnamese_Vietnam, SkTEndian_SwapBE16(0x042A)))
                ((Welsh_UnitedKingdom, SkTEndian_SwapBE16(0x0452)))
                ((Wolof_Senegal, SkTEndian_SwapBE16(0x0488)))
                ((Yakut_Russia, SkTEndian_SwapBE16(0x0485)))
                ((Yi_PRC, SkTEndian_SwapBE16(0x0478)))
                ((Yoruba_Nigeria, SkTEndian_SwapBE16(0x046A)))
                SK_SEQ_END,
            (value)SK_SEQ_END)
        } windows;
        //languageTagID - 0x8000 is an index into the langTagRecord array.
        SK_OT_USHORT languageTagID;
    } languageID;
    union NameID {
       struct Predefined {
            SK_TYPED_ENUM(Value, SK_OT_USHORT,
                ((CopyrightNotice, SkTEndian_SwapBE16(0)))
                ((FontFamilyName, SkTEndian_SwapBE16(1)))
                ((FontSubfamilyName, SkTEndian_SwapBE16(2)))
                ((UniqueFontIdentifier, SkTEndian_SwapBE16(3)))
                ((FullFontName, SkTEndian_SwapBE16(4)))
                ((VersionString, SkTEndian_SwapBE16(5))) //Version <number>.<number>
                ((PostscriptName, SkTEndian_SwapBE16(6))) //See spec for constraints.
                ((Trademark, SkTEndian_SwapBE16(7)))
                ((ManufacturerName, SkTEndian_SwapBE16(8)))
                ((Designer, SkTEndian_SwapBE16(9)))
                ((Description, SkTEndian_SwapBE16(10)))
                ((URLVendor, SkTEndian_SwapBE16(11)))
                ((URLDesigner, SkTEndian_SwapBE16(12)))
                ((LicenseDescription, SkTEndian_SwapBE16(13)))
                ((LicenseInfoURL, SkTEndian_SwapBE16(14)))
                ((PreferredFamily, SkTEndian_SwapBE16(16)))
                ((PreferredSubfamily, SkTEndian_SwapBE16(17)))
                ((CompatibleFullName, SkTEndian_SwapBE16(18)))
                ((SampleText, SkTEndian_SwapBE16(19)))
                ((PostscriptCIDFindfontName, SkTEndian_SwapBE16(20)))
                ((WWSFamilyName, SkTEndian_SwapBE16(21)))
                ((WWSSubfamilyName, SkTEndian_SwapBE16(22)))
                SK_SEQ_END,
            (value)SK_SEQ_END)
        } predefined;
        //values > 256 are font specific strings.
        SK_OT_USHORT fontSpecific;
    } nameID;
    SK_OT_USHORT length;
    SK_OT_USHORT offset; //From start of storage area.
};

#pragma pack(pop)


SK_COMPILE_ASSERT(sizeof(SkOTTableName) == 6, sizeof_SkOTTableName_not_6);
SK_COMPILE_ASSERT(sizeof(SkOTTableNameF1) == 2, sizeof_SkOTTableNameF1_not_2);
SK_COMPILE_ASSERT(sizeof(SkOTTableNameLangTagRecord) == 4, sizeof_SkOTTableNameLangTagRecord_not_4);
SK_COMPILE_ASSERT(sizeof(SkOTTableNameRecord) == 12, sizeof_SkOTTableNameRecord_not_12);

#endif
