/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_head_DEFINED
#define SkOTTable_head_DEFINED

#include "SkEndian.h"
#include "SkOTTableTypes.h"
#include "SkTypedEnum.h"

#pragma pack(push, 1)

struct SkOTTableHead {
    static const SK_OT_CHAR TAG0 = 'h';
    static const SK_OT_CHAR TAG1 = 'e';
    static const SK_OT_CHAR TAG2 = 'a';
    static const SK_OT_CHAR TAG3 = 'd';
    static const SK_OT_ULONG TAG = SkOTTableTAG<SkOTTableHead>::value;

    SK_OT_Fixed version;
    static const SK_OT_Fixed version1 = SkTEndian_SwapBE32(0x00010000);
    SK_OT_Fixed fontRevision;
    static const uint32_t fontChecksum = 0xB1B0AFBA; //checksum of all TT fonts
    SK_OT_ULONG checksumAdjustment;
    SK_OT_ULONG magicNumber;
    static const SK_OT_ULONG magicNumberConst = SkTEndian_SwapBE32(0x5F0F3CF5);
    union Flags {
        struct Field {
            //8-15
            SK_OT_BYTE_BITFIELD(
                GXMetamorphosis_Apple,
                HasStrongRTL_Apple,
                HasIndicStyleRearrangement,
                AgfaMicroTypeExpressProcessed,
                FontConverted,
                DesignedForClearType,
                LastResort,
                Reserved15)
            //0-7
            SK_OT_BYTE_BITFIELD(
                BaselineAtY0,
                LeftSidebearingAtX0,
                InstructionsDependOnPointSize,
                IntegerScaling,
                InstructionsAlterAdvanceWidth,
                VerticalCenteredGlyphs_Apple,
                Reserved06,
                RequiresLayout_Apple)
        } field;
        struct Raw {
            static const SK_OT_USHORT BaselineAtY0Mask = SkTEndian_SwapBE16(1 << 0);
            static const SK_OT_USHORT LeftSidebearingAtX0Mask = SkTEndian_SwapBE16(1 << 1);
            static const SK_OT_USHORT InstructionsDependOnPointSizeMask = SkTEndian_SwapBE16(1 << 2);
            static const SK_OT_USHORT IntegerScalingMask = SkTEndian_SwapBE16(1 << 3);
            static const SK_OT_USHORT InstructionsAlterAdvanceWidthMask = SkTEndian_SwapBE16(1 << 4);
            static const SK_OT_USHORT VerticalCenteredGlyphs_AppleMask = SkTEndian_SwapBE16(1 << 5);
            //Reserved
            static const SK_OT_USHORT RequiresLayout_AppleMask = SkTEndian_SwapBE16(1 << 7);

            static const SK_OT_USHORT GXMetamorphosis_AppleMask = SkTEndian_SwapBE16(1 << 8);
            static const SK_OT_USHORT HasStrongRTL_AppleMask = SkTEndian_SwapBE16(1 << 9);
            static const SK_OT_USHORT HasIndicStyleRearrangementMask = SkTEndian_SwapBE16(1 << 10);
            static const SK_OT_USHORT AgfaMicroTypeExpressProcessedMask = SkTEndian_SwapBE16(1 << 11);
            static const SK_OT_USHORT FontConvertedMask = SkTEndian_SwapBE16(1 << 12);
            static const SK_OT_USHORT DesignedForClearTypeMask = SkTEndian_SwapBE16(1 << 13);
            static const SK_OT_USHORT LastResortMask = SkTEndian_SwapBE16(1 << 14);
            //Reserved
            SK_OT_USHORT value;
        } raw;
    } flags;
    SK_OT_USHORT unitsPerEm;
    SK_OT_LONGDATETIME created;
    SK_OT_LONGDATETIME modified;
    SK_OT_SHORT xMin;
    SK_OT_SHORT yMin;
    SK_OT_SHORT xMax;
    SK_OT_SHORT yMax;
    union MacStyle {
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
                Bold,
                Italic,
                Underline,
                Outline,
                Shadow,
                Condensed,
                Extended,
                Reserved07)
        } field;
        struct Raw {
            static const SK_OT_USHORT BoldMask = SkTEndian_SwapBE16(1);
            static const SK_OT_USHORT ItalicMask = SkTEndian_SwapBE16(1 << 1);
            static const SK_OT_USHORT UnderlineMask = SkTEndian_SwapBE16(1 << 2);
            static const SK_OT_USHORT OutlineMask = SkTEndian_SwapBE16(1 << 3);
            static const SK_OT_USHORT ShadowMask = SkTEndian_SwapBE16(1 << 4);
            static const SK_OT_USHORT CondensedMask = SkTEndian_SwapBE16(1 << 5);
            static const SK_OT_USHORT ExtendedMask = SkTEndian_SwapBE16(1 << 6);

            SK_OT_USHORT value;
        } raw;
    } macStyle;
    SK_OT_USHORT lowestRecPPEM;
    struct FontDirectionHint {
        SK_TYPED_ENUM(Value, SK_OT_SHORT,
            ((FullyMixedDirectionalGlyphs, SkTEndian_SwapBE16(0)))
            ((OnlyStronglyLTR, SkTEndian_SwapBE16(1)))
            ((StronglyLTR, SkTEndian_SwapBE16(2)))
            ((OnlyStronglyRTL, static_cast<SK_OT_SHORT>(SkTEndian_SwapBE16((uint16_t)-1))))
            ((StronglyRTL, static_cast<SK_OT_SHORT>(SkTEndian_SwapBE16((uint16_t)-2))))
            SK_SEQ_END,
        (value)SK_SEQ_END)
    } fontDirectionHint;
    struct IndexToLocFormat {
        SK_TYPED_ENUM(Value, SK_OT_SHORT,
            ((ShortOffsets, SkTEndian_SwapBE16(0)))
            ((LongOffsets, SkTEndian_SwapBE16(1)))
            SK_SEQ_END,
        (value)SK_SEQ_END)
    } indexToLocFormat;
    struct GlyphDataFormat {
        SK_TYPED_ENUM(Value, SK_OT_SHORT,
            ((CurrentFormat, SkTEndian_SwapBE16(0)))
            SK_SEQ_END,
        (value)SK_SEQ_END)
    } glyphDataFormat;
};

#pragma pack(pop)


#include <stddef.h>
static_assert(offsetof(SkOTTableHead, glyphDataFormat) == 52, "SkOTTableHead_glyphDataFormat_not_at_52");
static_assert(sizeof(SkOTTableHead) == 54, "sizeof_SkOTTableHead_not_54");

#endif
