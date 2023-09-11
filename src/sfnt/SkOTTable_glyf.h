/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOTTable_glyf_DEFINED
#define SkOTTable_glyf_DEFINED

#include "src/base/SkEndian.h"
#include "src/sfnt/SkOTTableTypes.h"
#include "src/sfnt/SkOTTable_head.h"
#include "src/sfnt/SkOTTable_loca.h"

#pragma pack(push, 1)

struct SkOTTableGlyphData;

struct SkOTTableGlyph {
    static const SK_OT_CHAR TAG0 = 'g';
    static const SK_OT_CHAR TAG1 = 'l';
    static const SK_OT_CHAR TAG2 = 'y';
    static const SK_OT_CHAR TAG3 = 'f';
    static const SK_OT_ULONG TAG = SkOTTableTAG<SkOTTableGlyph>::value;

    class Iterator {
    public:
        Iterator(SkOTTableGlyph& glyf,
                 const SkOTTableIndexToLocation& loca,
                 SkOTTableHead::IndexToLocFormat locaFormat)
        : fGlyf(glyf)
        , fLoca(loca)
        , fLocaFormat(locaFormat)
        , fCurrentGlyph(0)
        , fCurrentGlyphOffset(0)
        {
            SkASSERT(locaFormat.value == SkOTTableHead::IndexToLocFormat::ShortOffsets ||
                     locaFormat.value == SkOTTableHead::IndexToLocFormat::LongOffsets);
        }

        void advance(uint16_t num) {
            fCurrentGlyph += num;
            if (fLocaFormat.value == SkOTTableHead::IndexToLocFormat::ShortOffsets) {
                fCurrentGlyphOffset =
                    SkEndian_SwapBE16(fLoca.offsets.shortOffset[fCurrentGlyph]) << 1;
            } else if (fLocaFormat.value == SkOTTableHead::IndexToLocFormat::LongOffsets) {
                fCurrentGlyphOffset = SkEndian_SwapBE32(fLoca.offsets.longOffset[fCurrentGlyph]);
            }
        }
        SkOTTableGlyphData* next() {
            uint32_t previousGlyphOffset = fCurrentGlyphOffset;
            advance(1);
            if (previousGlyphOffset == fCurrentGlyphOffset) {
                return nullptr;
            } else {
                return reinterpret_cast<SkOTTableGlyphData*>(
                    reinterpret_cast<SK_OT_BYTE*>(&fGlyf) + previousGlyphOffset
                );
            }
        }
    private:
        SkOTTableGlyph& fGlyf;
        const SkOTTableIndexToLocation& fLoca;
        SkOTTableHead::IndexToLocFormat fLocaFormat;
        uint32_t fCurrentGlyph;
        uint32_t fCurrentGlyphOffset;
    };
};

struct SkOTTableGlyphData {
    SK_OT_SHORT numberOfContours; //== -1 Composite, > 0 Simple
    SK_OT_FWORD xMin;
    SK_OT_FWORD yMin;
    SK_OT_FWORD xMax;
    SK_OT_FWORD yMax;

    struct Simple {
        SK_OT_USHORT endPtsOfContours[1/*numberOfContours*/];

        struct Instructions {
            SK_OT_USHORT length;
            SK_OT_BYTE data[1/*length*/];
        };

        union Flags {
            struct Field {
                SK_OT_BYTE_BITFIELD(
                    OnCurve,
                    xShortVector,
                    yShortVector,
                    Repeat,
                    xIsSame_xShortVectorPositive,
                    yIsSame_yShortVectorPositive,
                    Reserved6,
                    Reserved7)
            } field;
            struct Raw {
                static const SK_OT_USHORT OnCurveMask = SkTEndian_SwapBE16(1 << 0);
                static const SK_OT_USHORT xShortVectorMask = SkTEndian_SwapBE16(1 << 1);
                static const SK_OT_USHORT yShortVectorMask = SkTEndian_SwapBE16(1 << 2);
                static const SK_OT_USHORT RepeatMask = SkTEndian_SwapBE16(1 << 3);
                static const SK_OT_USHORT xIsSame_xShortVectorPositiveMask = SkTEndian_SwapBE16(1 << 4);
                static const SK_OT_USHORT yIsSame_yShortVectorPositiveMask = SkTEndian_SwapBE16(1 << 5);
                SK_OT_BYTE value;
            } raw;
        };

        //xCoordinates
        //yCoordinates
    };

    struct Composite {
        struct Component {
            union Flags {
                struct Field {
                    //8-15
                    SK_OT_BYTE_BITFIELD(
                        WE_HAVE_INSTRUCTIONS,
                        USE_MY_METRICS,
                        OVERLAP_COMPOUND,
                        SCALED_COMPONENT_OFFSET,
                        UNSCALED_COMPONENT_OFFSET,
                        Reserved13,
                        Reserved14,
                        Reserved15)
                    //0-7
                    SK_OT_BYTE_BITFIELD(
                        ARG_1_AND_2_ARE_WORDS,
                        ARGS_ARE_XY_VALUES,
                        ROUND_XY_TO_GRID,
                        WE_HAVE_A_SCALE,
                        RESERVED,
                        MORE_COMPONENTS,
                        WE_HAVE_AN_X_AND_Y_SCALE,
                        WE_HAVE_A_TWO_BY_TWO)
                } field;
                struct Raw {
                    static const SK_OT_USHORT ARG_1_AND_2_ARE_WORDS_Mask = SkTEndian_SwapBE16(1 << 0);
                    static const SK_OT_USHORT ARGS_ARE_XY_VALUES_Mask = SkTEndian_SwapBE16(1 << 1);
                    static const SK_OT_USHORT ROUND_XY_TO_GRID_Mask = SkTEndian_SwapBE16(1 << 2);
                    static const SK_OT_USHORT WE_HAVE_A_SCALE_Mask = SkTEndian_SwapBE16(1 << 3);
                    static const SK_OT_USHORT RESERVED_Mask = SkTEndian_SwapBE16(1 << 4);
                    static const SK_OT_USHORT MORE_COMPONENTS_Mask = SkTEndian_SwapBE16(1 << 5);
                    static const SK_OT_USHORT WE_HAVE_AN_X_AND_Y_SCALE_Mask = SkTEndian_SwapBE16(1 << 6);
                    static const SK_OT_USHORT WE_HAVE_A_TWO_BY_TWO_Mask = SkTEndian_SwapBE16(1 << 7);

                    static const SK_OT_USHORT WE_HAVE_INSTRUCTIONS_Mask = SkTEndian_SwapBE16(1 << 8);
                    static const SK_OT_USHORT USE_MY_METRICS_Mask = SkTEndian_SwapBE16(1 << 9);
                    static const SK_OT_USHORT OVERLAP_COMPOUND_Mask = SkTEndian_SwapBE16(1 << 10);
                    static const SK_OT_USHORT SCALED_COMPONENT_OFFSET_Mask = SkTEndian_SwapBE16(1 << 11);
                    static const SK_OT_USHORT UNSCALED_COMPONENT_OFFSET_mask = SkTEndian_SwapBE16(1 << 12);
                    //Reserved
                    //Reserved
                    //Reserved
                    SK_OT_USHORT value;
                } raw;
            } flags;
            SK_OT_USHORT glyphIndex;
            union Transform {
                union Matrix {
                    /** !WE_HAVE_A_SCALE & !WE_HAVE_AN_X_AND_Y_SCALE & !WE_HAVE_A_TWO_BY_TWO */
                    struct None { } none;
                    /** WE_HAVE_A_SCALE */
                    struct Scale {
                        SK_OT_F2DOT14 a_d;
                    } scale;
                    /** WE_HAVE_AN_X_AND_Y_SCALE */
                    struct ScaleXY {
                        SK_OT_F2DOT14 a;
                        SK_OT_F2DOT14 d;
                    } scaleXY;
                    /** WE_HAVE_A_TWO_BY_TWO */
                    struct TwoByTwo {
                        SK_OT_F2DOT14 a;
                        SK_OT_F2DOT14 b;
                        SK_OT_F2DOT14 c;
                        SK_OT_F2DOT14 d;
                    } twoByTwo;
                };
                /** ARG_1_AND_2_ARE_WORDS & ARGS_ARE_XY_VALUES */
                struct WordValue {
                    SK_OT_FWORD e;
                    SK_OT_FWORD f;
                    SkOTTableGlyphData::Composite::Component::Transform::Matrix matrix;
                } wordValue;
                /** !ARG_1_AND_2_ARE_WORDS & ARGS_ARE_XY_VALUES */
                struct ByteValue {
                    SK_OT_CHAR e;
                    SK_OT_CHAR f;
                    SkOTTableGlyphData::Composite::Component::Transform::Matrix matrix;
                } byteValue;
                /** ARG_1_AND_2_ARE_WORDS & !ARGS_ARE_XY_VALUES */
                struct WordIndex {
                    SK_OT_USHORT compoundPointIndex;
                    SK_OT_USHORT componentPointIndex;
                    SkOTTableGlyphData::Composite::Component::Transform::Matrix matrix;
                } wordIndex;
                /** !ARG_1_AND_2_ARE_WORDS & !ARGS_ARE_XY_VALUES */
                struct ByteIndex {
                    SK_OT_BYTE compoundPointIndex;
                    SK_OT_BYTE componentPointIndex;
                    SkOTTableGlyphData::Composite::Component::Transform::Matrix matrix;
                } byteIndex;
            } transform;
        } component;//[] last element does not set MORE_COMPONENTS

        /** Comes after the last Component if the last component has WE_HAVE_INSTR. */
        struct Instructions {
            SK_OT_USHORT length;
            SK_OT_BYTE data[1/*length*/];
        };
    };
};

#pragma pack(pop)

#endif
