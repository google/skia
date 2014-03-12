/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_OS_2_VA_DEFINED
#define SkOTTable_OS_2_VA_DEFINED

#include "SkEndian.h"
#include "SkIBMFamilyClass.h"
#include "SkOTTableTypes.h"
#include "SkPanose.h"
#include "SkTypedEnum.h"

#pragma pack(push, 1)

//Original V0 TT
struct SkOTTableOS2_VA {
    SK_OT_USHORT version;
    //SkOTTableOS2_VA::VERSION and SkOTTableOS2_V0::VERSION are both 0.
    //The only way to differentiate these two versions is by the size of the table.
    static const SK_OT_USHORT VERSION = SkTEndian_SwapBE16(0);

    SK_OT_SHORT xAvgCharWidth;
    struct WeightClass {
        SK_TYPED_ENUM(Value, SK_OT_USHORT,
            ((UltraLight, SkTEndian_SwapBE16(1)))
            ((ExtraLight, SkTEndian_SwapBE16(2)))
            ((Light, SkTEndian_SwapBE16(3)))
            ((SemiLight, SkTEndian_SwapBE16(4)))
            ((Medium, SkTEndian_SwapBE16(5)))
            ((SemiBold, SkTEndian_SwapBE16(6)))
            ((Bold, SkTEndian_SwapBE16(7)))
            ((ExtraBold, SkTEndian_SwapBE16(8)))
            ((UltraBold, SkTEndian_SwapBE16(9)))
            SK_SEQ_END,
        (value)SK_SEQ_END)
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
                Reserved06,
                Reserved07)
        } field;
        struct Raw {
            static const SK_OT_USHORT ItalicMask = SkOTSetUSHORTBit<0>::value;
            static const SK_OT_USHORT UnderscoreMask = SkOTSetUSHORTBit<1>::value;
            static const SK_OT_USHORT NegativeMask = SkOTSetUSHORTBit<2>::value;
            static const SK_OT_USHORT OutlinedMask = SkOTSetUSHORTBit<3>::value;
            static const SK_OT_USHORT StrikeoutMask = SkOTSetUSHORTBit<4>::value;
            static const SK_OT_USHORT BoldMask = SkOTSetUSHORTBit<5>::value;
            SK_OT_USHORT value;
        } raw;
    } fsSelection;
    SK_OT_USHORT usFirstCharIndex;
    SK_OT_USHORT usLastCharIndex;
};

#pragma pack(pop)


SK_COMPILE_ASSERT(sizeof(SkOTTableOS2_VA) == 68, sizeof_SkOTTableOS2_VA_not_68);

#endif
