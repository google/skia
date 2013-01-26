
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkIntArray_DEFINED
#define SkIntArray_DEFINED

#include "SkColor.h"
#include "SkDisplayType.h"
#include "SkMath.h"
#include "SkTDArray_Experimental.h"

class SkActive;
class SkAnimateBase;
class SkDataInput;
class SkDisplayable;
class SkDisplayEvent;
class SkDrawable;
class SkDrawColor;
class SkMatrixPart;
struct SkMemberInfo;
class SkPathPart;
class SkPaintPart;
class SkTypedArray;
class SkString;
union SkOperand;

typedef SkIntArray(int) SkTDIntArray;
typedef SkIntArray(SkColor) SkTDColorArray;
typedef SkIntArray(SkDisplayTypes) SkTDDisplayTypesArray;
typedef SkIntArray(SkMSec) SkTDMSecArray;
typedef SkIntArray(SkScalar) SkTDScalarArray;

typedef SkLongArray(SkActive*) SkTDActiveArray;
typedef SkLongArray(SkAnimateBase*) SkTDAnimateArray;
typedef SkLongArray(SkDataInput*) SkTDDataArray;
typedef SkLongArray(SkDisplayable*) SkTDDisplayableArray;
typedef SkLongArray(SkDisplayEvent*) SkTDDisplayEventArray;
typedef SkLongArray(SkDrawable*) SkTDDrawableArray;
typedef SkLongArray(SkDrawColor*) SkTDDrawColorArray;
typedef SkLongArray(SkMatrixPart*) SkTDMatrixPartArray;
typedef SkLongArray(const SkMemberInfo*) SkTDMemberInfoArray;
typedef SkLongArray(SkPaintPart*) SkTDPaintPartArray;
typedef SkLongArray(SkPathPart*) SkTDPathPartArray;
typedef SkLongArray(SkTypedArray*) SkTDTypedArrayArray;
typedef SkLongArray(SkString*) SkTDStringArray;
typedef SkLongArray(SkOperand) SkTDOperandArray;
typedef SkLongArray(SkOperand*) SkTDOperandPtrArray;

#endif // SkIntArray_DEFINED
