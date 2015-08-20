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
            static const SK_OT_USHORT Installable = 0;
            static const SK_OT_USHORT RestrictedMask = SkOTSetUSHORTBit<1>::value;
            static const SK_OT_USHORT PreviewPrintMask = SkOTSetUSHORTBit<2>::value;
            static const SK_OT_USHORT EditableMask = SkOTSetUSHORTBit<3>::value;
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
                static const SK_OT_ULONG BasicLatinMask = SkOTSetULONGBit<0>::value;
                static const SK_OT_ULONG Latin1SupplementMask = SkOTSetULONGBit<1>::value;
                static const SK_OT_ULONG LatinExtendedAMask = SkOTSetULONGBit<2>::value;
                static const SK_OT_ULONG LatinExtendedBMask = SkOTSetULONGBit<3>::value;
                static const SK_OT_ULONG IPAExtensionsMask = SkOTSetULONGBit<4>::value;
                static const SK_OT_ULONG SpacingModifierLettersMask = SkOTSetULONGBit<5>::value;
                static const SK_OT_ULONG CombiningDiacriticalMarksMask = SkOTSetULONGBit<6>::value;
                static const SK_OT_ULONG BasicGreekMask = SkOTSetULONGBit<7>::value;
                static const SK_OT_ULONG GreekSymbolsAndCCopticMask = SkOTSetULONGBit<8>::value;
                static const SK_OT_ULONG CyrillicMask = SkOTSetULONGBit<9>::value;
                static const SK_OT_ULONG ArmenianMask = SkOTSetULONGBit<10>::value;
                static const SK_OT_ULONG BasicHebrewMask = SkOTSetULONGBit<11>::value;
                static const SK_OT_ULONG HebrewExtendedABMask = SkOTSetULONGBit<12>::value;
                static const SK_OT_ULONG BasicArabicMask = SkOTSetULONGBit<13>::value;
                static const SK_OT_ULONG ArabicExtendedMask = SkOTSetULONGBit<14>::value;
                static const SK_OT_ULONG DevanagariMask = SkOTSetULONGBit<15>::value;
                static const SK_OT_ULONG BengaliMask = SkOTSetULONGBit<16>::value;
                static const SK_OT_ULONG GurmukhiMask = SkOTSetULONGBit<17>::value;
                static const SK_OT_ULONG GujaratiMask = SkOTSetULONGBit<18>::value;
                static const SK_OT_ULONG OriyaMask = SkOTSetULONGBit<19>::value;
                static const SK_OT_ULONG TamilMask = SkOTSetULONGBit<20>::value;
                static const SK_OT_ULONG TeluguMask = SkOTSetULONGBit<21>::value;
                static const SK_OT_ULONG KannadaMask = SkOTSetULONGBit<22>::value;
                static const SK_OT_ULONG MalayalamMask = SkOTSetULONGBit<23>::value;
                static const SK_OT_ULONG ThaiMask = SkOTSetULONGBit<24>::value;
                static const SK_OT_ULONG LaoMask = SkOTSetULONGBit<25>::value;
                static const SK_OT_ULONG BasicGeorgianMask = SkOTSetULONGBit<26>::value;
                static const SK_OT_ULONG GeorgianExtendedMask = SkOTSetULONGBit<27>::value;
                static const SK_OT_ULONG HangulJamoMask = SkOTSetULONGBit<28>::value;
                static const SK_OT_ULONG LatinExtendedAdditionalMask = SkOTSetULONGBit<29>::value;
                static const SK_OT_ULONG GreekExtendedMask = SkOTSetULONGBit<30>::value;
                static const SK_OT_ULONG GeneralPunctuationMask = SkOTSetULONGBit<31>::value;
            };
            struct l1 {
                static const SK_OT_ULONG SuperscriptsAndSubscriptsMask = SkOTSetULONGBit<32 - 32>::value;
                static const SK_OT_ULONG CurrencySymbolsMask = SkOTSetULONGBit<33 - 32>::value;
                static const SK_OT_ULONG CombiningDiacriticalMarksForSymbolsMask = SkOTSetULONGBit<34 - 32>::value;
                static const SK_OT_ULONG LetterlikeSymbolsMask = SkOTSetULONGBit<35 - 32>::value;
                static const SK_OT_ULONG NumberFormsMask = SkOTSetULONGBit<36 - 32>::value;
                static const SK_OT_ULONG ArrowsMask = SkOTSetULONGBit<37 - 32>::value;
                static const SK_OT_ULONG MathematicalOperatorsMask = SkOTSetULONGBit<38 - 32>::value;
                static const SK_OT_ULONG MiscellaneousTechnicalMask = SkOTSetULONGBit<39 - 32>::value;
                static const SK_OT_ULONG ControlPicturesMask = SkOTSetULONGBit<40 - 32>::value;
                static const SK_OT_ULONG OpticalCharacterRecognitionMask = SkOTSetULONGBit<41 - 32>::value;
                static const SK_OT_ULONG EnclosedAlphanumericsMask = SkOTSetULONGBit<42 - 32>::value;
                static const SK_OT_ULONG BoxDrawingMask = SkOTSetULONGBit<43 - 32>::value;
                static const SK_OT_ULONG BlockElementsMask = SkOTSetULONGBit<44 - 32>::value;
                static const SK_OT_ULONG GeometricShapesMask = SkOTSetULONGBit<45 - 32>::value;
                static const SK_OT_ULONG MiscellaneousSymbolsMask = SkOTSetULONGBit<46 - 32>::value;
                static const SK_OT_ULONG DingbatsMask = SkOTSetULONGBit<47 - 32>::value;
                static const SK_OT_ULONG CJKSymbolsAndPunctuationMask = SkOTSetULONGBit<48 - 32>::value;
                static const SK_OT_ULONG HiraganaMask = SkOTSetULONGBit<49 - 32>::value;
                static const SK_OT_ULONG KatakanaMask = SkOTSetULONGBit<50 - 32>::value;
                static const SK_OT_ULONG BopomofoMask = SkOTSetULONGBit<51 - 32>::value;
                static const SK_OT_ULONG HangulCompatibilityJamoMask = SkOTSetULONGBit<52 - 32>::value;
                static const SK_OT_ULONG CJKMiscellaneousMask = SkOTSetULONGBit<53 - 32>::value;
                static const SK_OT_ULONG EnclosedCJKLettersAndMonthsMask = SkOTSetULONGBit<54 - 32>::value;
                static const SK_OT_ULONG CJKCompatibilityMask = SkOTSetULONGBit<55 - 32>::value;
                static const SK_OT_ULONG HangulMask = SkOTSetULONGBit<56 - 32>::value;
                //Reserved
                //Reserved
                static const SK_OT_ULONG CJKUnifiedIdeographsMask = SkOTSetULONGBit<59 - 32>::value;
                static const SK_OT_ULONG PrivateUseAreaMask = SkOTSetULONGBit<60 - 32>::value;
                static const SK_OT_ULONG CJKCompatibilityIdeographsMask = SkOTSetULONGBit<61 - 32>::value;
                static const SK_OT_ULONG AlphabeticPresentationFormsMask = SkOTSetULONGBit<62 - 32>::value;
                static const SK_OT_ULONG ArabicPresentationFormsAMask = SkOTSetULONGBit<63 - 32>::value;
            };
            struct l2 {
                static const SK_OT_ULONG CombiningHalfMarksMask = SkOTSetULONGBit<64 - 64>::value;
                static const SK_OT_ULONG CJKCompatibilityFormsMask = SkOTSetULONGBit<65 - 64>::value;
                static const SK_OT_ULONG SmallFormVariantsMask = SkOTSetULONGBit<66 - 64>::value;
                static const SK_OT_ULONG ArabicPresentationFormsBMask = SkOTSetULONGBit<67 - 64>::value;
                static const SK_OT_ULONG HalfwidthAndFullwidthFormsMask = SkOTSetULONGBit<68 - 64>::value;
                static const SK_OT_ULONG SpecialsMask = SkOTSetULONGBit<69 - 64>::value;
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
            static const SK_OT_USHORT ItalicMask = SkOTSetUSHORTBit<0>::value;
            static const SK_OT_USHORT UnderscoreMask = SkOTSetUSHORTBit<1>::value;
            static const SK_OT_USHORT NegativeMask = SkOTSetUSHORTBit<2>::value;
            static const SK_OT_USHORT OutlinedMask = SkOTSetUSHORTBit<3>::value;
            static const SK_OT_USHORT StrikeoutMask = SkOTSetUSHORTBit<4>::value;
            static const SK_OT_USHORT BoldMask = SkOTSetUSHORTBit<5>::value;
            static const SK_OT_USHORT RegularMask = SkOTSetUSHORTBit<6>::value;
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
                static const SK_OT_ULONG Latin1_1252Mask = SkOTSetULONGBit<0>::value;
                static const SK_OT_ULONG Latin2EasternEurope_1250Mask = SkOTSetULONGBit<1>::value;
                static const SK_OT_ULONG Cyrillic_1251Mask = SkOTSetULONGBit<2>::value;
                static const SK_OT_ULONG Greek_1253Mask = SkOTSetULONGBit<3>::value;
                static const SK_OT_ULONG Turkish_1254Mask = SkOTSetULONGBit<4>::value;
                static const SK_OT_ULONG Hebrew_1255Mask = SkOTSetULONGBit<5>::value;
                static const SK_OT_ULONG Arabic_1256Mask = SkOTSetULONGBit<6>::value;
                static const SK_OT_ULONG WindowsBaltic_1257Mask = SkOTSetULONGBit<7>::value;
                static const SK_OT_ULONG Thai_874Mask = SkOTSetULONGBit<16>::value;
                static const SK_OT_ULONG JISJapan_932Mask = SkOTSetULONGBit<17>::value;
                static const SK_OT_ULONG ChineseSimplified_936Mask = SkOTSetULONGBit<18>::value;
                static const SK_OT_ULONG KoreanWansung_949Mask = SkOTSetULONGBit<19>::value;
                static const SK_OT_ULONG ChineseTraditional_950Mask = SkOTSetULONGBit<20>::value;
                static const SK_OT_ULONG KoreanJohab_1361Mask = SkOTSetULONGBit<21>::value;
                static const SK_OT_ULONG MacintoshCharacterSetMask = SkOTSetULONGBit<29>::value;
                static const SK_OT_ULONG OEMCharacterSetMask = SkOTSetULONGBit<30>::value;
                static const SK_OT_ULONG SymbolCharacterSetMask = SkOTSetULONGBit<31>::value;
            };
            struct l1 {
                static const SK_OT_ULONG IBMGreek_869Mask = SkOTSetULONGBit<48 - 32>::value;
                static const SK_OT_ULONG MSDOSRussian_866Mask = SkOTSetULONGBit<49 - 32>::value;
                static const SK_OT_ULONG MSDOSNordic_865Mask = SkOTSetULONGBit<50 - 32>::value;
                static const SK_OT_ULONG Arabic_864Mask = SkOTSetULONGBit<51 - 32>::value;
                static const SK_OT_ULONG MSDOSCanadianFrench_863Mask = SkOTSetULONGBit<52 - 32>::value;
                static const SK_OT_ULONG Hebrew_862Mask = SkOTSetULONGBit<53 - 32>::value;
                static const SK_OT_ULONG MSDOSIcelandic_861Mask = SkOTSetULONGBit<54 - 32>::value;
                static const SK_OT_ULONG MSDOSPortuguese_860Mask = SkOTSetULONGBit<55 - 32>::value;
                static const SK_OT_ULONG IBMTurkish_857Mask = SkOTSetULONGBit<56 - 32>::value;
                static const SK_OT_ULONG IBMCyrillic_855Mask = SkOTSetULONGBit<57 - 32>::value;
                static const SK_OT_ULONG Latin2_852Mask = SkOTSetULONGBit<58 - 32>::value;
                static const SK_OT_ULONG MSDOSBaltic_775Mask = SkOTSetULONGBit<59 - 32>::value;
                static const SK_OT_ULONG Greek_737Mask = SkOTSetULONGBit<60 - 32>::value;
                static const SK_OT_ULONG Arabic_708Mask = SkOTSetULONGBit<61 - 32>::value;
                static const SK_OT_ULONG WELatin1_850Mask = SkOTSetULONGBit<62 - 32>::value;
                static const SK_OT_ULONG US_437Mask = SkOTSetULONGBit<63 - 32>::value;
            };
            SK_OT_ULONG value[2];
        } raw;
    } ulCodePageRange;
};

#pragma pack(pop)


static_assert(sizeof(SkOTTableOS2_V1) == 86, "sizeof_SkOTTableOS2_V1_not_86");

#endif
