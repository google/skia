/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_OS_2_V0_DEFINED
#define SkOTTable_OS_2_V0_DEFINED

#include "src/base/SkEndian.h"
#include "src/sfnt/SkIBMFamilyClass.h"
#include "src/sfnt/SkOTTableTypes.h"
#include "src/sfnt/SkPanose.h"

#pragma pack(push, 1)

struct SkOTTableOS2_V0 {
    SK_OT_USHORT version;
    //SkOTTableOS2_VA::VERSION and SkOTTableOS2_V0::VERSION are both 0.
    //The only way to differentiate these two versions is by the size of the table.
    static const SK_OT_USHORT VERSION = SkTEndian_SwapBE16(0);

    SK_OT_SHORT xAvgCharWidth;
    struct WeightClass {
        enum Value : SK_OT_USHORT {
            Thin = SkTEndian_SwapBE16(100),
            ExtraLight = SkTEndian_SwapBE16(200),
            Light = SkTEndian_SwapBE16(300),
            Normal = SkTEndian_SwapBE16(400),
            Medium = SkTEndian_SwapBE16(500),
            SemiBold = SkTEndian_SwapBE16(600),
            Bold = SkTEndian_SwapBE16(700),
            ExtraBold = SkTEndian_SwapBE16(800),
            Black = SkTEndian_SwapBE16(900),
        };
        SK_OT_USHORT value;
    } usWeightClass;
    struct WidthClass {
        enum Value : SK_OT_USHORT {
            UltraCondensed = SkTEndian_SwapBE16(1),
            ExtraCondensed = SkTEndian_SwapBE16(2),
            Condensed = SkTEndian_SwapBE16(3),
            SemiCondensed = SkTEndian_SwapBE16(4),
            Medium = SkTEndian_SwapBE16(5),
            SemiExpanded = SkTEndian_SwapBE16(6),
            Expanded = SkTEndian_SwapBE16(7),
            ExtraExpanded = SkTEndian_SwapBE16(8),
            UltraExpanded = SkTEndian_SwapBE16(9),
        } value;
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
    SK_OT_ULONG ulCharRange[4];
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
};

#pragma pack(pop)


static_assert(sizeof(SkOTTableOS2_V0) == 78, "sizeof_SkOTTableOS2_V0_not_78");

#endif
