/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_OS_2_V1_DEFINED
#define SkOTTable_OS_2_V1_DEFINED

#include "SkEndian.h"
#include "SkIBMFamilyClass.h"
#include "SkOTTableTypes.h"
#include "SkPanose.h"
#include "SkTypedEnum.h"

#pragma pack(push, 1)

struct SkOTTableOS2_V1 {
    SK_OT_USHORT version;
    static const SK_OT_USHORT VERSION = SkTEndian_SwapBE16(1);

    SK_OT_SHORT xAvgCharWidth;
    struct WeightClass {
        SK_TYPED_ENUM(Value, SK_OT_USHORT,
            ((Thin, SkTEndian_SwapBE16(100)))
            ((ExtraLight, SkTEndian_SwapBE16(200)))
            ((Light, SkTEndian_SwapBE16(300)))
            ((Normal, SkTEndian_SwapBE16(400)))
            ((Medium, SkTEndian_SwapBE16(500)))
            ((SemiBold, SkTEndian_SwapBE16(600)))
            ((Bold, SkTEndian_SwapBE16(700)))
            ((ExtraBold, SkTEndian_SwapBE16(800)))
            ((Black, SkTEndian_SwapBE16(900)))
            SK_SEQ_END,
        SK_SEQ_END)
        SK_OT_USHORT value;
    } usWeightClass;
    struct WidthClass {
        SK_TYPED_ENUM(Value, SK_OT_USHORT,
            ((UltraCondensed, SkTEndian_SwapBE16(1)))
            ((ExtraCondensed, SkTEndian_SwapBE16(2)))
            ((Condensed, SkTEndian_SwapBE16(3)))
            ((SemiCondensed, SkTEndian_SwapBE16(4)))
            ((Medium, SkTEndian_SwapBE16(5)))
            ((SemiExpanded, SkTEndian_SwapBE16(6)))
            ((Expanded, SkTEndian_SwapBE16(7)))
            ((ExtraExpanded, SkTEndian_SwapBE16(8)))
            ((UltraExpanded, SkTEndian_SwapBE16(9)))
            SK_SEQ_END,
        (value)SK_SEQ_END)
    } usWidthClass;
    union Type {
        struct Field {
            //8-15
            SK_OT_BYTE_BITFIELD(
                Reserved08,
                Reserved09,
                Reserved10,
                Reserved11,
                Reserved12,
                Reserved13,
                Reserved14,
                Reserved15)
            //0-7
            SK_OT_BYTE_BITFIELD(
                Reserved00,
                Restricted,
                PreviewPrint,
                Editable,
                Reserved04,
                Reserved05,
                Reserved06,
                Reserved07)
        } field;
        struct Raw {
            static const SK_OT_USHORT Installable = SkTEndian_SwapBE16(0);
            static const SK_OT_USHORT RestrictedMask = SkTEndian_SwapBE16(1 << 1);
            static const SK_OT_USHORT PreviewPrintMask = SkTEndian_SwapBE16(1 << 2);
            static const SK_OT_USHORT EditableMask = SkTEndian_SwapBE16(1 << 3);
            SK_OT_USHORT value;
        } raw;
    } fsType;
    SK_OT_SHORT ySubscriptXSize;
    SK_OT_SHORT ySubscriptYSize;
    SK_OT_SHORT ySubscriptXOffset;
    SK_OT_SHORT ySubscriptYOffset;
    SK_OT_SHORT ySuperscriptXSize;
    SK_OT_SHORT ySuperscriptYSize;
    SK_OT_SHORT ySuperscriptXOffset;
    SK_OT_SHORT ySuperscriptYOffset;
    SK_OT_SHORT yStrikeoutSize;
    SK_OT_SHORT yStrikeoutPosition;
    SkIBMFamilyClass sFamilyClass;
    SkPanose panose;
    union UnicodeRange {
        struct Field {
            //l0 24-31
            SK_OT_BYTE_BITFIELD(
                Thai,
                Lao,
                BasicGeorgian,
                GeorgianExtended,
                HangulJamo,
                LatinExtendedAdditional,
                GreekExtended,
                GeneralPunctuation)
            //l0 16-23
            SK_OT_BYTE_BITFIELD(
                Bengali,
                Gurmukhi,
                Gujarati,
                Oriya,
                Tamil,
                Telugu,
                Kannada,
                Malayalam)
            //l0 8-15
            SK_OT_BYTE_BITFIELD(
                GreekSymbolsAndCoptic,
                Cyrillic,
                Armenian,
                BasicHebrew,
                HebrewExtendedAB,
                BasicArabic,
                ArabicExtended,
                Devanagari)
            //l0 0-7
            SK_OT_BYTE_BITFIELD(
                BasicLatin,
                Latin1Supplement,
                LatinExtendedA,
                LatinExtendedB,
                IPAExtensions,
                SpacingModifierLetters,
                CombiningDiacriticalMarks,
                BasicGreek)

            //l1 24-31
            SK_OT_BYTE_BITFIELD(
                Hangul,
                Reserved057,
                Reserved058,
                CJKUnifiedIdeographs,
                PrivateUseArea,
                CJKCompatibilityIdeographs,
                AlphabeticPresentationForms,
                ArabicPresentationFormsA)
            //l1 16-23
            SK_OT_BYTE_BITFIELD(
                CJKSymbolsAndPunctuation,
                Hiragana,
                Katakana,
                Bopomofo,
                HangulCompatibilityJamo,
                CJKMiscellaneous,
                EnclosedCJKLettersAndMonths,
                CJKCompatibility)
            //l1 8-15
            SK_OT_BYTE_BITFIELD(
                ControlPictures,
                OpticalCharacterRecognition,
                EnclosedAlphanumerics,
                BoxDrawing,
                BlockElements,
                GeometricShapes,
                MiscellaneousSymbols,
                Dingbats)
            //l1 0-7
            SK_OT_BYTE_BITFIELD(
                SuperscriptsAndSubscripts,
                CurrencySymbols,
                CombiningDiacriticalMarksForSymbols,
                LetterlikeSymbols,
                NumberForms,
                Arrows,
                MathematicalOperators,
                MiscellaneousTechnical)

            //l2 24-31
            SK_OT_BYTE_BITFIELD(
                Reserved088,
                Reserved089,
                Reserved090,
                Reserved091,
                Reserved092,
                Reserved093,
                Reserved094,
                Reserved095)
            //l2 16-23
            SK_OT_BYTE_BITFIELD(
                Reserved080,
                Reserved081,
                Reserved082,
                Reserved083,
                Reserved084,
                Reserved085,
                Reserved086,
                Reserved087)
            //l2 8-15
            SK_OT_BYTE_BITFIELD(
                Reserved072,
                Reserved073,
                Reserved074,
                Reserved075,
                Reserved076,
                Reserved077,
                Reserved078,
                Reserved079)
            //l2 0-7
            SK_OT_BYTE_BITFIELD(
                CombiningHalfMarks,
                CJKCompatibilityForms,
                SmallFormVariants,
                ArabicPresentationFormsB,
                HalfwidthAndFullwidthForms,
                Specials,
                Reserved70,
                Reserved71)

            //l3 24-31
            SK_OT_BYTE_BITFIELD(
                Reserved120,
                Reserved121,
                Reserved122,
                Reserved123,
                Reserved124,
                Reserved125,
                Reserved126,
                Reserved127)
            //l3 16-23
            SK_OT_BYTE_BITFIELD(
                Reserved112,
                Reserved113,
                Reserved114,
                Reserved115,
                Reserved116,
                Reserved117,
                Reserved118,
                Reserved119)
            //l3 8-15
            SK_OT_BYTE_BITFIELD(
                Reserved104,
                Reserved105,
                Reserved106,
                Reserved107,
                Reserved108,
                Reserved109,
                Reserved110,
                Reserved111)
            //l3 0-7
            SK_OT_BYTE_BITFIELD(
                Reserved096,
                Reserved097,
                Reserved098,
                Reserved099,
                Reserved100,
                Reserved101,
                Reserved102,
                Reserved103)
        } field;
        struct Raw {
            struct l0 {
                static const SK_OT_ULONG BasicLatinMask = SkTEndian_SwapBE32(1 << 0);
                static const SK_OT_ULONG Latin1SupplementMask = SkTEndian_SwapBE32(1 << 1);
                static const SK_OT_ULONG LatinExtendedAMask = SkTEndian_SwapBE32(1 << 2);
                static const SK_OT_ULONG LatinExtendedBMask = SkTEndian_SwapBE32(1 << 3);
                static const SK_OT_ULONG IPAExtensionsMask = SkTEndian_SwapBE32(1 << 4);
                static const SK_OT_ULONG SpacingModifierLettersMask = SkTEndian_SwapBE32(1 << 5);
                static const SK_OT_ULONG CombiningDiacriticalMarksMask = SkTEndian_SwapBE32(1 << 6);
                static const SK_OT_ULONG BasicGreekMask = SkTEndian_SwapBE32(1 << 7);
                static const SK_OT_ULONG GreekSymbolsAndCCopticMask = SkTEndian_SwapBE32(1 << 8);
                static const SK_OT_ULONG CyrillicMask = SkTEndian_SwapBE32(1 << 9);
                static const SK_OT_ULONG ArmenianMask = SkTEndian_SwapBE32(1 << 10);
                static const SK_OT_ULONG BasicHebrewMask = SkTEndian_SwapBE32(1 << 11);
                static const SK_OT_ULONG HebrewExtendedABMask = SkTEndian_SwapBE32(1 << 12);
                static const SK_OT_ULONG BasicArabicMask = SkTEndian_SwapBE32(1 << 13);
                static const SK_OT_ULONG ArabicExtendedMask = SkTEndian_SwapBE32(1 << 14);
                static const SK_OT_ULONG DevanagariMask = SkTEndian_SwapBE32(1 << 15);
                static const SK_OT_ULONG BengaliMask = SkTEndian_SwapBE32(1 << 16);
                static const SK_OT_ULONG GurmukhiMask = SkTEndian_SwapBE32(1 << 17);
                static const SK_OT_ULONG GujaratiMask = SkTEndian_SwapBE32(1 << 18);
                static const SK_OT_ULONG OriyaMask = SkTEndian_SwapBE32(1 << 19);
                static const SK_OT_ULONG TamilMask = SkTEndian_SwapBE32(1 << 20);
                static const SK_OT_ULONG TeluguMask = SkTEndian_SwapBE32(1 << 21);
                static const SK_OT_ULONG KannadaMask = SkTEndian_SwapBE32(1 << 22);
                static const SK_OT_ULONG MalayalamMask = SkTEndian_SwapBE32(1 << 23);
                static const SK_OT_ULONG ThaiMask = SkTEndian_SwapBE32(1 << 24);
                static const SK_OT_ULONG LaoMask = SkTEndian_SwapBE32(1 << 25);
                static const SK_OT_ULONG BasicGeorgianMask = SkTEndian_SwapBE32(1 << 26);
                static const SK_OT_ULONG GeorgianExtendedMask = SkTEndian_SwapBE32(1 << 27);
                static const SK_OT_ULONG HangulJamoMask = SkTEndian_SwapBE32(1 << 28);
                static const SK_OT_ULONG LatinExtendedAdditionalMask = SkTEndian_SwapBE32(1 << 29);
                static const SK_OT_ULONG GreekExtendedMask = SkTEndian_SwapBE32(1 << 30);
                static const SK_OT_ULONG GeneralPunctuationMask = SkTEndian_SwapBE32(1 << 31);
            };
            struct l1 {
                static const SK_OT_ULONG SuperscriptsAndSubscriptsMask = SkTEndian_SwapBE32(1 << (32 - 32));
                static const SK_OT_ULONG CurrencySymbolsMask = SkTEndian_SwapBE32(1 << (33 - 32));
                static const SK_OT_ULONG CombiningDiacriticalMarksForSymbolsMask = SkTEndian_SwapBE32(1 << (34 - 32));
                static const SK_OT_ULONG LetterlikeSymbolsMask = SkTEndian_SwapBE32(1 << (35 - 32));
                static const SK_OT_ULONG NumberFormsMask = SkTEndian_SwapBE32(1 << (36 - 32));
                static const SK_OT_ULONG ArrowsMask = SkTEndian_SwapBE32(1 << (37 - 32));
                static const SK_OT_ULONG MathematicalOperatorsMask = SkTEndian_SwapBE32(1 << (38 - 32));
                static const SK_OT_ULONG MiscellaneousTechnicalMask = SkTEndian_SwapBE32(1 << (39 - 32));
                static const SK_OT_ULONG ControlPicturesMask = SkTEndian_SwapBE32(1 << (40 - 32));
                static const SK_OT_ULONG OpticalCharacterRecognitionMask = SkTEndian_SwapBE32(1 << (41 - 32));
                static const SK_OT_ULONG EnclosedAlphanumericsMask = SkTEndian_SwapBE32(1 << (42 - 32));
                static const SK_OT_ULONG BoxDrawingMask = SkTEndian_SwapBE32(1 << (43 - 32));
                static const SK_OT_ULONG BlockElementsMask = SkTEndian_SwapBE32(1 << (44 - 32));
                static const SK_OT_ULONG GeometricShapesMask = SkTEndian_SwapBE32(1 << (45 - 32));
                static const SK_OT_ULONG MiscellaneousSymbolsMask = SkTEndian_SwapBE32(1 << (46 - 32));
                static const SK_OT_ULONG DingbatsMask = SkTEndian_SwapBE32(1 << (47 - 32));
                static const SK_OT_ULONG CJKSymbolsAndPunctuationMask = SkTEndian_SwapBE32(1 << (48 - 32));
                static const SK_OT_ULONG HiraganaMask = SkTEndian_SwapBE32(1 << (49 - 32));
                static const SK_OT_ULONG KatakanaMask = SkTEndian_SwapBE32(1 << (50 - 32));
                static const SK_OT_ULONG BopomofoMask = SkTEndian_SwapBE32(1 << (51 - 32));
                static const SK_OT_ULONG HangulCompatibilityJamoMask = SkTEndian_SwapBE32(1 << (52 - 32));
                static const SK_OT_ULONG CJKMiscellaneousMask = SkTEndian_SwapBE32(1 << (53 - 32));
                static const SK_OT_ULONG EnclosedCJKLettersAndMonthsMask = SkTEndian_SwapBE32(1 << (54 - 32));
                static const SK_OT_ULONG CJKCompatibilityMask = SkTEndian_SwapBE32(1 << (55 - 32));
                static const SK_OT_ULONG HangulMask = SkTEndian_SwapBE32(1 << (56 - 32));
                //Reserved
                //Reserved
                static const SK_OT_ULONG CJKUnifiedIdeographsMask = SkTEndian_SwapBE32(1 << (59 - 32));
                static const SK_OT_ULONG PrivateUseAreaMask = SkTEndian_SwapBE32(1 << (60 - 32));
                static const SK_OT_ULONG CJKCompatibilityIdeographsMask = SkTEndian_SwapBE32(1 << (61 - 32));
                static const SK_OT_ULONG AlphabeticPresentationFormsMask = SkTEndian_SwapBE32(1 << (62 - 32));
                static const SK_OT_ULONG ArabicPresentationFormsAMask = SkTEndian_SwapBE32(1 << (63 - 32));
            };
            struct l2 {
                static const SK_OT_ULONG CombiningHalfMarksMask = SkTEndian_SwapBE32(1 << (64 - 64));
                static const SK_OT_ULONG CJKCompatibilityFormsMask = SkTEndian_SwapBE32(1 << (65 - 64));
                static const SK_OT_ULONG SmallFormVariantsMask = SkTEndian_SwapBE32(1 << (66 - 64));
                static const SK_OT_ULONG ArabicPresentationFormsBMask = SkTEndian_SwapBE32(1 << (67 - 64));
                static const SK_OT_ULONG HalfwidthAndFullwidthFormsMask = SkTEndian_SwapBE32(1 << (68 - 64));
                static const SK_OT_ULONG SpecialsMask = SkTEndian_SwapBE32(1 << (69 - 64));
            };
            SK_OT_ULONG value[4];
        } raw;
    } ulUnicodeRange;
    SK_OT_CHAR achVendID[4];
    union Selection {
        struct Field {
            //8-15
            SK_OT_BYTE_BITFIELD(
                Reserved08,
                Reserved09,
                Reserved10,
                Reserved11,
                Reserved12,
                Reserved13,
                Reserved14,
                Reserved15)
            //0-7
            SK_OT_BYTE_BITFIELD(
                Italic,
                Underscore,
                Negative,
                Outlined,
                Strikeout,
                Bold,
                Regular,
                Reserved07)
        } field;
        struct Raw {
            static const SK_OT_USHORT ItalicMask = SkTEndian_SwapBE16(1 << 0);
            static const SK_OT_USHORT UnderscoreMask = SkTEndian_SwapBE16(1 << 1);
            static const SK_OT_USHORT NegativeMask = SkTEndian_SwapBE16(1 << 2);
            static const SK_OT_USHORT OutlinedMask = SkTEndian_SwapBE16(1 << 3);
            static const SK_OT_USHORT StrikeoutMask = SkTEndian_SwapBE16(1 << 4);
            static const SK_OT_USHORT BoldMask = SkTEndian_SwapBE16(1 << 5);
            static const SK_OT_USHORT RegularMask = SkTEndian_SwapBE16(1 << 6);
            SK_OT_USHORT value;
        } raw;
    } fsSelection;
    SK_OT_USHORT usFirstCharIndex;
    SK_OT_USHORT usLastCharIndex;
    //version0
    SK_OT_SHORT sTypoAscender;
    SK_OT_SHORT sTypoDescender;
    SK_OT_SHORT sTypoLineGap;
    SK_OT_USHORT usWinAscent;
    SK_OT_USHORT usWinDescent;
    //version1
    union CodePageRange {
        struct Field {
            //l0 24-31
            SK_OT_BYTE_BITFIELD(
                Reserved24,
                Reserved25,
                Reserved26,
                Reserved27,
                Reserved28,
                MacintoshCharacterSet,
                OEMCharacterSet,
                SymbolCharacterSet)
            //l0 16-23
            SK_OT_BYTE_BITFIELD(
                Thai_874,
                JISJapan_932,
                ChineseSimplified_936,
                KoreanWansung_949,
                ChineseTraditional_950,
                KoreanJohab_1361,
                Reserved22,
                Reserved23)
            //l0 8-15
            SK_OT_BYTE_BITFIELD(
                Reserved08,
                Reserved09,
                Reserved10,
                Reserved11,
                Reserved12,
                Reserved13,
                Reserved14,
                Reserved15)
            //l0 0-7
            SK_OT_BYTE_BITFIELD(
                Latin1_1252,
                Latin2EasternEurope_1250,
                Cyrillic_1251,
                Greek_1253,
                Turkish_1254,
                Hebrew_1255,
                Arabic_1256,
                WindowsBaltic_1257)

            //l1 24-31
            SK_OT_BYTE_BITFIELD(
                IBMTurkish_857,
                IBMCyrillic_855,
                Latin2_852,
                MSDOSBaltic_775,
                Greek_737,
                Arabic_708,
                WELatin1_850,
                US_437)
            //l1 16-23
            SK_OT_BYTE_BITFIELD(
                IBMGreek_869,
                MSDOSRussian_866,
                MSDOSNordic_865,
                Arabic_864,
                MSDOSCanadianFrench_863,
                Hebrew_862,
                MSDOSIcelandic_861,
                MSDOSPortuguese_860)
            //l1 8-15
            SK_OT_BYTE_BITFIELD(
                Reserved40,
                Reserved41,
                Reserved42,
                Reserved43,
                Reserved44,
                Reserved45,
                Reserved46,
                Reserved47)
            //l1 0-7
            SK_OT_BYTE_BITFIELD(
                Reserved32,
                Reserved33,
                Reserved34,
                Reserved35,
                Reserved36,
                Reserved37,
                Reserved38,
                Reserved39)
        } field;
        struct Raw {
            struct l0 {
                static const SK_OT_ULONG Latin1_1252Mask = SkTEndian_SwapBE32(1 << 0);
                static const SK_OT_ULONG Latin2EasternEurope_1250Mask = SkTEndian_SwapBE32(1 << 1);
                static const SK_OT_ULONG Cyrillic_1251Mask = SkTEndian_SwapBE32(1 << 2);
                static const SK_OT_ULONG Greek_1253Mask = SkTEndian_SwapBE32(1 << 3);
                static const SK_OT_ULONG Turkish_1254Mask = SkTEndian_SwapBE32(1 << 4);
                static const SK_OT_ULONG Hebrew_1255Mask = SkTEndian_SwapBE32(1 << 5);
                static const SK_OT_ULONG Arabic_1256Mask = SkTEndian_SwapBE32(1 << 6);
                static const SK_OT_ULONG WindowsBaltic_1257Mask = SkTEndian_SwapBE32(1 << 7);
                static const SK_OT_ULONG Thai_874Mask = SkTEndian_SwapBE32(1 << 16);
                static const SK_OT_ULONG JISJapan_932Mask = SkTEndian_SwapBE32(1 << 17);
                static const SK_OT_ULONG ChineseSimplified_936Mask = SkTEndian_SwapBE32(1 << 18);
                static const SK_OT_ULONG KoreanWansung_949Mask = SkTEndian_SwapBE32(1 << 19);
                static const SK_OT_ULONG ChineseTraditional_950Mask = SkTEndian_SwapBE32(1 << 20);
                static const SK_OT_ULONG KoreanJohab_1361Mask = SkTEndian_SwapBE32(1 << 21);
                static const SK_OT_ULONG MacintoshCharacterSetMask = SkTEndian_SwapBE32(1 << 29);
                static const SK_OT_ULONG OEMCharacterSetMask = SkTEndian_SwapBE32(1 << 30);
                static const SK_OT_ULONG SymbolCharacterSetMask = SkTEndian_SwapBE32(1 << 31);
            };
            struct l1 {
                static const SK_OT_ULONG IBMGreek_869Mask = SkTEndian_SwapBE32(1 << (48 - 32));
                static const SK_OT_ULONG MSDOSRussian_866Mask = SkTEndian_SwapBE32(1 << (49 - 32));
                static const SK_OT_ULONG MSDOSNordic_865Mask = SkTEndian_SwapBE32(1 << (50 - 32));
                static const SK_OT_ULONG Arabic_864Mask = SkTEndian_SwapBE32(1 << (51 - 32));
                static const SK_OT_ULONG MSDOSCanadianFrench_863Mask = SkTEndian_SwapBE32(1 << (52 - 32));
                static const SK_OT_ULONG Hebrew_862Mask = SkTEndian_SwapBE32(1 << (53 - 32));
                static const SK_OT_ULONG MSDOSIcelandic_861Mask = SkTEndian_SwapBE32(1 << (54 - 32));
                static const SK_OT_ULONG MSDOSPortuguese_860Mask = SkTEndian_SwapBE32(1 << (55 - 32));
                static const SK_OT_ULONG IBMTurkish_857Mask = SkTEndian_SwapBE32(1 << (56 - 32));
                static const SK_OT_ULONG IBMCyrillic_855Mask = SkTEndian_SwapBE32(1 << (57 - 32));
                static const SK_OT_ULONG Latin2_852Mask = SkTEndian_SwapBE32(1 << (58 - 32));
                static const SK_OT_ULONG MSDOSBaltic_775Mask = SkTEndian_SwapBE32(1 << (59 - 32));
                static const SK_OT_ULONG Greek_737Mask = SkTEndian_SwapBE32(1 << (60 - 32));
                static const SK_OT_ULONG Arabic_708Mask = SkTEndian_SwapBE32(1 << (61 - 32));
                static const SK_OT_ULONG WELatin1_850Mask = SkTEndian_SwapBE32(1 << (62 - 32));
                static const SK_OT_ULONG US_437Mask = SkTEndian_SwapBE32(1 << (63 - 32));
            };
            SK_OT_ULONG value[2];
        } raw;
    } ulCodePageRange;
};

#pragma pack(pop)


SK_COMPILE_ASSERT(sizeof(SkOTTableOS2_V1) == 86, sizeof_SkOTTableOS2_V1_not_86);

#endif
