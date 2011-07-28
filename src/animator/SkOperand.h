
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkOperand_DEFINED
#define SkOperand_DEFINED

#include "SkDisplayType.h"

class SkTypedArray;
class SkDisplayable;
class SkDrawable;
class SkString;

union SkOperand {
//  SkOperand() {}
//  SkOperand(SkScalar scalar) : fScalar(scalar) {}
    SkTypedArray* fArray;
    SkDisplayable* fDisplayable;
    SkDrawable* fDrawable;
    void* fObject;
    int32_t fS32;
    SkMSec fMSec;
    SkScalar fScalar;
    SkString* fString;
};

struct SkScriptValue {
    SkOperand fOperand;
    SkDisplayTypes fType;
    SkTypedArray* getArray() { SkASSERT(fType == SkType_Array); return fOperand.fArray; }
    SkDisplayable* getDisplayable() { SkASSERT(fType == SkType_Displayable); return fOperand.fDisplayable; }
    SkDrawable* getDrawable() { SkASSERT(fType == SkType_Drawable); return fOperand.fDrawable; }
    int32_t getS32(SkAnimateMaker* maker) { SkASSERT(fType == SkType_Int || fType == SkType_Boolean ||
        SkDisplayType::IsEnum(maker, fType)); return fOperand.fS32; }
    SkMSec getMSec() { SkASSERT(fType == SkType_MSec); return fOperand.fMSec; }
    SkScalar getScalar() { SkASSERT(fType == SkType_Float); return fOperand.fScalar; }
    SkString* getString() { SkASSERT(fType == SkType_String); return fOperand.fString; }
};

#endif // SkOperand_DEFINED
