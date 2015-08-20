/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkIBMFamilyClass_DEFINED
#define SkIBMFamilyClass_DEFINED

#include "SkOTTableTypes.h"
#include "SkTypedEnum.h"

#pragma pack(push, 1)

struct SkIBMFamilyClass {
    SK_TYPED_ENUM(Class, SK_OT_BYTE,
        ((NoClassification, 0))
        ((OldstyleSerifs, 1))
        ((TransitionalSerifs, 2))
        ((ModernSerifs, 3))
        ((ClarendonSerifs, 4))
        ((SlabSerifs, 5))
        //6 reserved for future use
        ((FreeformSerifs, 7))
        ((SansSerif, 8))
        ((Ornamentals, 9))
        ((Scripts, 10))
        //11 reserved for future use
        ((Symbolic, 12))
        //13-15 reserved for future use
        SK_SEQ_END,
    (familyClass)SK_SEQ_END)
    union SubClass {
        struct OldstyleSerifs {
            SK_TYPED_ENUM(Value, SK_OT_BYTE,
                ((NoClassification, 0))
                ((IBMRoundedLegibility, 1))
                ((Garalde, 2))
                ((Venetian, 3))
                ((ModifiedVenetian, 4))
                ((DutchModern, 5))
                ((DutchTraditional, 6))
                ((Contemporary, 7))
                ((Calligraphic, 8))
                //9-14 reserved for future use
                ((Miscellaneous, 15))
                SK_SEQ_END,
            (value)SK_SEQ_END)
        } oldstyleSerifs;
        struct TransitionalSerifs {
            SK_TYPED_ENUM(Value, SK_OT_BYTE,
                ((NoClassification, 0))
                ((DirectLine, 1))
                ((Script, 2))
                //3-14 reserved for future use
                ((Miscellaneous, 15))
                SK_SEQ_END,
            (value)SK_SEQ_END)
        } transitionalSerifs;
        struct ModernSerifs {
            SK_TYPED_ENUM(Value, SK_OT_BYTE,
                ((NoClassification, 0))
                ((Italian, 1))
                ((Script, 2))
                //3-14 reserved for future use
                ((Miscellaneous, 15))
                SK_SEQ_END,
            (value)SK_SEQ_END)
        } modernSerifs;
        struct ClarendonSerifs {
            SK_TYPED_ENUM(Value, SK_OT_BYTE,
                ((NoClassification, 0))
                ((Clarendon, 1))
                ((Modern, 2))
                ((Traditional, 3))
                ((Newspaper, 4))
                ((StubSerif, 5))
                ((Monotone, 6))
                ((Typewriter, 7))
                //8-14 reserved for future use
                ((Miscellaneous, 15))
                SK_SEQ_END,
            (value)SK_SEQ_END)
        } clarendonSerifs;
        struct SlabSerifs {
            SK_TYPED_ENUM(Value, SK_OT_BYTE,
                ((NoClassification, 0))
                ((Monotone, 1))
                ((Humanist, 2))
                ((Geometric, 3))
                ((Swiss, 4))
                ((Typewriter, 5))
                //6-14 reserved for future use
                ((Miscellaneous, 15))
                SK_SEQ_END,
            (value)SK_SEQ_END)
        } slabSerifs;
        struct FreeformSerifs {
            SK_TYPED_ENUM(Value, SK_OT_BYTE,
                ((NoClassification, 0))
                ((Modern, 1))
                //2-14 reserved for future use
                ((Miscellaneous, 15))
                SK_SEQ_END,
            (value)SK_SEQ_END)
        } freeformSerifs;
        struct SansSerif {
            SK_TYPED_ENUM(Value, SK_OT_BYTE,
                ((NoClassification, 0))
                ((IBMNeoGrotesqueGothic, 1))
                ((Humanist, 2))
                ((LowXRoundGeometric, 3))
                ((HighXRoundGeometric, 4))
                ((NeoGrotesqueGothic, 5))
                ((ModifiedNeoGrotesqueGothic, 6))
                //7-8 reserved for future use
                ((TypewriterGothic, 9))
                ((Matrix, 10))
                //11-14 reserved for future use
                ((Miscellaneous, 15))
                SK_SEQ_END,
            (value)SK_SEQ_END)
        } sansSerif;
        struct Ornamentals {
            SK_TYPED_ENUM(Value, SK_OT_BYTE,
                ((NoClassification, 0))
                ((Engraver, 1))
                ((BlackLetter, 2))
                ((Decorative, 3))
                ((ThreeDimensional, 4))
                //5-14 reserved for future use
                ((Miscellaneous, 15))
                SK_SEQ_END,
            (value)SK_SEQ_END)
        } ornamentals;
        struct Scripts {
            SK_TYPED_ENUM(Value, SK_OT_BYTE,
                ((NoClassification, 0))
                ((Uncial, 1))
                ((Brush_Joined, 2))
                ((Formal_Joined, 3))
                ((Monotone_Joined, 4))
                ((Calligraphic, 5))
                ((Brush_Unjoined, 6))
                ((Formal_Unjoined, 7))
                ((Monotone_Unjoined, 8))
                //9-14 reserved for future use
                ((Miscellaneous, 15))
                SK_SEQ_END,
            (value)SK_SEQ_END)
        } scripts;
        struct Symbolic {
            SK_TYPED_ENUM(Value, SK_OT_BYTE,
                ((NoClassification, 0))
                //1-2 reserved for future use
                ((MixedSerif, 3))
                //4-5 reserved for future use
                ((OldstyleSerif, 6))
                ((NeoGrotesqueSansSerif, 7))
                //8-14 reserved for future use
                ((Miscellaneous, 15))
                SK_SEQ_END,
            (value)SK_SEQ_END)
        } symbolic;
    } familySubClass;
};

#pragma pack(pop)


static_assert(sizeof(SkIBMFamilyClass) == 2, "sizeof_SkIBMFamilyClass_not_2");

#endif
